#ifndef __LCD_FUNC_H
#define __LCD_FUNC_H

//#ifndef TS_7800
//#warning "TS_7800 not defined in lcd_func"
//#else
#ifdef TS_7800
#define	LCDBASEADD	0xE8000000
#else 
#define	LCDBASEADD	 0x80840000
#endif

volatile UINT *dioptr;
volatile  int dio_fd;

#define dio_set_bit8(A) setbiobit(portfb,1,A)
#define dio_set_ddr8(A) setbiobit(portfd,1,A)

//#if 0
#define PADR    0							// address offset of LCD
#define PADDR   (0x10 / sizeof(UINT))		// address offset of DDR LCD
#define PHDR    (0x40 / sizeof(UINT))		// bits 3-5: EN, RS, WR
#define PHDDR   (0x44 / sizeof(UINT))		// DDR for above
#define DIODDR	(0x14 / sizeof(UINT))
#define DIOADR	(0x04 / sizeof(UINT))
#define PORTFB  (0x30 / sizeof(UINT))
#define PORTFD	(0x34 / sizeof(UINT))
#define PORTLED	(0x20 / sizeof(UINT))

//#endif
//The RED and Green LEDs can be controlled at physical address location 0x8084_0020.
//Bit 1 is the RED LED and bit 0 is the Green LED. A Logic “1” turns the LED on.

// These delay values are calibrated for the EP9301
// CPU running at 166 Mhz, but should work also at 200 Mhz
#define SETUP   15
#define PULSE   36
#define HOLD    22

void red_led(int onoff);
void green_led(int onoff);
static int setbiobit(UCHAR *ptr,int n,int v);
static int setdioddr(int n,int v);
static int getdioddr(int n);
void close_dio(void);
void print_mem1(void);
static void init_master(void);
static void init_slave(void);
static void spi_master(void);
static void spi_slave(void);
volatile UINT *phdr;
volatile UINT *phddr;
volatile UINT *padr;
volatile UINT *paddr;
volatile UINT *dio_addr;
volatile UINT *dio_ddr;
volatile UINT *portfb;
volatile UINT *portfd;
volatile UINT *portled;

#define SETSS() setdioline(SS,0)
#define CLRSS() setdioline(SS,1)
#define SETSCLK() setdioline(SCLK,0)
#define CLRSCLK() setdioline(SCLK,1)
#define SETMOSI() setdioline(MOSI,0)
#define CLRMOSI() setdioline(MOSI,1)
#define SETMISO() setdioline(MISO,0)
#define CLRMISO() setdioline(MISO,1)

//#endif
#endif

