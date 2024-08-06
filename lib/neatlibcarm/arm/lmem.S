
.global __memcpylong
__memcpylong:
	mov	r12, r0           @ Save the base address in r12
memcpylong_loop:
	subs	r2, r2, #4       @ Decrement the counter (in bytes)
	bmi	memcpylong_ret    @ If the counter is negative, exit
	ldr	r3, [r1], #4     @ Load 4 bytes from address in r1 into r3 and increment r1
	str	r3, [r0], #4     @ Store 4 bytes from r3 to address in r0 and increment r0
	b	memcpylong_loop    @ Repeat the loop
memcpylong_ret:
	mov	r0, r12          @ Restore the original base address
	mov	pc, lr           @ Return from function

.global __memsetlong
__memsetlong:
	mov	r12, r0           @ Save the base address in r12
memsetlong_loop:
	subs	r2, r2, #4       @ Decrement the counter (in bytes)
	bmi	memsetlong_ret    @ If the counter is negative, exit
	str	r1, [r0], #4     @ Store 4 bytes of the value in r1 to address in r0 and increment r0
	b	memsetlong_loop    @ Repeat the loop
memsetlong_ret:
	mov	r0, r12          @ Restore the original base address
	mov	pc, lr           @ Return from function
