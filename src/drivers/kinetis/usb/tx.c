#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

/** Stall the specified endpoint.
 *  This tells the host that we're unable to handle the request it just made.
 */
void usbStall(int endp) {
	USB_ENDPT_REG(endp) |= USB_ENDPT_EPSTALL;
	USB_DPRINT("Stall endp %02X\r\n", endp);
}


/** Send some data to the specified endpoint.
 *  data:   Data to send.
 *  length: Length of data.
 *  endp:   Endpoint to send to.
 *  Returns 0 on success, or a negative error code on failure.
 *  If a transmission is already pending, this one will be queued after it,
 *  which requires malloc().
 */
int usbSend(const void *data, int length, int endp) {
	//XXX implement a timeout.
	USB_DPRINT("usbSend(%p, %d, %d): begin tx\r\n", data, length, endp);

	int r = 0;
	usbTx_t *tx = _usbPrepareTx(data, length, &r);
	if(!tx) return r;

	r = _usbQueueTx(tx, endp);
	USB_DPRINT("usbSend(%p, %d, %d): OK, %d\r\n", data, length, endp, r);
	return r;
}


/** Send some data to the specified endpoint, only if there isn't already a
 *  transmission queued on that endpoint.
 *  data:   Data to send.
 *  length: Length of data.
 *  endp:   Endpoint to send to.
 *  Returns 0 on success, or a negative error code on failure.
 */
int usbTrySend(const void *data, int length, int endp) {
	int r;
	irqDisable(); //avoid tx queue being modified while we read it
	if(usbEndpCfg[endp].tx) r = -EBUSY;
	else r = usbSend(data, length, endp);
	irqEnable();
	return r;
}


/** Send a null-terminated string to the specified endpoint.
 *  str:  String to send.
 *  endp: Endpoint to send to.
 *  Returns 0 on success, or a negative error code on failure.
 */
int usbSendStr(const char *str, int endp) {
	return usbSend(str, strlen(str), endp);
}


/** Send some data to an endpoint, and call a callback function once the
 *  transmission is complete.
 *  data:   Data to send.
 *  length: Length of data.
 *  endp:   Endpoint to send to.
 *  cb:     Callback function.
 *  Returns 0 on success, or a negative error code on failure.
 */
int usbSendCallback(const void *data, int length, int endp,
usbTxCompleteCallback cb) {
	int r = 0;
	usbTx_t *tx = _usbPrepareTx(data, length, &r);
	if(!tx) return r;
	tx->onComplete = cb;
	return _usbQueueTx(tx, endp);
}


/** Query how much data is queued to transmit.
 *  endp:  Endpoint to query.
 *  count: If not NULL, will be set to the number of pending transmissions.
 *  Returns the number of bytes queued to transmit.
 */
size_t usbTxQueueLen(int endp, int *count) {
	int n = 0;
	size_t bytes = 0;

	irqDisable(); //avoid tx queue being modified while we read it

	usbTx_t *tx = usbEndpCfg[endp].tx;
	while(tx) {
		n++;
		bytes += tx->len - tx->pos;
		tx = tx->next;
	}

	irqEnable();
	if(count) *count = n;
	return bytes;
}


/** Query whether there is a transmission pending.
 *  endp: Endpoint to query.
 *  Returns 1 if a transmission is pending, 0 if not.
 */
int usbTxInProgress(int endp) {
	return usbEndpCfg[endp].tx != NULL;
}


/** Cancel all pending transmissions on the specified endpoint.
 *  endp: Endpoint to cancel.
 *  On success, returns the number of transmissions cancelled (which may be
 *  zero). On failure, returns a negative error code.
 */
int usbCancelTx(int endp) {
	int count = 0;
	irqDisable(); //avoid tx queue being modified while we read it

	usbTx_t *tx = usbEndpCfg[endp].tx;
	while(tx) {
		count++;
		usbTx_t *next = tx->next;
		//XXX should we call tx->onComplete()?
		if(tx->shouldFree) free((void*)tx->data);
		free(tx);
		tx = next;
	}
	usbEndpCfg[endp].tx = NULL;

	irqEnable();
	return count;
}


#ifdef __cplusplus
	} //extern "C"
#endif
