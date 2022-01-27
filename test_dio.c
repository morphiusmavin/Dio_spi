#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include "mytypes.h"
#include "lcd_func.h"
//#include "../serial_io.h"
static void menu(void);
static void mydelay(unsigned long i);
#ifndef TS_7800
#define LCD_BUF_LINES 30
#define LCD_BUF_COLS 32
static UCHAR buffer[LCD_BUF_LINES][LCD_BUF_COLS];
static UCHAR *buf_ptr;
static int cur_buf_line;
static int cur_disp_line;
static int cur_buf_size;
static int inited;
static int gfd;
#define NWDIO 8
#define DIO_0	0
#define DIO_1	1
#define DIO_2	2
#define DIO_3	3
#define DIO_4	4
#define DIO_5	5
#define DIO_6	6
#define DIO_7	7

//static int wdio_lines[NWDIO] = {0,1,2,3,4,5,6,7};
static int wdio_lines[NWDIO] = {DIO_0,DIO_1,DIO_2,DIO_3,DIO_4,DIO_5,DIO_6,DIO_7};
static UCHAR data_in[10];
static UCHAR data_out[10];

#define SS 		DIO_0
#define SCLK	DIO_1
#define MOSI	DIO_2 
#define MISO	DIO_3
#warning "still using non-woke, racist, white supremisist SPI definitions"
#ifdef MASTER
#warning "MASTER defined"
#else
#warning "SLAVE defined"
#endif
// DIO9 - bit 0		when these are written to SPI_RW
// DIO11 - bit 2
// SPI_MOSI - bit 3
// DIO13 - bit 4
// SPI_CLK - bit 5
// DIO 15 - bit 6
/*											works
1)	DIO_0		bottom left (dot)			*
2)	gnd			top left
3)	DIO_1									*
4)	Port_C0
5)	DIO_2									*
6)	SPI_FRAME
7)	DIO_3									*
8)	DIO_8									* (reversed from others)
9)	DIO_4									*
10)	SPI_MISO 
11)	DIO_5									*
12)	SPI_MOSI
13)	DIO_6									*
14)	SPI_CLK
15)	DIO_7		bottom right				*
16)	3.3v		top right

not using built-in SPI pins - instead use:

for:				master				slave

DIO_0 SS			out 				in
DIO_1 SCLK			out 				in
DIO_2 MOSI			out 				in
DIO_3 MISO			in 					out 

*/
/*********************************************************************/
int uSleep(time_t sec, long nanosec)
{
/* Setup timespec */
	struct timespec req;
	req.tv_sec = sec;
	req.tv_nsec = nanosec;

/* Loop until we've slept long enough */
	do
	{
/* Store remainder back on top of the original required time */
		if( 0 != nanosleep( &req, &req ) )
		{
/* If any error other than a signal interrupt occurs, return an error */
			if(errno != EINTR)
			{
				printf("uSleep error\n");
//             return -1;
			}
		}
		else
		{
/* nanosleep succeeded, so exit the loop */
			break;
		}
	} while ( req.tv_sec > 0 || req.tv_nsec > 0 );

	return 0;									  /* Return success */
}

/**********************************************************************************************************/
static void lcdinit(void)
{
	
	gfd = open("/dev/mem", O_RDWR);
	int psize = getpagesize();
//	psize /= 2;
//	psize = 1000;
	printf("pagesize: %d\r\n",psize);
	int i;

// if compiling with -static we have to use the MAP_FIXED flag
//	spi_dioptr = (UINT *)mmap(0, getpagesize(),PROT_READ|PROT_WRITE, MAP_SHARED, gfd, LCDBASEADD);
	dioptr = (UINT *)mmap(0, psize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, gfd, LCDBASEADD);
	printf("opened fd: %d %d %4x \n", gfd, psize, *dioptr);
	phdr = &dioptr[PHDR];
	padr = &dioptr[PADR];
	paddr = &dioptr[PADDR];
	phddr = &dioptr[PHDDR];
	dio_addr = &dioptr[DIOADR];
	dio_ddr = &dioptr[DIODDR];
	portfb = &dioptr[PORTFB];
	portfd = &dioptr[PORTFD];
	portled = &dioptr[PORTLED];

//	printf("1:  %4x %4x \n", gpio[PADR],gpio[PADDR]);
//	*paddr = 0x0;								  // All of port A to inputs
//	*phddr |= 0x38;								  // bits 3:5 of port H to outputs
//	*phdr &= ~0x18;								  // de-assert EN, de-assert RS
//	usleep(15000);

//	for(i = 0;i < 6;i++)							// set the 1st 6 bits of DIO to inputs
//		setdioddr(i,0);

//	setdioddr(7,1);									// set last one to output
	setdioddr(7,0);									// set last one to input
	for(i = 0;i < 10;i++)
	{
		red_led(1);
		mydelay(50);
		red_led(0);
		mydelay(50);
	}

	inited = 1;
	//close(dio_fd);
}
int ttoggle(int toggle)
{
	int tog = toggle;
	if(tog > 0)
		tog = 0;
	else tog = 1;
	printf("%d ",tog);
	return tog;
}
/**********************************************************************************************************/
void red_led(int onoff)
{
	setbiobit((UCHAR *)portled,1,onoff);
}

/*********************************************************************************************************/
void green_led(int onoff)
{
	setbiobit((UCHAR *)portled,0,onoff);
}

/*********************************************************************************************************/
static int setbiobit(UCHAR *ptr,int n,int v)
{
	unsigned char d;

	if (n>7 || n<0 || v>1 || v<0) return(-1);
	d=*ptr;
	d= (d&(~(1<<n))) | (v<<n);
	*ptr=d;
	return(v);
}

/*********************************************************************************************************/
static UCHAR dio_get_ddr(void)
{
	return(*dio_ddr);
}

/*********************************************************************************************************/
static UCHAR dio_set_ddr(UCHAR b)
{
	*dio_ddr=b;
	return(b);
}

/*********************************************************************************************************/
int getdioline(int n)
{
	int d;

	if (n>8 || n<0) return(-1);
	if (n==8) return(((*portfb)>>1)&1);
	return((*dio_addr>>n)&1);
}


/* setdioline(n,v) set DIO Line n to value v
 *                 return v, or -1 on error.
 * This does a read-modify-write sequence, so only the line
 * specified is altered. If the line is not set as output in the DDR
 * then this routine sets it as output
 */
/*********************************************************************************************************/
int setdioline(int n,int v)
{
	unsigned char d;
	if(inited == 0)
		return -1;

	if (n>8 || n<0 || v>1 || v<0) return(-1);
	if (n==8)
	{
		d=(*portfd)&2;
		if (!d) *portfd|=2;
		d=*portfb & 0xFD;
		*portfb=d|(v<<1);
	}
	else
	{
		d=*dio_ddr;
		if ( ! (d&(1<<n)) )
		{
			d|=(1<<n);
			*dio_ddr=d;
		}
		d=*dio_ddr;
		d= (d&(~(1<<n))) | (v<<n);
		*dio_ddr=d;
	}
	return(v);
}


/* getdioddr(n)  read the DDR of the DIO Line n
 *                return 0 or 1, or -1 on error
 */
/*********************************************************************************************************/
static int getdioddr(int n)
{
	int d;

	if (n>8 || n<0) return(-1);
	if (n==8) return((*portfd>>1)&1);
	return((dio_get_ddr()>>n)&1);
}


/* setdioddr(n,v) set DDR for line n to value v
 *                 return v, or -1 on error.
 * This does a read-modify-write sequence, so only the line
 * specified is altered.
 */
/*********************************************************************************************************/
static int setdioddr(int n,int v)
{
	unsigned char d;

	if (n>8 || n<0 || v>1 || v<0) return(-1);
	if (n==8)
	{
		dio_set_ddr8(v);
	}
	else
	{
		d=dio_get_ddr();
		d= (d&(~(1<<n))) | (v<<n);
		dio_set_ddr(d);
	}
	return(v);
}

/*********************************************************************************************************/
static void mydelay(unsigned long i)
{
	unsigned long j;

	do
	{
		for(j = 0;j < 10000;j++);
		i--;
	}while(i > 0);
}
/*********************************************************************************************************/
void print_mem1(void)
{
	int i;
	if(inited == 1)
	{
		printf("DIO memory:\n");
		printf("phdr:     %4x %4x\n",phdr,*phdr);	  // 40
		printf("padr:     %4x %4x\n",padr,*padr);	  // 0
		printf("paddr:    %4x %4x\n",paddr,*paddr);	  // 10
		printf("phddr:    %4x %4x\n",phddr,*phddr);	  // 44
		printf("dio_addr: %4x %4x\n",dio_addr,*dio_addr);
		printf("dio_ddr:  %4x %4x\n",dio_ddr,*dio_ddr);
		printf("portfb:   %4x %4x\n",portfb,*portfb);  // 30
		printf("portfd:   %4x %4x\n",portfd,*portfd);  // 34
		printf("portled:   %4x %4x\n",portled,*portled);  // 34
	}
}
/*********************************************************************************************************/
void close_dio(void)
{
	printf("closing dio_fd: %d\n",dio_fd);
	close(dio_fd);
}
#ifdef MASTER
void init_master(void)
{
	int i,j;
	printf("\nrunning as master\n\n");
	setdioddr(SS,1);
	mydelay(1);
	setdioddr(SCLK,1);
	mydelay(1);
	setdioddr(MOSI,1);
	mydelay(1);
	setdioddr(MISO,0);
	mydelay(1);
	setdioline(SS,1);	// SS is active low
	mydelay(1);
	setdioline(SCLK,1);	// SCLK is active low
	mydelay(1);
	for(i = 0;i < 10;i++)
	{
		data_out[i] = i+2;
	}
}
void spi_master(void)
{
	int i,j;
	UCHAR mask = 1;
	UCHAR temp = 0;
	UCHAR temp2 = 0;

	for(j = 0;j < 10;j++)
	{
		setdioline(SS,0);
		mydelay(10);
		temp = 0;
		temp2 = data_out[j];
		mask = 1;
		
		for(i = 0;i < 8;i++)
		{
			setdioline(SCLK,0);
			mydelay(2);
			if((mask & temp2) == mask)
				setdioline(MOSI,1);
			else 
				setdioline(MOSI,0);
			mydelay(10);
			if(getdioline(MISO) == 1)
				temp |= mask;
			printf("%02x ",temp);		
			mask <<= 1;
			mydelay(10);
			setdioline(SCLK,1);
			mydelay(10);
		}
		printf("\n");
		data_in[j] = temp;
		setdioline(SS,1);
		mydelay(50);
	}
	for(j = 0;j < 10;j++)
	{
		printf("%d\n",data_in[j]);
	}
}
#else 
void init_slave(void)
{
	int i,j;
	printf("\nrunning as slave\n\n");

	setdioddr(SS,0);
	mydelay(1);
	setdioddr(SCLK,0);
	mydelay(1);
	setdioddr(MOSI,0);
	mydelay(1);
	setdioddr(MISO,1);
	mydelay(1);

	for(i = 0;i < 10;i++)
	{
		data_out[i] = i + 3;
	}
}
void spi_slave(void)
{
	int i,j;
	UCHAR mask = 1;
	UCHAR temp = 0;
	UCHAR temp2 = 0;

	for(j = 0;j < 10;j++)
	{
		while(getdioline(SS) == 1)		// wait for SS to go low
			mydelay(1);
		temp = 0;
		mask = 1;
		temp2 = data_out[j];
		for(i = 0;i < 8;i++)
		{
			while(getdioline(SCLK) == 1)	// read data on falling edge of SCLK
				mydelay(1);

			if(getdioline(MOSI) == 1)
				temp |= mask;
			printf("%02x ",temp);
			mydelay(1);
			if((mask & temp2) == mask)
				setdioline(MISO,1);
			else 
				setdioline(MISO,0);

			mydelay(1);
			
			while(getdioline(SCLK) == 0)	// wait for SCLK to go back to inactive
				mydelay(1);
			mask <<= 1;	
		}
		printf("\n");
		data_in[j] = temp;
		while(getdioline(SS) == 0)		// wait for SS to go high (inactive)
			mydelay(1);
	}
	for(j = 0;j < 10;j++)
		printf("%d\n",data_in[j]);
}	
#endif 
/*********************************************************************************************************/
int main(void)
{
	int i,j,key;
	UCHAR temp;
	inited = 0;
	int tog;
#if 1
	i = j = 0;

	lcdinit();
	printf("version 0.03\n");
#ifdef MASTER 
	init_master();
#else 
	init_slave();
#endif
	menu();
	
	do
	{
		key = getc(stdin);
		switch(key)
		{
			case 'a':
				setdioline(wdio_lines[0],tog = ttoggle(tog));	
				// bottom row, 1st on left (DIO_0)
				break;
			case 'b':
				setdioline(wdio_lines[1],tog = ttoggle(tog));	
				// bottom row, 2nd from left (DIO_1)
				break;
			case 'c':
				setdioline(wdio_lines[2],tog = ttoggle(tog));
				// bottom row, 3rd from left (DIO_2)
				break;
			case 'd':
				setdioline(wdio_lines[3],tog = ttoggle(tog));
				// bottom row, 4th from left (DOI_3)
				break;
			case 'e':
				setdioline(wdio_lines[4],tog = ttoggle(tog));
				// bottom row, 4th from right (DOI_4)
				break;
			case 'f':
				setdioline(wdio_lines[5],tog = ttoggle(tog));
				// bottom row, 3rd from right (DOI_5)
				break;
			case 'g':
				setdioline(wdio_lines[6],tog = ttoggle(tog));
				// bottom row, 2nd from right (DOI_6)
				break;
			case 'h':
				// test input: reading this on scope is inverted
				setdioline(wdio_lines[6],tog = ttoggle(tog));
				printf("input: %d\n",tog);
				tog = getdioline(wdio_lines[7]);
				tog = ttoggle(tog);
				printf("res: %d\n",tog);
//				setdioline(wdio_lines[7],tog = ttoggle(tog));
				// bottom row, 1st on right (DOI_7)
				break;
			case 'i':
				for(i = 0;i < 1000;i++)
				{
					setdioline(0,1);
					mydelay(3);
					setdioline(0,0);
					mydelay(3);
				}
				setdioline(0,1);
				break;
			case 'j':
				for(i = 0;i < 200;i++)
				{
					for(j = 0;j < NWDIO;j++)
					{
						setdioline(wdio_lines[j],0);
						mydelay(10);
						red_led(1);
						mydelay(10);
						red_led(0);
						mydelay(10);
						setdioline(wdio_lines[j],1);
						mydelay(10);
						green_led(1);
						mydelay(10);
						green_led(0);
						mydelay(10);
					}
				}
				setdioline(1,1);
				printf("done testing all lines\r\n");
				break;
#endif
			case 'k':
// hit 'k' on master first and then on slave 			
#ifdef MASTER
				spi_master();
#else
				spi_slave();
#endif
				break;
#if 1
			case 'l':
				break;
			case 'm':
				menu();
				break;
			case 'n':	
				break;
			case 'o':
				lcdinit();
				break;
			case 'p':
				print_mem1();
				break;
			case 's':
				break;
			case 'r':
				
				break;
			case '2':
				printf("blinking led's\r\n");
				for(i = 0;i < 30;i++)
				{
					red_led(1);
					mydelay(30);
					red_led(0);
					mydelay(30);
					green_led(1);
					mydelay(30);
					green_led(0);
					mydelay(30);
				}
				break;
#endif
			default:
				break;
		}
	}while(key != 'q');
	if(inited == 1)
	{
		setdioline(0,1);
		setdioline(1,1);
		setdioline(2,1);
		setdioline(3,1);
		setdioline(4,1);
		setdioline(5,1);
		setdioline(6,1);
		setdioline(7,1);
	}
	close_dio();
	printf("\r\n\r\ndone!\r\n\r\n");
}
/*********************************************************************************************************/
static void menu(void)
{
	printf("a - set DIO 0\r\n");
	printf("b - clear DIO 0\r\n");
	printf("c - set DIO 1\r\n");
	printf("d - clear DIO 1\r\n");
	printf("i - toggle DIO 0\r\n");
	printf("j - toggle DIO 1\r\n");
#ifdef MASTER
	printf("k - start master SPI\r\n");
#else
	printf("k - start slave SPI\r\n");
#endif
	printf("l - \r\n");
	printf("m - show menu\r\n");
	printf("n - set all outputs\r\n");
	printf("o - init led\r\n");
	printf("p - print memory 1\r\n");
	printf("r -\r\n");
	printf("s -\r\n");
	printf("2 - toggle LED's\r\n");
	printf("q - quit\r\n");
}
#else
#warning "can't use TS_7800"
#endif

