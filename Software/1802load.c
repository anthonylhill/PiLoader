//***********************************************************
//
// 1802 Membership Card Program Loader -   Revision 3.2
//
//  - for PiLoader4 PCB
//  - rev 3.2 merged Nick D's mods to support non-zero load and run address
//
//  Released under terms of the GPL-3.0 license
//
//
// WARNING : loading overwrites all memory location below the load address with 0x00
//
//***********************************************************

// TODO
// =====
//   - allow loading from Intel HEX format files 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <pigpio.h>
#include "loader_gpio.h"

#define HIGH (1)
#define LOW  (0)

// a short delay between bytes loaded - should be possible to reduce this for large files
#define DELAY 50

// bit mask for byte to GPIO bit I/O
unsigned char mask[] = { 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 } ;


// routine to output a byte to the GPIO lines connected to data bus and initiate a DMA in cycle via EF4

void set_data_pins( unsigned char data_byte )
{
    char value ;
    int pin ;

    for ( pin = 0 ; pin < 8 ; pin++ )
    {
        value = ( data_byte & mask[pin] ) ? HIGH : LOW ;
        gpioWrite( pins[pin], value );
    }
    gpioWrite( pins[ef4_pin], LOW );
    gpioDelay(20) ;
    gpioWrite( pins[ef4_pin], HIGH );
    gpioDelay(20) ;
}


//=================================================================================================
//
// Program Start
//
//=================================================================================================

int main( int argc, char *argv[])
{
   FILE * fptr;
   char * filename ;
   char * endptr;
   uint byte_count = 0 ;
   char data_byte;
   int pin ;
   uint load_addr = 0x0000 ;
   uint exec_addr = 0x0000 ;
   bool load_n_go = false ;
   bool verbose = false ;


    // scan command line for file name, load address, execute address, and switches

    int param = 1 ;

    for ( int i = 1 ; i < argc ; i++ )
    {

        // handle any command line switchs firt 

        if (strcmp(argv[i], "-h") == 0)
        {
            printf("\nUsage: 1802load -h                  (prints this help)\n");
            printf("       1802load file                (loads file at 0x0000, runs at 0x0000)\n");
            printf("       1802load file 4000           (loads file at 0x4000, runs in a loop at 0x0000)\n");
            printf("       1802load file 4000 4020      (loads file at 0x4000, runs at 0x4020)\n");
            printf("       add -v to command line for verbose mode\n");
            printf("       add -g to command line to execute at load address\n\n");
            exit(1) ;
        }

        if (strcmp(argv[i], "-v") == 0)
        {
            verbose = true ;
            continue ;
        }

        if (strcmp(argv[i], "-g") == 0)
        {
            load_n_go = true ;
            continue ;
        }

        // check for a valid binary file to be loaded

        if ( param == 1 )
        {

            filename = argv[i]  ;

            // open the binary file to be loaded

            fptr=fopen( filename, "rb" );

            if (!fptr)
            {
                printf("\nError : unable to open file %s \n\n", filename );
                exit(1) ;
            }

            param++ ;
            continue ;
        }

        // check for a load address (override default load at 0x0000 )

        if ( param == 2 )
        {
            load_addr = strtol(argv[i], &endptr, 16);
            param++ ;
            continue ;
        }   

        // check for an execution address (override start at load address)

        if ( param == 3 )
        {
            exec_addr = strtol(argv[i], &endptr, 16);
            param++ ;
            continue ;
        }

    }

    // validate and adjust resulting configuration

    if ( param == 1 )                               // check for a file to load
    { 
        printf("\nError : filename needed. Use -h for help.\n\n");
	    exit(1) ;
    }

    if ( verbose )
    {
        printf("\nLoading code from file : %s", filename );

        if ( param == 2 )                               // check load address
        {
            printf("\nNo load address specified. Defaulting to 0x0000");
        }

        if ( param == 3 )                               // check execute address
        {       
            printf("\nLoad address set to 0x%4.4X", load_addr);
            if ( load_n_go ) printf("\nNo execution address specified. Command line switch -g forces run at load address 0x%4.4X.", load_addr ) ;
            else printf("\nNo execution address specified. Defaulting to loop to self at 0x0000");
        }
    
        if ( param == 4 )                               // if execution address is specified then make sure load_n_go is enabled
        {
            printf("\nLoad address set to 0x%4.4X", load_addr);
            printf("\nExecution address set to 0x%4.4X", exec_addr);
        }
        
    }

    if (param == 2 ) load_n_go = false ;        // no load address specified so it will default to 0x0000 and load_n_go must be false

    if ((param == 3) && (load_n_go == true ))   // if no execute address was specified but load_n_go was requested set execute address to load address
    {
        exec_addr = load_addr ;
    }

    if (param == 4)                             // if an execute address was specifed then make sure a jump instruction can be safely added
    {
        load_n_go = true ;

        if ((load_addr >= 0) && (load_addr < 5) && (load_n_go == true))   // make sure to not add the five bytes to load and go if load_addr is 0000
        {   
            load_n_go = false ;
            exec_addr = 0x0000 ;
            printf("\nWarning : execution address ignored for programs loaded at 0x0000.") ;
        }
    }

    if (load_n_go)
    {
        if ( verbose ) printf("\nAdding jump code to address 0x%4.4x to memory at 0x0000", exec_addr);
    }


    // set all pins to output mode, pull ups enabled, state = high

	int elements = sizeof(pins)/sizeof(pins[0]) ;

    if (gpioInitialise() < 0 )
    {
        printf("\nERROR :  unable to initialize GPIO access\n");
        exit(1) ;
    }

    if ( verbose ) printf("\nSetting all GPIO pins to output mode, pull ups enabled, state = high");
    for ( pin=0 ; pin < elements ; pin++ )
    {
	    gpioSetMode( pins[pin], PI_OUTPUT );                        // set GPIO as output
	    gpioSetPullUpDown(pins[pin], PI_PUD_UP );                   // set internal resistor to pull up mode
    	gpioWrite( pins[pin], HIGH);                                      // set all pin states high
        if ( verbose) printf("\n %02d %02d %02d", pins[pin], pin, HIGH);
    }


   // set mux to override panel switches

    gpioWrite( pins[mux_pin], LOW);      // set MUX pin state low to override front panel switches
	gpioDelay( DELAY ) ;

   // set clear = 0  ( reset the 1802 - note : must do this with WAIT = 1 to go through RESET mode )

    gpioWrite( pins[clear_pin], LOW);  // set clear pin state low to reset 1802
	gpioDelay( DELAY ) ;

   // set wait = 0   ( puts 1802 into LOAD mode -  CLEAR = 0 as well )

    gpioWrite( pins[wait_pin], LOW);  // set wait pin state low to reset 1802
	gpioDelay( DELAY ) ;

   // set jump code at 0x0000 and then go to load address ...

    if ( load_addr > 0x000 )
    {
        if ( verbose) printf("\nPreloading memory at 0x0000 : ");
        int jmp_addr = 0;
        int i = load_addr ;
        while ( i-- > 0 )
        {
            switch( jmp_addr )
            {
                case 0 :
                    data_byte = 0x71 ;                            // DIS
                    break ;
                case 1 :
                    data_byte = 0x00 ;                            // PC = 0  SP = 0
                    break ;
                case 2 :
                    data_byte = 0xC0 ;                           // LBR
                    break ;
                case 3 :
                    data_byte = exec_addr >> 8  ;   // start address high byte
                    break ;
                case 4 :
                    data_byte = exec_addr & 0x00FF ;   // start address low byte
                    break ;
                default :
                    data_byte = 0x00 ;                          // memory fill byte 
                    break ;
            }

            if ( verbose && (jmp_addr < 5)) printf("%2.2X ",data_byte);
            jmp_addr++ ;
            set_data_pins( data_byte ) ;
        }
    }

   // pump the loaded program code to 1802, toggled in via ef4

    if ( verbose) printf("\nLoading program data at 0x%4.4X : ", load_addr );

    while ( fread(&data_byte,sizeof(data_byte),1,fptr) != 0 )
    {
        if ( verbose) printf("%X ",data_byte);
	    byte_count++ ;
        set_data_pins( data_byte ) ;
    }
    fclose(fptr);


  // all done so reset and then execute

    gpioWrite( pins[wait_pin], HIGH );  // put 1802 into RESET mode (as clear = 0 at this point )
	gpioDelay( DELAY ) ;
	gpioWrite( pins[clear_pin], HIGH ); // put 1802 into RUN mode   
	gpioDelay( DELAY ) ;
    gpioWrite( pins[mux_pin], HIGH );    // set mux to override panel switches
	gpioDelay( DELAY ) ;

    printf("\nProgram %s loaded successfully at 0x%4.4X. Byte count = %d. Restarting 1802 at 0x%4.4X\n\n", filename, load_addr, byte_count, exec_addr);

// and finally close gpio access to pins

	gpioTerminate();

}
