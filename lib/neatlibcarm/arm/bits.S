
.global htonl
.global ntohl
ntohl:
htonl:
	eor	r1, r0, r0, ror #16
	bic	r1, r1, #0x00ff0000
	mov	r0, r0, ror #8
	eor	r0, r0, r1, lsr #8
	mov	pc, lr

.global htons
.global ntohs
ntohs:
htons:
	mov	r1, r0, lsr #8
	and	r0, r0, #255
	and	r1, r1, #255
	mov	r0, r0, lsl #8
	orr	r0, r0, r1
	mov	pc, lr
