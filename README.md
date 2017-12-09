# environmental-monitoring-C167
Legacy code for Siemens C167 microcontroller board which was for the EU Transport futures project. 

The Compiler used was from KEIL.

This code reads the ADC values for R,G,B channels from a photovolaic spectrometer which used the Mazet 3 channel photo diode with an OPA111 preamplifier from Burr Brown. The probe was used in the exhaust of a bus, and was also tested with various smoke sources
to allow detection of particulates and particulat bursts during vehicle operation.

This is legacy source code from 2001, so another of my old embedded projects

The files are all Ansi C and include:

1. Project main file
2. Flash memory files
3. Drivers for the main program. 

Interfacing to the microcontroller is via a monitor program which is accessed by the serial
terminal and pressing <ESC> while the microcontroller reboots.

