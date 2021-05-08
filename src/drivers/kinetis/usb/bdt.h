//USB buffer descriptor tables.
//See see K20P64M72SF1RM.pdf § 41.4.1.
#ifndef _USB_BDT_H_
#define _USB_BDT_H_

/** Flags for USB BDT entry `desc` field.
 *  These are defined by the USB module.
 */
#define USB_BDT_OWN		BIT(7) //USB module owns the buffer
#define USB_BDT_DATA1	BIT(6) //send DATA1 field
#define USB_BDT_DATA0	0      //send DATA0 field
#define USB_BDT_KEEP	BIT(5) //USB module does not release buffer ownership
#define USB_BDT_NINC	BIT(4) //do not increment address (for FIFOs)
#define USB_BDT_DTS		BIT(3) //perform Data Toggle Synchronization.
#define USB_BDT_STALL	BIT(2) //stall if trying to receive into this buffer.
#define USB_BDT_RESERVED (BIT(1) | BIT(0)) //must be zero

//Bits 2-5 are STALL/DTS/NINC/KEEP flags when we give the buffer to the USB
//module. it writes the token PID here before giving the buffer back.
#define USB_BDT_PID(n)	(((n) >> 2) & 15)

//Extract length from BDT desc.
#define USB_BDT_LEN(buf) (((buf)->desc >> 16) & 0x3FF)

/** Compute BDT entry number.
 *  endp: Which endpoint.
 *  tx:   Whether you want tx or rx buffer.
 *  odd:  Whether you want odd or even buffer.
 */
#define USB_BDT_INDEX(endp, tx, odd) \
	(((endp) << 2)  | \
	((tx)  ? 2 : 0) | \
	((odd) ? 1 : 0))

//Build BDT desc field from packet length and data0 flag.
#define USB_BDT_DESC(count, data) ( \
	USB_BDT_OWN | USB_BDT_DTS \
	| ((data) ? USB_BDT_DATA1 : USB_BDT_DATA0) \
	| ((count) << 16) )

#define USB_TX    1
#define USB_RX    0
#define USB_ODD   1
#define USB_EVEN  0
#define USB_DATA0 0
#define USB_DATA1 1

//The actual buffer descriptor table entry, as defined by the USB module.
typedef struct {
	uint32_t desc; //buffer descriptor
	void* addr;    //buffer address
} usbBdt_t;

#endif //_USB_BDT_H_
