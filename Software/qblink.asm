
DELAY EQU $F000
RATIO EQU 10

    ORG 0000H               ; memory where this program is toggled into
    DIS
    DB  $00

    OUT 4
    DB  $00

START:
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
	DEC 2
	GHI 2
	BNZ	LOOP2
	SEQ

	BR START

    END

