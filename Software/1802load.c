
/// 1802 Program Loader
//  - for PiLoader4 PCB


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>

#include "loader_gpio.h"

#define HIGH (1)
#define LOW  (0)
#define PIN_WRITE( PIN, VALUE ) gpioWrite( pins[PIN], VALUE ) ;

#define GDELAY( USEC ) gpioDelay( USEC )
#define DELAY 50

unsigned char mask[] = { 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 } ;


int main( int argc, char *argv[])
{
   FILE *fptr;
   char data_byte;
   uint pin ;
   uint byte_count = 0 ;
   unsigned char value ;

	// check for command line parameters

    if ( argc <2 ) 
    { 
	   printf("Error : filename needed.\n");
	   exit(1) ;
    }

    fptr=fopen( argv[1], "rb" );

    if (!fptr)
    {
        printf("Error : unable to open file %s \n", argv[1] );
        exit(1) ;
    }

    // set all pins to output mode

	int elements = sizeof(pins)/sizeof(pins[0]) ;
	// printf("\n elements = %d\n", elements ) ;
    // printf("\n Loading : %s\n[ ", argv[1]);
    printf("\nLoading : %s\n", argv[1]);

	if (gpioInitialise() < 0 )
	{
		printf("\nERROR :  unable to initialize GPIO access\n");
		exit(1) ;
	}	
    for ( pin=0 ; pin < elements ; pin++ )
    {
	 // printf("%d ", pin);
		gpioSetMode( pins[pin], PI_OUTPUT );       // set GPIO as output
		gpioSetPullUpDown(pins[pin], PI_PUD_UP );  // set internal resistor to pull up mode
    }

	// printf(" ]\n");

    // set all pins to HIGH state so membership card has control

    for ( pin=0 ; pin < elements ; pin++ )
    {
	//	printf(" %d %d %d\n", pins[pin], pin, HIGH);
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

    printf("\nprogram data : ");

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

	printf("\n\n%d byte program loaded\n\n", byte_count );

  // all done so reset and then run

    PIN_WRITE( wait_pin, HIGH );  // put 1802 into RESET mode (as clear = 0 at this point )
	GDELAY( DELAY ) ;
	PIN_WRITE( clear_pin, HIGH ); // put 1802 into RUN mode   
	GDELAY( DELAY ) ;
    PIN_WRITE( mux_pin, HIGH );    // set mux to override panel switches
	GDELAY( DELAY ) ;


// and finally close gpio access to pins

	gpioTerminate();

}
