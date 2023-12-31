=============================================================================
Software for the Raspberry Pi Zero Loader Card for Lee Hart's Membership Card
=============================================================================
version 1.0   2023/10/20
version 3.2   2023/12/04 - improved 1802load executable
version 3.5   2023/12/05 - added -x switch to skip load and just execute at load/execute address
version 4.4   2023/12/24 - added -l switch to load code but not execute, and added verbose mode user confirmation


=============
 makefile
=============
    - standard make file for building Loader C and 1802 Assembler files


=============
 1802load
=============
     - loads a binary file to 1802 memory

    usage :   sudo 1802load filename [switches] <file name> <load_address> <execute_address>

    - filename is a binary file to be loaded
    - load_address is an optional hex memory address to load the file.  Default at 0x0000
    - execute_address is an optional hex run address to use after the file is loaded.  Default is the load address

    switches are :
            -h for help
            -q for quiet non-verbose output
            -y default user prompt answers to Yes
            -x execute only, no file loaded
            -l load only, processor stays in HALT mode
            -s stop (halt) only

    examples:
    1802load <file>             - loads file at 0x0000, runs at 0x0000
    1802load <file> 4000        - loads file at 0x4000, runs at 0x4000
    1802load <file> 4000 4020   - loads file at 0x4000, runs at 0x4020
    1802load -l <file> 4000     - loads file at 0x4000, processor left halted
    1802load -x 4030            - no file loaded, runs at 0x4030 via LBR at 0x0000
    1802load -s                 - no file loaded, processor halted

    NOTES & WARNINGS :
        - switches can be used anywhere in the command line after 1802load  (space delimited)
        - using the 1802load program to execute code anywhere other than 0x000 overwrites the first 5 bytes of memory at 0x000
        - using the 1802load program to load code anywhere other than 0x000 overwrite the first 15 bytes of memory at 0x000
        - the -y, -x , -s and -l switches can be used effectively in batch files (Bash scripts) to load multiple files with the 1802 halted

===========
1802scan
===========
    - scans the front panel LED display port and reports on changes

    usage :   sudo 1802scan



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

bit_bang_serial.asm  - routines to bit bang serial I/O at 9600 baud or 19200 baud for 1.8Mhz or 4 MHz Membership Cards
6digitDisplay.asm  - demo program for latest Membership Card front panel 6 x 7 digit LED display
demo.asm   - demo program to scroll the 8 front panel LED's
idle.asm    - demo program to slow blink the Q LED
qblink.asm   - another Q blinker program
serial_test.asm  - test program that echo the state of serial input ef line back to the serial output Q line
1802reg.asm  - small include file so that A18 assembler recognizes register names in source file


====================================================
GPIO Interface Methods
====================================================

- currently only pigpio is supported and tested. Use the others with care.

1) pigpio      https://abyz.me.uk/rpi/pigpio/
               https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library

2) sysfs interface    https://www.ics.com/blog/gpio-programming-using-sysfs-interface
                      https://elinux.org/RPi_GPIO_Code_Samples#sysfs
    - no special dependencies
    - uses include file loader_util.h

3) WiringPI   http://wiringpi.com/

4) Direct Register Access (no implemented)  https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access

see also : https://www.bigmessowires.com/2018/05/26/raspberry-pi-gpio-programming-in-c/