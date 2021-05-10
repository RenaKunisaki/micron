#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static const uint8_t i2c_scl_pin[] = {
    I2C0_SCL_PIN,
    #if NUM_I2C > 1
        I2C1_SCL_PIN,
    #endif
};
static const uint8_t i2c_sda_pin[] = {
    I2C0_SDA_PIN,
    #if NUM_I2C > 1
        I2C1_SDA_PIN,
    #endif
};
volatile i2cRegs_t *i2cRegs[NUM_I2C] = {
	(volatile i2cRegs_t*)I2C_REG_BASE(0),
    #if NUM_I2C > 1
    	(volatile i2cRegs_t*)I2C_REG_BASE(1)
    #endif
};
static const uint32_t i2c_pin_mode =
	PCR_OPEN_DRAIN | PCR_SLEW_SLOW | PCR_DRIVE_STRENGTH_HI | PCR_MUX(2);
WEAK uint8_t i2cInterruptPriority = 48; //0 = highest priority, 255 = lowest

//XXX this is still a huge mess that needs cleanup and documentation

int kinetis_i2cWaitForBus(uint8_t port, uint32_t timeout) {
    micronI2cState *state = i2cState[port];
    i2cRegs_t *reg = i2cRegs[port];

	do {
		if(millis() >= timeout) return -EBUSY;
		reg->S.byte = I2C_S_IICIF | I2C_S_ARBL; //clear IRQ and ARBL flags (write 1)
		if(reg->C1.MST) {
			//we are already master, so send repeated START
			//enable I2C, use master mode, repeat start, use transmit mode
			reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST |
				I2C_C1_RSTA | I2C_C1_TX;
		}
		else {
			while(reg->S.BUSY) {
				if(millis() >= timeout) return -EBUSY;
				idle(); //wait for bus to be ready
			}
			//C1.MST isn't set, so setting it will generate a START
			reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX;
			state->slaveMode = 0;
		}
	} while(!reg->C1.MST);
	return 0;
}

int kinetis_i2cInit(uint8_t port, int address) {
    /** Initialize specified I2C port with specified address.
     *  @param port Which I2C port to use.
     *  @param address The slave address to respond to, or -1 for none.
     *  @return 0 on success, or a negative error code on failure.
     */
	#ifndef I2C_CLOCK
		//we don't know what to set the clocks to for this CPU speed.
		return -ENOSYS;
	#endif

    micronI2cState *state = i2cState[port];
	i2cRegs_t *reg = i2cRegs[port];
	int_fast8_t scl = i2c_scl_pin[port];
	int_fast8_t sda = i2c_sda_pin[port];
	state->scl_pin  = scl;
	state->sda_pin  = sda;

	//enable clock for I2C module. XXX use bitband register here
	//XXX make a function to set/clear/check the SIM_SCGC* bits.
	SIM_SCGC4 |= (SIM_SCGC4_I2C0 << port);
	reg->C1 .byte = 0;
	reg->C2 .byte = 0;
	reg->SMB.byte = 0;

	//set up pins
    kinetis_internalSetPinMode(scl, i2c_pin_mode, PIN_DIR_OUTPUT);
    kinetis_internalSetPinMode(sda, i2c_pin_mode, PIN_DIR_OUTPUT);

	//set up module
    reg->F  .byte = I2C_CLOCK;   //set clock speed
	reg->FLT.byte = I2C_FILTER;  //set noise filter
	reg->C2 .byte = I2C_C2_HDRS; //set high drive mode
	if(address >= 0) reg->A1.AD = address;

	//set up interrupts
	reg->C1.byte = I2C_C1_IICIE | I2C_C1_IICEN;
	irqSetPriority(IRQ_I2C0 + port, i2cInterruptPriority);
	irqEnableInterrupt(IRQ_I2C0 + port);
	return 0;
}

int kinetis_i2cShutdown(uint8_t port) {
    /** Shut down specified I2C port, if it's enabled.
     *  @param port Which I2C port to use.
     *  @return 0 on success, or a negative error code on failure.
     */
	irqDisableInterrupt(IRQ_I2C0 + port);

	//disable clock for I2C module. XXX use bitband register here
	SIM_SCGC4 &= ~(SIM_SCGC4_I2C0 << port);

    return 0;
}

int kinetis_i2cGetStatus(uint8_t port) {
    /** Get I2C bus status.
     *  @param port Which I2C port to use.
     *  @return 0 if no problems, or a negative error code.
     */
    i2cRegs_t *reg = i2cRegs[port];
	uint8_t status = reg->S.byte;
	//if(status & I2C_S_RXAK) return -EBUSY; //device didn't respond
	if(status & I2C_S_ARBL) return -ECONNRESET; //we lost bus arbitration
	return 0;
}

int kinetis_i2cSendByte(uint8_t port, uint8_t data) {
    /** Send a byte on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param data Byte to send.
     *  @return 0 if no problems, or a negative error code.
     *  @note This would normally be called from an ISR or i2cEndTx().
     */
    i2cRegs_t *reg = i2cRegs[port];
    reg->DATA = data;
    return 0;
}

int kinetis_i2cRead(micronI2cState *state, uint8_t *buffer, uint32_t length,
uint32_t timeout) {
	i2cRegs_t *reg = i2cRegs[state->port];

	//int err = _i2cWaitForIdle(state, reg, millis() + 100);
	//if(err) return err;

	//wait for the bus, then enter master transmit mode
	//int err = _i2cWaitForBus(state, reg, timeout);
	//if(err) return err;
	//printf("C1 = %02X\r\n", reg->C1.byte);

	reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
	reg->S.byte  = I2C_S_IICIF | I2C_S_ARBL; //clear IRQ and ARBL flags (write 1)
    if(!state->slaveMode) {
    	UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
    }

	//wait for rx to complete
	while(state->state != MICRON_I2C_STATE_IDLE) {
		uint32_t now = millis();
		if(now >= timeout) {
			reg->S.byte = I2C_S_ARBL; //acknowledge
			state->state = MICRON_I2C_STATE_IDLE;
			return -EBUSY; //XXX indicate how many bytes were received
		}
		irqWait(); //XXX deadlock

		uint8_t status = reg->S.byte;
		if(status & I2C_S_ARBL) {
			reg->S.byte = I2C_S_ARBL; //acknowledge
			state->state = MICRON_I2C_STATE_IDLE;
			return -ECONNRESET; //we lost bus arbitration
			//this can also happen if the device doesn't respond.
			//XXX clear rxbuf.data, indicate somehow how many bytes were received
		}
	}
    return 0;
}


/** Send a request to a device on the I2C bus.
 *  port: Which I2C port to use.
 *  address: Destination device address.
 *  length: Number of bytes to read.
 *  stop: Whether to send a STOP condition after completing the read.
 *  buffer: Destination buffer to read into.
 *  timeout: Maximum number of milliseconds to wait for response.
 *  Returns 0 on success or a negative error code on failure.
 *
 * XXX make the parameters/order less silly; support slave mode; don't deadlock
 * if receiving less than expected; return # bytes received
 */
int kinetis_i2cRequest(micronI2cState *state, uint8_t address, uint32_t length,
uint8_t stop, uint8_t *buffer, uint32_t timeout) {
	i2cRegs_t *reg = i2cRegs[state->port];

	//send address and read-flag
	reg->DATA = (address << 1) | 1;
	reg->S.byte = I2C_S_IICIF | I2C_S_ARBL; //clear IRQ and ARBL flags (write 1)

	//wait for rx to complete
	//while(state->rxbuf.idx != state->rxbuf.len) {
	while(state->state != MICRON_I2C_STATE_IDLE) {
		uint32_t now = millis();
		if(now >= timeout) {
			//reg->S.byte = I2C_S_ARBL; //acknowledge
			state->state = MICRON_I2C_STATE_IDLE;
			return -ETIMEDOUT; //XXX indicate how many bytes were received
		}
		//irqWait(); //XXX deadlock
        idle();

		uint8_t status = reg->S.byte;
		if(status & I2C_S_ARBL) {
			reg->S.byte = I2C_S_ARBL; //acknowledge
			state->state = MICRON_I2C_STATE_IDLE;
			return -ECONNRESET; //we lost bus arbitration
			//this can also happen if the device doesn't respond.
			//XXX clear rxbuf.data, indicate somehow how many bytes were received
		}
	}
	//state->receiving = 0;
	state->rxbuf.data = NULL;
	//if(stop) reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
	if(stop) {
		//printf("wait for stop...\r\n");
		state->state = MICRON_I2C_STATE_STOP;
		reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
		//while(state->state != MICRON_I2C_STATE_IDLE) irqWait();
		state->state = MICRON_I2C_STATE_IDLE;
	}
	//printf("done\r\n");
	return 0;
}


static void isrI2CStateTx(micronI2cState *state, i2cRegs_t *reg) {
	if(state->txbuf.tail != state->txbuf.head) {
		//printf("tx: %d / %d  -  rx: %d / %d: %p\r\n", state->txbuf.tail,
		//	state->txbuf.head, state->rxbuf.idx, state->rxbuf.len,
		//	state->rxbuf.data);
		//transmit next byte
		state->txbuf.tail = (state->txbuf.tail + 1) % I2C_TX_BUFSIZE;
		reg->DATA = state->txbuf.data[state->txbuf.tail];
	}
	if(state->txbuf.tail == state->txbuf.head) {
		//state->transmitting = 0;
		//XXX is it possible that another IRQ won't trigger, so we'll never
		//process the STOP state?
		if(state->sendStop) state->state = MICRON_I2C_STATE_STOP;
		else if(state->doRecv) {
			state->state = MICRON_I2C_STATE_RX;
			reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST;
			UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
		}
		else state->state = MICRON_I2C_STATE_IDLE;
		//state->state = MICRON_I2C_STATE_WAIT;
		//printf("tx finished  ");
	}
}


static void isrI2CStateRx(micronI2cState *state, i2cRegs_t *reg) {
	uint8_t C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST;
	if(reg->C1.TX) {
		//we're still in transmit mode, so this interrupt tells us that
		//the transmission (of address and read flag) completed.
		//XXX is there no better way to do this? I'd use the TCF or
		//RXAK flags, but TCF seems to be always set and RXAK only
		//when we finish receiving.
		//Teensy code just checks for IICIF flag and doesn't clear it
		//in the ISR?
		uint32_t left = state->rxbuf.len - state->rxbuf.idx;
		if(left == 1) C1 |= I2C_C1_TXAK;
		if(left == 0) C1 |= I2C_C1_TX;
		//XXX are 0-length reads ever used? if so, does this work?

		reg->C1.byte = C1;
		UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
		if(left == 0) state->state = MICRON_I2C_STATE_IDLE;
	}

	else {
		//receive next byte
		//this code is a bit repetitive because it's important that
		//we set C1 before reading DATA.
		uint32_t left = state->rxbuf.len - (state->rxbuf.idx + 1);
		if(left == 1) C1 |= I2C_C1_TXAK;
		if(left == 0) C1 |= I2C_C1_TX;
		reg->C1.byte = C1;

		uint8_t d = reg->DATA;
		state->rxbuf.data[state->rxbuf.idx++] = d;
		if(left == 0) state->state = MICRON_I2C_STATE_IDLE;
	}
}


static void isrI2CStateStop(micronI2cState *state, i2cRegs_t *reg) {
	reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
	if(state->doRecv) {
		state->state = MICRON_I2C_STATE_RX;
		UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
	}
	else state->state = MICRON_I2C_STATE_IDLE;
}


static void isrI2CStateWait(micronI2cState *state, i2cRegs_t *reg) {
	//waiting for another interrupt after doing something else.
	state->state = MICRON_I2C_STATE_IDLE;
}

void i2cDefaultSlaveRx(uint8_t data) {
    //do nothing
}
void i2cSlaveRx(uint8_t data) WEAK ALIAS("i2cDefaultSlaveRx");
void i2cDefaultSlaveTx(uint8_t data) {
    //do nothing
}
void i2cSlaveTx(uint8_t data) WEAK ALIAS("i2cDefaultSlaveTx");

//called from default I2C ISRs
void isrI2C(int port) {
    //digitalWrite(13, 1);
	micronI2cState *state = i2cState[port];
	i2cRegs_t *reg = i2cRegs[port];

    if(reg->S.IAAS) {
        if(reg->S.SRW) { //master read request
            i2cSlaveTx(0);
        }
        else { //master sent us a byte
            //just reading DATA acknowledges.
            i2cSlaveRx(reg->DATA);
        }
    }
	else if(state) {
        switch(state->state) {
			case MICRON_I2C_STATE_TX:   isrI2CStateTx  (state, reg); break;
			case MICRON_I2C_STATE_RX:   isrI2CStateRx  (state, reg); break;
			case MICRON_I2C_STATE_STOP: isrI2CStateStop(state, reg); break;
			case MICRON_I2C_STATE_WAIT: isrI2CStateWait(state, reg); break;
			case MICRON_I2C_STATE_IDLE: break;
		}
	}
	else {
		//printf("i2c not init!");
		reg->S.byte = 0xFF;
        return;
	}
    reg->S.byte = I2C_S_IICIF | I2C_S_ARBL; //acknowledge interrupt.
}


#ifdef __cplusplus
	} //extern "C"
#endif
