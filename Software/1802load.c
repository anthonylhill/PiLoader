
//***********************************************************
//
// 1802 Membership Card Program Loader
//
//  - for PiLoader4 PCB
//  - rev 3.2 merged Nick D's mods to support non-zero load and run address
//  - rev 3.5 added -x switch for execute only
//  - rev 4 added helper code so that program loads withoug overwriting memory below it
//  - rev 4.3 added additional command line switches for "load only",  "reset & run only" and "halt only"
//  - rev 4.4 added user confirmation with command line switch disable ( -y )  and more input validation
//
//  Released under terms of the GPL-3.0 license
//
//***********************************************************

#define VERSION  4
#define REVISION 4

#define DEBUG false

// TODO
// =====
//   - support loading from Intel HEX format files
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <pigpio.h>
#include "loader_gpio.h"

#define HIGH (1)
#define LOW  (0)

// a short delay between GPIO state changes - chould be possible to reduce this for large files
//  - note that CLEAR pulls down an RC network when LOW but a diode stops it from recharging the RC so extra delay is needed
#define PIN_DELAY         50
#define CONTROL_DELAY   1000
#define CLEAR_DELAY   250000


// bit mask for byte to GPIO bit I/O
unsigned char mask[] = { 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 } ;


// command line switch selected modes

bool verbose_mode = true ;        // console output verbose or mostly quiet ?
bool execute_only_mode = false ;  // no file load, run at 0x0000 ir the specified execution address
bool load_only_mode = false ;     // load binary file but leave processor in RESET mode  (no helper code added at 0x0000 )
bool confirm_mode = true ;        // check for user confirmation to proceed after verbose mode give the actions that will happen

// partially initialized data buffers for assembled code to be sent via DMA to the 1802 - note that code writes address info into these at runtime

    char dma_buffer[] =  { 0x71, 0x00,                  // 0000 DIS $00
                           0x90, 0xB3,                  // 0002 GHI R0 ,  PHI R3
                           0x80, 0xA3,                  // 0004 GLO R0 ,  PLO R3
                           0x13,                        // 0006 INC R3
                           0xD3,                        // 0007 SEP R3
                           0xF8, 0x00,                  // 0008 LDI <load address high byte>
                           0xB0,                        // 000A PHI R0
                           0xF8, 0x00,                  // 000C LDI <load address low byte>
                           0xA0,                        // 000D PLO R0
                           0x00  };                     // 000E IDL

    char jump_buffer[] = { 0x71, 0x00,                  // DIS $00
                           0xC0, 0x00, 0x00 } ;         // LBR <address>


// routine to set the state of a GPIO pin and then delay

void set_gpio( int pin_name , int state , int delay)
{
    gpioWrite( pins[pin_name] , state );
    gpioDelay( delay ) ;
}


// routine to output a byte to the GPIO lines connected to data bus and initiate a DMA in cycle via EF4

void set_data_pins( unsigned char data_byte )
{
    char value ;
    int pin ;

    for ( pin = 0 ; pin < 8 ; pin++ )
    {
        value = ( data_byte & mask[pin] ) ? HIGH : LOW ;
        set_gpio( pin, value , 0 );
    }
    gpioDelay( PIN_DELAY ) ;
    set_gpio( ef4_pin, LOW,  PIN_DELAY );
    set_gpio( ef4_pin, HIGH, PIN_DELAY );
}


// routine to DMA output a buffer to the GPIO lines

void dma_transfer( char * bptr , int length )
{
    char * jptr = bptr ;
    while( jptr != bptr + length )
    {
        if ( verbose_mode ) printf("%2.2X ", *jptr);
        set_data_pins( *jptr ) ;
        jptr++ ;
    }
}


// routine to setup the GPIO pins

void set_GPIOs()
{
     int pin ;

    // set all pins to output mode, pull ups enabled, state = high

    int elements = sizeof(pins)/sizeof(pins[0]) ;

    if (gpioInitialise() < 0 )
    {
        printf("\nERROR :  unable to initialize GPIO access\n");
        exit(1) ;
    }

    if ( verbose_mode ) printf("\n - setting all GPIO pins to output mode, pull ups enabled, state = high");

    for ( pin=0 ; pin < elements ; pin++ )
    {
        gpioSetMode( pins[pin], PI_OUTPUT );                        // set GPIO as output
        gpioSetPullUpDown(pins[pin], PI_PUD_UP );                   // set internal resistor to pull up mode
        if ( pin< wait_pin )                                        // set all pin states high except mux, clear, wait
        {
            set_gpio(pin, HIGH, 0 );
            if ( DEBUG ) printf("\n %02d %02d %02d", pins[pin], pin, HIGH);
        }
    }
    gpioDelay( CONTROL_DELAY ) ;
}


// routine to put Membership Card 1802 into a RESET state

void set_1802_halt()
{
    set_GPIOs() ;
    set_gpio( clear_pin, LOW,  PIN_DELAY);
    set_gpio( wait_pin,  HIGH, PIN_DELAY);
    set_gpio( mux_pin, LOW, PIN_DELAY);      // set MUX pin state low to override front panel switches
    set_gpio( wait_pin, LOW, PIN_DELAY);  // set wait pin state low to reset 1802
}

// routine to valid a 1 to 4 digit hex input

int validate_hex(const char* hex){
    int count = 0 ;
    while(*hex != 0)
    {
        if( isxdigit(*hex) == false ) return 0;
        if ( ++count > 4 ) return 0 ;
        hex++ ;
    }
    return 1;
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
   uint load_addr = 0x0000 ;
   uint exec_addr = 0x0000 ;


    printf("\nPi Membership Card Loader %d.%d", VERSION , REVISION ) ;

    // scan command line for file name, load address, execute address, and switches

    int param = 1 ;

    for ( int i = 1 ; i < argc ; i++ )
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            printf("\n\nUsage:  1802load [switches] <file name> <load address> <execute address>\n\n");
            printf("  where :\n");
            printf("       <file name>        - binary file to load\n");
            printf("                          - not needed with -x or -s switches\n");
            printf("       <load address>     - address to load binary data\n");
            printf("                          - default = 0x0000 if not given\n");
            printf("       <execute address>  - address to jump to after load\n");
            printf("                          - default = <load address> if not given\n\n");
            printf("  switches:\n");
            printf("       -h prints this help\n");
            printf("       -q quiet mode, verbose output disabled\n");
            printf("       -y confirm mode, defaults user answers to Yes\n");
            printf("       -x execute only, no file loaded\n");
            printf("       -l load only, processor stays in RESET mode\n");
            printf("       -s stop (halt) only\n\n");
            printf("  examples:\n");
            printf("        1802load <file>             - loads file at 0x0000, runs at 0x0000\n");
            printf("        1802load <file> 4000        - loads file at 0x4000, runs at 0x4000\n");
            printf("        1802load <file> 4000 4020   - loads file at 0x4000, runs at 0x4020\n");
            printf("        1802load -l <file> 4000     - loads file at 0x4000, processor left halted\n");
            printf("        1802load -x 4030            - no file loaded, runs at 0x4030 via LBR at 0x0000\n");
            printf("        1802load -s                 - no file loaded, processor halted\n\n");
            printf("  warning : first 5 to 15 memory bytes overwritten when using non-zero load or execute address\n\n");
            exit(0) ;
        }


        if (strcmp(argv[i], "-q") == 0)
        {
            verbose_mode = false ;
            continue ;
        }

        if (strcmp(argv[i], "-y") == 0)
        {
            confirm_mode = false ;
            continue ;
        }

        if (strcmp(argv[i], "-s") == 0)         // put processor into RESET mode at specified address without loading any binary data from file
        {
            verbose_mode = false ;
            printf("\nnSetting 1802 to RESET state.  No binary file loaded\n\n");
            set_1802_halt() ;
            exit(0) ;
        }

        if (strcmp(argv[i], "-x") == 0)         // put processor into RUN mode at specified address without loading any binary data from file
        {
            execute_only_mode = true ;
            continue ;
        }

        if (strcmp(argv[i], "-l") == 0)        // load binary data at specified address and leave processor in RESET mode
        {
            load_only_mode = true ;
            continue ;
        }

        // get name of binary file to be loaded
        if ( param == 1 )
        {
            filename = argv[i]  ;
            param++ ;
            continue ;
        }

        // check for a load address (overrides default load at 0x0000 )
        if ( param == 2 )
        {
            if  ( validate_hex(argv[i]) == 1 ) load_addr = strtol(argv[i], &endptr, 16);
            else
            {
                printf("\n\nError : invalid load address\n\n");
                exit(1) ;
            }
            param++ ;
            continue ;
        }

        // check for an execution address (overrides start at load address)
        if ( param == 3 )
        {
            if  ( validate_hex(argv[i]) == 1 ) exec_addr = strtol(argv[i], &endptr, 16);
            else
            {
                printf("\n\nError : invalid execute address\n\n");
                exit(1) ;
            }
            param++ ;
            continue ;
        }
    }

    // validate resulting configuration

    if ( param != 4 )                                   // execute address = load address if not specified
    {
        exec_addr = load_addr;
    }

    if (( load_only_mode == true) && ( execute_only_mode == true ))
    {
        printf("\nError : -x and -l switches cannot both be selected.\n\n");
        exit(1) ;
    }

    if ( execute_only_mode == true )                        // don't bother opening a file in execute_only_mode
    {
        if ( param == 2 )                                   // load address is in filename buffer  ?
        {
            if  ( validate_hex(filename) == 1 ) exec_addr = strtol(filename, &endptr, 16);
            else
            {
                printf("\nError : invalid execute address\n\n");
                exit(1) ;
            }
        }

    }
    else
    {
        if ( param == 1 )                                   // check if a file name was possibly specified
        {
            printf("\nError : filename needed. Use -h for help.\n\n");
            exit(1) ;
        }

        fptr=fopen( filename, "rb" );                       // open the binary file to be loaded

        if (!fptr)
        {
            printf("\nError : unable to open file %s \n\n", filename );
            exit(1) ;
        }
    }

    // let the user know what we are going to do if verbose mode active

    if ( verbose_mode )
    {
        if ( execute_only_mode == true )
        {
            printf("\n - execute only mode selected, no file to be loaded. Execution address = 0x%4.4X", exec_addr ) ;
        }
        else
        {
            printf("\n - loading code from file : %s", filename );

            if ( load_only_mode == false )
            {
				if ( param == 2 )                               // check for a  load address
				{
					printf("\n - no load or execute address specified. Defaulting to 0x0000");
				}
				else printf("\n - load address set to 0x%4.4X", load_addr);
				
                if ( param == 3 )                               // check for an execute address
                {
                    printf("\n - no execution address specified. Defaulted to run at load address 0x%4.4X.", exec_addr ) ;
                }

                if ( param == 4 )                               // if execution address is specified then make sure M_load_n_go is enabled
                {
                    printf("\n - execution address set to 0x%4.4X", exec_addr);
                    printf("\n - adding jump code to address 0x%4.4x to memory at 0x0000", exec_addr);
                }

                if (( load_addr == 0 ) && ( exec_addr != 0 ))
                {
                    printf("\n - warning : execution address 0x%4.4X requires overwrite of data loaded at 0x0000 to 0x0005", exec_addr);
                }
            }
			else 
			{
				printf("\n - load only mode selected");
				if ( param == 2 )                               // check for a  load address
				{
					printf("\n - no load address specified. Defaulting to 0x0000");
				}
				else printf("\n - load address set to 0x%4.4X", load_addr);				
			}
		}

        if ( confirm_mode == true )
        {
            char choice = 'n' ;
            printf("\n\nWould you like to continue? (Y/n): ");	
			scanf("%c",&choice) ;
            if ( (choice != 'y' ) && (choice != 'Y') && (choice != 0x0A) && (choice != 0x0D) ) 
			{
				printf("\n") ;
				exit(0) ;
			}
        }

    }


    /**************************************************************/
    /** Configure Raspberry Pi GPIO interface to Membership Card **/
    /**************************************************************/

    set_GPIOs() ;

   // preset CLEAR and LOAD pins prior to using mux to override panel switches so that 1802 is put into RESET state

    set_gpio( clear_pin, LOW,  PIN_DELAY);
    set_gpio( wait_pin,  HIGH, PIN_DELAY);

    // switch control from front panel to Raspberry Pi GPIOs

    set_gpio( mux_pin, LOW, PIN_DELAY);      // set MUX pin state low to override front panel switches

   // set wait = 0 to put the 1802 into LOAD mode. Note : must do this with WAIT = 1 , CLEAR = 0 to go through RESET mode first

    set_gpio( wait_pin, LOW, PIN_DELAY);  // set wait pin state low to reset 1802

    if( execute_only_mode == false )
    {
        if ( load_addr > (sizeof(dma_buffer)/sizeof(dma_buffer[0])))
        {
           // load helper code to set R0 to the DMA load address selected
            if ( verbose_mode ) printf("\n - preloading helper code at 0x0000 : ");
            dma_buffer[9] =  load_addr >> 8 ;
            dma_buffer[12] = load_addr  & 0x00FF ;
            dma_transfer( dma_buffer , (sizeof(dma_buffer)/sizeof(dma_buffer[0])));


            // run the helper code to load R0

            if ( verbose_mode ) printf("\n - running preloaded code.");

            gpioDelay( CONTROL_DELAY ) ;
            set_gpio( wait_pin,  HIGH, CONTROL_DELAY );  // put 1802 into RESET mode (as WAIT = 0 at this point)
            set_gpio( clear_pin, HIGH, CLEAR_DELAY );    // put 1802 into RUN mode again (no reset so R0 stays at load address)
            gpioDelay( 100000 ) ;                        // give loaded code alot of time to complete
            set_gpio( wait_pin,  LOW,  CONTROL_DELAY );  // put 1802 into PAUSE mode
            set_gpio( clear_pin, LOW,  CONTROL_DELAY );  // put 1802 into LOAD mode again (no reset so R0 stays at load address);
        }

       // pump the loaded program code to 1802, toggled in via ef4

        if ( verbose_mode) printf("\n - loading program data at 0x%4.4X : ", load_addr );

        int  char_count = 33 ;

        while ( fread(&data_byte,sizeof(data_byte),1,fptr) != 0 )
        {
            if ( verbose_mode )
            {
                if (++char_count > 31)
                {
                    printf("\n    ");
                    char_count = 0 ;
                }
                printf("%2.2X ",data_byte);
            }

            byte_count++ ;
            set_data_pins( data_byte ) ;
        }

       // toggle WAIT to 1 and back to 0   ( resets 1802 and puts it back into LOAD mode -  CLEAR = 0 as well )

        gpioDelay( CONTROL_DELAY ) ;
        set_gpio( wait_pin, HIGH, CONTROL_DELAY );  // set wait pin state high to reset 1802
        set_gpio( wait_pin, LOW,  CONTROL_DELAY );   // set wait pin state low to go back into LOAD mode
    }

    if ( load_only_mode )
    {
        // leave the mux pointed at the Raspberry PI GIOs and the 1802 in RESET mode
        set_gpio( wait_pin,  HIGH, PIN_DELAY );     // put 1802 into RESET mode (as clear = 0 at this point)
    }
    else
    {
       // load the jump address for the code we just loaded

        if ( exec_addr > 0 )
        {
            if ( verbose_mode ) printf("\n - postloading jump code at 0x0000 : ");
            jump_buffer[3] = exec_addr >> 8 ;
            jump_buffer[4] = exec_addr & 0x00FF ;
            dma_transfer( jump_buffer , (sizeof(jump_buffer)/sizeof(jump_buffer[0]))) ;
            if ( verbose_mode ) printf("\n - running postloaded jump code. ");
        }

      // all done so reset and then execute

        gpioDelay( CONTROL_DELAY ) ;
        set_gpio( wait_pin,  HIGH, PIN_DELAY );     // put 1802 into RESET mode (as clear = 0 at this point)
        set_gpio( clear_pin, HIGH, PIN_DELAY );     // put 1802 into RUN mode
        set_gpio( mux_pin,   HIGH, PIN_DELAY );     // set mux pin high to stop override panel switches
    }



    if ( execute_only_mode )    printf("\nNo program loaded. Restarting 1802 at 0x%4.4X\n\n", exec_addr);
    else if ( load_only_mode )  printf("\nProgram %s loaded successfully at 0x%4.4X. Byte count = %d. 1802 in RESET mode\n\n", filename, load_addr, byte_count);
         else                   printf("\nProgram %s loaded successfully at 0x%4.4X. Byte count = %d. Restarting 1802 at 0x%4.4X\n\n", filename, load_addr, byte_count, exec_addr);

    // and finally close gpio access to pins

    gpioTerminate();
	
	exit(0) ;

}
