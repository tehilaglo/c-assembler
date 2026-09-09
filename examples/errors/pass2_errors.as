.entry MAIN
    .extern s2
    .entry MAIN
MAIN: mov S8.1, OK
        add r2, r2
LOOP: jmp END
    macro mac
        get  r6
        mov s8.2 ,r3
         jsr PENNY
    endmacro
        .entry vari
bne LOOP
mac
    .entry s2
END: hlt
S8: .struct 770, "seven seventy$"
PENNY: .data 1
    .entry nums
mac