//most of this code copied from
//http://kevincuzner.com/2014/12/12/teensy-3-1-bare-metal-writing-a-usb-driver/
#ifndef _MICRON_USB_H_
#define _MICRON_USB_H_

#ifdef __cplusplus
	extern "C" {
#endif

//keep count of packet types per interface?
#define USB_DEBUG_COUNT_PIDS 0
#define USB_ENABLE_DPRINT 0

#if USB_ENABLE_DPRINT
#define USB_DPRINT(fmt, ...) do { \
	if(stderr) \
		fprintf(stderr, "[%08ld %s:%4u] " fmt, millis(), \
        __FILE__, __LINE__, \
        ##__VA_ARGS__); \
} while(0)
#else
#define USB_DPRINT(fmt, ...)
#endif

#define USB_ENDP0_SIZE 64 //XXX does it have to be this value?

#include "ch9.h"
#include "descriptors.h"
#include "registers.h"
#include "langid.h"
#include "bdt.h"
#include "cdc.h"
#include "hid.h"

//endpoint packet/request handler
typedef void (*usbEndpCallback)(
	int endp, volatile usbBdt_t *buffer, uint8_t stat);

//transmit complete callback
struct usbTx_t;
typedef void (*usbTxCompleteCallback)(int endp, usbTx_t *tx);

//manual transmit handler
typedef void (*usbTxHandler)(int endp, volatile usbBdt_t *buffer,
uint8_t stat);

//USB setup packet
typedef struct {
    union {
        struct {
            uint8_t bmRequestType;
            uint8_t bRequest;
        };
        uint16_t wRequestAndType; //note, bRequest is high byte
    };
    union {
		uint16_t wValue;
		struct {
			uint8_t wValueLo;
			uint8_t wValueHi;
		};
	};
    union {
		uint16_t wIndex;
		struct {
			uint8_t wIndexLo;
			uint8_t wIndexHi;
		};
	};
    uint16_t wLength;
} PACKED usbSetup_t;

typedef struct usbTx_t {
	usbTx_t *next; //next queued transmission
	uint16_t len;  //total length to transmit
	uint16_t pos;  //position in buffer to begin transmitting from
	usbTxCompleteCallback onComplete; //completion callback
	uint8_t shouldFree : 1;           //should we free data when done with it?
	const uint8_t *data;              //data to transmit
	uint8_t buf[];                    //data can be stored here
} usbTx_t;

typedef volatile struct { //endpoint config/status
	usbTx_t *tx;                //current transmission
    usbEndpCallback pkHandler;  //received packet handler callback
    usbTxHandler txHandler;     //manual transmit handler
    uint16_t maxPkSize;         //max packet size
	uint8_t  config;            //USB0_ENDPTn config value
	uint8_t *rxbuf[2];          //receive buffers
	uint8_t  txFlagOdd     : 1; //using odd tx buffer
	uint8_t  txFlagData    : 1; //using DATA1 tx token
	uint8_t  shouldFreeRx0 : 1; //should we free rxbuf[0] when done with it?
	uint8_t  shouldFreeRx1 : 1; //should we free rxbuf[1] when done with it?

	//debug
#if USB_DEBUG_COUNT_PIDS
	uint32_t nIn, nOut, nSetup, nSOF;
#endif
} PACKED usbEndpCfg_t;


//usb.c
extern const    char        *usbPidNames[];
extern volatile uint8_t      usbAddress;   //our device address
extern volatile uint8_t      usbConfigID;  //our selected configuration
int  usbInit();
int  usbConnect(USBSpeed speed);
int  usbConfigureEndpoint(uint8_t endp, uint8_t size, uint8_t config,
usbEndpCallback pkHandler);
int  usbSetTxHandler(uint8_t endp, usbTxHandler handler);

//descriptors.c
extern const usbDevDcr_t       usbDevDescriptor;
extern const usbDevQualifier_t usbDevQualifier;
extern const void           *p_usbCfgDescriptor;
extern const uint16_t          usbCfgDescriptorLen;
extern const usbDcrEntry_t     usbDefaultDescriptors[];
extern const usbDcrEntry_t     usbDescriptors[];

//internal.c
extern volatile usbBdt_t     usbBdt[USB_MAX_ENDPOINTS * 4];
extern          usbEndpCfg_t usbEndpCfg[USB_MAX_ENDPOINTS];
void _usbInternalReset();
const usbDcrEntry_t* _usbFindDescriptor(const usbDcrEntry_t *list,
uint16_t wValue, uint16_t wIndex);
void _usbFinishTx(int endp);
usbTx_t* _usbPrepareTx(const void *data, int length, int *err);
int  _usbQueueTx(usbTx_t *tx, int endp);
void _usbContinueTx(int endp);

//setup.c
int  usbHandleSetup(usbSetup_t* packet, const void** data);
int  usbDefaultHandleSetup(usbSetup_t* packet, const void** data);
void usbHandleEndp0(int endp, volatile usbBdt_t *buffer, uint8_t stat);
void usbDefaultHandleEndp0(int endp, volatile usbBdt_t *buffer, uint8_t stat);
WEAK COLD int usbHandleMagicReboot(void);

//tx.c
void usbStall(int endp);
int  usbSend(const void *data, int length, int endp);
int  usbTrySend(const void *data, int length, int endp);
int  usbSendStr(const char *str, int endp);
int  usbSendCallback(const void *data, int length, int endp,
usbTxCompleteCallback cb);
size_t usbTxQueueLen(int endp, int *count);
int  usbTxInProgress(int endp);
int  usbCancelTx(int endp);


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_USB_H_
