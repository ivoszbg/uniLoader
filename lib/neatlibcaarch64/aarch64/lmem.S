
.global __memcpylong
__memcpylong:
    mov     x12, x0           // Save the base address in x12
memcpylong_loop:
    subs    x2, x2, #4        // Decrement the counter (in bytes)
    bmi     memcpylong_ret    // If the counter is negative, exit
    ldr     w3, [x1], #4      // Load 4 bytes from address in x1 into w3 and increment x1
    str     w3, [x0], #4      // Store 4 bytes from w3 to address in x0 and increment x0
    b       memcpylong_loop   // Repeat the loop
memcpylong_ret:
    mov     x0, x12           // Restore the original base address
    ret                       // Return from function

.global __memsetlong
__memsetlong:
    mov     x12, x0           // Save the base address in x12
memsetlong_loop:
    subs    x2, x2, #4        // Decrement the counter (in bytes)
    bmi     memsetlong_ret    // If the counter is negative, exit
    str     w1, [x0], #4      // Store 4 bytes of the value in w1 to address in x0 and increment x0
    b       memsetlong_loop   // Repeat the loop
memsetlong_ret:
    mov     x0, x12           // Restore the original base address
    ret                       // Return from function

