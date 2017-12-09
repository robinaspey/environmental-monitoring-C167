/************************************************************/
/* Project:		Merseyside Transport Futures        */
/* Author:		Robin Aspey (G03)		    */
/* Version:		1.1 				    */
/* Date:		8 November 2000			    */	
/* 							    */
/*	Skeleton program for C167 uController 		    */ 
/************************************************************/

#include <reg167.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pragmas.h>
#include <srom.h>
#include <intrins.h>

#define ESCAPE 		0x1B
#define LF		0x0A
#define CR		0x0D
#define DEL		0x7F
#define CNTLQ		0x11
#define CNTLS		0x13
#define BACKSPACE	0x08
#define XON		19
#define XOFF		17
#define LED_port_DP 	DP2
#define LED_port    	P2

#define FL_SEC0		0x20000			// is defined in init()
#define FL_SEC1		FL_SEC0+0x10000
#define FL_SEC2		FL_SEC0+0x20000
#define FL_SEC3		FL_SEC0+0x30000
#define FL_SEC4		FL_SEC0+0x40000
#define FL_SEC5		FL_SEC0+0x50000

/**************************************/
/* Data Logging Main Program          */
/**************************************/

void initialise(void);
void display_message(void);
void display_cs(void);
void setup_flash_base(void);
void init_serial_comms(void);
void read_rgb_values(void);
void user_menu(void);
void monitor(void);
void restart(void);
void timeout(void);
void delay(int);

unsigned read_adc(int);

// Variables placed in NDATA

unsigned short _bytes_received = 0;
unsigned short _ticks = 0;
unsigned short _logging = 0;

char const replies[]={ 0x1b, '1','2','3','4','5', '6', CR };	// define valid responses
char const build_date[]=__DATE__;
char const build_time[]=__TIME__;

extern int 	PFlash_Write(void huge *, void huge *);
extern int 	PFlash_Erase(void huge *);
extern void PFlash_Reset(void);
extern int 	get_address(int);
extern void getline(char near *, unsigned char); 
extern char _getserialkey(void);

void prompt(void)
{
puts("\n<ESC> for menu.\n<CR> to re-start logging.");
}

char  isvalid(ch)
{
if (ch=='\t' || ch=='\n' || ch==0x0d || ch==0x0e)
	return('%');
return(ch);
}

void dump_memory(unsigned offset)
{
#define SIZEBUF 128
unsigned char huge *dpt, *pt, ch;
unsigned char buf[SIZEBUF], copy[SIZEBUF]; 
unsigned index=0, i=0, j=0, k=0;

dpt=(unsigned char huge *)FL_SEC0;	// can point to anything
xmemcpy(buf, dpt+offset, 128);
printf("\n\n\nDumping memory from 0x20%04Xh\n", offset);
while(index < 128)
	{
	if (index%16==0) 	
		{
		printf("\n10:%04X> ", offset+index);
		}
	printf("%02X ", *(dpt + offset + index++));
	if (index%16==0) 			// This loop prints ascii data
		{
		for (j=0; j<16; j++)	// count out characters
			{			
			putchar(isvalid(*(dpt + offset + j + (k*16))));
			}
		++k;
		}
	}	printf("\n<ESC> for menu or <7> to continue.");
}

char _rx_byte = 0xFF;			// until a key pressed!

void read_rgb_values(void)
{
unsigned short vadc[4], i, j=10;
for (j=0; j <= 10; j++)
	{
	for (i=0; i <= 4; i++)
		{
		read_adc(i);
		}	printf("\nADC values r: %d g: %d b: %d", vadc[0], vadc[1], vadc[2]);
	}
	puts("\n<ESC> for menu or <1> to continue.");
	_rx_byte = 0;
}

unsigned read_adc(int channel)
{
ADCON = channel;
ADST  = 1;
while(ADBSY){;}
return(ADDAT & 0x03ff);
}

void rx_handler(void) interrupt 0x2B using RXREGS
{
S0RIE 	= 1;		// enable receive interrupt
S0REN 	= 0;		// stop async comms
S0RIR 	= 0;		// delete interrupt received flag
_rx_byte = S0RBUF;
S0RBUF 	= 0;
S0REN	= 1;		// restart async comms
}

/*** LED Status Flasher Interrupt Routine ***/   

void status_interrupt(void) interrupt 0x23 
{
if (!_logging)	_ticks++;
LED_port ^= 0x01 ; 	// Flash LED #2.0 as status indicator
P7^= 0x01;
if (isalnum(_rx_byte) && strchr(replies, _rx_byte))
	{
	printf("\n_rx_byte: %02x", _rx_byte);
	_rx_byte = 0;
	}
}

void init_serial_comms(void)
{
P3     |= 0x400;	// set port 3.10 to TXD
DP3    |= 0x400;	// set direction control to output
DP3    &= 0xF7FF;	// reset port 3.11 direction control for RXD
S0TIC	= 0x80;		// set transmit interrupt flag
S0RIC	= 0x00;		// delete receive interrupt flag
S0BG 	= 0x40;		// 9600 baud
S0CON	= 0x8011;	// set serial mode to async 8,n,1
}
	
void display_message(void)
{
printf(	"\nC167 Exhaust Data Logging Card (ROM Vers 1.0)"
		"\nAuthor: Robin Aspey (build date: %s)\n\n"
        "<ESC> for monitor. (Abort data logging)\n", build_date);
}

void initialise(void)
{
init_serial_comms();
S0RIC 	= 0x3F;		// sets up interrupt priorities
S0RIE 	= 1;		// use maximum priority for rx data
IEN   	= 1;		// allow timer and serial port to run
LED_port_DP |= 0x0F ; 	// Ports 2.0 - 0.3 are output LEDs   	
			// Initialise A 1ms Interrupt On GPT1 */
DP7    |= 0x0F;		// initialise port 7.0 to 7.3 as outputs 
T2CON 	= 0x27 ;
T3CON 	= 0x0007 ;
T3UD 	= 1 ;                
T3 = T2 = 2500;		// 2500  is 1 second I think ???
T3IC 	= 0x04 ;   	// Timer 3 interrupt is priority 1
T3IE 	= 1    ;   	// Enable Timer 3 interrupt                                   
T3R 	= 0 ;   	// Disable Global Interrupts until ready to service IRQ 
IEN 	= 1 ;   	// May be required for debugger
ADDRSEL3= 0x0207;	// sets Flash start address to 
			// 0x100000 and block size 512K Byte
BUSCON3 = 0xFFC3;	// /CS dependant on /RD and /WR lines 
			// wait states set with ALE lengthened
ADDRSEL2= 0x0;		// 4K size disabled
BUSCON2 = 0;		// disable external bus on CS2
}
   
void timeout(void)
{
unsigned short dcount = 10;
_rx_byte   = 0xff;
_logging   = 0;
while(!_logging)
	{
	if (_rx_byte == 0x1B) // if ESC key is pressed
		{ printf("\n%02x or <ESC> trapped.\n\n", _rx_byte); return; }
	if (dcount <= 0) 
		_logging = 1;
	else
		{
		delay(4);
		printf("\rTimeout in %02d seconds", dcount--);
		}
	} 	
if (_logging==1)	
	{	
	T3R=1; // enable interrupts on status monitor
	puts("\nRunning ..\n\n"); 	
	}
}

void user_menu(void)
{
puts(	"\nC167 monitor and setup options\n"
	"1. Read ADC values on RGB ports.\n"
	"2. <Unused option>.\n"
	"3. Erase flash memory (sec 0).\n"
	"4. Write data to base flash (sec 0).\n"
	"5. Read base flash (sec 0).\n"
	"6. Display setup and hardware information.\n"   
	"7. Dump base flash memory address.\n"
	"8. Read a line of text.\n"
	"\n<ESC> for menu. \n<CR> to re-start data logging.\n");
}

void monitor(void)
{
unsigned char const fl_name[32]={"XXXXXXC167 FLASH(512K x 8)"};
unsigned char line_buff[64];
unsigned segment;
unsigned offset, status;
T3R = 0;

user_menu();
offset = 0;
while(!strchr(_rx_byte, replies)) 	
	{	// sit in loop waiting for keypress
	status=0;
	switch(_rx_byte)
		{
		case 0x31:	
			puts("\nReading ADC ..."); 	
			read_rgb_values(); 					
			break;
		case 0x32:	
			S0RBUF=0;	
			_rx_byte=0;	
			break;
		case 0x33:	
			status = PFlash_Erase((unsigned huge *)FL_SEC0);
			printf("\nErasing flash memory. (Code: %xh)", status); 
			offset=0;
			prompt();
			S0RBUF=0;	
			_rx_byte=0;	
			break;
		case 0x34: 	
			status = PFlash_Write((void huge *)FL_SEC0, (void huge *)(fl_name));
			printf("\nDescriptor written: %12s (Code: %xh).", fl_name, status);
			prompt();
			S0RBUF=0;	
			_rx_byte=0;	
			break;
		case 0x35:
			printf(	"\nReading flash ident string"
				": %20s.", (char huge *)FL_SEC0);
			prompt();
			S0RBUF=0;  
			_rx_byte=0;
			break;
		case 0x36: 	
			puts("\nReading setup information."); 	 	
			display_cs();	// show chip select info.
			prompt();
			S0RBUF=0;	
			_rx_byte=0;	
			break;
		case 0x37:
			dump_memory(offset);
			offset+=128;
			S0RBUF=0;	
			_rx_byte=0;
			break;
		case 0x38:
			puts("\nEnter data descriptor with date/time: ");
			getline(line_buff, 20);
			printf("Descriptor: %12s", line_buff);
			S0RBUF=0;	
			_rx_byte=0;
			break;
		case ESCAPE:	
			user_menu();
			S0RBUF=0;   
			_rx_byte=0; 
			T3R=0;		// disable interrupts on timer counter
			break;
		case CR:
			T3R=1;
			puts("\nData logging started.");
			return;
		default: 		
			S0RBUF=0; 
			break;
		}
	}
}

#define MONITOR

#ifdef  MONITOR

void display_address_info(unsigned cs_reg, char *s)
{
unsigned address;		// Addrsel register is 16 bits
unsigned long absolute=0;
unsigned block;
address =(unsigned)(cs_reg & 0xfff0);
block   =(unsigned)(cs_reg & 0x000f);
absolute=(unsigned long)address;

switch(block)
	{
	case 0: 	block=4;	break;
	case 1: 	block=8;	break;
	case 2: 	block=16;	break;
	case 3: 	block=32;	break;
	case 4: 	block=64;	break;
	case 5: 	block=128;	break;
	case 6: 	block=256;	break;
	case 7: 	block=512;	break;
	case 8: 	block=1024;	break;
	default:	block=-1;	break;
	}
printf("\n%03s Address: ", s);
printf(" (ADRS)%04x00h", address);
printf(" Size: %dK", block);
}
#endif

void display_cs(void)
{
unsigned short a1, a2, a3, a4;
a1=ADDRSEL1; a2=ADDRSEL2;
a3=ADDRSEL3; a4=ADDRSEL4;

printf(	"\nChip Select Address MSB (RGSAD) and Block Sizes (RGSZ)\n"
	"(RGSZ uses bits 0 - 3) 0000 = 4k, MUL by two per bit:\n"
	"/CS1: 0x%04x /CS2: 0x%04x\n"
	"/CS3: 0x%04x /CS4: 0x%04x\n", a1, a2, a3, a4);

printf(	"\nBus Control Registers: \n"
	"BC0: 0x%04x BC1: 0x%04x BC2: 0x%04x\n" 
	"BC3: 0x%04x BC4: 0x%04x\n", 
	BUSCON0, BUSCON1, BUSCON2, BUSCON3, BUSCON4);
printf(	"\nSystem control SYSCON: 0x%04x\n\n",
	"<ESC> for menu or <6> to continue.\n", SYSCON);
#ifdef MONITOR
display_address_info(a1, "CS1 (RAM Bank1.)");
display_address_info(a2, "CS2 (Not used..)");
display_address_info(a3, "CS3 (Flash DATA)");
display_address_info(a4, "CS4 (Not used..)");
#endif
}

void main(void) 
{
initialise();			// set up serial port and timer status LED
display_message();
timeout();

while(1) 
	{ 	
	switch(_rx_byte)
		{
		case ESCAPE: 	_rx_byte=0; monitor(); 		break;
		case CR:	_rx_byte=0; display_message(); break;
		default:	 break;
		}		// reset byte received otherwise loop is endless
    	}
}

   
