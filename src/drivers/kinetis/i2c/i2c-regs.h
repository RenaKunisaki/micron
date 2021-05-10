#ifndef _MICRON_I2C_REGS_H_
#define _MICRON_I2C_REGS_H_

typedef struct {
	//These unhelpful names courtesy of the Cortex-M4 manual.
	union { //Address Register 1
		uint8_t byte;
		struct {
			//note, when using bit fields, LSB is first
			uint8_t    : 1; //0:  reserved
			uint8_t AD : 7; //rw: slave address
		};
	} A1;
	union { //Frequency Divider register
		uint8_t byte;
		struct {
			uint8_t ICR  : 6; //rw: clock rate
			uint8_t MULT : 2; //rw: freq multiplier: [1, 2, 4, reserved]
		};
	} F;
	union { //Control Register 1
		uint8_t byte;
		struct {
			uint8_t DMAEN : 1; //rw: enable DMA
			uint8_t WUEN  : 1; //rw: allow I2C to wake up CPU
			uint8_t RSTA  : 1; //0w: set to generate a repeated START
			uint8_t TXAK  : 1; //rw: transmit ACK disable
			uint8_t TX    : 1; //rw: transmit enable
			uint8_t MST   : 1; //rw: master mode enable
			                   //    changing this generates START/STOP
			uint8_t IICIE : 1; //rw: interrupt request enable
			uint8_t IICEN : 1; //rw: I2C module enable
		};
	} C1;
	union { //Status register
		uint8_t byte;
		struct {
			uint8_t RXAK  : 1; //r:   receive acknowledge detected
			uint8_t IICIF : 1; //w1c: interrupt pending flag
			uint8_t SRW   : 1; //r:   slave transmit flag
			uint8_t RAM   : 1; //rw:  Range Address Match
			uint8_t ARBL  : 1; //w1c: arbitration lost flag
			uint8_t BUSY  : 1; //r:   bus busy flag
			uint8_t IAAS  : 1; //rw:  Addressed As Slave
			uint8_t TCF   : 1; //r:   transfer complete flag
		};
	} S;
	union { //Data I/O register
		struct {
			uint8_t byte; //rw: data
		} D;
		uint8_t DATA;
	};
	union { //Control Register 2
		uint8_t byte;
		struct {
			uint8_t AD    : 3; //rw: high 3 bits of slave address
			uint8_t RMEN  : 1; //rw: Enable Range Address Matching
			uint8_t SRBC  : 1; //rw: slave baud rate indep. of master
			uint8_t HDRS  : 1; //rw: Enable High Drive Mode
			uint8_t ADEXT : 1; //rw: Enable 10-bit addressing
			uint8_t GCAEN : 1; //rw: General Call Address Enable
		};
	} C2;
	union { //Programmable Input Glitch Filter register
		uint8_t byte;
		struct {
			uint8_t FLT : 5; //rw: filter glitches up to (31*n) bus clocks
			uint8_t     : 2; //0:  reserved
			uint8_t     : 1; //rw: reserved
		};
	} FLT;
	union { //Range Address register
		uint8_t byte;
		struct {
			uint8_t     : 1; //0:  reserved
			uint8_t RAD : 7; //rw: Range Slave Address
		};
	} RA;
	union { //SMBus Control and Status register
		uint8_t byte;
		struct {
			uint8_t SHTF2IE : 1; //rw:  SCL/SDA timeout interrupt enable
			uint8_t SHTF2   : 1; //w1c: SCL high + SDA low  timeout
			uint8_t SHTF1   : 1; //r:   SCL high + SDA high timeout
			uint8_t SLTF    : 1; //w1c: SCL low timeout flag (w1c)
			uint8_t TCKSEL  : 1; //rw:  0=timeout counter at 1/64 bus speed
								 //     1=timeout counter at 1x   bus speed
			uint8_t SIICAEN : 1; //rw:  enable second address register
			uint8_t ALERTEN : 1; //rw:  alert response address matching enable
			uint8_t FACK    : 1; //rw:  fast ACK enable
		};
	} SMB;
	union { //Address Register 2
		uint8_t byte;
		struct {
			uint8_t     : 1; //0:  reserved
			uint8_t SAD : 7; //rw: secondary SMBus address
		};
	} A2;
	union {
		struct {
			struct {
				uint8_t byte; //SCL Low Timeout Register Low
			} SLTL;
			struct {
				uint8_t byte; //SCL Low Timeout Register High
			} SLTH;
		};
		uint16_t SLT;
	};
} volatile i2cRegs_t;

#endif //_MICRON_I2C_REGS_H_
