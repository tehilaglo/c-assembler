MAIN: get r1
mov r1, r2
 mov #0  , r3
inc r1, r3
cmp #1, r1
not r3
jmp MAIN
cmp #*(<3), r1
mov r2, mov, r3
A: .string "memo"   !!
B: .string "
    C: .string     0
STRC1: .struct 0, "zero"
nums: .data 1, -2, 3, , -4, 5, -6, 7, -8
STRC2: .struct , "ONE"
STRR: .string "*one *two *three *four", 1, 2, 3, 4
FLAG: .data ,1
NOT: .data 0,
    STRC3: .struct 2,
    ARR: .data 1,,0
EMPTY: .string
CHARS: .data ,,