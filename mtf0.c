/************************************************************/
/* Project:		Merseyside Transport Futures             	*/
/* Author:		Robin Aspey (G03)							*/
/* Date:		16 October 2000								*/	
/* 															*/
/*	Skeleton program for C167 uController 					*/ 

unsigned short t3_count   ; 			// Medium to fast variable in NDATA
unsigned short idata fvar ; 			// Fast variable in on-chip IRAM
unsigned short sdata svar ; 			// Fast variable in on-chip XRAM (Not C161 or C164)
unsigned short big_array[0x80] ; 		// Array that could be up to 64k bytes in size
unsigned short xhuge vbig_array[0x80] ; // Array that could be up to 16MB bytes in size
unsigned short *hptr ; 					// A pointer that can point at anything up to 64K in size
unsigned short xhuge *xptr ; 			// A pointer that can point at anything of any size
unsigned short bdata flags ; 			// A word variable in the bit-addressible IRAM area on-chip
sbit flag0 = flags^0 ;       			// A single bit flag located in bit 0 of word "flags"
bit bflag ; 							// Single bit variable
										// Constant Declaration For EPROM 
unsigned short const nconstant = 1 ; 	// Medium to fast constant in EPROM

/************************************************************/

#include <reg167.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pragmas.h>

#define ESCAPE 	0x1B
#define LF		0x0A
#define CR		0x0D
#define XON		19
#define XOFF	17

#ifdef _LEDS_P2_
#define LED_port_DP DP2
#define LED_port    P2
#endif

/**************************************/
/* Data Logging Main Program          */
/**************************************/

void initialise(void);
void display_message(void);
void display_cs(void);
void setup_flash_base(void);
void init_serial_comms(int);
void read_rgb_values(void);
unsigned read_adc(int);
void monitor(void);
void restart(void);
void timeout(void);
void delay(int);
char _getserialkey(void);

char _global_key;
char _rx_byte = 0xFF;			// until a key pressed!

unsigned _rx_errors      = 0;
unsigned _bytes_received = 0;
unsigned _ticks = 0;
unsigned _logging = 0;

const char replies[]={ 0x1b, '1','2','3','4','5', '6', CR };	// define valid responses
char const build_date[]=__DATE__;
char const build_time[]=__TIME__;

extern int init_flash(int, int); 
extern int write_flash_byte(int, int);
extern int read_address(int, int);

void main(void) 
{
char ch=0;

initialise();			// set up serial port and timer status LED
//func1 ();
//func2 ();
//func3 ();

display_message();
timeout();

while(1) 
	{ 	
	switch(_rx_byte)
		{
		case ESCAPE: _rx_byte=0; monitor(); 			break;
		case CR:	 _rx_byte=0; display_message();  	break;
//		case 0x31:	 _rx_byte=0; read_rgb_values();		break;
//		case 0x32:	 _rx_byte=0; puts("2 pressed!");	break;
//		case 0x33:	 _rx_byte=0; puts("3 pressed!");	break;
		default:	 break;
		}				// reset byte received otherwise loop is endless
    }
}

void read_rgb_values(void)
{
int  vadc[4], i, j=10;
for (j=0; j <= 10; j++)
	{
	for (i=0; i <= 4; i++)
		{
		read_adc(i);
		}	printf("\nADC values r: %d g: %d b: %d", vadc[0], vadc[1], vadc[2]);
	}
S0RBUF=0; 	
_rx_byte=0;	
}

unsigned read_adc(int channel)
{
ADCON = channel;
ADST  = 1;
while(ADBSY){ ; }

return(ADDAT & 0x03ff);
}

void rx_handler(void) interrupt 0x2B using RXREGS
{
S0RIE=1;		// enable receive interrupt
S0REN=0;		// stop asyn comms
S0RIR=0;		// delete interrupt received flag
_rx_byte=S0RBUF;
S0REN=1;		// restart async comms
}

/*** LED Status Flasher Interrupt Routine ***/   

void status_interrupt(void) interrupt 0x23 
{
if (!_logging)	_ticks++;
LED_port ^= 0x01 ; 		// Flash LED #2.0 as status indicator
P7^= 0x01;
if (isalnum(_rx_byte) && strchr(replies, _rx_byte))
	{
	printf("\n_rx_byte: %02x", _rx_byte);
	_rx_byte = 0;
	}
}

//void init_serial_interrupt(void){ S0RIE=1; }
//void  end_serial_interrupt(void){ S0RIE=0; }

char _getserialkey(void)
{
char c;
IEN = 0;
while(!S0RIR) {;}
c = S0RBUF;			// where the character received is stored
S0RIR = 0;			// reset receive interrupt flag
S0RBUF= 0;			// not sure if this is read only?
IEN = 1;			
return(c);
}

void restart(void)
{
puts("\nIf nothing else after this you have a problem!");
// enter reset code here
}
   
void init_serial_comms(int rate)
{
P3 	   |= 0x400;	// set port 3.10 to TXD
DP3    |= 0x400;	// set direction control to output
DP3	   &= 0xF7FF;	// reset port 3.11 direction control for RXD
S0TIC	=0x80;		// set transmit interrupt flag
S0RIC	=0x00;		// delete receive interrupt flag
switch(rate)
	{
	case 9600:	S0BG = 0x40; break;
	case 19200:	S0BG = 0x20; break;
	case 38400:	S0BG = 0x10; break;
	default:	S0BG = 0x40; break;
	}
S0CON	= 0x8011;	// set serial mode to async 8,n,1
}
	
void display_message(void)
{
printf(	"\nC167 Exhaust Data Logging Card (ROM Vers 1.0)"
		"\nAuthor: Robin Aspey (build date: %s)\n\n", build_date);
puts("<ESC> for monitor. Also aborts data logging\n");
}

void initialise(void)
{
init_serial_comms(9600);

S0RIC = 0x3F;			// sets up interrupt priorities
S0RIE = 1;				// use maximum priority for rx data
IEN   = 1;				// allow timer and serial port to run
LED_port_DP |= 0x0F ; 	// Ports 2.0 - 0.3 are output LEDs   	
						// Initialise A 1ms Interrupt On GPT1 */
DP7 |= 0x0F;			// initialise port 7.0 to 7.3 as outputs 
T2CON 	= 0x27 ;
T3CON 	= 0x0007 ;
T3UD 	= 1 ;                
T3 = T2 = 2500;			// 2500  is 1 second I think ???
T3IC 	= 0x04 ;   		// Timer 3 interrupt is priority 1
T3IE 	= 1    ;   		// Enable Timer 3 interrupt                                   
T3R 	= 0 ;   		// Disable Global Interrupts until ready to service IRQ 
IEN 	= 1 ;   		// May be required for debugger
}
   
void timeout(void)
{
int dcount = 10;
_rx_byte   = 0xff;
_logging   = 0;
while(!_logging)
	{
	if (_rx_byte == 0x1B) // if ESC key is pressed
		{ printf("\n%02x or <ESC> trapped.\n\n", _rx_byte); return; }
	if (dcount <= 0) _logging = 1;
	else{
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

//void delay(int time)
//{
//unsigned long i=0, j=0;
//for (i=0; i<60000; i++)
//	{
//	for (j=0; j<time; j++) {;}
//	}
//}

void monitor(void)
{
#ifndef NOMONITOR
T3R = 0;
puts("\nC167 monitor and setup options\n");
puts("1. Read ADC values on RGB ports.");
puts("2. Set flash memory base address.");
puts("3. Dump flash memory to screen.");
puts("4. Dump flash data to RS232 (9600,8,1,n).");
puts("5. Display setup and hardware information.");   
puts("6. Start data logging.");
puts("\n<ESC> to return. Re-start data logging.");
while(!strchr(_rx_byte, replies)) 	
	{							// sit in loop waiting for keypress
	switch(_rx_byte)
		{
		case 0x31:	
					puts("\n1. Code for reading ADC"); 	
					read_rgb_values(); 					
					break;
		case 0x32:	
					puts("\n2. Code for flash base"); 	
//					init_flash();
					S0RBUF=0;	_rx_byte=0;	
					break;
		case 0x33:	
					puts("\n3. Code to dump memory");  	
					S0RBUF=0;	_rx_byte=0;	
					break;
		case 0x34: 	
					puts("\n4. Code for serial output"); 	
					S0RBUF=0;	_rx_byte=0;	
					break;
		case 0x35: 	
					puts("\n5. pressed .. Reading setup information."); 	 	
					display_cs();	// show chip select info.
					S0RBUF=0;	
					_rx_byte=0;	
					break;
		case 0x36:  
					puts("\n6. Start data logging");      
					S0RBUF=0;   
					_rx_byte=0; 
					break;
		case ESCAPE:	
					display_message();	
					S0RBUF=0;   
					_rx_byte=0; 
					T3R=1;		// enable interrupts on timer counter
					return;
		default: 		
					S0RBUF=0; 
					break;
		} 
#endif 		//	NOMONITOR
	}
}

void display_cs(void)
{
puts("\nChip Select Address MSB (RGSAD) and Block Sizes (RGSZ)");
puts("(RGSZ uses bits 0 - 3) 0000 = 4k, MUL by two per bit:   ");
printf("/CS1: 0x%04x /CS2: 0x%04x\n"
	   "/CS3: 0x%04x /CS4: 0x%04x\n", 
		ADDRSEL1, ADDRSEL2, ADDRSEL3, ADDRSEL4);
puts("\nBus Control Registers: ");
printf("BC0: 0x%04x BC1: 0x%04x BC2: 0x%04x\n" 
		"BC3: 0x%04x BC4: 0x%04x\n", 
		BUSCON0, BUSCON1, BUSCON2, BUSCON3, BUSCON4);
printf("\nSystem control SYSCON: 0x%04x", SYSCON);
}

void setup_flash_base(void)
{
ADDRSEL3=0x0807;	// sets flash start address to 
					// 0x8000 and block size 512K Byte
BUSCON3=0xFFC3;		// /CS dependant on /RD and /WR lines 
					// wait states set with ALE lengthened
}


   
