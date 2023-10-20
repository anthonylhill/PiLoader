
;
;  Membership Card Demo Program
;

    INCL "1802reg.asm"

DELAY EQU $2000

    ORG 0000H               ;
    DIS                     ; disable interrupts, PC=0 SP=7
    DB  $70                 ;

LOOP1:
	LDI high TWIDDLE
	PHI R7
	LDI low TWIDDLE
	PLO R7

LOOP2:
	LDI high DELAY
	PHI R8
	LDI low DELAY
	PLO R8

BLINK:
	LSQ
	SEQ
	SKP
	REQ

WAIT:
	DEC R8
	GHI R8
	BNZ	WAIT

    OUT 4
    GLO R7
    SMI low TWIDDLE_END
    BNZ LOOP2
	BR  LOOP1

TWIDDLE: 
    DB $01,$02,$04,$08,$10,$20,$40,$80
    DB $80,$40,$20,$10,$08,$04,$02,$01
    DB $01,$02,$04,$08,$10,$20,$40,$80
    DB $80,$40,$20,$10,$08,$04,$02,$01

    DB $00,$81,$42,$24,$18
    DB $00,$81,$42,$24,$18
    DB $00,$81,$42,$24,$18
    DB $00,$81,$42,$24,$18
TWIDDLE_END:
    DB $00

    END

