.entry mySTRUCT
    .entry myINT
        .entry mySTRING

 macro comment
 ;this is a comment line
 endmacro

get r1
get r2
get    r3
lea	 mySTRUCT.2, r7
lea mySTRING, r6
prn r6
mov r2, r0
prn r0
comment
mov    #0, r3
lea	mySTRUCT.1   , myINT
prn myINT
clr r1

mov #+61,r1
cmp   r1,      r6
prn r6
dec r1

mySTRUCT: .struct 33, "33"
myINT: .data +100
mySTRING: .string "Peter Piper picked a pack of pickled peppers\n"
comment