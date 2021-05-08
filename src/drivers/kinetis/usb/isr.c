#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

/** Called when host requests a given configuration.
 *  config: Which configuration is requested.
 *  Returns 1 if OK, 0 if not.
 */
int isrDefaultUsbConfigure(uint8_t config) {
	USB_DPRINT("CONFIG %d\r\n", config);

	//enable all configured endpoints.
	volatile uint8_t *reg = &USB0_ENDPT0;
	for(int i=0; i<USB_MAX_ENDPOINTS; i++) {
		usbEndpCfg_t *cfg = &usbEndpCfg[i];
		//if(i > 0) cfg->txFlagData = 0;
		*reg = cfg->config;
		 reg += 4; //to next endpoint

		uint8_t size = cfg->maxPkSize;
		usbBdt[USB_BDT_INDEX(i, USB_RX, USB_EVEN)].addr = cfg->rxbuf[0];
		usbBdt[USB_BDT_INDEX(i, USB_RX, USB_EVEN)].desc = USB_BDT_DESC(size, USB_DATA0);
		usbBdt[USB_BDT_INDEX(i, USB_RX, USB_ODD )].addr = cfg->rxbuf[1];
		usbBdt[USB_BDT_INDEX(i, USB_RX, USB_ODD )].desc = USB_BDT_DESC(size, USB_DATA1);
		usbBdt[USB_BDT_INDEX(i, USB_TX, USB_EVEN)].desc = 0;
		usbBdt[USB_BDT_INDEX(i, USB_TX, USB_ODD )].desc = 0;
	}
	return 1;
}


/** RESET interrupt handler.
 *  Called from isrUsb() when reset request is received.
 */
void isrDefaultUsbReset() {
	USB_DPRINT("RESET\r\n");

	//initialize endpoint 0 ping-pong buffers
	USB0_CTL |= USB_CTL_ODDRST;
	usbEndpCfg[0].txFlagOdd = 0;
	_usbInternalReset();

	//initialize endpoint0 to 0x0D (see K20P64M72SF1RM.pdf § 41.5.23)
	//receive, transmit, and handshake
	usbConfigureEndpoint(0, USB_ENDP0_SIZE,
		USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK,
		usbHandleEndp0);

	USB0_ERRSTAT = 0xFF; //clear all interrupts...this is a reset
	USB0_ISTAT   = 0xFF;
	USB0_ADDR    = 0; //after reset, we are address 0, per USB spec
	usbAddress   = 0;
	usbConfigID  = 0;

	//enable USB interrupts
	USB0_ERREN = 0xFF; //Enable all error interrupts.
	USB0_INTEN = USB_INTEN_USBRSTEN | USB_INTEN_ERROREN |
		USB_INTEN_SOFTOKEN | USB_INTEN_TOKDNEEN |
		USB_INTEN_SLEEPEN  | USB_INTEN_STALLEN;

	DataSync(); //flush memory caches
}


/** ERROR interrupt handler.
 *  Called from isrUsb() when an error occurs.
 *  This happens when we unplug the USB cable. Sometimes you'll see an error
 *  written to UART before the cable is fully removed.
 */
void isrDefaultUsbError() {
	/** USB0_ERRSTAT Fields:
	 *  Bit 7: BTSERR:  Bit stuff error
	 *  Bit 6: ------   Reserved
	 *  Bit 5: DMAERR:  DMA failed - bus busy, or packet too large
	 *  Bit 4: BTOERR:  Bus turnaround timeout
	 *  Bit 3: DFN8:    Data field not 8 bits long
	 *  Bit 2: CRC16:   Data packet CRC16 error
	 *  Bit 1: CRC5EOF: In peripheral mode: CRC5 error in host token packet
	 *                  In host mode: SOF counter == 0
	 *  Bit 0: PIDERR:  PID check failed
	 */
	USB_DPRINT(" --- USB ERROR %02X ---\r\n", USB0_ERRSTAT);
	USB0_ERRSTAT = USB0_ERRSTAT;    //acknowledge error condition(s)
	USB0_ISTAT   = USB_ISTAT_ERROR; //acknowledge interrupt
}


/** SOFTOK (Start-Of-Frame Token) interrupt handler.
 *  Called from isrUsb() when a Start Of Frame Token is received
 *  (which is quite often). That's our cue to transmit.
 */
void isrDefaultUsbSof() {
    uint8_t stat = USB0_STAT;
	for(int i=0; i<USB_MAX_ENDPOINTS; i++) {
		//resume any pending transmissions
        usbEndpCfg_t *cfg = &usbEndpCfg[i];
        if(cfg->txHandler) {
            USB_DPRINT("Call TX handler for endp %d\r\n", i);
            volatile usbBdt_t* bdt = &usbBdt[USB_BDT_INDEX(i,
        		(stat & USB_STAT_TX)  >> USB_STAT_TX_SHIFT,
        		(stat & USB_STAT_ODD) >> USB_STAT_ODD_SHIFT)];
            cfg->txHandler(i, bdt, stat);
        }
		else _usbContinueTx(i);
	}
	USB0_ISTAT = USB_ISTAT_SOFTOK; //acknowledge interrupt
}


/** TOKDNE (Token Done) interrupt handler.
 *  Called from isrUsb() when a token is fully processed.
 */
void isrDefaultUsbToken() {
	/* This interrupt fires when either:
	 * 1. We've received an OUT or SETUP token and corresponding DATA packet
	 * 2. We've sent an IN token and received corresponding ACK
	 * The USB0_STAT register tells what happened, which endpoint the data
	 * was sent to/from, and which buffer (even/odd) was used.
	 */
	uint8_t stat = USB0_STAT;
	int endpoint = stat >> 4; //which endpoint?
	usbEndpCfg_t *cfg = &usbEndpCfg[endpoint];

	//look up the corresponding buffer.
	volatile usbBdt_t* bdt = &usbBdt[USB_BDT_INDEX(endpoint,
		(stat & USB_STAT_TX)  >> USB_STAT_TX_SHIFT,
		(stat & USB_STAT_ODD) >> USB_STAT_ODD_SHIFT)];
	int pid = USB_BDT_PID(bdt->desc);

#if USB_ENABLE_DPRINT
	if(usbConfigID && endpoint >= 0) {
		//if(pid == USB_PID_SETUP) USB_DPRINT("\r\n");
		int len = USB_BDT_LEN(bdt);
		const uint8_t *in = (const uint8_t*)bdt->addr;
		USB_DPRINT("%-6s (%X) E%d %c%c, D%c, bdt %p, len %3d, addr %p: ",
			usbPidNames[pid], pid, endpoint,
			(stat & USB_STAT_TX)  ? 'T' : 'R',
			(stat & USB_STAT_ODD) ? 'D' : 'E',
			(bdt->desc & USB_BDT_DATA1) ? '1' : '0',
			bdt, len, in);
		if(stderr) {
			static const char *hex = "0123456789ABCDEF";
			for(int i=0; i<len; i++) {
				char c[5];
				c[0] = in[i];
				if(c[0] < 0x20 || c[0] > 0x7E) {
					unsigned char n = (unsigned char)c[0];
					c[0] = '\\'; c[1] = 'x';
					c[2] = hex[n >> 4]; c[3] = hex[n & 0xF];
					c[4] = '\0';
				}
				else c[1] = '\0';
				fprintf(stderr, "%s", c);
				//fprintf(stderr, "%02X%s", in[i], (i & 1) ? " " : "");
			}
			fputs("\r\n", stderr);
		}
	}
#endif

	if(pid == USB_PID_IN) {
		//acknowledged our transmission; continue it, if necessary.
        if(cfg->txHandler) cfg->txHandler(endpoint, bdt, stat);
		else _usbContinueTx(endpoint);
	}
	else if(pid == USB_PID_SETUP) {
		//clear any pending IN stuff
		usbBdt[USB_BDT_INDEX(endpoint, USB_TX, USB_EVEN)].desc = 0;
		usbBdt[USB_BDT_INDEX(endpoint, USB_TX, USB_ODD )].desc = 0;

		// first IN after Setup is always DATA1
		cfg->txFlagData = 1;
	}

	//call the handler
	cfg->pkHandler(endpoint, bdt, stat);

	//give the buffer back
	if(pid == USB_PID_OUT || pid == USB_PID_SETUP) {
		//Ugh.
		//bdt->desc = USB_BDT_DESC(cfg->maxPkSize, (stat & USB_STAT_ODD));
		//bdt->desc = USB_BDT_DESC(cfg->maxPkSize, !cfg->txFlagData);
		if(endpoint == 0) bdt->desc = USB_BDT_DESC(cfg->maxPkSize, USB_DATA1);
		else bdt->desc = USB_BDT_DESC(cfg->maxPkSize,
			((uint32_t)bdt & 8) ? USB_DATA1 : USB_DATA0);
		DataSync();
	}

	//debug
#if USB_DEBUG_COUNT_PIDS
	if     (pid == USB_PID_IN)    cfg->nIn++;
	else if(pid == USB_PID_OUT)   cfg->nOut++;
	else if(pid == USB_PID_SETUP) cfg->nSetup++;
	else if(pid == USB_PID_SOF)   cfg->nSOF++;
#endif

	//acknowledge receipt of packet, and free up the slot
	//in the receive FIFO so that we can receive more packets.
	USB0_CTL   = USB_CTL_USBENSOFEN; //USB enabled, clear busy/suspend bit
	USB0_ISTAT = USB_ISTAT_TOKDNE;   //acknowledge TOKDNE interrupt
}


/** SLEEP interrupt handler.
 *  Called from isrUsb() when the bus is idle.
 *  Usually this indicates we aren't connected or something went totally wrong.
 */
void isrDefaultUsbSleep() {
	USB_DPRINT("SLEEP\r\n");
	USB0_ISTAT = USB_ISTAT_SLEEP; //acknowledge interrupt
}


/** RESUME interrupt handler.
 *  Called from isrUsb() when remote wakeup is signalled. (XXX what?)
 */
void isrDefaultUsbResume() {
	USB0_ISTAT = USB_ISTAT_RESUME; //acknowledge interrupt
}


/** ATTACH interrupt handler.
 *  Called from isrUsb() when a device is attached in host mode.
 */
void isrDefaultUsbAttach() {
	USB0_ISTAT = USB_ISTAT_ATTACH; //acknowledge interrupt
}


/** STALL interrupt handler.
 *  Called from isrUsb() when we've stalled.
 *  This indicates we're unable to handle a request.
 */
void isrDefaultUsbStall() {
	USB_DPRINT("STALL  stat=%02X\r\n", USB0_STAT);
	USB0_ISTAT = USB_ISTAT_STALL; //acknowledge interrupt
}


//The actual isrDefaultUsb() is in isr.c (not usb/isr.c) because for some
//reason aliases have to be defined in the same file as the symbol they're
//aliased to. That's also why these are here:

void isrUsbStall()   WEAK ALIAS("isrDefaultUsbStall");
void isrUsbAttach()  WEAK ALIAS("isrDefaultUsbAttach");
void isrUsbResume()  WEAK ALIAS("isrDefaultUsbResume");
void isrUsbSleep()   WEAK ALIAS("isrDefaultUsbSleep");
void isrUsbToken()   WEAK ALIAS("isrDefaultUsbToken");
void isrUsbSof()     WEAK ALIAS("isrDefaultUsbSof");
void isrUsbError()   WEAK ALIAS("isrDefaultUsbError");
void isrUsbReset()   WEAK ALIAS("isrDefaultUsbReset");
int  isrUsbConfigure(uint8_t config) WEAK ALIAS("isrDefaultUsbConfigure");

#ifdef __cplusplus
	} //extern "C"
#endif
