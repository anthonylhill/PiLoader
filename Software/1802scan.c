
/// Scanner for  1802 Parrallel Port Output
//  - for PiLoader4 PCB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>

#include "loader_gpio.h"

#define HIGH (1)
#define LOW  (0)

unsigned char mask[] = { 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 } ;

int main( int argc, char *argv[])
{
	int bit[8] ;
    char pch ;
    int switches, old_switches =0x100 ; 


	printf("\n 1802 Scan : PIGPIO Version \n") ;


	// check for command line parameters

    if ( argc > 1 ) 
    { 
	   printf("Error : no parameters needed.\n");
	   exit(1) ;
    }

    // set shift register control pins to output mode

	if (gpioInitialise() < 0 )
	{
		printf("\nERROR :  unable to initialize GPIO access\n");
		exit(1) ;
	}
	
	    printf("\nShift Register Data :");
	
		gpioSetMode(       shift_register_load,  PI_OUTPUT );  // set GPIO as output
		gpioSetPullUpDown( shift_register_load,  PI_PUD_UP );  // set internal resistor to pull up mode
		gpioSetMode(       shift_register_clock, PI_OUTPUT );  // set GPIO as output
		gpioSetPullUpDown( shift_register_clock, PI_PUD_UP );  // set internal resistor to pull up mode

		gpioSetMode(       shift_register_data, PI_INPUT );  // set GPIO as input

    // set control pins to required default state
    	gpioWrite( shift_register_load, HIGH); // shift register load pin = HIGH
    	gpioWrite( shift_register_clock, LOW); // shift register clock pin = LOW

		while ( 1 )
		{
            switches = 0 ;

    	// load shift register
        	gpioWrite( shift_register_load, LOW);
        	gpioWrite( shift_register_load, HIGH);

        // clock data in from shift register

		    for (int i=0 ; i< 8 ; i++ )
		    {
		    	bit[i] = gpioRead(shift_register_data) ;
                switches = switches*2 + bit[i] ;
    	    	gpioWrite( shift_register_clock, HIGH);
    	    	gpioWrite( shift_register_clock, LOW);
		    }

            if ( switches != old_switches )
            {
                if (( switches >= 0x20) && (switches < 0x7F ))
                {
                    pch = switches ;
                }
                else
                {
                    pch = 0x20 ;
                }
                for (int i=0 ; i< 8 ; i++ ) printf("%d ", bit[i] ) ;
		        printf("  %-2X %c \n", switches , pch ); 
                old_switches = switches ;
            }

            gpioDelay( 1000 ) ;
		}

	    gpioTerminate();

}
