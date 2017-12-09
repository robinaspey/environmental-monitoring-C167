/*----------- 
FILESYS.C 
-----------*/

#include <reg167.h>

#define CNTLQ 		0x11
#define CNTLS		0x13
#define DEL			0x7F
#define BACKSPACE	0x08
#define CR			0x0d
#define LF			0x0a
#define XON			0x11
#define XOFF		0x13


char putchar(signed char c) 
{
	if (c == '\n') 	{
	if (S0RIR)	{
		if (S0RBUF == XOFF) {
			do	{
				S0RIR = 0;
				while(!S0RIR) {;}
				}
			while(S0RBUF != XON);
			S0RIR = 0;
		}
	}
	while(!S0TIR){;}
	S0TIR = 0;
	S0TBUF = 0x0d;
}
if (S0RIR)	
	{
	if (S0RBUF == XOFF) 
		{
		do 	{
			S0RIR = 0;
			while(!S0RIR){;}
			}
	while(S0RBUF != XON);
	S0RIR=0;
	}
}

while(!S0TIR){;}
S0TIR = 0;
return(S0TBUF = c);
}


char getkey()
{
char c;
IEN = 0;
while(!S0RIR){;}
c = S0RBUF;
S0RIR = 0;
IEN = 1;
return(c);
}


void getline(char near *line, unsigned char n)
{
unsigned char cnt = 0;
char c;

do {
	if ((c = getkey()) == CR) c = LF;
	if (c == BACKSPACE || c == DEL) 
		{
		if (cnt!=0)
			{
			cnt--;
			line--;
			putchar(0x08);
			putchar(' ');
			putchar(0x08);
			}
		}
		else
			if (c != CNTLQ && c != CNTLS)
				{
				putchar(*line = c);
				line++;
				cnt++;
				}
			}
		while(cnt < n - 1 && c != LF);
	*line = 0;
}


void delay(int time)
{
unsigned long i=0, j=0;
for (i=0;i< 60000;i++)
	{	for (j=0; j < time; j++) {;}  }
}


