//************************************************************
//Project:	Merseyside Transport Futures
//Author(s):	Robin Aspey, PhD
//Date:		16/10/2001
//************************************************************

#include <reg167.h>
#include <intrins.h>
#include <stdio.h>
#include <ctype.h>
#include <pragmas.h>

#define _LEDS_P2_				// default for Phytec C167 board

#ifdef _LEDS_P2_
#define LED_port_DP      DP2
#define LED_port         P2
#endif

//#ifdef _LEDS_P1_
//#define LED_port_DP      DP1L
//#define LED_port         P1L
//#endif

/**************************************/
/* Global Data Declarations           */
/**************************************/

unsigned short t3_count   ; 			// Medium to fast variable in NDATA
unsigned short idata fvar ; 			// Fast variable in on-chip IRAM
unsigned short sdata svar ; 			// Fast variable in on-chip XRAM (Not C161 or C164)
unsigned short big_array[0x80] ; 		// Array that could be up to 64k bytes in size
unsigned short xhuge vbig_array[0x80] ; 	// Array that could be up to 16MB bytes in size
unsigned short *hptr ; 				// A pointer that can point at anything up to 64K in size
unsigned short xhuge *xptr ; 			// A pointer that can point at anything of any size
unsigned short bdata flags ; 			// A word variable in the bit-addressible IRAM area on-chip
sbit flag0 = flags^0 ;       			// A single bit flag located in bit 0 of word "flags"
bit bflag ; 					// Single bit variable

/*** Example Constant Declaration Destined For EPROM ***/

unsigned short const nconstant = 1 ; 		// Medium to fast constant in EPROM

/**************************************/
/* Simple Main Program                */
/**************************************/

void initialise(void);
char _getkey(void);

void main(void) 
{ 
char ch;
puts("\nC167 Exhaust data logging card.");
initialise();
while(1) 
	{
	if (ch = _getkey() == 0x1B)
		{
		puts("\nEntering command mode.");
		}
	}   	
}

char _getkey(void)
{
char c;
while(!S0RIR){;} 				// poll for interrupt flag
c=S0RBUF;
S0RIR=0;
return(c);
}
						// Simple Interrupt Routine 
void Timer3_interrupt(void) interrupt 0x23 
{
LED_port ^= 0x01 ; 				// Flash bottom 4 LEDs
t3_count++ ;   
}   

void initialise(void)
{
LED_port_DP |= 0x01 ; 				// Ports x.0 - 0.3 are output LEDs
   						// Initialise A 1ms Interrupt On GPT1 
T2CON 	= 0x27 ;
T3CON 	= 0x0007 ;
T3UD 	= 1 ;                
T3 = T2 = 2500 ;
t3_count= 0 ;
T3IC 	= 0x04 ;   				// Timer 3 interrupt is priority 1
T3IE 	= 1 ;   				// Enable Timer 3 interrupt                                   
T3R 	= 1 ;   				/* Enable Global Interrupts */
IEN 	= 1 ;   				// May be required for debugger 
}		

   
