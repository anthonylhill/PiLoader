
; 1802 serial echo test program

    INCL  "1802reg.asm"

ef_line EQU 3
invert  EQU 0

    ORG 0000H               ; 

	DIS						; disable interrrups
	DB $00					;

START:                       ; monitor EF4 and echo changes to Q
    GHI R0                   ;
    PHI R4                   ;
    LDI $80                  ;
    PLO R4                   ; R4 = 0080H
    SEX R4                   ; X = 4
	LDI $00
loop:
	STR R4
	OUT 4
	DEC R4
 if invert = 1
    SEQ                      ; echo the line status
 else
    REQ                      ; echo the line status
 endi
    LDI $00                  ; timer set to zero
wait:                        ; wait for start bit
 if ef_line = 3
    B3 wait                  ; loop while idle (EF3 false, EF3 pin high)
 else
    B4 wait
 endi
 if invert = 1
    REQ                      ; echo the line status
 else
    SEQ                      ; echo the line status
 endi

Time:                        ; measure remaining time in Data bit D5
    ADI 1                    ; D=D+1 for each loopi
    NOP                      ; add six cycles for timing
    NOP                      ;
 if ef_line = 3
    BN3 Time                 ; loop until end of bit
 else
    BN4 Time                 ; loop until end of bit
 endi
 if invert = 1
    SEQ                      ; echo the line status
 else
    REQ                      ; echo the line status
 endi
    BR loop                 ;
    END

