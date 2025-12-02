// SPDX-License-Identifier: GPL-2.0-only
/*
 * Runtime compiler stubs
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

/* make sure these are not inlined and are visible to the linker */
__attribute__((used, noinline))
int raise(int sig) {
   (void)sig;
   while(1);
   return 0;
}

__attribute__((used, noinline))
void __aeabi_unwind_cpp_pr0(void) {
   /* C++ exception unwinding stub - not needed */
}

__attribute__((used, noinline))
void __aeabi_unwind_cpp_pr1(void) {
   /* C++ exception unwinding stub - not needed */
}

__attribute__((used, noinline))
int __aeabi_idiv0(int return_value) {
   return 0;
}

__attribute__((used, noinline))
long long __aeabi_ldiv0(long long return_value) {
   return 0;
}
