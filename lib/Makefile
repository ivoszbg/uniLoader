# SIMPLEFB
lib-$(CONFIG_SIMPLE_FB) += simplefb/simplefb.o

# DEBUG
lib-y += debug/debug.o

# Neatlibc (excluding signal and syscalls)
ifeq ($(ARCH), arm)
lib-y += neatlibcarm/arm/bits.o \
	 neatlibcarm/arm/lmem.o \
	 neatlibcarm/arm/string.o
else
lib-y += neatlibcaarch64/$(ARCH)/bits.o \
	 neatlibcaarch64/$(ARCH)/lmem.o \
	 neatlibcaarch64/$(ARCH)/string.o
endif

lib-y += neatlibc/ctype.o \
	 neatlibc/strftime.o \
	 neatlibc/mkstemp.o \
	 neatlibc/regex.o \
	 neatlibc/rand.o \
	 neatlibc/stdarg.o \
	 neatlibc/memtst.o \
	 neatlibc/dirent.o \
	 neatlibc/localtime.o \
	 neatlibc/malloc.o \
	 neatlibc/termios.o \
	 neatlibc/atoi.o \
	 neatlibc/errno.o \
	 neatlibc/stdio.o \
	 neatlibc/stdlib.o \
	 neatlibc/qsort.o \
	 neatlibc/stringc.o \
	 neatlibc/isatty.o \
	 neatlibc/scanf.o \
	 neatlibc/string.o
