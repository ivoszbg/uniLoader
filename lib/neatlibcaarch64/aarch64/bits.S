.global htonl
.global ntohl
ntohl:
htonl:
    eor     x1, x0, x0, ror #16
    bic     x1, x1, #0x00ff0000
    mov     x0, x0, ror #8
    eor     x0, x0, x1, lsr #8
    ret

.global htons
.global ntohs
ntohs:
htons:
    mov     x1, x0, lsr #8
    and     x0, x0, #255
    and     x1, x1, #255
    mov     x0, x0, lsl #8
    orr     x0, x0, x1
    ret
