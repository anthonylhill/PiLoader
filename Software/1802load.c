
//***********************************************************
//
// 1802 Membership Card Program Loader -   Revision 3.0
//
//  - for PiLoader4 PCB
//  - works with either native file mode access,  the PIGPIO library, or Wiring Pi (future)
//
//  Released under terms of the GPL-3.0 license
//
//***********************************************************

// TODO
// =====
//   - implement Wiring Pi version of GPIO control
//   - allow a command line option to load at a nonzero address
//   - allow loading from Intel HEX format files 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "loader_gpio.h"


//** Define which Raspberry Pi GPIO access method to use

#define FS_ACCESS 1
#define PIGPIO    2
#define WIRING_PI 3

//#define GPIO_METHOD FS_ACCESS
#define GPIO_METHOD PIGPIO
//#define GPIO_METHOD WIRING_PI

#if GPIO_METHOD == FS_ACCESS
#include "loader_utils.h"
#define PIN_WRITE( PIN, VALUE ) pinwrite( pin_state[PIN], VALUE )
#define GDELAY( USEC ) usleep( USEC )
#endif

#if GPIO_METHOD == PIGPIO
#include <pigpio.h>
#define HIGH (1)
#define LOW  (0)
#define PIN_WRITE( PIN, VALUE ) gpioWrite( pins[PIN], VALUE ) ;
#define GDELAY( USEC ) gpioDelay( USEC )
#endif

// a short delay between bytes loaded - should be possible to reduce this for large files

#define DELAY 50

// bit mask for byte to GPIO bit I/O

unsigned char mask[] = { 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 } ;

//=================================================================================================
//
// Program Start
//
//=================================================================================================

int main( int argc, char *argv[])
{
   FILE *fptr;
   char data_byte;
   uint pin ;
   uint byte_count = 0 ;
   unsigned char value ;

#if GPIO_METHOD == FS_ACCESS
    printf("\n 1802 Loader : File System Version ") ;
#elseif GPIO_METHOD == PIGPIO
    printf("\n 1802 Loader : PIGPIO Version ") ;
#else
    printf("\n 1802 Loader : WIRING_PI Version ") ;	
#endif

    // check for command line parameters

    if ( argc <2 )
    {
       printf("\nError : filename needed.\n");
       exit(1) ;
    }

    fptr=fopen( argv[1], "rb" );

    if (!fptr)
    {
        printf("\nError : unable to open file %s \n", argv[1] );
        exit(1) ;
    }

    // set all pins to output mode

    int elements = sizeof(pins)/sizeof(pins[0]) ;
    printf("\n elements = %d\n", elements ) ;
    printf("\n Loading %s\n[ ", argv[1]);



#if GPIO_METHOD == FS_ACCESS
    pin_t pin_state[ sizeof(pins)/sizeof(pins[0]) ] ;

    for ( pin=0 ; pin < elements ; pin++ )
    {
        printf("%d ", pin);
        pin_state[pin] = pinopen( pins[pin], OUTPUT); // set all used pins as an outputs
    }
#elseif GPIO_METHOD == PIGPIO	

    if (gpioInitialise() < 0 )
    {
        printf("\nERROR :  unable to initialize GPIO access\n");
        exit(1) ;
    }
    for ( pin=0 ; pin < elements ; pin++ )
    {
        printf("%d ", pin);
        gpioSetMode( pins[pin], PI_OUTPUT );       // set GPIO as output
        gpioSetPullUpDown(pins[pin], PI_PUD_UP );  // set internal resistor to pull up mode
    }
	
#else
// FIXME : need to implement Wiring Pi I/O code here
	   printf("\Warning : Wiring Pi gpio access not current supported.\n");
       exit(1) ;
#endif

    printf(" ]\n");

    // set all pins to HIGH state so membership card has control

    for ( pin=0 ; pin < elements ; pin++ )
    {
        printf(" %d %d %d\n", pins[pin], pin, HIGH);
        PIN_WRITE( pin, HIGH); // set all pin states high
    }


   // set mux to override panel switches

    PIN_WRITE( mux_pin, LOW);      // set MUX pin state low to override front panel switches
    GDELAY( DELAY ) ;

   // set clear = 0  ( reset the 1802 - note : must do this with WAIT = 1 to go through RESET mode )

    PIN_WRITE( clear_pin, LOW);  // set clear pin state low to reset 1802
    GDELAY( DELAY ) ;

   // set wait = 0   ( puts 1802 into LOAD mode -  CLEAR = 0 as well )

    PIN_WRITE( wait_pin, LOW);  // set wait pin state low to reset 1802
    GDELAY( DELAY ) ;

   // pump data out to 1802, toggled in via ef4

    printf("\n program data : ");

    while ( fread(&data_byte,sizeof(data_byte),1,fptr) != 0 )
    {
        printf("%X ",data_byte);

        for ( pin = 0 ; pin < 8 ; pin++ )
        {
            value = ( data_byte & mask[pin] ) ? HIGH : LOW ;
            PIN_WRITE( pin, value );
        }

        PIN_WRITE( ef4_pin, LOW );
        GDELAY(20) ;
        PIN_WRITE( ef4_pin, HIGH );
        GDELAY(20) ;

        byte_count++ ;
    }
    fclose(fptr);

    printf("\n %4d byte program loaded\n", byte_count );

  // all done so reset and then run

    PIN_WRITE( wait_pin, HIGH );  // put 1802 into RESET mode (as clear = 0 at this point )
    GDELAY( DELAY ) ;
    PIN_WRITE( clear_pin, HIGH ); // put 1802 into RUN mode
    GDELAY( DELAY ) ;
    PIN_WRITE( mux_pin, HIGH );    // set mux to override panel switches
    GDELAY( DELAY ) ;


// and finally close all the file handles used to control the pins

#if GPIO_METHOD == FS_ACCESS
    for ( pin=0 ; pin < sizeof(pins)/sizeof(pins[0]) ; pin++ )
    {
        pinclose( pin_state[pin]); // all done with this pin
    }
#elseif GPIO_METHOD == PIGPIO
    gpioTerminate();
#else
		// TODO : add WiringPi code here 
#endif

}
