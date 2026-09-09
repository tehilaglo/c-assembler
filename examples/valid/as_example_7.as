.extern GLO
 .extern ptrArr
.extern MYFUNC
    .extern SUBR

lea    GLO, r1
add    #1, ptrArr
cmp    r2 ,r3
get r5
cmp   r5, ptrArr
bne MYFUNC

clr r5
lea	 ptrArr, r6
mov r6, r0
inc r0
mov    #-73, r3
lea	GLO, r2
cmp   r3, r2
jmp MYFUNC
jsr SUBR