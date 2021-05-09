#include "startup.h"

//Interrupt vector table in RAM
#if defined(__MK20DX128__)
	SECTION(".dmabuffers") USED_SYMBOL ALIGN(256)
#else
	SECTION(".dmabuffers") USED_SYMBOL ALIGN(512)
#endif
void (* _VectorsRam[NVIC_NUM_INTERRUPTS+16])(void);

//Interrupt vector table in flash.
//This table gets copied to the table in RAM at startup, but it can also be
//used on its own by editing the startup code to prevent doing that.
SECTION(".vectors") USED_SYMBOL
void (* const _VectorsFlash[NVIC_NUM_INTERRUPTS+16])(void) = {
	(void (*)(void))((unsigned long)&_estack), //0 ARM: Initial Stack Pointer
	ResetHandler,                 //1 ARM: Initial Program Counter
	isrNmi,                       //2 ARM: Non-maskable Interrupt (NMI)
	isrHardFault,                 //3 ARM: Hard Fault
	isrMemFault,                  //4 ARM: MemManage Fault
	isrBusFault,                  //5 ARM: Bus Fault
	isrUsageFault,                //6 ARM: Usage Fault
	isrFault,                     //7 --
	isrFault,                     //8 --
	isrFault,                     //9 --
	isrFault,                     //10 --
	isrSvcall,                    //11 ARM: Supervisor call (SVCall)
	isrDebugmonitor,              //12 ARM: Debug Monitor
	isrFault,                     //13 --
	isrPendablesrvreq,            //14 ARM: Pendable req serv(PendableSrvReq)
	isrSystick,                   //15 ARM: System tick timer (SysTick)
#if defined(__MK20DX128__)
	isrDmaCh0,                    //16 DMA channel 0 transfer complete
	isrDmaCh1,                    //17 DMA channel 1 transfer complete
	isrDmaCh2,                    //18 DMA channel 2 transfer complete
	isrDmaCh3,                    //19 DMA channel 3 transfer complete
	isrDmaError,                  //20 DMA error interrupt channel
	isrUnused,                    //21 DMA --
	isrFlashCmd,                  //22 Flash Memory Command complete
	isrFlashError,                //23 Flash Read collision
	isrLowVoltage,                //24 Low-voltage detect/warning
	isrWakeup,                    //25 Low Leakage Wakeup
	isrWatchdog,                  //26 Both EWM and WDOG interrupt
	isrI2c0,                      //27 I2C0
	isrSpi0,                      //28 SPI0
	isrI2s0Tx,                    //29 I2S0 Transmit
	isrI2s0Rx,                    //30 I2S0 Receive
	isrUart0Lon,                  //31 UART0 CEA709.1-B (LON) status
	isrUart0Status,               //32 UART0 status
	isrUart0Error,                //33 UART0 error
	isrUart1Status,               //34 UART1 status
	isrUart1Error,                //35 UART1 error
	isrUart2Status,               //36 UART2 status
	isrUart2Error,                //37 UART2 error
	isrAdc0,                      //38 ADC0
	isrCmp0,                      //39 CMP0
	isrCmp1,                      //40 CMP1
	isrFtm0,                      //41 FTM0
	isrFtm1,                      //42 FTM1
	isrCmt,                       //43 CMT
	isrRtcAlarm,                  //44 RTC Alarm interrupt
	isrRtcSeconds,                //45 RTC Seconds interrupt
	isrPit0,                      //46 PIT Channel 0
	isrPit1,                      //47 PIT Channel 1
	isrPit2,                      //48 PIT Channel 2
	isrPit3,                      //49 PIT Channel 3
	isrPdb,                       //50 PDB Programmable Delay Block
	isrUsb,                       //51 USB OTG
	isrUsbCharge,                 //52 USB Charger Detect
	isrTsi0,                      //53 TSI0
	isrMcg,                       //54 MCG
	isrLptmr,                     //55 Low Power Timer
	isrPorta,                     //56 Pin detect (Port A)
	isrPortb,                     //57 Pin detect (Port B)
	isrPortc,                     //58 Pin detect (Port C)
	isrPortd,                     //59 Pin detect (Port D)
	isrPorte,                     //60 Pin detect (Port E)
	isrSoftware,                  //61 Software interrupt
#elif defined(__MK20DX256__)
	isrDmaCh0,                    //16 DMA channel 0 transfer complete
	isrDmaCh1,                    //17 DMA channel 1 transfer complete
	isrDmaCh2,                    //18 DMA channel 2 transfer complete
	isrDmaCh3,                    //19 DMA channel 3 transfer complete
	isrDmaCh4,                    //20 DMA channel 4 transfer complete
	isrDmaCh5,                    //21 DMA channel 5 transfer complete
	isrDmaCh6,                    //22 DMA channel 6 transfer complete
	isrDmaCh7,                    //23 DMA channel 7 transfer complete
	isrDmaCh8,                    //24 DMA channel 8 transfer complete
	isrDmaCh9,                    //25 DMA channel 9 transfer complete
	isrDmaCh10,                   //26 DMA channel 10 transfer complete
	isrDmaCh11,                   //27 DMA channel 10 transfer complete
	isrDmaCh12,                   //28 DMA channel 10 transfer complete
	isrDmaCh13,                   //29 DMA channel 10 transfer complete
	isrDmaCh14,                   //30 DMA channel 10 transfer complete
	isrDmaCh15,                   //31 DMA channel 10 transfer complete
	isrDmaError,                  //32 DMA error interrupt channel
	isrUnused,                    //33 --
	isrFlashCmd,                  //34 Flash Memory Command complete
	isrFlashError,                //35 Flash Read collision
	isrLowVoltage,                //36 Low-voltage detect/warning
	isrWakeup,                    //37 Low Leakage Wakeup
	isrWatchdog,                  //38 Both EWM and WDOG interrupt
	isrUnused,                    //39 --
	isrI2c0,                      //40 I2C0
	isrI2c1,                      //41 I2C1
	isrSpi0,                      //42 SPI0
	isrSpi1,                      //43 SPI1
	isrUnused,                    //44 --
	isrCan0Message,               //45 CAN OR'ed Message buffer (0-15)
	isrCan0BusOff,                //46 CAN Bus Off
	isrCan0Error,                 //47 CAN Error
	isrCan0TxWarn,                //48 CAN Transmit Warning
	isrCan0RxWarn,                //49 CAN Receive Warning
	can0_isrWakeup,               //50 CAN Wake Up
	isrI2s0Tx,                    //51 I2S0 Transmit
	isrI2s0Rx,                    //52 I2S0 Receive
	isrUnused,                    //53 --
	isrUnused,                    //54 --
	isrUnused,                    //55 --
	isrUnused,                    //56 --
	isrUnused,                    //57 --
	isrUnused,                    //58 --
	isrUnused,                    //59 --
	isrUart0Lon,                  //60 UART0 CEA709.1-B (LON) status
	isrUart0Status,               //61 UART0 status
	isrUart0Error,                //62 UART0 error
	isrUart1Status,               //63 UART1 status
	isrUart1Error,                //64 UART1 error
	isrUart2Status,               //65 UART2 status
	isrUart2Error,                //66 UART2 error
	isrUnused,                    //67 --
	isrUnused,                    //68 --
	isrUnused,                    //69 --
	isrUnused,                    //70 --
	isrUnused,                    //71 --
	isrUnused,                    //72 --
	isrAdc0,                      //73 ADC0
	isrAdc1,                      //74 ADC1
	isrCmp0,                      //75 CMP0
	isrCmp1,                      //76 CMP1
	isrCmp2,                      //77 CMP2
	isrFtm0,                      //78 FTM0
	isrFtm1,                      //79 FTM1
	isrFtm2,                      //80 FTM2
	isrCmt,                       //81 CMT
	isrRtcAlarm,                  //82 RTC Alarm interrupt
	isrRtcSeconds,                //83 RTC Seconds interrupt
	isrPit0,                      //84 PIT Channel 0
	isrPit1,                      //85 PIT Channel 1
	isrPit2,                      //86 PIT Channel 2
	isrPit3,                      //87 PIT Channel 3
	isrPdb,                       //88 PDB Programmable Delay Block
	isrUsb,                       //89 USB OTG
	isrUsbCharge,                 //90 USB Charger Detect
	isrUnused,                    //91 --
	isrUnused,                    //92 --
	isrUnused,                    //93 --
	isrUnused,                    //94 --
	isrUnused,                    //95 --
	isrUnused,                    //96 --
	isrDac0,                      //97 DAC0
	isrUnused,                    //98 --
	isrTsi0,                      //99 TSI0
	isrMcg,                       //100 MCG
	isrLptmr,                     //101 Low Power Timer
	isrUnused,                    //102 --
	isrPorta,                     //103 Pin detect (Port A)
	isrPortb,                     //104 Pin detect (Port B)
	isrPortc,                     //105 Pin detect (Port C)
	isrPortd,                     //106 Pin detect (Port D)
	isrPorte,                     //107 Pin detect (Port E)
	isrUnused,                    //108 --
	isrUnused,                    //109 --
	isrSoftware,                  //110 Software interrupt
#endif
};
