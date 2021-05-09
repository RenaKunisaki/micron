#ifndef _MICRON_ISR_H_
#define _MICRON_ISR_H_

#ifdef __cplusplus
	extern "C" {
#endif

//Interrupt numbers - not the same as IRQ_NUMBER_t in kinetis.h
typedef enum {
	INT_Initial_Stack_Pointer    = 0,  //Initial stack pointer
	INT_Initial_Program_Counter  = 1,  //Initial program counter
	INT_NMI                      = 2,  //Non-maskable interrupt
	INT_Hard_Fault               = 3,  //Hard fault exception
	INT_Mem_Manage_Fault         = 4,  //Memory Manage Fault
	INT_Bus_Fault                = 5,  //Bus fault exception
	INT_Usage_Fault              = 6,  //Usage fault exception
	INT_Reserved7                = 7,  //Reserved interrupt 7
	INT_Reserved8                = 8,  //Reserved interrupt 8
	INT_Reserved9                = 9,  //Reserved interrupt 9
	INT_Reserved10               = 10, //Reserved interrupt 10
	INT_SVCall                   = 11, //A supervisor call exception
	INT_DebugMonitor             = 12, //Debug Monitor
	INT_Reserved13               = 13, //Reserved interrupt 13
	INT_PendableSrvReq           = 14, //PendSV exception - request for system level service
	INT_SysTick                  = 15, //SysTick Interrupt
	INT_DMA0                     = 16, //DMA Channel 0 Transfer Complete
	INT_DMA1                     = 17, //DMA Channel 1 Transfer Complete
	INT_DMA2                     = 18, //DMA Channel 2 Transfer Complete
	INT_DMA3                     = 19, //DMA Channel 3 Transfer Complete
	INT_DMA4                     = 20, //DMA Channel 4 Transfer Complete
	INT_DMA5                     = 21, //DMA Channel 5 Transfer Complete
	INT_DMA6                     = 22, //DMA Channel 6 Transfer Complete
	INT_DMA7                     = 23, //DMA Channel 7 Transfer Complete
	INT_DMA8                     = 24, //DMA Channel 8 Transfer Complete
	INT_DMA9                     = 25, //DMA Channel 9 Transfer Complete
	INT_DMA10                    = 26, //DMA Channel 10 Transfer Complete
	INT_DMA11                    = 27, //DMA Channel 11 Transfer Complete
	INT_DMA12                    = 28, //DMA Channel 12 Transfer Complete
	INT_DMA13                    = 29, //DMA Channel 13 Transfer Complete
	INT_DMA14                    = 30, //DMA Channel 14 Transfer Complete
	INT_DMA15                    = 31, //DMA Channel 15 Transfer Complete
	INT_DMA_Error                = 32, //DMA Error Interrupt
	INT_MCM                      = 33, //Normal interrupt
	INT_FTFL                     = 34, //FTFL Interrupt
	INT_Read_Collision           = 35, //Read Collision Interrupt
	INT_LVD_LVW                  = 36, //Low Voltage Detect, Low Voltage Warning
	INT_LLW                      = 37, //Low Leakage Wakeup
	INT_Watchdog                 = 38, //WDOG Interrupt
	INT_Reserved39               = 39, //Reserved Interrupt 39
	INT_I2C0                     = 40, //I2C0 interrupt
	INT_I2C1                     = 41, //I2C1 interrupt
	INT_SPI0                     = 42, //SPI0 Interrupt
	INT_SPI1                     = 43, //SPI1 Interrupt
	INT_Reserved44               = 44, //Reserved interrupt 44
	INT_CAN0_ORed_Message_buffer = 45, //CAN0 OR'd Message Buffers Interrupt
	INT_CAN0_Bus_Off             = 46, //CAN0 Bus Off Interrupt
	INT_CAN0_Error               = 47, //CAN0 Error Interrupt
	INT_CAN0_Tx_Warning          = 48, //CAN0 Tx Warning Interrupt
	INT_CAN0_Rx_Warning          = 49, //CAN0 Rx Warning Interrupt
	INT_CAN0_Wake_Up             = 50, //CAN0 Wake Up Interrupt
	INT_I2S0_Tx                  = 51, //I2S0 transmit interrupt
	INT_I2S0_Rx                  = 52, //I2S0 receive interrupt
	INT_Reserved53               = 53, //Reserved interrupt 53
	INT_Reserved54               = 54, //Reserved interrupt 54
	INT_Reserved55               = 55, //Reserved interrupt 55
	INT_Reserved56               = 56, //Reserved interrupt 56
	INT_Reserved57               = 57, //Reserved interrupt 57
	INT_Reserved58               = 58, //Reserved interrupt 58
	INT_Reserved59               = 59, //Reserved interrupt 59
	INT_UART0_LON                = 60, //UART0 LON interrupt
	INT_UART0_RX_TX              = 61, //UART0 Receive/Transmit interrupt
	INT_UART0_ERR                = 62, //UART0 Error interrupt
	INT_UART1_RX_TX              = 63, //UART1 Receive/Transmit interrupt
	INT_UART1_ERR                = 64, //UART1 Error interrupt
	INT_UART2_RX_TX              = 65, //UART2 Receive/Transmit interrupt
	INT_UART2_ERR                = 66, //UART2 Error interrupt
	INT_UART3_RX_TX              = 67, //UART3 Receive/Transmit interrupt
	INT_UART3_ERR                = 68, //UART3 Error interrupt
	INT_UART4_RX_TX              = 69, //UART4 Receive/Transmit interrupt
	INT_UART4_ERR                = 70, //UART4 Error interrupt
	INT_Reserved71               = 71, //Reserved interrupt 71
	INT_Reserved72               = 72, //Reserved interrupt 72
	INT_ADC0                     = 73, //ADC0 interrupt
	INT_ADC1                     = 74, //ADC1 interrupt
	INT_CMP0                     = 75, //CMP0 interrupt
	INT_CMP1                     = 76, //CMP1 interrupt
	INT_CMP2                     = 77, //CMP2 interrupt
	INT_FTM0                     = 78, //FTM0 fault, overflow and channels interrupt
	INT_FTM1                     = 79, //FTM1 fault, overflow and channels interrupt
	INT_FTM2                     = 80, //FTM2 fault, overflow and channels interrupt
	INT_CMT                      = 81, //CMT interrupt
	INT_RTC                      = 82, //RTC interrupt
	INT_RTC_Seconds              = 83, //RTC seconds interrupt
	INT_PIT0                     = 84, //PIT timer channel 0 interrupt
	INT_PIT1                     = 85, //PIT timer channel 1 interrupt
	INT_PIT2                     = 86, //PIT timer channel 2 interrupt
	INT_PIT3                     = 87, //PIT timer channel 3 interrupt
	INT_PDB0                     = 88, //PDB0 Interrupt
	INT_USB0                     = 89, //USB0 interrupt
	INT_USBDCD                   = 90, //USBDCD Interrupt
	INT_Reserved91               = 91, //Reserved interrupt 91
	INT_Reserved92               = 92, //Reserved interrupt 92
	INT_Reserved93               = 93, //Reserved interrupt 93
	INT_Reserved94               = 94, //Reserved interrupt 94
	INT_Reserved95               = 95, //Reserved interrupt 95
	INT_Reserved96               = 96, //Reserved interrupt 96
	INT_DAC0                     = 97, //DAC0 interrupt
	INT_Reserved98               = 98, //Reserved interrupt 98
	INT_TSI0                     = 99, //TSI0 Interrupt
	INT_MCG                      = 100,//MCG Interrupt
	INT_LPTimer                  = 101,//LPTimer interrupt
	INT_Reserved102              = 102,//Reserved interrupt 102
	INT_PORTA                    = 103,//Port A interrupt
	INT_PORTB                    = 104,//Port B interrupt
	INT_PORTC                    = 105,//Port C interrupt
	INT_PORTD                    = 106,//Port D interrupt
	INT_PORTE                    = 107,//Port E interrupt
	INT_Reserved108              = 108,//Reserved interrupt 108
	INT_Reserved109              = 109,//Reserved interrupt 109
	INT_SWI                      = 110,//Software interrupt
} IRQInterruptIndex;
#define MAX_INTERRUPT INT_SWI

//Interrupt vector table in RAM
extern void (* _VectorsRam[NVIC_NUM_INTERRUPTS+16])(void);

//Default handlers for unused ISRs.
//isrFault() is used for entries in the ISR table which don't correspond
//to any event (i.e. reserved entries). It locks up the program.
//isrUnused() is used for entries which are valid, but which don't have
//any handler attached. The default implementation just calls isrFault().
WEAK void isrFault(void);
WEAK void isrUnused(void);
ISRFUNC void isrDefaultSystick(void);
ISRFUNC void isrDefaultPortA(void);
ISRFUNC void isrDefaultPortB(void);
ISRFUNC void isrDefaultPortC(void);
ISRFUNC void isrDefaultPortD(void);
ISRFUNC void isrDefaultPortE(void);
ISRFUNC void isrDefaultUsb();
ISRFUNC void isrDefaultI2c0(void);
ISRFUNC void isrDefaultI2c1(void);
ISRFUNC void isrDefaultI2c2(void);

//other defaults are externed in kinetis.h.

//Following aren't "real" ISRs, but are called by default ISRs.
//it's important that all weak symbols be extern'd, or else the application's
//replacements for them will go silently ignored, because derp.
extern void isrPin(int pin); //called from isrDefaultPort*()
extern void isrI2C(int port); //called from isrDefaultI2c*()

//USB ISR hooks called from isrDefaultUsb()
void isrDefaultUsbStall();
void isrDefaultUsbAttach();
void isrDefaultUsbResume();
void isrDefaultUsbSleep();
void isrDefaultUsbToken();
void isrDefaultUsbSof();
void isrDefaultUsbError();
void isrDefaultUsbReset();
int  isrDefaultUsbConfigure(uint8_t config);
//the actual aliases are in usb/isr.c, because for some reason the alias
//has to be defined in the same file as the symbol it's aliased to.
extern void isrUsbStall();
extern void isrUsbAttach();
extern void isrUsbResume();
extern void isrUsbSleep();
extern void isrUsbToken();
extern void isrUsbSof();
extern void isrUsbError();
extern void isrUsbReset();
extern int  isrUsbConfigure(uint8_t config);

//void enable_irq(int irq);
//void disable_irq (int irq);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_ISR_H_
