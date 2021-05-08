#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>


/** Called to handle a USB setup packet on endpoint 0.
 *  packet: The packet.
 *  data:   Output: set to the response data, if any.
 *  Return value:
 *   -return  0 to reply with an empty acknowledgement.
 *   -return -1 if unable to handle this packet.
 *   -otherwise, return the length to transmit, and set data
 *    to point to the response.
 */
WEAK int usbHandleSetup(usbSetup_t *packet, const void **data) {
	return -1;
}

/** Called when we receive the magic reboot command from Teensy
 *  programmer. If this function returns 0, we'll reboot. Otherwise,
 *  we'll ignore the command.
 */
WEAK COLD int usbHandleMagicReboot(void) {
    return 0;
}

/** Default handlers for USB setup packets that aren't handled
 *  by the user-provided handler.
 */
int usbDefaultHandleSetup(usbSetup_t *packet, const void **data) {
	switch(packet->wRequestAndType) {
		//high byte is request type, low byte is request

		case 0x0080: { //get device status
			//response:
			//bit 0: set if device is self-powered
			//bit 1: set if remote wakeup is enabled
			//bits 2-15: 0 (reserved)
			static uint16_t response = 0;
			*data = &response;
			return sizeof(response);
		}

		case 0x0500: //set address
			//per USB spec, we don't actually set the address until
			//the next IN packet. for now, just acknowledge the packet.
			//usbDefaultHandleEndp0() takes care of setting the address.
			return 0;

		case 0x0680: //get descriptor
		case 0x0681: {
			const usbDcrEntry_t* entry = _usbFindDescriptor(
				usbDescriptors, packet->wValue, packet->wIndex);
			if(entry) {
				USB_DPRINT("GETDCR 0x%04X:%04X: %3d bytes @ 0x%08lX: %s\r\n",
					packet->wValue, packet->wIndex,
					entry->length, (uint32_t)entry->addr, entry->name);
				*data = (const uint8_t*)entry->addr;
				return entry->length;
			}
			USB_DPRINT("GETDCR 0x%04X:%04X: <not found>\r\n",
				packet->wValue, packet->wIndex);
			break;
		}

		case 0x0880: //get configuration
			*data = (const void*)&usbConfigID;
			return sizeof(usbConfigID);

		case 0x0900: { //set configuration
			//wValue tells us which configuration.
			int ok = isrUsbConfigure(packet->wValue);
			if(ok) {
				usbConfigID = packet->wValue;
				//USB_DPRINT("Selected USB config %d\r\n", usbConfigID);
				return 0;
			}
			break;
		}

		case 0x2021: //USBCDC_REQ_SET_LINE_CODING
			//used by Teensy uploader as a reboot signal.
			//we don't need to do anything here except acknowledge the packet.
			return 0;

		default: { //stall
			//write debug message
			static const char *hex = "0123456789ABCDEF";
			char msg[1024];
			char *dst = msg;
			uint8_t *data = (uint8_t*)packet;
			for(unsigned int i=0; i<sizeof(usbSetup_t); i++) {
				*(dst++) = hex[data[i] >> 4];
				*(dst++) = hex[data[i] & 0xF];
				if(i & 1) *(dst++) = ' ';
			}
			USB_DPRINT(" *** Unhandled SETUP request: %s\r\n", msg);
			break;
		}
    }
	return -1; //stall
}


//Default handler for endpoint 0.
void usbDefaultHandleEndp0(
int endp, volatile usbBdt_t *buffer, uint8_t stat) {
	static usbSetup_t lastSetup;
	usbSetup_t *packet = (usbSetup_t*)buffer->addr;

	int pid = USB_BDT_PID(buffer->desc);
	/* USB_DPRINT("USBEP0 %s, %s, %s, D%d\r\n", usbPidNames[pid],
		(stat & USB_STAT_TX) ? "TX" : "RX",
		(stat & USB_STAT_ODD) ? "Odd" : "Evn",
		(buffer->desc & USB_BDT_DATA1) ? 1 : 0); */

	switch(pid) {
		case USB_PID_SETUP: {
			lastSetup = *packet; //keep a copy around, we'll need it later.

			//try the user handler, then the default handler.
			const void* data = NULL;
			int length = usbHandleSetup(packet, &data); //call user handler
			if(length < 0) length = usbDefaultHandleSetup(packet, &data);

			//send response if we have any.
			if(length < 0) {
				//USB_DPRINT("SETUP0 resp: STALL\r\n");
				USB0_ENDPT0 |= USB_ENDPT_EPSTALL;
			}
			else {
				if(length > packet->wLength) length = packet->wLength;
				usbSend(data, length, 0);
			}
			break;
		}

		case USB_PID_IN: //sent data to host and received ACK
			if(lastSetup.wRequestAndType == 0x0500) {
				//Actually set the address now.
				//USB spec requires that we wait until an IN token
				//before changing the address.
				USB0_ADDR  = lastSetup.wValue;
				usbAddress = lastSetup.wValue;
				USB_DPRINT("ADRESS %d\r\n", usbAddress);
			}
			break;

		case 2: //??? XXX what is this?
		case USB_PID_OUT: { //received data from host
			//Magic. Teensy uploader sends CDC_SET_LINE_CODING packet with a
			//single data byte, value 134, to request soft reboot.
			//That arrives as a SETUP packet followed by this data packet.
			if(lastSetup.wRequestAndType == 0x2021 //CDC_SET_LINE_CODING
			&& lastSetup.wLength == 1
			&& (*(uint8_t*)buffer->addr) == 134) {
				USB_DPRINT("Rebooting!\r\n");
				osBootloader();
			}
			break;
		}

		case USB_PID_SOF: //start of frame
			//don't care. we have SOF interrupt anyway
			break;

		/* The manual lists these additional PIDs:
		 * 0x0h Bus Timeout
		 * 0x2h ACK
		 * 0x3h DATA0
		 * 0xAh NAK
		 * 0xBh DATA1
		 * 0xEh STALL
		 * 0xFh Data Error
		 * But it says they're only used in host mode...
		 */

		default:
			USB_DPRINT("USB: Unhandled PID %X on endpoint 0\r\n", pid);
	}
}

void usbHandleEndp0(int endp, volatile usbBdt_t *buffer, uint8_t stat)
	WEAK ALIAS("usbDefaultHandleEndp0");

#ifdef __cplusplus
	} //extern "C"
#endif
