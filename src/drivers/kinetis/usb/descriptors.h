//Common USB descriptor structures
#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_
#include "langid.h"

/** Helper macro: evaluates to the number of arguments passed,
 *  after t (the type of the arguments).
 *  e.g. NARGS(int, 0, 9, 15, 9) => 4
 */
#define NARGS(t, ...)  (sizeof((t[]){0, ##__VA_ARGS__})/sizeof(t)-1)

/** Define a USB string descriptor.
 *  name: The variable name of the struct.
 *  text: The text content of the descriptor.
 *  The text must be a wide string! (e.g. u"Hello")
 */
#define USB_STRING_DCR(name, text) struct {        \
	usbStringDcr_t descriptor;                     \
	char16_t str[sizeof(text) / sizeof(char16_t)]; \
} PACKED name = {                                  \
	.descriptor = {                                \
		.bLength         = sizeof(name),           \
		.bDescriptorType = USB_DT_STRING,          \
	},                                             \
	text,                                          \
}

/** Define a USB language ID list descriptor.
 *  name: The variable name of the struct.
 *  ...:  The LANGIDs to list.
 */
#define USB_LANG_LIST_DCR(name, ...) struct {          \
	usbStringDcr_t descriptor;                         \
	uint16_t langIDs[NARGS(uint16_t, __VA_ARGS__)];    \
} PACKED name = {                                      \
	.descriptor = {                                    \
		.bLength         = sizeof(name),               \
		.bDescriptorType = USB_DT_STRING,              \
	},                                                 \
	.langIDs = {                                       \
		__VA_ARGS__                                    \
	},                                                 \
}

/** Define an entry in the USB descriptor list.
 *  wValue, wIndex: The GET_DESCRIPTOR parameters to select this descriptor.
 *  dcr: The descriptor (not a pointer to it).
 */
#define USB_DCR_ENTRY(wValue, wIndex, dcr) \
	{wValue, wIndex, &dcr, sizeof(dcr), #dcr}

/*
#define USB_MANUFACTURER_LEN sizeof(USB_MANUFACTURER) / sizeof(char16_t)
#define USB_PRODUCT_LEN      sizeof(USB_PRODUCT)      / sizeof(char16_t)
#ifdef USB_SERIALNO //no default serial number
#define USB_SERIALNO_LEN     sizeof(USB_SERIALNO)     / sizeof(char16_t)
#endif
*/

/*********************** Descriptor struct definitions ************************/

typedef struct { //Device Descriptor
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} PACKED usbDevDcr_t;

typedef struct { //Endpoint Descriptor
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} PACKED usbEndpDcr_t;

typedef struct { //Interface Descriptor
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
	//usbEndpDcr_t endpoints[];
} PACKED usbIfaceDcr_t;

typedef struct { //Configuration Descriptor
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
    //usbIfaceDcr_t interfaces[];
} PACKED usbCfgDcr_t;

typedef struct { //Device Qualifier (for negotiating high speed)
	uint8_t  bLength;
	uint8_t  bDescriptorType; //USB_DT_DEVICE_QUALIFIER
	uint16_t bcdUSB; //USB version
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint8_t  bNumConfigurations;
	uint8_t  bReserved;
} PACKED usbDevQualifier_t;

typedef struct { //String Descriptor
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	//int16_t wString[];
} PACKED usbStringDcr_t;

//Entry in table of descriptors used to look up GET_DESCRIPTOR responses
typedef struct {
    uint16_t    wValue; //descriptor type & idx
    uint16_t    wIndex; //language ID for strings, else 0
    const void *addr;
    uint16_t    length;
	const char *name; //for debug
} PACKED usbDcrEntry_t;


/**************************** Default descriptors *****************************/


//Product ID language list - list of LANGIDs for which we have strings.
WEAK extern const USB_LANG_LIST_DCR(usbLangidList, USB_LANGID_EN_US);

//Manufacturer name (EN-US)
WEAK extern const USB_STRING_DCR(usbStrMfr_en_us, USB_MANUFACTURER);

//Product name (EN-US)
WEAK extern const USB_STRING_DCR(usbStrProd_en_us, USB_PRODUCT);

//Product serial number, if any
#ifdef USB_SERIALNO
	WEAK extern const USB_STRING_DCR(usbStrSerialno, USB_SERIALNO);
#endif


//Default device descriptor
WEAK extern const usbDevDcr_t usbDevDescriptor = {
	.bLength =            sizeof(usbDevDcr_t),
	.bDescriptorType =    USB_DT_DEVICE,
	.bcdUSB =             USB_VERSION,
	.bDeviceClass =       USB_DEVICE_CLASS,
	.bDeviceSubClass =    USB_DEVICE_SUBCLASS,
	.bDeviceProtocol =    USB_DEVICE_PROTOCOL,
	.bMaxPacketSize0 =    USB_ENDP0_SIZE,
	.idVendor =           USB_VID,
	.idProduct =          USB_PID,
	.bcdDevice =          USB_DEVICE_VERSION, //device version #
	.iManufacturer =      1, //string descriptor 0x0301
	.iProduct =           2, //string descriptor 0x0302
#ifdef USB_SERIALNO
	.iSerialNumber =      3, //string descriptor 0x0303
#else
	.iSerialNumber =      0, //none
#endif
	.bNumConfigurations = USB_NUM_CONFIGURATIONS,
};


//Device Qualifier Descriptor
//This is sent sometimes when communicating with USB 2.x devices.
//It's similar to the device descriptor, but contains only the fields that can
//change when switching speeds.
//Responding to the request for this descriptor indicates that we're able to
//operate at USB 2.0 speeds.
WEAK extern const usbDevQualifier_t usbDevQualifier = {
	.bLength =            sizeof(usbDevQualifier_t),
	.bDescriptorType =    USB_DT_DEVICE_QUALIFIER,
	.bcdUSB =             USB_VERSION,
	.bDeviceClass =       USB_DEVICE_CLASS,
	.bDeviceSubClass =    USB_DEVICE_SUBCLASS,
	.bDeviceProtocol =    USB_DEVICE_PROTOCOL,
	.bMaxPacketSize0 =    USB_ENDP0_SIZE,
	.bNumConfigurations = USB_NUM_CONFIGURATIONS,
	.bReserved =          0,
};

//List of default descriptors. ------------  wValue  wIndex  Descriptor
#define USB_DEFAULT_DEV_DCR    USB_DCR_ENTRY(0x0100, 0x0000, usbDevDescriptor)
#define USB_DEFAULT_LANGID_DCR USB_DCR_ENTRY(0x0300, 0x0000, usbLangidList)
#define USB_DEFAULT_MFR_DCR    USB_DCR_ENTRY(0x0301, USB_LANGID_EN_US, usbStrMfr_en_us)
#define USB_DEFAULT_PROD_DCR   USB_DCR_ENTRY(0x0302, USB_LANGID_EN_US, usbStrProd_en_us)
#define USB_DEFAULT_DQ_DCR     USB_DCR_ENTRY(0x0600, 0x0000, usbDevQualifier)

//needs special handling because it may not exist.
//we don't want to leave a trailing comma in that case. so we put it here
//instead of in the actual list.
#ifdef USB_SERIALNO
//why does the serial number have a language ID, anyway?
//are we expected to translate it?
#define USB_DEFAULT_SERIAL_DCR USB_DCR_ENTRY(0x0303, USB_LANGID_EN_US, usbStrSerialno)
#define USB_DEFAULT_SERIAL_DCR_COMMA USB_DEFAULT_SERIAL_DCR,
#else
#define USB_DEFAULT_SERIAL_DCR /* nothing */
#define USB_DEFAULT_SERIAL_DCR_COMMA /* nothing */
#endif

#define USB_DEFAULT_DESCRIPTORS  \
	USB_DEFAULT_DEV_DCR,         \
	USB_DEFAULT_LANGID_DCR,      \
	USB_DEFAULT_MFR_DCR,         \
	USB_DEFAULT_PROD_DCR,        \
	USB_DEFAULT_SERIAL_DCR_COMMA \
	USB_DEFAULT_DQ_DCR

#endif //_USB_DESCRIPTORS_H_
