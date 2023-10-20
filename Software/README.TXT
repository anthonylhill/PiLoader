
Raspberry Pi Zero Loader Card for Lee Hart's Membership Card
============================================================
version 1.0   2023/10/20 


=============
 makefile
=============
- standard make file for building Loader C and 1802 Assembler files


=============
 1802load
=============
- loads a binary file to 1802 memory

usage :   sudo 1802load <filename>    where filename is a binary file to be loaded at 0x0000

 GPIO Interface Methods :
-selected at compile time via #define GPIO_METHOD

1) sysfs interface    https://www.ics.com/blog/gpio-programming-using-sysfs-interface
                      https://elinux.org/RPi_GPIO_Code_Samples#sysfs
	- no special dependencies
	- uses include file loader_util.h

2) WiringPI   http://wiringpi.com/
	
3) pigpio      https://abyz.me.uk/rpi/pigpio/
               https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library


4) Direct Register Access (no implemented)  https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access

see also : https://www.bigmessowires.com/2018/05/26/raspberry-pi-gpio-programming-in-c/

To Do
-----
1) Support loading intel hex format files (needs to be able to load at addresses other than 0x0000
2) Impleent other gpio access methods like WiringPi or direct register access
3) Allow any interface methods to be selected at run time rather than compile time



===========
1802scan
===========
- scans the front panel LED display port and reports on changes

usage :   sudo 1802scan

- uses sysfs interface to gpio's


===========
gpio_get
===========
- reads status of gpio pin connected to membership card interface connector
- useful for hardware testing

usage :   sudo gpio_get <pin_name>  - where pin_name is one of [ mux wait clear ef4 d0 d1 d2 d3 d4 d5 d6 d7 ]

===========
gpio_set
===========
- set state of gpio pin connected to membership card interface connector
- useful for hardware testing

usage :   sudo gpio_set <pin_name> <0/1>   - where pin_name is one of [ mux wait clear ef4 d0 d1 d2 d3 d4 d5 d6 d7 ]

====================================================
Assorted 1802 Membership Card Demo and Test Programs
====================================================

1802_bit_bang_serial.asm  - routines to bit bang serial I/O at 9600 baud or 19200 baud for 1.8Mhz or 4 MHz Membership Cards
6digitDisplay.asm  - demo program for latest Membership Card front panel 6 x 7 digit LED display
demo.asm   - demo program to scroll the 8 front panel LED's
idle.asm    - demo program to slow blink the Q LED
qblink.asm   - another Q blinker program
serial_test.asm  - test program that echo the state of serial input ef line back to the serial output Q line
1802reg.asm  - small include file so that A18 assembler recognizes register names in source file


