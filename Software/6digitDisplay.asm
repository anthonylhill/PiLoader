
        INCL "1802reg.asm"
        
        ORG 00000           ; 

;** Jack Noble "hello" program **
; simple program to establish and initialize interupt handler
; and display an initial message - and loop otherwise. 
; code and commentary by Jack Noble, some edits by Herb Johnson
; For this program to run, the interrupt handler MUST be in memory. 

                            ;
         DIS                ; disable interrupts until established
         DB $20             ;   PC = SP = R0

		LBR START			;

;		ORG $0100
START:                      ;
         LDI low ISR        ; R1 -> interrupt handler
         PLO R1             ;
         LDI high ISR       ;
         PHI R1             ;
                            ;
         LDI low STACK      ; R2 -> stack space
         PLO R2             ;
         LDI high STACK     ;
         PHI R2             ;

 ;       SEX R2
         
         INP 4              ; read front panel switches
         ANI $01            ; check first switch
         BNZ BANNER         ; go display banner if set
         
         LDI low MESG1      ; RF -> initial/default message
         PLO RE             ;
         PLO RF             ;
         LDI high MESG1     ;
         PHI RE             ;
         PHI RF             ;
              
		 SEX R0             ;
         RET                ; interrupt enable
         DB $20             ; PC = R0   SP = R2
                            ;
LOCK:    BR LOCK            ; loop here between interrupts
                            ;
#MESG1:    DB "HELLO!"       ; initial six-byte ASCII message
MESG1:    DB "LOaDeR"       ; initial six-byte ASCII message
                            ;

; ** Jack Noble "banner" program **
; program to initialize interrupt handler
; and display a message in "banner" or scrolling mode
; code and commentary by Jack Noble, some edits by Herb Johnson
;
; Note : display glitch is possible if banner text crosses a page boundary

BANNER:                     ;
        LDI low BANNERTXT   ; intialize message pointers and counter
        PLO RE              ;
        PLO RF              ;
        LDI high BANNERTXT  ;
        PHI RE              ;
        PHI RF              ;

		SEX R0				;
        RET                 ; interrupt enable
        DB $20              ; PC = R0   SP = R2

BLOOP1: LDI low BANNERTXT   ; reset pointer and counter to start of banner text
        PLO RF              ;
        LDI high BANNERTXT  ;
        PHI RF              ;
        LDI low BANNERLEN   ;
        PLO R4              ;

BLOOP2: LDI $40             ; delay using R3
        PHI R3              ; put in high byte, clock low byte
DELAY:  DEC R3              ; give time for human to read display before scroll
        GHI R3              ;
        BNZ DELAY           ; delay time depends on CPU speed
		LSNQ				; blink Q on each scroll
		REQ					;
		SKP					;
		SEQ					;
        INC RF              ; scroll start of buffer pointer to the left
        DEC R4              ; decrement the position counter
        GLO R4              ;
        BNZ BLOOP2          ; scroll if not at end of message
        BR BLOOP1           ;

BANNERTXT:    DB "      1802 MEMbErshIp CArd PI LOAdER      "
BANNERLEN     EQU $ - BANNERTXT - 6

 
 ; The Jack Noble interrupt handler
; Interrupt Service Routine by Jack Noble July 19 2022 
; for 1802 Membership Card Rev L six-digit display
; commentary by Jack Noble, some edits by Herb Johnson
;
; Set register RE to point to 6 bytes of a default message in memory.
; The value in RF does not need to be initialized. 
; Then re-enable interrupts, and you're good to go!
;
; R1, R2 and RF must be preserved by all running programs.
; On interrupt, R1 is automatically the PC, and X is set to R2 automatically.
;
; In use: RE is set to point to wherever in memory
; contains the 6-byte message to be displayed. 
; 
; RF is used by the ISR to keep track of where it is in the message during display
; (So don't touch that register).
;
         

        RET                 ; usual 1802 return route, sets current PC=R1 to next instruction
ISR:    DEC R2              ;
        SAV                 ; push T state (old X and P) for RET to restore  
        DEC R2              ;
        STXD                ; push whatever was in D
							;
        LDA RE              ; get current character from text buffer
;       INC RE              ; point to next char to display
        STR R2              ; save on stack 
        OUT 4               ; sends char to display latch
                            ;
        NOP                 ; hang loose for some CPU clock time. Calibrate
        NOP                 ; .. relative to the length of the interrupt signal
        NOP                 ; ...to avoid double interrupts.
        NOP                 ;
        NOP                 ;
        NOP                 ; 
        NOP                 ; 
        NOP                 ;
        NOP                 ;
        NOP                 ;
        NOP                 ; 
                            ;
                            ;
        B2  NODSP6          ; monitor EF2 for display reset timer (first char timer)
                            ; if not ready don't reset display pointer
        GHI RF              ; if ready then reset RE to start of message
        PHI RE              ;
        GLO RF              ;
        PLO RE              ;
                            ;
NODSP6: 					;
		LDXA                ; restore D and clean up stack
        BR ISR-1            ; 

STACK  EQU    ( $ + $0200  AND $FF00 ) - $0001

          END
