#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>


/** USB Buffer Descriptor Table
 *  See see K20P64M72SF1RM.pdf § 41.4.1
 *  4 entries for each endpoint: 2 TX buffers and 2 RX buffers.
 *  Device can have a maximum of 16 endpoints,
 *  with endpoint 0 used for device control.
 *  Has its own RAM section mainly because it needs to be aligned
 *  to a 512-byte boundary. The buffers themselves do not need to be aligned.
 */
SECTION(".usb_bdt") volatile usbBdt_t usbBdt[USB_MAX_ENDPOINTS*4];

/** Our internal endpoint configuration and state tables.
 */
SECTION(".bss") usbEndpCfg_t usbEndpCfg[USB_MAX_ENDPOINTS];


/** Default descriptor list.
 *  Your program can override this to add more descriptors.
 */
WEAK extern const usbDcrEntry_t usbDescriptors[] = {
	USB_DEFAULT_DESCRIPTORS,
	{0x0000, 0x0000, NULL, 0, ""} //end of list
};


/** Called from default USB ISR handler and usbInit().
 */
void _usbInternalReset() {
	//USB_DPRINT("RESET internal\r\n");

	//disable all endpoints.
	volatile uint8_t *reg = &USB0_ENDPT0;
	for(int i=0; i<USB_MAX_ENDPOINTS; i++) {
		usbEndpCfg_t *cfg = &usbEndpCfg[i];
		while(cfg->tx) _usbFinishTx(i);
		*reg = 0;
		reg += 4; //to next endpoint
	}
}


/** Search a list of USB descriptors for the one specified by the given
 *  wValue and wIndex, and return its pointer.
 */
const usbDcrEntry_t* _usbFindDescriptor(const usbDcrEntry_t *list,
uint16_t wValue, uint16_t wIndex) {
	const usbDcrEntry_t* entry = NULL;
	for(entry = list; entry->addr != NULL; entry++) {
		if(wValue == entry->wValue && wIndex == entry->wIndex) {
			//this is the descriptor to send
			return entry;
		}
	}
	return NULL;
}


/** Internal function to finish a transmission.
 */
void _usbFinishTx(int endp) {
	usbEndpCfg_t *cfg = &usbEndpCfg[endp];
    usbTx_t *tx = cfg->tx;
	if(!tx) return;
	if(tx->onComplete) tx->onComplete(endp, tx);
	if(tx->shouldFree) free((void*)tx->data);
	cfg->tx = tx->next;
	free(tx);
}


/** Allocate and initialize a usbTx_t.
 *  data: The data to transmit.
 *  length: The length to transmit.
 *  err: Set to 0 on success or a negative error code on failure.
 *  On success, returns a usbTx_t* (which should be free()d when it's no longer
 *  needed). On failure, returns NULL.
 */
usbTx_t* _usbPrepareTx(const void *data, int length, int *err) {
	usbTx_t *tx;

	if(length > 0) {
		//copy into RAM.
		//if source is in flash, it needs to be copied to RAM because flash is
		//too slow for the USB module and CPU to both use at the same time.
		//if source is in RAM, we still need to copy it because it might be a
		//temporary buffer and we can't guarantee it will still be there when we
		//go to actually transmit.
		tx = (usbTx_t*)malloc(sizeof(usbTx_t) + length);
		if(!tx) {
			//irqEnable();
			if(err) *err = -ENOMEM;
			return NULL;
		}
		memcpy(tx->buf, data, length);
		//USB_DPRINT("; buf=%p", tx->buf);
		tx->data = tx->buf;
		//puts((const char*)data);
	}
	else {
		//No need to copy, but we do need a transmit state.
		tx = (usbTx_t*)malloc(sizeof(usbTx_t));
		if(!tx) {
			//irqEnable();
			if(err) *err = -ENOMEM;
			return NULL;
		}
		tx->data = (const uint8_t*)data;
	}

	USB_DPRINT("TX data %p -> %p\r\n", data, tx->data);
	tx->next       = NULL;
	tx->len        = length;
	tx->pos        = 0;
	tx->onComplete = NULL;
	tx->shouldFree = 0;

	if(err) *err = 0;
	return tx;
}


/** Append a transmission to the end of an endpoint's tx queue.
 *  tx:   The transmission to queue.
 *  endp: Which endpoint to transmit on.
 *  Returns 0 on success, or a negative error code on failure.
 */
int _usbQueueTx(usbTx_t *tx, int endp) {
	irqDisable(); //don't let the Tx queue be modified during this.
	usbEndpCfg_t *cfg = &usbEndpCfg[endp];

	//Append this transmission to the end of the queue.
	int count = 0;
    //XXX this line generates a warning that &cfg->tx might be unaligned.
    //can we be sure that won't happen, and stop the warning?
	usbTx_t **next = (usbTx_t**)&cfg->tx;
	while(*next) {
		next = &(*next)->next;
		count++;
	}

	USB_DPRINT("TXQUEUE %d %p [%p]\r\n", count, next, tx->data);
	*next = tx;
	DataSync();

	//_usbContinueTx(endp);
	//wait for SOF interrupt to actually transmit.

	irqEnable();
	return 0;
}


/** Internal function called from USB ISR to continue a pending transmission.
 */
void _usbContinueTx(int endp) {
	irqDisable();
	usbEndpCfg_t *cfg = &usbEndpCfg[endp];
	usbTx_t *curTx = cfg->tx;
	if(!curTx) {
		irqEnable();
		return; //no transmission in progress.
	}

	//Is there a free buffer slot?
	//We check this even if there's nothing left to transmit, because that way
	//we know the hardware has finished transmitting. Otherwise, we might end up
	//freeing the buffer before it's actually been sent, and corrupting it.
	volatile usbBdt_t *bdt= &usbBdt[USB_BDT_INDEX(endp, USB_TX, cfg->txFlagOdd)];
	if(bdt->desc & USB_BDT_OWN) {
		//This one's busy. Try the other.
		bdt = &usbBdt[USB_BDT_INDEX(endp, USB_TX, !cfg->txFlagOdd)];
		if(bdt->desc & USB_BDT_OWN) {
			//Both are busy.
			irqEnable();
			return;
		}

		//Other isn't busy. Update flag accordingly.
		//It would seem like we don't even need this flag, and we can just check
		//for either buffer to be free, but we get communication failures if we
		//do that; ie we need to try one buffer first, then the other.
		//XXX investigate why this is; it's probably a bug.
		cfg->txFlagOdd ^= 1;
	}


	//Are we finished transmitting?
	int len = curTx->len - curTx->pos; //remaining length
	if(len == 0 && curTx->len > 0) {
		USB_DPRINT("TXDONE %d; next=%p -> %p\r\n", endp, curTx->next,
			curTx->next ? curTx->next->data : NULL);
		_usbFinishTx(endp);
		irqEnable();
		return;
	}

	//If the total transmit length is zero, we just send an empty packet, which
	//serves as an acknowledgement that we received a message but have nothing
	//to say about it.
	//We don't want to send an empty packet after sending a non-empty reply.
	//That will just cause random communication errors which you'll go crazy
	//trying to track down.

	int pklen = cfg->maxPkSize; //max packet length.
	if(len > pklen) len = pklen;
	if(len > 0x3FF) len = 0x3FF; //max length allowed by spec

#if USB_ENABLE_DPRINT
	if(len > 0 /* && cur_tx->len >= len*/ ) {
		USB_DPRINT("TXPART %d%c %3d (%3d/%3d): %p: %s\r\n",
			endp, cfg->txFlagOdd ? 'd' : 'e', len,
			curTx->pos, curTx->len, curTx->data,
			(const char*)&curTx->data[curTx->pos]);
	}
#endif

	//begin transmitting.
	USB_ENDPT_REG(endp) &= ~USB_ENDPT_EPSTALL; //un-stall the endpoint
	bdt->addr = (void*)&curTx->data[curTx->pos]; //data to transmit
	bdt->desc = USB_BDT_OWN //release buffer ownership to USB module
		| USB_BDT_DTS //perform Data Toggle Synchronization
		| (cfg->txFlagData ? USB_BDT_DATA1 : USB_BDT_DATA0) //DATAn flag
		| (len << 16); //length to transmit

	cfg->txFlagData ^= 1; //toggle both flags
	cfg->txFlagOdd  ^= 1;
	DataSync(); //ensure buffer is written to memory

	//if this was a 0-length packet, we can finish immediately.
	if(curTx->len == 0) {
		USB_DPRINT("TXACK  %d\r\n", endp);
		_usbFinishTx(endp);
		irqEnable();
		return;
	}


	//update length and data pointers.
	uint32_t pos = curTx->pos + len;
	if(pos > curTx->len) pos = curTx->len;
	curTx->pos = pos;
	USB_DPRINT("Remaining len: %d\r\n", curTx->len - curTx->pos);
	irqEnable();
}

#ifdef __cplusplus
	} //extern "C"
#endif
