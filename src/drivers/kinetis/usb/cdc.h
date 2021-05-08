//USB Communications Device Class
#ifndef _USB_CDC_H_
#define _USB_CDC_H_

/** Most of following is copied and reformatted from USB CDC specifications
 *  documents stated in the comments.
 *  Note that the page and table numbers may differ between revisions
 *  of those documents.
 */


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 1: Data Class Protocol Wrapper Layout
 */
typedef struct {
	uint16_t wLength;      //Size of wrapper in bytes
	uint8_t  bDstProtocol; //Destination protocol ID.
	uint8_t  bSrcProtocol; //Source protocol ID.
	uint8_t  bData[];      //Protocol data.
} PACKED usbcdc_protocol_wrapper_t;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 2: Communications Device Class Code, and
 *  Table 3: Communications Interface Class Code
 */
//#define USB_CLASS_COMM 2 //already covered in ch9.h


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 4: Communications Class Subclass Codes
 */
typedef enum {
	//0x00: RESERVED
	USBCDC_DIRECT_LINE_CONTROL =      0x01, //Direct Line Control Model
	USBCDC_ABSTRACT_CONTROL =         0x02, //Abstract Control Model
	USBCDC_TELEPHONE_CONTROL =        0x03, //Telephone Control Model
	USBCDC_MULTI_CHANNEL_CONTROL =    0x04, //Multi-Channel Control Model
	USBCDC_CAPI_CONTROL =             0x05, //CAPI Control Model
	USBCDC_ETHERNET_CONTROL =         0x06, //Ethernet Networking Control Model
	USBCDC_ATM_CONTROL =              0x07, //ATM Networking Control Model
	USBCDC_WIRELESS_HANDSET_CONTROL = 0x08, //Wireless Handset Control Model
	USBCDC_DEVICE_MANAGEMENT =        0x09, //Device Management
	USBCDC_MOBILE_DIRECT_LINE =       0x0A, //Mobile Direct Line Model
	USBCDC_OBEX =                     0x0B, //OBEX
	USBCDC_ETHERNET_EMULATION =       0x0C, //Ethernet Emulation Model
	USBCDC_NETWORK_CONTROL =          0x0D, //Network Control Model
	//RESERVED (future use) =         0x0E-0x7F
	//RESERVED (vendor specific) =    0x80-0xFF
} USBCDC_SUBCLASS;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 5: Communications Class Protocol Codes
 */
typedef enum {
	USBCDC_PROTOCOL_NONE =                0x00, //No class specific protocol required
	USBCDC_PROTOCOL_AT_V250 =             0x01, //ITU-T V.250  AT Commands: V.250 etc
	USBCDC_PROTOCOL_AT_PCC1_101 =         0x02, //PCCA-101     AT Commands defined by PCCA-101
	USBCDC_PROTOCOL_PCCA_101_ANNEX_O =    0x03, //PCCA-101     AT Commands defined by PCCA-101 & Annex O
	USBCDC_PROTOCOL_GSM_707 =             0x04, //GSM 7.07     AT Commands defined by GSM 07.07
	USBCDC_PROTOCOL_3GPP_2707 =           0x05, //3GPP 27.07   AT Commands defined by 3GPP 27.007
	USBCDC_PROTOCOL_C_S10017_0 =          0x06, //C-S0017-0    AT Commands defined by TIA for CDMA
	USBCDC_PROTOCOL_ETHERNET_EMULATION0 = 0x07, //USB EEM      Ethernet Emulation Model
	//RESERVED (future use) =             0x08-0xFD
	USBCDC_PROTOCOL_EXTERNAL =            0xFE, //External Protocol: Commands defined by Command Set Functional Descriptor
	USBCDC_PROTOCOL_VENDOR_SPECIFIC =     0xFF,
} USBCDC_CLASS_PROTOCOL;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 6: Data Interface Class Code
 */
//#define USB_CLASS_CDC_DATA 0x0a //already covered in ch9.h

/** Data Interface Class SubClass Codes:
 *  At this time this field is un-used for Data Class interfaces and should have
 *  a value of 00h.
 */


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 7: Data Interface Class Protocol Codes
 */
typedef enum {
	USBCDC_IPROTO_NONE                = 0x00, //No class specific protocol required
	USBCDC_IPROTO_NETWORK_XFER        = 0x01, //Network Transfer Block (USBNCM1.0)
	USBCDC_IPROTO_ISDN_PHYSICAL       = 0x30, //Physical interface protocol for ISDN BRI (I.430)
	USBCDC_IPROTO_HDLC                = 0x31, //HDLC (ISO/IEC 3309-1993)
	USBCDC_IPROTO_TRANSPARENT         = 0x32, //Transparent
	USBCDC_IPROTO_Q921_MGMT           = 0x50, //Management protocol for Q.921 data link protocol (Q.921M)
	USBCDC_IPROTO_Q931_DATA_LINK      = 0x51, //Data link protocol for Q.931 (Q.921)
	USBCDC_IPROTO_Q921_TEI_MX         = 0x52, //TEI-multiplexor for Q.921 data link protocol (Q921TM)
	USBCDC_IPROTO_V42BIS_COMPRESSION  = 0x90, //Data compression procedures (V.42bis)
	USBCDC_IPROTO_EURO_ISDN           = 0x91, //Euro-ISDN protocol control (Q.931/Euro-ISDN)
	USBCDC_IPROTO_V24_ISDN_RATE_ADAPT = 0x92, //V.24 rate adaptation to ISDN (V.120)
	USBCDC_IPROTO_CAPI                = 0x93, //CAPI Commands (CAPI2.0)
	USBCDC_IPROTO_HOST_BASED          = 0xFD, //Host based driver. Note: This protocol code should only be used in messages between host and device to identify the host driver portion of a protocol stack.
	USBCDC_IPROTO_EXTERNAL            = 0xFE, //The protocol(s) are described using a Protocol Unit Functional Descriptors on Communications Class Interface. (CDC Specification)
	USBCDC_IPROTO_VENDOR_SPECIFIC     = 0xFF, //Vendor-specific (USB Specification)
} USBCDC_IFACE_PROTOCOL;


/* Universal Serial Bus Class Definitions for Communications Devices,
revision 1.2 (Errata 1), Nov 3 2010,
Table 8: Communications Device Class Descriptor Requirements
Field               Value     Description
bDeviceClass        02h       Communications Device Class code as defined in Table 2
bDeviceSubClass     00h       Communications Device Subclass code, unused at this time.
bDeviceProtocol     00h       Communications Device Protocol code, unused at this time.

Table 9: Communications Class Interface Descriptor Requirements
Field               Value     Description
bInterfaceClass     Class     Communications Interface Class code as defined in Table 3.
bInterfaceSubClass  Subclass  Communications Interface Subclass code as defined in Table 4.
bInterfaceProtocol  Protocol  Communications Interface Class Protocol code which applies to the subclass, as specified in the previous field, is defined in Table 5.

Table 10: Data Class Interface Descriptor Requirements
Field               Value     Description
bInterfaceClass     0Ah       Data Interface Class code as defined in Table 6.
bInterfaceSubClass  00h       Data Interface Subclass code.
bInterfaceProtocol  Protocol  Data Class Protocol code, which applies to the subclass, as specified in the previous field, is defined in Table 7.
*/


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 11: Functional Descriptor General Format
 */
typedef struct {
	uint8_t  bFunctionLength;    //Size of this descriptor.
	uint8_t  bDescriptorType;    //CS_INTERFACE, as defined in Table 12.
	uint8_t  bDescriptorSubtype; //Identifier (ID) of functional descriptor. For a list of the supported values, see Table 13
	uint8_t  bData[];            //function specific data byte. These fields will vary depending on the functional descriptor being represented.
} PACKED usbcdc_func_dcr_t;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 12: Type Values for the bDescriptorType Field
 */
#define USBCDC_CS_INTERFACE 0x24
#define USBCDC_CS_ENDPOINT  0x25


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 13: bDescriptor SubType in Communications Class Functional Descriptors
 */
typedef enum {
	USBCDC_DT_HEADER                   = 0x00, //Header Functional Descriptor, which marks the beginning of the concatenated set of functional descriptors for the interface.
	USBCDC_DT_CALL_MGMT                = 0x01, //Call Management Functional Descriptor.
	USBCDC_DT_ABSTRACT_CONTROL_MGMT    = 0x02, //Abstract Control Management Functional Descriptor.
	USBCDC_DT_DIRECT_LINE_MGMT         = 0x03, //Direct Line Management Functional Descriptor.
	USBCDC_DT_RINGER                   = 0x04, //Telephone Ringer Functional Descriptor.
	USBCDC_DT_CALL_LINE_STATE          = 0x05, //Telephone Call and Line State Reporting Capabilities Functional Descriptor.
	USBCDC_DT_UNION                    = 0x06, //Union Functional Descriptor
	USBCDC_DT_COUNTRY_SELECTION        = 0x07, //Country Selection Functional Descriptor
	USBCDC_DT_TELEPHONE_MODES          = 0x08, //Telephone Operational Modes Functional Descriptor
	USBCDC_DT_USB_TERMINAL             = 0x09, //USB Terminal Functional Descriptor
	USBCDC_DT_NETWORK_CHANNEL_TERMINAL = 0x0A, //Network Channel Terminal Descriptor
	USBCDC_DT_PROTOCOL_UNIT            = 0x0B, //Protocol Unit Functional Descriptor
	USBCDC_DT_EXTENSION_UNIT           = 0x0C, //Extension Unit Functional Descriptor
	USBCDC_DT_MULTICHANNEL_MGMT        = 0x0D, //Multi-Channel Management Functional Descriptor
	USBCDC_DT_CAPI_CONTROL_MGMT        = 0x0E, //CAPI Control Management Functional Descriptor
	USBCDC_DT_ETHERNET                 = 0x0F, //Ethernet Networking Functional Descriptor
	USBCDC_DT_ATM                      = 0x10, //ATM Networking Functional Descriptor
	USBCDC_DT_WIRELESS_HANDSET         = 0x11, //Wireless Handset Control Model Functional Descriptor
	USBCDC_DT_MOBILE_DIRECT_LINE       = 0x12, //Mobile Direct Line Model Functional Descriptor
	USBCDC_DT_MDLM_DETAIL              = 0x13, //MDLM Detail Functional Descriptor
	USBCDC_DT_DEVICE_MGMT              = 0x14, //Device Management Model Functional Descriptor
	USBCDC_DT_OBEX                     = 0x15, //OBEX Functional Descriptor
	USBCDC_DT_CMD_SET                  = 0x16, //Command Set Functional Descriptor
	USBCDC_DT_CMD_SET_DETAIL           = 0x17, //Command Set Detail Functional Descriptor
	USBCDC_DT_TELEPHONE_CONTROL        = 0x18, //Telephone Control Model Functional Descriptor
	USBCDC_DT_OBEX_SERVICE_ID          = 0x19, //OBEX Service Identifier Functional Descriptor
	USBCDC_DT_NCM                      = 0x1A, //NCM Functional Descriptor
	//0x1B-0x7F: RESERVED (future use)
	//0x80-0xFE: RESERVED (vendor specific)
} USBCDC_COMM_DT;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 14: bDescriptor SubType in Data Class Functional Descriptors
 */
typedef enum {
	USBCDC_DATA_DT_HEADER = 0x00, //Header Functional Descriptor, which marks the beginning of the concatenated set of functional descriptors for the interface.
	//0x01-0x7F: RESERVED (future use)
	//0x80-0xFE: RESERVED (vendor specific)
} USBCDC_DATA_DT;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 15: Class-Specific Descriptor Header Format
 */
typedef struct {
	uint8_t  bFunctionLength;    //Size of this descriptor in bytes.
	uint8_t  bDescriptorType;    //CS_INTERFACE descriptor type.
	uint8_t  bDescriptorSubtype; //Header functional descriptor subtype as defined in Table 13.
	uint16_t bcdCDC;             //USB Class Definitions for Communications Devices Specification release number in binary-coded decimal.
} PACKED usbcdc_class_dcr_header_t;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 16: Union Interface Functional Descriptor
 */
typedef struct {
	uint8_t  bFunctionLength;         //Size of this functional descriptor, in bytes.
	uint8_t  bDescriptorType;         //CS_INTERFACE
	uint8_t  bDescriptorSubtype;      //Union Functional Descriptor SubType as defined in Table 13.
	uint8_t  bControlInterface;       //The interface number of the Communications or Data Class interface, designated as the controlling interface for the union.*
	//uint8_t  bSubordinateInterface[]; //Interface numbers of subordinate interfaces in the union. *
    // * Zero based index of the interface in this configuration (bInterfaceNum)*
	//It's easier if we leave this out of the struct, and define our own struct
	//containing a usbcdc_union_iface_func_dcr_t followed by a uint8_t[].
	//Otherwise it's difficult to initialize, since gcc wants to know the size
	//before it gets to the actual elements.
} PACKED usbcdc_union_iface_func_dcr_t;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 17: Country Selection Functional Descriptor
 */
typedef struct {
	uint8_t  bFunctionLength;     //Size of this functional descriptor, in bytes.
	uint8_t  bDescriptorType;     //CS_INTERFACE
	uint8_t  bDescriptorSubtype;  //Country Selection Functional Descriptor Subtype as defined in Table 13.
	uint8_t  iCountryCodeRelDate; //Index of a string giving the release date for the implemented ISO 3166 Country Codes. Date shall be presented as ddmmyyyy with dd=day, mm=month, and yyyy=year.
	//uint16_t wCountryCode[];    //Country code in the format as defined in [ISO3166], release date as specified in offset 3 for the first supported country.
} PACKED usbcdc_country_sel_func_dcr_t;


/* Universal Serial Bus Class Definitions for Communications Devices,
revision 1.2 (Errata 1), Nov 3 2010,
Table 18: Sample Communications Class Specific Interface Descriptor*
   * This descriptor is specific to the Communications Class.
Field                 Value  Description
bFunctionLength         05h  Size of this functional descriptor, in bytes.
bDescriptorType         24h  CS_INTERFACE
bDescriptorSubtype      00h  Header. This is defined in Table 13, which defines this as a header.
bcdCDC                0110h  USB Class Definitions for Communications Devices Specification release number in binary-coded decimal.
bFunctionLength         04h  Size of this functional descriptor, in bytes.
bDescriptorType         24h  CS_INTERFACE
bDescriptorSubtype      02h  Abstract Control Management functional descriptor subtype as defined in Table 13.
bmCapabilities          0Fh  This field contains the value 0Fh, because the device supports all the corresponding commands for the Abstract Control Model interface.
bFunctionLength         05h  Size of this functional descriptor, in bytes
bDescriptorType         24h  CS_INTERFACE
bDescriptorSubtype      06h  Union Descriptor Functional Descriptor subtype as defined in Table 13.
bControlInterface       00h  Interface number of the control (Communications Class) interface
bSubordinateInterface0  01h  Interface number of the subordinate (Data Class) interface
bFunctionLength         05h  Size of this functional descriptor, in bytes
bDescriptorType         24h  CS_INTERFACE
bDescriptorSubtype      01h  Call Management Functional Descriptor subtype as defined in Table 13.
bmCapabilities          03h  Indicate that the device handles call management itself (bit D0 is set), and will process commands multiplexed over the data interface in addition to commands sent using SendEncapsulatedCommand (bit D1 is set).
bDataInterface          01h  Indicates that multiplexed commands are handled via data interface 01h (same value as used in the UNION Functional Descriptor)
*/


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 19: Class-Specific Request Codes
 */
typedef enum {
	USBCDC_REQ_SEND_ENCAPSULATED_COMMAND =                    0x00,
	USBCDC_REQ_GET_ENCAPSULATED_RESPONSE =                    0x01,
	USBCDC_REQ_SET_COMM_FEATURE =                             0x02,
	USBCDC_REQ_GET_COMM_FEATURE =                             0x03,
	USBCDC_REQ_CLEAR_COMM_FEATURE =                           0x04,
	USBCDC_REQ_SET_AUX_LINE_STATE =                           0x10,
	USBCDC_REQ_SET_HOOK_STATE =                               0x11,
	USBCDC_REQ_PULSE_SETUP =                                  0x12,
	USBCDC_REQ_SEND_PULSE =                                   0x13,
	USBCDC_REQ_SET_PULSE_TIME =                               0x14,
	USBCDC_REQ_RING_AUX_JACK =                                0x15,
	USBCDC_REQ_SET_LINE_CODING =                              0x20,
	USBCDC_REQ_GET_LINE_CODING =                              0x21,
	USBCDC_REQ_SET_CONTROL_LINE_STATE =                       0x22,
	USBCDC_REQ_SEND_BREAK =                                   0x23,
	USBCDC_REQ_SET_RINGER_PARMS =                             0x30,
	USBCDC_REQ_GET_RINGER_PARMS =                             0x31,
	USBCDC_REQ_SET_OPERATION_PARMS =                          0x32,
	USBCDC_REQ_GET_OPERATION_PARMS =                          0x33,
	USBCDC_REQ_SET_LINE_PARMS =                               0x34,
	USBCDC_REQ_GET_LINE_PARMS =                               0x35,
	USBCDC_REQ_DIAL_DIGITS =                                  0x36,
	USBCDC_REQ_SET_UNIT_PARAMETER =                           0x37,
	USBCDC_REQ_GET_UNIT_PARAMETER =                           0x38,
	USBCDC_REQ_CLEAR_UNIT_PARAMETER =                         0x39,
	USBCDC_REQ_GET_PROFILE =                                  0x3A,
	USBCDC_REQ_SET_ETHERNET_MULTICAST_FILTERS =               0x40,
	USBCDC_REQ_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x41,
	USBCDC_REQ_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x42,
	USBCDC_REQ_SET_ETHERNET_PACKET_FILTER =                   0x43,
	USBCDC_REQ_GET_ETHERNET_STATISTIC =                       0x44,
	USBCDC_REQ_SET_ATM_DATA_FORMAT =                          0x50,
	USBCDC_REQ_GET_ATM_DEVICE_STATISTICS =                    0x51,
	USBCDC_REQ_SET_ATM_DEFAULT_VC =                           0x52,
	USBCDC_REQ_GET_ATM_VC_STATISTICS =                        0x53,
	USBCDC_REQ_MDLM_MODEL_SPECIFIC =                          0x60, //to 0x7F
	USBCDC_REQ_GET_NTB_PARAMETERS =                           0x80,
	USBCDC_REQ_GET_NET_ADDRESS =                              0x81,
	USBCDC_REQ_SET_NET_ADDRESS =                              0x82,
	USBCDC_REQ_GET_NTB_FORMAT =                               0x83,
	USBCDC_REQ_SET_NTB_FORMAT =                               0x84,
	USBCDC_REQ_GET_NTB_INPUT_SIZE =                           0x85,
	USBCDC_REQ_SET_NTB_INPUT_SIZE =                           0x86,
	USBCDC_REQ_GET_MAX_DATAGRAM_SIZE =                        0x87,
	USBCDC_REQ_SET_MAX_DATAGRAM_SIZE =                        0x88,
	USBCDC_REQ_GET_CRC_MODE =                                 0x89,
	USBCDC_REQ_SET_CRC_MODE =                                 0x8A,
} USBCDC_REQUEST_CODE;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 20: Class-Specific Notification Codes
 */
typedef enum {
	USBCDC_NOTIF_NETWORK_CONNECTION =      0x00,
	USBCDC_NOTIF_RESPONSE_AVAILABLE =      0x01,
	USBCDC_NOTIF_AUX_JACK_HOOK_STATE =     0x08,
	USBCDC_NOTIF_RING_DETECT =             0x09,
	USBCDC_NOTIF_SERIAL_STATE =            0x20,
	USBCDC_NOTIF_CALL_STATE_CHANGE =       0x28,
	USBCDC_NOTIF_LINE_STATE_CHANGE =       0x29,
	USBCDC_NOTIF_CONNECTION_SPEED_CHANGE = 0x2A,
	USBCDC_NOTIF_MDML_MODEL_SPECIFIC =     0x40, //to 0x5F
} USBCDC_NOTIFICATION_CODE;


/** Universal Serial Bus Class Definitions for Communications Devices,
 *  revision 1.2 (Errata 1), Nov 3 2010,
 *  Table 21: ConnectionSpeedChange Data Structure
 */
typedef struct {
	uint32_t DLBitRate; //Contains the downlink bit rate, in bits per second, as sent on the IN pipe
	uint32_t ULBitRate; //Contains the uplink bit rate, in bits per second, as sent on the OUT pipe
} PACKED usbcdc_ConnectionSpeedChange;
//Note that uplink/downlink is the 3gpp definition which is not the same as the
//upstream/downstream in the USB2.0 specification.


/** USB Class Definitions for Communication Devices, v1.1, Jan 19 1999,
 *  Table 27: Call Management Functional Descriptor
 */
typedef struct {
	uint8_t  bFunctionLength; //Size of this functional descriptor, in bytes.
	uint8_t  bDescriptorType; //CS_INTERFACE
	uint8_t  bDescriptorSubtype; //Call Management functional descriptor subtype, as defined in Table 25.
	uint8_t  bmCapabilities; //The capabilities that this configuration supports (see enum below)
	uint8_t  bDataInterface; //Interface number of Data Class interface optionally used for call management. *
	// * Zero based index of the interface in this configuration. (bInterfaceNum)
} PACKED usbcdc_callmgmt_func_dcr_t;

typedef enum {
	//D7..D2: RESERVED (Reset to zero)

	//D1: 0 - Device sends/receives call management information only over the
	//        Communication Class interface.
	//    1 - Device can send/receive call management information over a
	//        Data Class interface.
	USBCDC_CALLMGMT_USE_DATA_IFACE = BIT(1),

	//D0: 0 - Device does not handle call management itself.
	//    1 - Device handles call management itself.
	USBCDC_CALLMGMT_SELF_HANDLED = BIT(0),

	//The previous bits, in combination, identify which call management scenario
	//is used. If bit D0 is reset to 0, then the value of bit D1 is ignored.
	//In this case, bit D1 is reset to zero for future compatibility.
} USBCDC_CALLMGMT_CAPABILITIES;


/** USB Class Definitions for Communication Devices, v1.1, Jan 19 1999,
 *  Table 28: Abstract Control Management Functional Descriptor
 */
typedef struct {
	uint8_t  bFunctionLength; //Size of this functional descriptor, in bytes.
	uint8_t  bDescriptorType; //CS_INTERFACE
	uint8_t  bDescriptorSubtype; //Abstract Control Management functional descriptor subtype as defined in Table 25.
	uint8_t  bmCapabilities; //The capabilities that this configuration supports. (see enum below)
} PACKED usbcdc_acm_func_dcr_t;

typedef enum {
	//D7..D4: RESERVED (Reset to zero)

	//D3: 1 - Device supports the notification Network_Connection.
	USBCDC_ACM_SUPPORT_NETWORK_CONNECTION = BIT(3),

	//D2: 1 - Device supports the request Send_Break
	USBCDC_ACM_SUPPORT_SEND_BREAK = BIT(2),

	//D1: 1 - Device supports the request combination of Set_Line_Coding,
	//Set_Control_Line_State, Get_Line_Coding, and the notification
	//Serial_State.
	USBCDC_ACM_SUPPORT_LINE_CODING = BIT(1),

	//D0: 1 - Device supports the request combination of Set_Comm_Feature,
	//Clear_Comm_Feature, and Get_Comm_Feature.
	USBCDC_ACM_SUPPORT_COMM_FEATURE = BIT(0),

	//The previous bits, in combination, identify which requests/notifications
	//are supported by a Communication Class interface with the SubClass code of
	//Abstract Control Model.
	//(A bit value of zero means that the request is not supported.)
} USBCDC_ACM_CAPABILITIES;

#endif //_USB_CDC_H_
