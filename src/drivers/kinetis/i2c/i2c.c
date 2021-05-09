#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//WARNING: debug print can be so slow that communications become unreliable!
#define I2C_DEBUG_PRINT 0

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
volatile i2c_regs_t *i2c_regs[NUM_I2C] = {
	(volatile i2c_regs_t*)I2C_REG_BASE(0),
    #if NUM_I2C > 1
    	(volatile i2c_regs_t*)I2C_REG_BASE(1)
    #endif
};
static const uint32_t i2c_pin_mode =
	PCR_OPEN_DRAIN | PCR_SLEW_SLOW | PCR_DRIVE_STRENGTH_HI | PCR_MUX(2);

SECTION(".bss") micron_i2c_state *i2c_state[NUM_I2C];
WEAK uint8_t i2c_interrupt_priority = 48; //0 = highest priority, 255 = lowest


#if I2C_DEBUG_PRINT
	static const char *names_S[] = {
		"RXAK", "IICIF", "SRW", "RAM", "ARBL", "BUSY", "IAAS", "TCF"};
	static const char *names_C1[] = {
		"DMAEN", "WUEN", "RSTA", "TXAK", "TX", "MST", "IICIE", "IICEN"};
	static const char *names_State[] = {"IDLE", "TX", "RX", "STOP", "WAIT"};

	static void print_flag(const char *name, uint8_t flag) {
		printf("\e[38;5;%dm%s\e[38;5;15m ", flag ? 10 : 8, name);
	}

	static void print_flags(const char **names, uint8_t data) {
		printf("%02X ", data);
		for(int i=7; i>=0; i--)
			print_flag(names[i], data & BIT(i));
	}
#endif


int _i2cWaitForBus(micron_i2c_state *state, i2c_regs_t *reg, uint32_t timeout) {
	#if I2C_DEBUG_PRINT
		printf("wait for bus, C1=%02X\r\n", reg->C1.byte);
	#endif

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
	#if I2C_DEBUG_PRINT
		printf("bus ready, C1=%02X\r\n", reg->C1.byte);
	#endif
	return 0;
}


/** Initialize specified I2C port with specified address.
 *  address: the slave address to respond to, or -1 for none.
 *  Returns 0 on success, or a negative error code on failure.
 */
int i2cInit(uint8_t port, int address) {
	if(port >= NUM_I2C) return -ENODEV; //No such device
	if(address > 127) return -EADDRNOTAVAIL;
	#ifndef I2C_CLOCK
		//we don't know what to set the clocks to for this CPU speed.
		return -ENOSYS;
	#endif

	//set up state variables
	i2c_state[port] = (micron_i2c_state*)malloc(sizeof(micron_i2c_state));
	if(i2c_state[port] == NULL) return -ENOMEM;
	micron_i2c_state *state = i2c_state[port];
	memset(state, 0, sizeof(micron_i2c_state));

	i2c_regs_t *reg = i2c_regs[port];
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
    //HACK: change clock and filter settings for long wires
	reg->F  .byte = I2C_CLOCK;   //set clock speed
	reg->FLT.byte = I2C_FILTER;  //set noise filter
	reg->C2 .byte = I2C_C2_HDRS; //set high drive mode
	if(address >= 0) {
		reg->A1.AD = address;
		state->slaveMode = 1;
	}

	//set up interrupts
	reg->C1.byte = I2C_C1_IICIE | I2C_C1_IICEN;
	irqSetPriority(IRQ_I2C0 + port, i2c_interrupt_priority);
	irqEnableInterrupt(IRQ_I2C0 + port);
	return 0;
}


/** Shut down specified I2C port, if it's enabled.
 */
void i2cShutdown(uint8_t port) {
	if(port >= NUM_I2C) return;
	if(i2c_state[port] == NULL) return; //already shutdown
	irqDisableInterrupt(IRQ_I2C0 + port);

	//disable clock for I2C module. XXX use bitband register here
	SIM_SCGC4 &= ~(SIM_SCGC4_I2C0 << port);
	free(i2c_state[port]);
	i2c_state[port] = NULL;
}


/** Internal function to add some data to an I2C transmit buffer.
 */
int _i2cBufferTxP(micron_i2c_state *state, i2c_regs_t *reg,
const void *data, uint32_t len) {
	uint8_t *dat = (uint8_t*)data;
	uint32_t i = 0;
	while(i < len) { //while more bytes to send
		uint32_t head = state->txbuf.head;
		uint32_t tail = state->txbuf.tail;
		uint32_t next = (head + 1) % I2C_TX_BUFSIZE;
		if(next == tail) { //buffer is full
			break;
		}
		state->txbuf.data[next] = dat[i++];
		state->txbuf.head = next;
	}
	state->state = MICRON_I2C_STATE_TX;
	return i;
}


/** Wrapper for _i2cBufferTxP() which takes an I2C port.
 */
int _i2cBufferTx(uint8_t port, const void *data, uint32_t len) {
	if(port >= NUM_I2C) return -ENODEV;

	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;

	if(len == 0) return 0;
	return _i2cBufferTxP(state, i2c_regs[port], data, len);
}


int _i2cWaitForIdle(micron_i2c_state *state, i2c_regs_t *reg, uint32_t timeout) {
	while(state->state != MICRON_I2C_STATE_IDLE) {
		if(millis() >= timeout) return -EBUSY;
		uint8_t status = reg->S.byte;
		//if(status & I2C_S_RXAK) return -EBUSY; //device didn't respond
		if(status & I2C_S_ARBL) return -ECONNRESET; //we lost bus arbitration
		//XXX are there more appropriate error codes?
		//should we do something else here?
	}
	return 0;
}


/** Initiate a transmission on the I2C bus.
 *  port: Which I2C port to use.
 *  address: Destination device address.
 *  Returns 1 on success, 0 if transmit buffer is full, or a negative error code
 *  on failure. (XXX improve this)
 */
int i2cBeginTx(uint8_t port, uint8_t address) {
	if(port >= NUM_I2C) return -ENODEV;
	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;
	//state->transmitting = 1;
	state->doRecv = 0;
	state->state  = MICRON_I2C_STATE_TX;

	//Queue one byte: the destination address and R/W flag.
	uint8_t data = address << 1;
	return _i2cBufferTxP(state, i2c_regs[port], &data, 1);
}


/** Queue some data to transmit on the I2C bus.
 *  port: Which I2C port to use.
 *  data: Data to send.
 *  len:  Number of bytes to send.
 *  On success, returns number of bytes queued, which may be as few as zero if
 *  the transmit buffer is full. On failure, returns a negative error code.
 */
int i2cSend(uint8_t port, const void *data, uint32_t len) {
	if(port >= NUM_I2C) return -ENODEV;
	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;

	//queue data to send.
	if(state->state == MICRON_I2C_STATE_TX || state->slaveMode) {
		return _i2cBufferTxP(state, i2c_regs[port], data, len);
	}
	//we need to call i2cBeginTx() first.
	return -EDESTADDRREQ;
}


/** Transmit all queued data on the I2C bus.
 *  port: Which I2C port to use.
 *  stop: Whether to send a STOP condition after completing the transmission.
 *  Returns 0 on success or a negative error code on failure.
 *
 *  This function actually transmits the data queued by `i2cBeginTx` and
 *  `i2cSend`. It waits for the bus to be ready, makes itself the bus master,
 *  and waits for the transmission to complete (or for an error).
 *
 *  XXX support slave mode
 */
int i2cEndTx(uint8_t port, uint8_t stop) {
	//do the actual transmission from the buffer.
	if(port >= NUM_I2C) return -ENODEV;
	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;
	i2c_regs_t *reg = i2c_regs[port];

	int err = _i2cWaitForBus(state, reg, millis() + 100);
	if(err) {
        //reg->C1.byte = 0; //don't keep SCL low
        return err;
    }

	state->sendStop = stop;
	state->txbuf.tail = (state->txbuf.tail + 1) % I2C_TX_BUFSIZE;
	uint8_t byte = state->txbuf.data[state->txbuf.tail];
#if I2C_DEBUG_PRINT
	printf("TX [%3d/%3d] %02X\r\n", state->txbuf.tail, state->txbuf.head, byte);
#endif
	reg->DATA = byte;

	//wait for device to respond, which will trigger IRQ to send next byte.
	err = _i2cWaitForIdle(state, reg, millis() + 100);
	if(err) return err;

	/* if(stop) {
		//we can't do STOP until the transmission is finished.
		//using 'sendStop' flag it's possible to miss sending it entirely.
		state->state = MICRON_I2C_STATE_STOP;
		err = _i2cWaitForIdle(state, reg); //XXX deadlock
		if(err) return err;
	} */

	return 0;
}


int i2cSendRecv(uint8_t port, uint8_t address, uint32_t txLen, const void *src,
uint32_t rxLen, uint8_t *dest, uint32_t timeout) {
    int err = i2cBeginTx(port, address);
    if(err >  0) err = i2cSend(port, src, rxLen);
    if(err >= 0) err = i2cEndTx(port, 1);
    if(err >= 0) err = i2cRequest(port, address, rxLen, 1, dest, timeout);
    return err;
}


int i2cGetNumRecv(uint8_t port) {
	if(port >= NUM_I2C) return -ENODEV;
	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;
	return state->rxbuf.idx;
}


int i2cRead(uint8_t port, uint32_t length, uint8_t *buffer, uint32_t timeout) {
	if(port >= NUM_I2C) return -ENODEV;

	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;

	i2c_regs_t *reg = i2c_regs[port];
	state->rxbuf.data = NULL;
	//state->receiving  = 0;
	timeout += millis();

	//int err = _i2cWaitForIdle(state, reg, millis() + 100);
	//if(err) return err;

	//wait for the bus, then enter master transmit mode
	//int err = _i2cWaitForBus(state, reg, timeout);
	//if(err) return err;
	//printf("C1 = %02X\r\n", reg->C1.byte);

 	//set rx buffer
	state->rxbuf.data = buffer;
	state->rxbuf.len  = length;
	state->rxbuf.idx  = 0;
	//state->receiving  = 1;

	printf("set RX\r\n");
	state->state = MICRON_I2C_STATE_RX;
	reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
	reg->S.byte  = I2C_S_IICIF | I2C_S_ARBL; //clear IRQ and ARBL flags (write 1)
    if(!state->slaveMode) {
    	UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
    }
	printf("wait RX\r\n");

	//wait for rx to complete
	//while(state->rxbuf.idx != state->rxbuf.len) {
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
	//state->receiving = 0;
	state->rxbuf.data = NULL;
	return state->rxbuf.idx;
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
int i2cRequest(uint8_t port, uint8_t address, uint32_t length, uint8_t stop,
uint8_t *buffer, uint32_t timeout) {
	if(port >= NUM_I2C) return -ENODEV;

	micron_i2c_state *state = i2c_state[port];
	if(state == NULL) return -EBADFD;

	i2c_regs_t *reg = i2c_regs[port];
	state->rxbuf.data = NULL;
	//state->receiving  = 0;
	timeout += millis();

	//wait for the bus, then enter master transmit mode
	int err = _i2cWaitForBus(state, reg, timeout);
	if(err) return err;
	//printf("C1 = %02X\r\n", reg->C1.byte);

 	//set rx buffer
	state->rxbuf.data = buffer;
	state->rxbuf.len  = length;
	state->rxbuf.idx  = 0;
	state->doRecv     = 0;
	//state->receiving  = 1;

	state->state = MICRON_I2C_STATE_RX;


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


static void isrI2CStateTx(micron_i2c_state *state, i2c_regs_t *reg) {
	if(state->txbuf.tail != state->txbuf.head) {
		//printf("tx: %d / %d  -  rx: %d / %d: %p\r\n", state->txbuf.tail,
		//	state->txbuf.head, state->rxbuf.idx, state->rxbuf.len,
		//	state->rxbuf.data);
		//transmit next byte
		state->txbuf.tail = (state->txbuf.tail + 1) % I2C_TX_BUFSIZE;
#if I2C_DEBUG_PRINT
		printf("tx [%3d/%3d] %02X",
			state->txbuf.tail, state->txbuf.head,
			state->txbuf.data[state->txbuf.tail]);
#endif
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


static void isrI2CStateRx(micron_i2c_state *state, i2c_regs_t *reg) {
	uint8_t C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST;
	if(reg->C1.TX) {
		//we're still in transmit mode, so this interrupt tells us that
		//the transmission (of address and read flag) completed.
		//XXX is there no better way to do this? I'd use the TCF or
		//RXAK flags, but TCF seems to be always set and RXAK only
		//when we finish receiving.
		//Teensy code just checks for IICIF flag and doesn't clear it
		//in the ISR?
#if I2C_DEBUG_PRINT
		printf("begin rx");
#endif

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
#if I2C_DEBUG_PRINT
		printf("rx %02X, %2ld/%2ld, %2ld left", d, state->rxbuf.idx,
			state->rxbuf.len, left);
#endif
		if(left == 0) state->state = MICRON_I2C_STATE_IDLE;
	}
}


static void isrI2CStateStop(micron_i2c_state *state, i2c_regs_t *reg) {
	reg->C1.byte = I2C_C1_IICEN | I2C_C1_IICIE;
	if(state->doRecv) {
		state->state = MICRON_I2C_STATE_RX;
		UNUSED_SYMBOL uint8_t dummy = reg->DATA; //initiate receive
	}
	else state->state = MICRON_I2C_STATE_IDLE;
}


static void isrI2CStateWait(micron_i2c_state *state, i2c_regs_t *reg) {
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
	micron_i2c_state *state = i2c_state[port];
	i2c_regs_t *reg = i2c_regs[port];

#if I2C_DEBUG_PRINT
	static uint32_t count = 0;
	uint8_t stat = reg->S. byte;
	uint8_t C1   = reg->C1.byte;

	//printf("\e[s\e[1;1H" //save cursor pos, cursor to 1,1
	//	"\e[48;5;4m" //set BG color: blue
	//	"%6d S=", count++);
	printf("[%6d] S=", count++);
	print_flags(names_S, stat);
	printf(" C1=");
	print_flags(names_C1, C1);
	printf("; state=%s; ", state ? names_State[state->state] : "N/A");
#endif

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

		#if 0 //use LEDs for debug
			int led = 0; //1=orange (onboard) 2=red 4=green 8=blue
			static int ledPins[] = {13, 2, 3, 4};
			static int OR=1, RD=2, GN=4, BL=8;
			switch(state->state) {
				case MICRON_I2C_STATE_TX:   led=RD; break;
				case MICRON_I2C_STATE_RX:   led=GN; break;
				case MICRON_I2C_STATE_STOP: led=RD|GN; break;
				case MICRON_I2C_STATE_WAIT: led=BL; break;
				case MICRON_I2C_STATE_IDLE: led=OR; break;
				default: led=RD+OR;
			}
			for(int i=0; i<4; i++) {
				digitalWrite(ledPins[i], led & BIT(i));
			}
		#endif
	}
	else {
		//printf("i2c not init!");
		reg->S.byte = 0xFF;
        return;
	}
    reg->S.byte = I2C_S_IICIF | I2C_S_ARBL; //acknowledge interrupt.

#if I2C_DEBUG_PRINT
	printf(" -> %s", state ? names_State[state->state] : "N/A");
	//printf("  \e[0m\e[u"); //clear attrs, restore cursor pos
	printf("\e[0m\r\n"); //clear attrs
#endif
}


#ifdef __cplusplus
	} //extern "C"
#endif
