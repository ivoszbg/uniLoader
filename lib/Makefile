# simplefb
lib-$(CONFIG_SIMPLE_FB) += simplefb/simplefb.o

# debug
lib-y += debug/debug.o

# unic (neatlibc still temporarily resides)
lib-y += unic/string.o

lib-$(CONFIG_LIBFDT) += \
	libfdt/fdt.o \
	libfdt/fdt_ro.o \
	libfdt/fdt_rw.o \
	libfdt/fdt_wip.o \
	libfdt/fdt_sw.o \
	libfdt/fdt_strerror.o

ccflags-y += -I$(srctree)/include/lib/libfdt
