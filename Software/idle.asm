
DELAY EQU $7000
RATIO EQU $40

    ORG 0000H

    DIS         ; disable interrupts
    DB  $00     ; PC and IX = 0

    OUT 4       ; set data LEDS and 7 segment LED buffer driver to $00
    DB  $00

START:
	SEQ
	LDI high DELAY/RATIO
	PHI 2
	LDI low DELAY/RATIO
	PLO 2
LOOP1:
	DEC 2
	GHI 2
	BNZ	LOOP1
	REQ

	LDI high DELAY
	PHI 2
	LDI low DELAY
	PLO 2
LOOP2:
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
	DEC 2
	GHI 2
	BNZ	LOOP2

	BR START

    END

