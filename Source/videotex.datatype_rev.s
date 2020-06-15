VERSION = 1
REVISION = 5

.macro DATE
.ascii "6.3.2010"
.endm

.macro VERS
.ascii "videotex.datatype 1.5"
.endm

.macro VSTRING
.ascii "videotex.datatype 1.5 (6.3.2010)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: videotex.datatype 1.5 (6.3.2010)"
.byte 0
.endm
