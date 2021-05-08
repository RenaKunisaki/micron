//USB registers on the Cortex-M4
#ifndef _USB_REGISTERS_H_
#define _USB_REGISTERS_H_

//Flags for the USB0_ENDPTn registers
#define USB_ENDPOINT_UNUSED 0
#define USB_ENDPOINT_TX (USB_ENDPT_EPCTLDIS | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK)
#define USB_ENDPOINT_RX (USB_ENDPT_EPCTLDIS | USB_ENDPT_EPRXEN | USB_ENDPT_EPHSHK)
#define USB_ENDPOINT_TX_RX (USB_ENDPOINT_TX | USB_ENDPOINT_RX)

#define USB_STAT_ODD_SHIFT  2
#define USB_STAT_TX_SHIFT   3
#define USB_STAT_ENDP_SHIFT 4

//endpoint control registers
#define USB_ENDPT_PTR(n) (&USB0_ENDPT0 + ((n) * 4))
#define USB_ENDPT_REG(n) (*(USB_ENDPT_PTR(n)))

typedef enum { //speed enum for usb_init()
	USB_SPEED_LOW = 0,    //  1.5 Mbit/s (USB 1.0 Low speed)
	USB_SPEED_FULL,       // 12.0 Mbit/s (USB 1.0 "Full" speed)
	USB_SPEED_HIGH,       //480.0 Mbit/s (USB 2.0 High speed)
	//USB 3.x is not supported on Teensy, but included here for completion.
	USB_SPEED_SUPER,      //  5.0 Gbit/s (USB 3.0 SuperSpeed)
	USB_SPEED_SUPER_PLUS, // 10.0 Gbit/s (USB 3.1 SuperSpeed+)
} USBSpeed;

//Packet IDs
#define USB_PID_OUT     0x1
#define USB_PID_IN      0x9
#define USB_PID_SOF     0x5
#define USB_PID_SETUP   0xD

//These are listed in the K10 reference manual.
//It says they're only used in host mode, but I get ACK in client mode too...
#define USB_PID_TIMEOUT 0x0
#define USB_PID_ACK     0x2
#define USB_PID_DATA0   0x3
#define USB_PID_NAK     0xA
#define USB_PID_DATA1   0xB
#define USB_PID_STALL   0xE
#define USB_PID_ERROR   0xF

#endif //_USB_REGISTERS_H_
