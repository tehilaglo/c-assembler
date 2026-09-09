str: .string	"%hhd"
str1: .string	" %c"
str2: .string	"%c"
str3: .string	"%d"
    vari: .data 30
    .entry KEY
KEY: .struct 23, "forever\tyoung =*\n"

LOOP: get r5
add #-12, r5
not r5

    .entry MAIN
 macro    mac1
mov	#0, r2
lea    str1, r2
add    #1, r3
cmp    r2 ,r3
    endmacro
mac1
cmp   r5, vari
jmp LOOP

clr r5
lea	 str2, r7
mov r7, r5
mac1
mov    #0, r5
lea	str3, vari
rts

MAIN: mov	r7           ,LIST

sub 	#52, r4
    lea 	KEY.1 , r2
lea	str, r2
    cmp r3, r4
    .extern LIST
mac1
    .data 0, -0 , -1,   -2, +2