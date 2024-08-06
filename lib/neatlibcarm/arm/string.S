.global memcpy
memcpy:
	mov	r12, r0
memcpy_loop:
	subs	r2, r2, #1
	bmi	memcpy_ret
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	b	memcpy_loop
memcpy_ret:
	mov	r0, r12
	mov	pc, lr

.global memset
memset:
	mov	r12, r0
memset_loop:
	subs	r2, r2, #1
	bmi	memset_ret
	strb	r1, [r0], #1
	b	memset_loop
memset_ret:
	mov	r0, r12
	mov	pc, lr

.global memmove
memmove:
	mov	r12, r0
	cmp	r0, r1
	ble	memmove_fw
	add	r3, r1, r2
	cmp	r0, r3
	bgt	memmove_fw

	@ copying the memory in reverse order
	add	r0, r0, r2
	add	r1, r1, r2
memmove_bw:
	subs	r2, r2, #1
	bmi	memmove_ret
	ldrb	r3, [r1, #-1]!
	strb	r3, [r0, #-1]!
	b	memmove_bw
memmove_fw:
	subs	r2, r2, #1
	bmi	memmove_ret
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	b	memmove_fw
memmove_ret:
	mov	r0, r12
	mov	pc, lr

.global memchr
memchr:
	subs	r2, r2, #1
	bmi	memchr_failed
	ldrb	r3, [r0], #1
	cmp	r3, r1
	bne	memchr
	sub	r0, r0, #1
	b	memchr_ret
memchr_failed:
	mov	r0, #0
memchr_ret:
	mov	pc, lr

.global memrchr
memrchr:
	mov	r12, #0
memrchr_loop:
	subs	r2, r2, #1
	bmi	memrchr_ret
	ldrb	r3, [r0], #1
	cmp	r3, r1
	subeq	r12, r0, #1
	b	memrchr_loop
memrchr_ret:
	mov	r0, r12
	mov	pc, lr

.global memcmp
memcmp:
	subs	r2, r2, #1
	bmi	memcmp_match
	ldrb	r3, [r0], #1
	ldrb	r12, [r1], #1
	subs	r3, r3, r12
	movne	r0, r3
	bne	memcmp_ret
	b	memcmp
memcmp_match:
	mov	r0, #0
memcmp_ret:
	mov	pc, lr

.global strlen
strlen:
	mov	r2, r0
strlen_loop:
	ldrb	r1, [r0], #1
	tst	r1, r1
	bne	strlen_loop
	sub	r0, r0, r2
	sub	r0, r0, #1
	mov	pc, lr

.global strchr
strchr:
	ldrb	r2, [r0], #1
	cmp	r1, r2
	subeq	r0, r0, #1
	beq	strchr_ret
	tst	r2, r2
	bne	strchr
	mov	r0, #0
strchr_ret:
	mov	pc, lr

.global strrchr
strrchr:
	mov	r3, #0
strrchr_loop:
	ldrb	r2, [r0], #1
	cmp	r1, r2
	subeq	r3, r0, #1
	tst	r2, r2
	bne	strrchr_loop
strrchr_ret:
	mov	r0, r3
	mov	pc, lr

.global strcmp
strcmp:
	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	r2, r3
	beq	strcmp
	sub	r0, r2, r3
	mov	pc, lr

.global strcpy
strcpy:
	mov	r3, r0
strcpy_loop:
	ldrb	r2, [r1], #1
	strb	r2, [r0], #1
	tst	r2, r2
	bne	strcpy_loop
	mov	r0, r3
	mov	pc, lr

.global strncmp
strncmp:
	mov	r12, r2
strncmp_loop:
	subs	r12, r12, #1
	movmi	r0, #0
	bmi	strncmp_ret
	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	r2, r3
	beq	strncmp_loop
	sub	r0, r2, r3
strncmp_ret:
	mov	pc, lr
