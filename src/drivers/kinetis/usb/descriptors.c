#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>


/** This table is searched by the default endpoint 0 SETUP handler to find
 *  the descriptor to send in response to a GET DESCRIPTOR request.
 *  You could override this symbol if you want to delete some descriptors, but
 *  if you just want to add new ones, use the usbDescriptors table below.
 */
WEAK extern const usbDcrEntry_t usbDefaultDescriptors[] = {
//wValue is high byte descriptor type, low byte descriptor index
//	 wValue, wIndex, address,             length,                     name
/*	{0x0100, 0x0000, &usbDevDescriptor, sizeof(usbDevDescriptor), "device descriptor"},
    {0x0200, 0x0000, p_usbCfgDescriptor, usbCfgDescriptorLen, "config descriptor"},

	{0x0300, 0x0000,           &usb_langid_list,  sizeof(usb_langid_list), "langID list"},
	{0x0301, USB_LANGID_EN_US, &usb_mfr_desc,     sizeof(usb_mfr_desc), "mfr desc"},
	{0x0302, USB_LANGID_EN_US, &usb_product_desc, sizeof(usb_product_desc), "product desc"},
#ifdef USB_SERIALNO
	{0x0303, USB_LANGID_EN_US, &usb_serialno_desc, sizeof(usb_serialno_desc), "product serial#"},
#endif

	{0x0600, 0x0000, &usbDevQualifier, sizeof(usbDevQualifier), "device qualifier"}, */

	{0x0000, 0x0000, NULL, 0, ""} //end of list
};

/** Second table of descriptors. This is checked first by the default endpoint 0
 *  SETUP handler. If a descriptor isn't found here, it then checks the
 *  usbDefaultDescriptors table above.
 *  Your program can override this symbol to add extra descriptors without
 *  having to duplicate the default list.
 */


#ifdef __cplusplus
	} //extern "C"
#endif
