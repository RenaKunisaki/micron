//USB Human Interface Device
#ifndef _USB_HID_H_
#define _USB_HID_H_

//Descriptor types
#define USBHID_DT_HID      0x21
#define USBHID_DT_REPORT   0x22
#define USBHID_DT_PHYSICAL 0x23
//0x24-0x2F: reserved


//HID Report Descriptor
typedef struct {
	uint8_t  bDescriptorType;
	uint16_t wDescriptorLength;
} PACKED usbhid_report_dcr_t;

//HID Interface Descriptor
typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdHID;
	uint8_t  bCountryCode;
	uint8_t  bNumDescriptors;
	usbhid_report_dcr_t report;
} PACKED usbhid_iface_dcr_t;


#endif //_USB_HID_H_
