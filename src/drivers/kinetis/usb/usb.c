#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

const char *usbPidNames[] = { //debug
	"TIMEOUT", "OUT",   "ACK",   "DATA0",
	"PID4",    "SOF",   "PID6",  "PID7",
	"PID8",    "IN",    "NAK",   "DATA1",
	"PIDC",    "SETUP", "STALL", "ERROR",
};

volatile uint8_t usbAddress = 0xFF; //device address (FF=none given yet)
//address 0 is the one we actually have at power-on, but we use FF here to
//indicate that we haven't even been given an address as opposed to being reset
//after getting one. it makes no difference to the host because we do still
//respond to address 0 and not FF (which is invalid anyway).

volatile uint8_t usbConfigID = 0; //current configuration #, 0=unconfigured


/** Initialize and enable USB system.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 *  Note: This function only turns on the USB module. It does not establish a
 *  connection. After calling this, set up the endpoints and then call
 *  usbConnect().
 */
int usbInit() {
	//Enable USB module's clock signal (turn on USB module)
	SIM_SCGC4 |= SIM_SCGC4_USBOTG;

	//Set clock signals
	//we use MCGPLLCLK divided by USB fractional divider
	SIM_SOPT2  |= SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL;
	SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1);
	//(USBFRAC + 0)/(USBDIV + 1) = (1 + 0)/(1 + 1) = 1/2 for 96Mhz clock

	//Reset the USB module
	USB0_USBTRC0 |= USB_USBTRC_USBRESET;

	//Init our buffers and variables while it initializes
	MEMSET_ALL(usbBdt, 0);
	_usbInternalReset();
	while(USB0_USBTRC0 & USB_USBTRC_USBRESET); //wait for reset to finish

	//Tell USB module where our buffer table is
	USB0_BDTPAGE1 = ((uint32_t)usbBdt) >>  8; //bits 15-9
	USB0_BDTPAGE2 = ((uint32_t)usbBdt) >> 16; //bits 23-16
	USB0_BDTPAGE3 = ((uint32_t)usbBdt) >> 24; //bits 31-24

	//Clear interrupt flags and enable interrupt sources
	USB0_ISTAT    = 0xFF;
	USB0_ERRSTAT  = 0xFF;
	USB0_ERREN    = 0xFF;
	USB0_OTGISTAT = 0xFF;
	USB0_USBTRC0 |= 0x40; //undocumented interrupt bit
	USB0_INTEN   |= USB_INTEN_USBRSTEN; //enable reset interrupt

	//Enable USB
	USB0_CTL     = USB_CTL_USBENSOFEN;
	USB0_USBCTRL = 0;

	//Enable the USB IRQ
	//NVIC_SET_PRIORITY(IRQ_USBOTG, 112);
	NVIC_SET_PRIORITY(IRQ_USBOTG, 32);
	NVIC_ENABLE_IRQ  (IRQ_USBOTG);

	return 0;
}


/** Enables the connection to USB host.
 *  speed: One of USB_SPEED_*; indicates desired operating speed.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 *  This function "plugs in" the device to the host, after enabling USB via
 *  usbInit(). It does not wait for the host to enumerate the device.
 */
int usbConnect(USBSpeed speed) {
	//Enable the pull-up resistor that tells the host we're connected.
	//Which resistor we use determines the connection speed.
	switch(speed) {
		//case USB_SPEED_LOW: //USB 1.x Low Speed
			//Pullup resistor on D-
			//XXX how to enable that?
			//break;

		case USB_SPEED_FULL: //USB 1.x Full Speed
		case USB_SPEED_HIGH: //USB 2.x High Speed
			USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG; //Pullup on D+

			//for 2.0, after we receive a RESET, we pull D- high.
			//XXX unsure how that works exactly.
			break;

		//USB 3.x is not supported by this hardware.
		default:
			return -ENOSYS; //unsupported mode
	}
	return 0;
}


/** Configures a USB endpoint.
 *  endp:      Which endpoint.
 *  size:      Maximum packet size.
 *  config:    One of USB_ENDPOINT_*.
 *  pkHandler: Function to handle received packets and requests
 *             directed to this endpoint.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 */
int usbConfigureEndpoint(uint8_t endp, uint8_t size, uint8_t config,
usbEndpCallback pkHandler) {
	if(endp >= USB_MAX_ENDPOINTS) return -ENXIO; //no such device or address
	usbEndpCfg_t *cfg = &usbEndpCfg[endp];

	//USB_DPRINT("ENPCFG %d, size %d, cfg %02X, handler %p\r\n",
	//	endp, size, config, pkHandler);

	if(config & USB_ENDPT_EPRXEN) {
		//if receiving is enabled and we don't have a receive buffer,
		//allocate one. we'll use one big block for both rx buffers.
		if(!cfg->rxbuf[0]) {
			uint8_t *buf = (uint8_t*)malloc(size * 2);
			if(!buf) return -ENOMEM;

			cfg->rxbuf[0] = buf;
			cfg->rxbuf[1] = &buf[size];
			cfg->shouldFreeRx0 = 1;
			cfg->shouldFreeRx1 = 0;
		}
	}
	else {
		//if receiving is disabled and we do have a receive buffer, free it.
		if(cfg->rxbuf[0] && cfg->shouldFreeRx0) free(cfg->rxbuf[0]);
		if(cfg->rxbuf[1] && cfg->shouldFreeRx1) free(cfg->rxbuf[1]);
		cfg->rxbuf[0] = NULL;
		cfg->rxbuf[1] = NULL;
	}
	if(!(config & USB_ENDPT_EPTXEN)) {
		//wait for transmission to complete
		while(cfg->tx) irqWait();
	}

    cfg->maxPkSize    = size;
	cfg->config       = config;
	cfg->pkHandler    = pkHandler;
	cfg->txHandler    = NULL;
	cfg->tx           = NULL;
	cfg->txFlagOdd    =  0;
	//cfg->txFlagData =  1;

	usbBdt[USB_BDT_INDEX(endp, USB_RX, USB_EVEN)].addr = cfg->rxbuf[0];
	usbBdt[USB_BDT_INDEX(endp, USB_RX, USB_EVEN)].desc = USB_BDT_DESC(size, USB_DATA0);
	usbBdt[USB_BDT_INDEX(endp, USB_RX, USB_ODD )].addr = cfg->rxbuf[1];
	usbBdt[USB_BDT_INDEX(endp, USB_RX, USB_ODD )].desc = USB_BDT_DESC(size, USB_DATA1);
	usbBdt[USB_BDT_INDEX(endp, USB_TX, USB_EVEN)].desc = 0;
	usbBdt[USB_BDT_INDEX(endp, USB_TX, USB_ODD )].desc = 0;

	//write this config to the module, even if we're not configured yet.
	volatile uint8_t *reg = (&USB0_ENDPT0) + (endp * 4);
	*reg = config;

	return 0;
}

int usbSetTxHandler(uint8_t endp, usbTxHandler handler) {
    usbEndpCfg[endp].txHandler = handler;
    return 0;
}

#ifdef __cplusplus
	} //extern "C"
#endif
