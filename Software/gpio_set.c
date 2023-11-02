
/// GPIO pin state set utility
//  - for PiLoader4 PCB
//  - uses fs access to gpio instead of pigpio.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "loader_gpio.h"

#define DELAY 50

// Pin modes:
#define INPUT (0)
#define OUTPUT (1)
#define LOW (0)
#define HIGH (1)

typedef struct {
        int     pin;
        char*   fn;
} pin_t;

static pin_t pinopen(int pin, int mode);
static void  pinclose(pin_t pin);
static void  pinwrite(pin_t pin, int value);

pin_t pinopen(int pin, int mode)
{
        char*   pinfn = malloc(1024);
        char    dirfn[1024];
        FILE*   dir = NULL;
        FILE*   fp = fopen("/sys/class/gpio/export", "w");
        if ( fp == NULL )
        {
            printf("\n** Error : fopen failed for gpio write access on pin %d, mode %d \n", pin, mode ) ;
            exit(0) ;
        }
        fprintf(fp, "%d", pin);
        fclose(fp);
        snprintf(dirfn, 1024, "/sys/class/gpio/gpio%d/direction", pin);
        snprintf(pinfn, 1024, "/sys/class/gpio/gpio%d/value", pin);
        while (dir == NULL) {
                dir = fopen(dirfn, "w");
        }
        if (mode == INPUT) {
                fprintf(dir, "in");
        } else {
                fprintf(dir, "out");
        }
        fclose(dir);
        return (pin_t) { pin, pinfn };
}

void pinclose(pin_t pin)
{
        FILE*   fp = fopen("/sys/class/gpio/unexport", "w");
        fprintf(fp, "%d", pin.pin);
        fclose(fp);
        free(pin.fn);
}

void pinwrite(pin_t pin, int value)
{
        FILE*   fp = fopen(pin.fn, "w");
        if (value == LOW) {
                fprintf(fp, "0");
        } else {
                fprintf(fp, "1");
        }
        fclose(fp);
}


int main( int argc, char *argv[])
{
   int pin = 0 ;
   int state = 0 ;

	// check for command line parameters

	pin_t pin_state ;

    if ( argc <3 ) 
    { 
	   printf("Error : gpio pin name and 0/1 state needed.\n");
	   printf("        pin names : [ mux wait clear ef4 d0 d1 d2 d3 d4 d5 d6 d7 load clock ].\n");
	   exit(1) ;
    }

	if ( !strcmp( argv[1] , "mux" ))    pin = mux ;
	if ( !strcmp( argv[1] , "wait" ))   pin = wait ;
	if ( !strcmp( argv[1] , "clear" ))  pin = clear ;
	if ( !strcmp( argv[1] , "ef4" ))    pin = ef4 ;
	if ( !strcmp( argv[1] , "d0" ))     pin = data0 ;
	if ( !strcmp( argv[1] , "d1" ))     pin = data1 ;
	if ( !strcmp( argv[1] , "d2" ))     pin = data2 ;
	if ( !strcmp( argv[1] , "d3" ))     pin = data3 ;
	if ( !strcmp( argv[1] , "d4" ))     pin = data4 ;
	if ( !strcmp( argv[1] , "d5" ))     pin = data5 ;
	if ( !strcmp( argv[1] , "d6" ))     pin = data6 ;
	if ( !strcmp( argv[1] , "d7" ))     pin = data7 ;
    if ( !strcmp( argv[1] , "sdl" ))    pin = shift_register_load ;
    if ( !strcmp( argv[1] , "sdc" ))    pin = shift_register_clock ;
    if ( !strcmp( argv[1] , "sdd" ))    pin = shift_register_data ;
	if ( pin == 0 )
	{
		printf("\nError : unknow pin %s \n", argv[1] );
		exit(1) ;
	}

	state = atoi( argv[2] ) ? HIGH : LOW ;

    // open pin file handle as output mode

  	pin_state = pinopen( pin, OUTPUT); // set pin as an output

    // pin to required state

    pinwrite( pin_state, state );     // set pin state high so front panel switches will work

	printf("\n gpio pin %d [ %s ] set to %d \n", pin, argv[1], state );

	// and finally close all the file handle used to control the pin

   	pinclose( pin_state ); // all done with this pin

}
