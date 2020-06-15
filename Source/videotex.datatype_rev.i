VERSION		EQU	1
REVISION	EQU	5

DATE	MACRO
		dc.b '6.3.2010'
		ENDM

VERS	MACRO
		dc.b 'videotex.datatype 1.5'
		ENDM

VSTRING	MACRO
		dc.b 'videotex.datatype 1.5 (6.3.2010)',13,10,0
		ENDM

VERSTAG	MACRO
		dc.b 0,'$VER: videotex.datatype 1.5 (6.3.2010)',0
		ENDM
