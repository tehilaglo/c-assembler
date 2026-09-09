.entry A
    .entry B
    .entry C
macro comment
;this is a comment
    endmacro
get r1
get r2
get r3
lea	 mySTRUCT.2, r7
lea mySTRING, r6
prn r6
mov r2, r0
prn r0
comment
mov    #0, r3
lea	mySTRUCT.1, myINT
macro macro1 macro2
    ;macro name is invalid
endmacro
prn myINT
clr r1
macro comment
         ;this macro is redefined
endmacro
comment
macro
;macro name is missing
endmacro
inc r3
comment
macro .data
    ;macro name is invalid
endmacro
comment
macro r1
        ;macro name is invalid
endmacro