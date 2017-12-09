# environmental-monitoring-C167
Legacy code for Siemens C167 microcontroller board which was for the EU Transport futures project. The Compiler used was KEIL Microvision 2. Assembler and Ansi C as well as binary files from the board OEM are included although the ASM and C driver and main program files were for the project. The additional hardware for the board was a custom photodetector board which had gain increments of x10, x100 and x1000 under control of the C167 dependent on the signal level

This code reads the ADC values for R,G,B channels from a photovolaic spectrometer which used the Mazet 3 channel photo diode with an OPA111 preamplifier from Burr Brown. The probe was used in the exhaust of a bus, and was also tested with various smoke sources
to allow detection of particulates and particulat bursts during vehicle operation.

This is legacy source code from 2001, so another of my old embedded projects

The files are all Ansi C and include:

1. Project main file: mtf0.c, mtf1.c, mtf2.c, mtf3.c, mtf4.c (revisions not under VCS)
2. Flash memory file system file (ff_flash.c)
3. Drivers for the main program  

Interfacing to the microcontroller is via a monitor program which is accessed by the serial
terminal and pressing <ESC> while the microcontroller reboots.

