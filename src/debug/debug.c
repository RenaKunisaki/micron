#include "../micron.h"

static const char *hex = "0123456789ABCDEF";
char debug_cmd_buf[256] = "";
static size_t buf_idx = 0;
static int redraw_prompt = 0;

void print_usb_buffers();
static void descriptor(const char *param);
static void list_descriptors(const char *param);
static void memdump(const char *param);
static void poke(const char *param);
static void reconnect(const char *param);
static void tx(const char *param);

static struct {
	const char *cmd;
	void(*func)(const char*);
} commands[] = {
	{"dcr",  descriptor},
	{"dcrs", list_descriptors},
	{"mem",  memdump},
	{"poke", poke},
	{"rc",   reconnect},
	{"tx",   tx},
	{NULL, NULL}
};

//read a hex value from string buffer.
//updates *param to point to the next char after the value.
uint32_t read_hex(const char **param) {
	const char *s = *param;
	uint32_t val = 0;
	if(*s == ' ') s++;
	char c;
	while((c = *s)) {
		if(c == ' ') break;
		if(c >= '0' && c <= '9') c -= '0';
		else if(c >= 'A' && c <= 'F') c -= ('A' - 10);
		else if(c >= 'a' && c <= 'f') c -= ('a' - 10);
		val = (val << 4) | c;
		s++;
	}
	*param = s;
	return val;
}


//print a hex value into given buffer.
//returns pointer to char after the value, which is set to null.
char* print_hex(char *buf, uint32_t num, int digits) {
	uint32_t shift = (digits - 1) * 4;
	for(int i=0; i<digits; i++) {
		*(buf++) = hex[(num >> shift) & 0xF];
		shift -= 4;
	}
	*buf = '\0';
	return buf;
}


//dump a memory region to console.
static void hexdump(const uint8_t *start, int len) {
	while(len > 0) {
		//print 16 cols at a time
		int cols = len;
		if(cols > 16) cols = 16;

		//print address
		char line[256];
		char *buf = print_hex(line, ((uint32_t)start) >> 16, 4);
		*(buf++) = ' ';
		buf = print_hex(buf, ((uint32_t)start) & 0xFFFF, 4);

		//read bytes and print their hex
		char byte[16];
		for(int i=0; i<cols; i++) {
			byte[i] = *start; //get byte
			*(buf++) = ' ';
			if(!(i & 3)) *(buf++) = ' '; //extra space
			buf = print_hex(buf, byte[i], 2);
			start++;
		}

		for(int i=cols; i<16; i++) {
			//pad remaining bytes
			*(buf++) = ' ';
			if(!(i & 3)) *(buf++) = ' '; //extra space
			*(buf++) = '.';
			*(buf++) = '.';
		}

		//add spaces and print bytes as text
		*(buf++) = ' ';
		*(buf++) = ' ';
		for(int i=0; i<cols; i++) {
			char c = byte[i];
			if(c < 0x20 || c > 0x7E) c = '.';
			*(buf++) = c;
			if((i & 3) == 3) *(buf++) = ' ';
		}
		*(buf++) = '\r';
		//*(buf++) = '\n';
		*buf     = '\0';
		puts(line);

		len -= cols;
	}
}


//disconnect and reconnect USB.
static void reconnect(const char *param) {
	USB0_CONTROL = 0;
	delayMS(2000);
	USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG;
}


//dump some memory to console.
static void memdump(const char *param) {
	uint32_t start = read_hex(&param);
	uint32_t len   = read_hex(&param);
	hexdump((const uint8_t*)start, len);
}


//write some bytes into memory.
//usage: poke 01234567 010203...
//spaces are allowed between bytes, but must be 2 digits
static void poke(const char *param) {
	uint8_t *ptr = (uint8_t*)read_hex(&param);
	char c;
	uint8_t val = 0;
	int chars = 0;
	while((c = *param)) {
		if(c == ' ') /* do nothing */ ;
		else {
			if     (c >= '0' && c <= '9') c -= '0';
			else if(c >= 'A' && c <= 'F') c -= ('A' - 10);
			else if(c >= 'a' && c <= 'f') c -= ('a' - 10);
			else break;
			val = (val << 4) | c;
			chars++;
		}

		if(chars == 2) {
			*(ptr++) = val;
			val = 0;
			chars = 0;
		}
		param++;
	}
}


static void tx(const char *param) { //send a USB packet
	uint8_t endp = (uint8_t)read_hex(&param);
	char c;
	uint8_t val = 0;
	int chars = 0;
	uint8_t buf[1024];
	size_t len = 0;


	while((c = *param)) {
		if(c == ' ') /* do nothing */ ;
		else if(c >= '0' && c <= '9') c -= '0';
		else if(c >= 'A' && c <= 'F') c -= ('A' - 10);
		else if(c >= 'a' && c <= 'f') c -= ('a' - 10);
		else break;
		val = (val << 4) | c;
		chars++;

		if(chars == 2) {
			buf[len++] = val;
			val = 0;
			chars = 0;
		}
		param++;
		if(len >= sizeof(buf)) break;
	}
	usbSend(buf, len, endp);
}


static void descriptor(const char *param) { //view USB descriptor
	uint32_t wValue = read_hex(&param);
	uint16_t wIndex;
	if(*param) wIndex = read_hex(&param);
	else {
		wIndex = wValue >> 16;
		wValue = wValue & 0xFFFF;
	}

	const usbDcrEntry_t *entry, *found=NULL;
	for(entry = usbDescriptors; entry->addr != NULL; entry++) {
		if(wValue == entry->wValue
		&& wIndex == entry->wIndex) {
			found = entry;
			break;
		}
	}

	printf("Descriptor %04lX:%04X", wValue, wIndex);
	if(!found) {
		puts(": not found");
		return;
	}

	printf(" @ 0x%08lX, len=%d: %s\r\n",
		(uint32_t)found->addr, found->length, found->name);

	//const uint8_t *data = (const uint8_t*)found->addr;
	//const uint8_t *dataEnd = data + found->length;
	hexdump((const uint8_t*)found->addr, found->length);
}


static void list_descriptors(const char *param) {
	const usbDcrEntry_t *entry;

	printf("usbDescriptors:\r\n"
		"╔════╦════╦════════╦═══╦════════════════════════╗\r\n"
		"║wVal║wIdx║Addr    ║Len║Name                    ║\r\n"
	);
	for(entry = usbDescriptors; entry->addr != NULL; entry++) {
		printf(
			"╟────╫────╫────────╫───╫────────────────────────╢\r\n"
			"║%04X║%04X║%08lX║%3zd║%-24s║\r\n",
			entry->wValue, entry->wIndex, (uint32_t)entry->addr,
			entry->length, entry->name);
	}
	printf("╚════╩════╩════════╩═══╩════════════════════════╝\r\n");
}


void print_usb_buffers() {
	//printing into a buffer and sending that to the TTY all at once
	//avoids the lag involved with a ton of small writes.
	static char text[8192];
	char *out = text;

	out += snprintf(out, MAX(0, out - text), "\r\n"
		"╔══╤═══╤════╦════════╤════════╤════╤══════╤═╦════════╤════════╤════╤══════╤═╦════════╤════════╦════════╦═══════╤═══════╗\r\n"
		"║Ep│DEP│PkSz║ TxBuf  │ TxDesc │Len │ODKNTS│P║ RxBuf  │ RxDesc │Len │ODKNTS│P║CurTxSrc│CurTxLen║HN-CRTXS║IN/SETP│OUT/SOF║\r\n");
	static const char *sep =
		"╟──┼───┼────╫────────┼────────┼────┼──────┼─╫────────┼────────┼────┼──────┼─╫────────┼────────╫────────╫───────┼───────╢\r\n";
	out += snprintf(out, MAX(0, out - text), "%s", sep);

	for(int endp=0; endp<3; endp++) {
		usbEndpCfg_t *cfg = &usbEndpCfg[endp];
		for(int buf=0; buf<2; buf++) {
			volatile usbBdt_t *tx = &usbBdt[USB_BDT_INDEX(endp, USB_TX, buf)];
			volatile usbBdt_t *rx = &usbBdt[USB_BDT_INDEX(endp, USB_RX, buf)];
			usbTx_t *curTx = cfg->tx;

			out += snprintf(out, MAX(0, out - text),
			//    Ep    D E P PkSz TxBuf TxDsc Len   O D K N T S  Pid RxBuf RxDsc Len   O D K N T S  Pid TxSrc  TxLn H N - C R T X S I/S  O/F
				"║%02X│%c%c%c│ %3d║%08lX│%08lX│%04lX│%c%c%c%c%c%c│%lX║%08lX│%08lX│%04lX│%c%c%c%c%c%c│%lX║%08lX│%08X║%c%c%c%c%c%c%c%c║%7d│%7d║\r\n",
				endp,
				cfg->txFlagData ? '1' : '0',
				(cfg->txFlagOdd == buf) ? '*' : ' ',
				cfg->tx ? '~' : ' ',
				cfg->maxPkSize,
				(uint32_t)tx->addr, tx->desc, USB_BDT_LEN(tx),
				(tx->desc & USB_BDT_OWN  ) ? 'O' : '-',
				(tx->desc & USB_BDT_DATA1) ? '1' : '0',
				(tx->desc & USB_BDT_KEEP ) ? 'K' : '-',
				(tx->desc & USB_BDT_NINC ) ? 'N' : '-',
				(tx->desc & USB_BDT_DTS  ) ? 'T' : '-',
				(tx->desc & USB_BDT_STALL) ? 'S' : '-',
				USB_BDT_PID(tx->desc),
				(uint32_t)rx->addr, rx->desc, USB_BDT_LEN(rx),
				(rx->desc & USB_BDT_OWN  ) ? 'O' : '-',
				(rx->desc & USB_BDT_DATA1) ? '1' : '0',
				(rx->desc & USB_BDT_KEEP ) ? 'K' : '-',
				(rx->desc & USB_BDT_NINC ) ? 'N' : '-',
				(rx->desc & USB_BDT_DTS  ) ? 'T' : '-',
				(rx->desc & USB_BDT_STALL) ? 'S' : '-',
				USB_BDT_PID(rx->desc),
				curTx ? (uint32_t)curTx->data : 0,
				curTx ? curTx->len : 0,
				(USB_ENDPT_REG(endp) & BIT(7)) ? 'H' : '-',  //HOSTWOHUB
				(USB_ENDPT_REG(endp) & BIT(6)) ? 'N' : '-',  //RETRYDIS
				(USB_ENDPT_REG(endp) & BIT(5)) ? '*' : '-',  //Reserved
				(USB_ENDPT_REG(endp) & BIT(4)) ? 'C' : '-',  //EPCTLDIS
				(USB_ENDPT_REG(endp) & BIT(3)) ? 'R' : '-',  //EPRXEN
				(USB_ENDPT_REG(endp) & BIT(2)) ? 'T' : '-',  //EPTXEN
				(USB_ENDPT_REG(endp) & BIT(1)) ? 'X' : '-',  //EPSTALL
				(USB_ENDPT_REG(endp) & BIT(0)) ? 'S' : '-',  //EPHSHK
#if USB_DEBUG_COUNT_PIDS
				(buf == 0 ? cfg->nIn  : cfg->nSetup),
				(buf == 0 ? cfg->nOut : cfg->nSOF  )
#else
				-1, -1
#endif
				);
			if(buf == 1 && endp < 2) {
				//puts(sep);
				out += snprintf(out, MAX(0, out - text), "%s", sep);
			}
		}
	}
	out += snprintf(out, MAX(0, out - text),
	//║Ep│DEP│PkSz║ TxBuf  │ TxDesc │Len │ODKNTS│P║ RxBuf  │ RxDesc │Len │ODKNTS│P║CurTxSrc│CurTxLen║HN-CRTXS║  I/S  │  O/F  ║
	 "╚══╧═══╧════╩════════╧════════╧════╧══════╧═╩════════╧════════╧════╧══════╧═╩════════╧════════╩════════╩═══════╧═══════╝\r\n");
	serialPuts(0, text);
}


const char* strcmp_cmd(const char *s1, const char *s2) {
	for(size_t j=0; j < sizeof(debug_cmd_buf); j++) {
		char c = s1[j];
		if(c == ' ') c = '\0';
		if(c != s2[j]) return NULL;
		if(c == '\0') return &s1[j];
	}
	return NULL;
}


void do_command() {
	for(int i=0; commands[i].cmd != NULL; i++) {
		const char *param = strcmp_cmd(debug_cmd_buf, commands[i].cmd);
		if(param) {
			commands[i].func(param);
			return;
		}
	}
	printf("Unknown command \"%s\"\r\n", debug_cmd_buf);
}


void check_debug_commands() {
	if(redraw_prompt) {
		//draw debug prompt and erase any stray text after it
		printf("\rdebug> %s      \b\b\b\b\b\b", debug_cmd_buf);
		redraw_prompt = 0;
	}

	int chr = serialGetchr(0);
	if(chr <= 0) return;
	redraw_prompt = 1;

	if(chr == 0x1B) { //Escape (or beginning of escape seq)
		uint32_t key = 0;
		uint32_t now = millis();
		while(1) {
			chr = serialGetchr(0);
			if(chr >= '0' && chr <= '9') key = (key * 10) + (chr - '0');
			else if(chr == '~') break;
			else if(chr == '[' || chr == 'O') /* do nothing */;
			else if(chr == 0) idle();
			else {
				//some F keys send \eOx where x is another letter
				key = (key << 7) | chr;
				break;
			}

			//I can't find any  better way to distinguish the Escape key.
			//This is all horrible. XXX
			if(millis() - now > 100) break;
		}
		chr = key | 0x8000;
	}

	if(chr >= 0x20 && chr <= 0x7E) { //ASCII printable
		debug_cmd_buf[buf_idx] = chr;
		debug_cmd_buf[buf_idx+1] = '\0';
		if(buf_idx < sizeof(debug_cmd_buf) - 1) buf_idx++;
	}
	else if(chr == '\r' || chr == '\n') { //Enter
		puts("\r");
		debug_cmd_buf[buf_idx] = '\0';
		if(buf_idx > 0) do_command();
		buf_idx = 0;
		debug_cmd_buf[buf_idx] = '\0';
	}
	else if(chr == '\b' || chr == 0x7F) { //Backspace
		if(buf_idx > 0) buf_idx--;
		debug_cmd_buf[buf_idx] = '\0';
	}
	else if(chr == 0x8003) { //Delete
		buf_idx = 0;
		debug_cmd_buf[buf_idx] = '\0';
	}
	else if(chr == 0x8000) { //Escape
		printf("Rebooting!\r\n");
		rebootIntoBootloader();
	}
	else if(chr == 0x8050) puts("\e[2J\e[14;1H"); //F1 - clear screen, cursor to 1,16
	else if(chr == 0x8018) print_usb_buffers(); //F12
	else {
		printf("key %02X\r\n", chr);
	}
}
