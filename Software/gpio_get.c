
/// GPIO pin state set utility
//  - for PiLoader4 PCB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "loader_utils.h"

#include "loader_gpio.h"

#define DELAY 50

#define txd    14
#define rxd    15

int main( int argc, char *argv[])
{
   int pin = 0 ;
   int state = 0 ;
   int old_state = -5 ;

	// check for command line parameters

	pin_t pin_state ;

    if ( argc < 2 ) 
    { 
	   printf("Error : gpio pin name needed.\n");
	   printf("        pin names : [ mux wait clear ef4 d0 d1 d2 d3 d4 d5 d6 d7 ].\n");
	   exit(1) ;
    }

	if ( !strcmp( argv[1] , "rxd" ))   pin = rxd ;

	if ( pin == 0 )
	{
		printf("\nError : unknow pin %s \n", argv[1] );
		exit(1) ;
	}

    // open pin file handle as output mode

  	pin_state = pinopen( pin, INPUT); // set pin as an output

	while(1)
	{
    	state = pinread( pin_state );     // set pin state high so front panel switches will work

		if( old_state != state )
		{
			printf("\n gpio pin %d [ %s ] state=%d \n", pin, argv[1], state );
			old_state = state ;
		}
		sleep(1) ;
	}

	// and finally close all the file handle used to control the pin

   	pinclose( pin_state ); // all done with this pin

}
