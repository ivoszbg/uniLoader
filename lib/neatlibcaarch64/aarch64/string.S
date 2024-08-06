.global memcpy
memcpy:
    mov     x12, x0
memcpy_loop:
    subs    x2, x2, #1
    bmi     memcpy_ret
    ldrb    w3, [x1], #1
    strb    w3, [x0], #1
    b       memcpy_loop
memcpy_ret:
    mov     x0, x12
    ret

.global memset
memset:
    mov     x12, x0
memset_loop:
    subs    x2, x2, #1
    bmi     memset_ret
    strb    w1, [x0], #1
    b       memset_loop
memset_ret:
    mov     x0, x12
    ret

.global memmove
memmove:
    mov     x12, x0
    cmp     x0, x1
    ble     memmove_fw
    add     x3, x1, x2
    cmp     x0, x3
    bgt     memmove_fw

    // copying the memory in reverse order
    add     x0, x0, x2
    add     x1, x1, x2
memmove_bw:
    subs    x2, x2, #1
    bmi     memmove_ret
    ldrb    w3, [x1, #-1]!
    strb    w3, [x0, #-1]!
    b       memmove_bw
memmove_fw:
    subs    x2, x2, #1
    bmi     memmove_ret
    ldrb    w3, [x1], #1
    strb    w3, [x0], #1
    b       memmove_fw
memmove_ret:
    mov     x0, x12
    ret

.global memchr
memchr:
    subs    x2, x2, #1
    bmi     memchr_failed
    ldrb    w3, [x0], #1
    cmp     w3, w1
    bne     memchr
    sub     x0, x0, #1
    b       memchr_ret
memchr_failed:
    mov     x0, #0
memchr_ret:
    ret

.global memrchr
memrchr:
    mov     x12, #0
memrchr_loop:
    subs    x2, x2, #1
    bmi     memrchr_ret
    ldrb    w3, [x0], #1
    cmp     w3, w1
    cset    x12, eq
    b       memrchr_loop
memrchr_ret:
    mov     x0, x12
    ret

.global memcmp
memcmp:
    subs    x2, x2, #1
    bmi     memcmp_match
    ldrb    w3, [x0], #1
    ldrb    w12, [x1], #1
    subs    w3, w3, w12
    cset    x0, ne
    bne     memcmp_ret
    b       memcmp
memcmp_match:
    mov     x0, #0
memcmp_ret:
    ret

.global strlen
strlen:
    mov     x2, x0
strlen_loop:
    ldrb    w1, [x0], #1
    tst     w1, w1
    bne     strlen_loop
    sub     x0, x0, x2
    sub     x0, x0, #1
    ret

.global strchr
strchr:
    ldrb    w2, [x0], #1
    cmp     w1, w2
    sub     x0, x0, #1
    beq     strchr_ret
    tst     w2, w2
    bne     strchr
    mov     x0, #0
strchr_ret:
    ret

.global strrchr
strrchr:
    mov     x3, #0
strrchr_loop:
    ldrb    w2, [x0], #1
    cmp     w1, w2
    cset    x3, eq
    tst     w2, w2
    bne     strrchr_loop
strrchr_ret:
    mov     x0, x3
    ret

.global strcmp
strcmp:
    ldrb    w2, [x0], #1
    ldrb    w3, [x1], #1
    cmp     w2, w3
    beq     strcmp
    sub     x0, x2, x3
    ret

.global strcpy
strcpy:
    mov     x3, x0
strcpy_loop:
    ldrb    w2, [x1], #1
    strb    w2, [x0], #1
    tst     w2, w2
    bne     strcpy_loop
    mov     x0, x3
    ret

.global strncmp
strncmp:
    mov     x12, x2
strncmp_loop:
    subs    x12, x12, #1
    cset    x0, mi
    bmi     strncmp_ret
    ldrb    w2, [x0], #1
    ldrb    w3, [x1], #1
    cmp     w2, w3
    beq     strncmp_loop
    sub     x0, x2, x3
strncmp_ret:
    ret

