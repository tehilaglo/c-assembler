str: .string "invalid input!\n"
    .extern PSTRLEN

add	#48, r3
rts
    .entry replChar
    .extern SUBR
    .extern vari
    .entry END

get r6
replChar: mov	#32, r6
sub	  #24, r6
mov	r6, r1
jsr SUBR
cmp    r6, vari
add    #1, r3
mov    r3, vari
rts

LOOP: lea	str, r2
mov	#0, r5
    .entry LOOP
jmp	LB6

cmp    vari, r5
get r1

 LB6: mov   #16, r1

sub 	r5, r1
mov	#12, r3
cmp	#65, r3
lea	str, r6
    bne LD2

get r5
inc r5
 bne SUBR
LD2: dec vari
clr r5
mov	#-2, r5

 jmp	END
  LD4: cmp	vari, #5
END: hlt