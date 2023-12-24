
;=========================================================================
;
;  === 1802 bit bash serial I/O  -  9600 baud and 19200 baud === 
;
;  Test Code
;   - runs in either echo mode where incoming characters are simply echo'd
;     back or in dump mode that just repeatedly sends the printable ASCII
;     characters (with CR/LF's inserted)
;
;
; === Serial Bit Rates === 
;  9600 baud  =  1/9600 seconds per bit = 104 uSec per bit (936 uSec per 8 bit word & start bit but not stop bits)
;  19200 baud =  1/9600 seconds per bit =  52 uSec per bit ( 468 uSec per 8 bit work & start bit but not stop bits)
;
; === 1.8 MHz Clock === 
;  1802 clock 1.8 MHz = .555 uSec per cycle = 4.444 uSec per machine cycle 
;             =  8,888 uSec per 2 cycle instruciton or 
;             = 13.33 uSec per 3 cycle instruction
;  9600  : 104 uSec = 11.7 instructions per bit   -> =  10x2 cycle intruction and 1x3 cycle instruction (102.2 uSec)
;  19200 :  52 uSec =  5.8 instructions per bit   -> =  4x2 cycle intruction and 1x3 cycle instruction (48.8 uSec)
;                                                 -> =  6x2 cycle intruction (53.3 uSec)
;
; === 4.0 MHz Clock ===
;  1802 clock 4.0 MHz = .25 uSec per cycle = 2 uSec per machine cycle 
;     =  4 uSec per 2 cycle instruction or 
;     =  6 uSec per 3 cycle instruction
;  9600  : 104 uSec = 26.0 instructions per bit    -> = 26x2 cycle instructions
;  19200 :  52 uSec = 13.0 instructions per bit    -> = 13x2 cycle instructiongs
;
;
;==========================================================================

;CLOCK_MHZ EQU 4  ;  4.0 MHz
CLOCK_MHZ EQU   2  ;  1.8 MHz

;ECHO_MODE EQU 0    ; jam in transmit - ignore input
ECHO_MODE EQU 1  ; echo back what is received

; BAUD  EQU 9600     ; 
BAUD  EQU 19200  ; 

TX_DELAY EQU 0

; register equivalents for A18
R0      EQU 0
R1      EQU 1
R2      EQU 2
R3      EQU 3
R4      EQU 4
R5      EQU 5
R6      EQU 6
R7      EQU 7
R8      EQU 8
R9      EQU 9
RA      EQU 10
RB      EQU 11
RC      EQU 12
RD      EQU 13
RE      EQU 14
RF      EQU 15
r0      EQU 0
r1      EQU 1
r2      EQU 2
r3      EQU 3
r4      EQU 4
r5      EQU 5
r6      EQU 6
r7      EQU 7
r8      EQU 8
r9      EQU 9
ra      EQU 10
rb      EQU 11
rc      EQU 12
rd      EQU 13
re      EQU 14
rf      EQU 15


;***************************************************************************************************

 if BAUD = 9600  
 
;----------------------------------------------------------------------

        ORG 0000H                            ;
        DIS                                  ; disable interrupts / PC = R0  SP =  R0
        DB $00                               ; DIS instruction loads 0 to P and X

        SEQ                                  ;
        LDI high START                       ; initialize R3 as program counter
        PHI R3                               ;
        LDI low START                        ;
        PLO R3                               ;
        LDI high STACK                       ; initialize R2 as stack pointer
        PHI R2                               ;
        LDI low STACK                        ;
        PLO R2                               ;
        LDI high txchar                      ; initialize R7 as tx char subroutine
        PHI R7                               ;
        LDI low txchar                       ;
        PLO R7                               ;
        LDI high rxchar                      ; initialize R8 as rx char subroutine
        PHI R8                               ;
        LDI low rxchar                       ;
        PLO R8                               ;
        SEP R3                               ;
 
 IF ECHO_MODE = 0
;****************************************************************
;
; Dump printable ASCII characters sequentially to serial output
;    (with CR/LF's inserted)
;
;****************************************************************
 
START:  LDI $1F                              ;
        PHI RB
RESTART:
        LDI $7F - $20
        PLO RB
NEXTCHAR:
        GHI RB
        ADI $01
        PHI RB
        SMI $7F
        BNZ NO_WRAP
        LDI $21
        PHI RB
NO_WRAP:
        GHI RB
        SEP R7                               ; call tx char routing

 if TX_DELAY > 0
		ldi $10
		phi r9
		ldi $ff
		plo r9
delay:  dec r9
		ghi r9
		bnz delay
 endi

        DEC RB
        GLO RB
        BNZ NEXTCHAR
        LDI $0D
        SEP R7
        LDI $0A
        SEP R7
        LDI $08                              ;
        PHI RF                               ;
        LDI $00                              ;
        PLO RF                               ;
LOOP:   DEC RF                               ;
        GHI RF                               ;
        BNZ LOOP                             ;
        BR RESTART                           ;

 ELSE
 
;****************************************************************
;
; Wait for an input character and then echo it back
;
;****************************************************************

START:  SEP R8                               ;
        SEP R7                               ;
        BR START                             ;

 ENDI
 
;********************************************************** 9600 BAUD ******
; Transmit Byte via Q connected to RS232 driver
;  call with R3 being the previous PC
;  Byte to send in D
;  Returns with D unaffected
;  re.1 = D
;  Destroys RE
;----------------------------------------------------------------------

    sep r3              ; 10.5
txchar:                 ;
    phi re              ;
    ldi 9               ; 9  bits to transmit (1 start + 8 data)
    plo re              ;
    ghi re              ;
    shl                 ; set start bit
    rshr                ; DF=0
                        ;
txcloop:                ;
    bdf t1              ; 10.5   jump to seq to send a 1 bit
    req                 ; 11.5   send a 0 bit
    br  t2              ; 1      jump +5 to next shift
                        ;
t1: seq                 ; 11.5   send a 1 bit
    br t2               ; 1      jump +2 to next shift (pseudo NOP for timing)
                        ;
t2: rshr                ; 2      shift next bit to DF flag
    phi re              ; 3      save D in re.1
    dec re              ; 4      dec bit count
    glo re              ; 5      get bit count
    bz txcret           ; 6      if 0 then all 9 bits (start and data) sent
    ghi re              ; 7      restore D
    nop                 ; 8.5    pause 1/2 time

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
    nop    ; 6.0
    nop    ; 7.5
    nop    ; 9.0
    nop    ; 10.5
    nop    ;  
    nop    ; 12.0 
    nop    ; 14.5

; tuning
    nop      ;  tune this if necessary - use a GHI RE here to shorten ) 
;    ghi re  ;  4 uSec -> ( 52 uSec total addition)

 endi

    br txcloop          ; 9.5    loop back to send next bit
                        ;
txcret:                 ;
    ghi re              ; 7      nop
    ghi re              ; 8      nop
    ghi re              ; 9      nop
    nop                 ; 10.5
    seq                 ; 11.5 stop bit
    nop                 ; 1
    nop                 ; 2.5
    nop                 ; 4
    nop                 ; 5.5
    nop                 ; 7
    nop                 ; 8.5
    
 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
    nop    ; 6.0
    nop    ; 7.5
    nop    ; 9.0
    nop    ; 10.5
    nop    ; 12.0 
    nop    ; 14.5

; tuning
    nop      ;  tune this if necessary - use a GHI RE here to shorten ) 
;    ghi re  ;  4 uSec -> ( 52 uSec total addition)

 endi

    br txchar-1         ; 9.5

;********************************************************* 9600 BAUD *************
;rx_char
;  Receive Byte via EFe connected to RS232 receiver
;  Recieves 8 bits
;  call with R3 being the previous PC
;  Returns with Byte received in D and re.1
;  Destroys RE
;----------------------------------------------------------------------

    sep r3              ; return
rxchar:                 ;
    ldi 8               ; get start bit + seven bits from loop, last bit on returning
    plo re              ; RE.0 = bit counter
    ldi 0               ; start off with empty byte
rxcw:                   ; wait for start bit
    bn3 rxcw            ; each instr takes 9us, we need 104us = 11.5
                        ; delay 1/2 bit time to center samples
    nop                 ;     Don't test for correct start bit
    nop                 ;     it will work, if there's too much
    nop                 ;     noise on the line, shorten the cable!
                        ;
rxcloop:                ;
    nop                 ; 10.5
    b3 x1               ; 11.5 sample rx input bit
    ori 80h             ; 1
    br x2               ; 2
x1: phi re              ; 1
x2: phi re              ; 2

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
    nop    ; 6.0
    nop    ; 7.5
    nop    ; 9.0
    nop    ; 10.5
    nop    ; 12.0 
    nop    ; 14.5

; tuning
    nop      ;  tune this if necessary - use a GHI RE here to shorten ) 
;    ghi re  ;  4 uSec -> ( 52 uSec total addition)

 endi

    shr                 ; 3
    phi re              ; 4
    dec re              ; 5
    glo re              ; 6
    bz rxcret           ; 7
    ghi re              ; 8
    br  rxcloop         ; 9
                        ;
rxcret:                 ;
    ghi re              ; 8
    ghi re              ; 9
    nop                 ; 10.5
    b3 x3               ; 11.5 sample last rx input bit
    ori 80h             ; set the last bit if it was a 1
x3: phi re              ; save the received character in RE.1
    br rxchar-1         ; all done so reset PC to start of subroutine and return


;******************************************************************************************9600 end
 ELSE
;*****************************************************************************************19200 start

 ;----------------------------------------------------------------------
        ORG 0000H                            ;
        DIS                                  ; disable interrupts / PC = R0  SP =  R0
        DB $00                               ; DIS instruction loads 0 to P and X

        SEQ                                  ;
		OUT 4								 ; blink front panel LEDS
		db $FF								 ;
        LDI $FF                              ;
        PHI R5                               ;
        LDI $20                              ;
        PLO R5                               ;
XLOOP:  DEC R5                               ;
        GHI R5                               ;
        BNZ XLOOP                            ;
		OUT 4								 ;
		db $00								 ;

        LDI high START                       ; initialize R3 as program counter
        PHI R3                               ;
        LDI low START                        ;
        PLO R3                               ;
        LDI high STACK                       ; initialize R2 as stack pointer
        PHI R2                               ;
        LDI low STACK                        ;
        PLO R2                               ;
        SEX R2                               ;
        LDI high TERMIO                      ; 19200 baud tx / rx routines via R6
        PHI R6                               ;
        LDI low TERMIO                       ;
        PLO R6                               ;
        SEP R3                               ;
START:
        INP 4                                ; check run mode based on front panel switch 0
        ANI $01                              ; 
        BZ  ECHO_JAM                         ;
        BR  TX_JAM                           ;
 
;****************************************************************
;
; Dump printable ASCII characters sequentially to serial output 
;    (with CR/LF's inserted)
;
;****************************************************************

TX_JAM: LDI $1F                              ;
        PHI RB
RESTART:
        LDI $7F - $20
        PLO RB
NEXTCHAR:
        GHI RB
        ADI $01
        PHI RB
        SMI $7F
        BNZ NO_WRAP
        LDI $21
        PHI RB
NO_WRAP:
        GHI RB
        INC R6          ; \_ This outputs the character in D
        SEP R6          ; /

 if TX_DELAY > 0
        LDI $10                              ; inter chararcter delay lopp
        PHI R5                               ;
        LDI $FF                              ;
        PLO R5                               ;
DLOOP:  DEC R5                               ;
        GHI R5                               ;
        BNZ DLOOP                            ;
 endi
 
        DEC RB                               ; character counter = EOL ?
        GLO RB                               ;
        BNZ NEXTCHAR                         ; time for a new line
        LDI $0D                              ; carriage return
        INC R6                               ; \_ This outputs the character in D
        SEP R6                               ; /
        LDI $0A                              ; line feed
        INC R6                               ; \_ This outputs the character in D
        SEP R6                               ; /
        LDI $08                              ; new line delay
        PHI RF                               ;
        LDI $00                              ;
        PLO RF                               ;
LOOP:   DEC RF                               ;
        GHI RF                               ;
        BNZ LOOP                             ;
        BR  START                            ;

;****************************************************************
;
; Wait for an input character and then echo it back
;
;****************************************************************
ECHO_JAM:
        GLO R6                               ; \_ This inputs a character into D
        SEP R6                               ; /
        INC R6                               ; \_ This outputs the character in D
        SEP R6                               ; /
        BR START                             ;


;********************************************************* 19200 BAUD *************
;
;  19200 BAUD Serial I/O Routines 
;
; - assumes R6 is the subroutine PC and code is located on zero page ( i.e. 0x00aa )
;
;   INC R6  \_ This outputs the character in D
;   SEP R6  /
;
;   GLO R6  \_ This inputs a character into D (assumes R6.0 != 0 )
;   SEP R6  /
;
;   GHI R6  \_ This checks if break is pressed (assumes R6.1 == 0 )
;   SEP R6  /
;
;----------------------------------------------------------------------

RETIO:
        SEP     R3

TERMIO:
        LSKP              ; entry here via  SEP R6          for character input
        BR      OUTPUT    ; entry here via  INC R6  SEP R6  for character output
        BNZ     INPUT

;********************************************************* 19200 BAUD *************
; qbreak 
;   check if serial data is waiting
;
;  - hack to monitor EF4 so M/C front panel switch sets break condition
; - could check serial line status (EF3) but that will be intermittent
; - could hack this into inner interpreter (one extra instructions per loop)
;
;----------------------------------------------------------------------

BREAK:  GLO     R6        ; R6.1 = 0 ( hack to save an immediate byte )
        BN4     RETIO     ; check if incoming serial data line is in a break (0) state
        GHI     R6        ; R6.1 != 0 ( hack to save an immediate byte )
        BR      RETIO     ; exit


;********************************************************* 19200 BAUD *************
;rx_char
;  Receive Byte via EFe connected to RS232 receiver
;  Recieves 8 bits
;  call with R3 being the previous PC
;  Returns with Byte received in D and re.1
;  Destroys RE
;----------------------------------------------------------------------
;
;  bit bang the EF3 line to receive a serial character into D
;
INPUT:  BN3     INPUT     ; wait for start bit
        GHI     R6        ; nop - one byte that also sets D=0 and being empty means
                          ; ... that  no bits will shift out the first 7 times through the loops below
        GHI     R6        ; nop  (first time through will set a bit to shift in. When it shifts out, 
                          ; ... 8 bits have been sent).
SPACE:  SMI     $0        ; set DF  (first time through will set a bit to shift in. When it shifts out, 
                          ; ... 8 bits have been sent).
MARK:   SHRC              ; shift next serial bit in to rx byte ( DF = 0 if we jump directly here.  
                          ; ... DF = 1 if we are falling throught from the SHRC instruction above)
        BDF     RETIO     ; if a bit pops out of D when shifted it means we have shifted 8 times

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
    nop    ; 6.0
	sep r6 ;
 endi
        SEP     R6        ; nop (as P = 6 already )
        SEP     R6        ; nop
        BN3     SPACE     ; jump if next serial bit a zero
        BR      MARK      ; otherwise it's a one

;********************************************************** 19200 BAUD ******
; Transmit Byte via Q connected to RS232 driver
;  call with R3 being the previous PC
;  Byte to send in D
;  Returns with D unaffected
;  re.1 = D
;  Destroys RE
;   - 1.8MHz =  5.8 2 cycle instructions per bit
;   - 4.0MHz = 13.0 2 cycle instructions per bit  (add 7 2 cycle or 4 NOP + 1 2 cycle)
;----------------------------------------------------------------------
;
;  bit bang the value in D out on the Q line
;
OUTPUT: REQ                 ; send start bit

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
	nop	   ; 8.0
	req	   ; 8.0
 endi 
        SHRC                ; check first data bit by shifting into DF
        PHI     R8          ; R8.1 = remaining bits to be sent
                            ;
        LDI     4           ; bit count = 4
        PLO     R8          ; R8.0 = bit counter
        BDF     SEQ1        ; jump if next data bit is a 1
                            ;
REQ1:   REQ                 ; data bit was a zero so reset Q

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
	nop	   ; 8.0
	req	   ; 8.0
 endi 
        GHI     R8          ; D = remaining data bits
        SHRC                ; move next bit into DF
        BDF     SEQ2        ; jump if it's a 1
REQ2:   SHRC                ; data bit was a zero
        PHI     R8          ; R8.1 = remainging data bits
        REQ                 ; data bit was a zero so reset Q

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
	nop	   ; 8.0
	req	   ; 8.0
 endi 
                            ;
        REQ                 ; 2 cycle "nop" in place of BR at end of EQ2
                            ;
OLOOP: 						;
		DEC     R8          ; decrement bit count
        GLO     R8          ; check if we are all done
        BZ      STOP        ; exit if so
        BNF     REQ1        ; else go process next bit
                            ;
SEQ1:   SEQ                 ; data bit was a one so set Q

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
	nop	   ; 8.0
	seq	   ; 8.0
 endi 
        GHI     R8          ; R8.1 = remainging data bits
        SHRC                ; move next bit into DF
        BNF     REQ2        ; jump if it's a 0
SEQ2:   SHRC                ; data bit was a zero
        PHI     R8          ; R8.1 = remainging data bits
        SEQ                 ; data bit was a zero so rest Q

 if CLOCK_MHZ = 4
    nop    ; 1.5 additional instruction ( 6 uSec per nop )
    nop    ; 3.0
    nop    ; 4.5
	nop	   ; 8.0
	seq	   ; 8.0
 endi 
        BR      OLOOP       ; go process next bit  (also a 2 cycle "nop")
                            ;
STOP:   SEQ                 ; all done so set line to idle state
        BR      RETIO       ; exit

 ENDI

STACK  EQU    ( $ + $0200  AND $FF00 )  - $0001

    end
