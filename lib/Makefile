lib-y += compiler-rt.o

# simplefb
lib-$(CONFIG_SIMPLE_FB) += simplefb/simplefb.o

# debug
lib-y += console/console.o

# unic (neatlibc still temporarily resides)
lib-y += unic/string.o

# libdeflate needs to be told that it is freestanding before compile
ccflags-y += -DFREESTANDING
lib-$(CONFIG_LIBDEFLATE) += \
	libdeflate/lib/adler32.o \
	libdeflate/lib/crc32.o \
	libdeflate/lib/deflate_compress.o \
	libdeflate/lib/deflate_decompress.o \
	libdeflate/lib/gzip_compress.o \
	libdeflate/lib/gzip_decompress.o \
	libdeflate/lib/utils.o \
	libdeflate/lib/zlib_compress.o \
	libdeflate/lib/zlib_decompress.o \
	libdeflate/lib/arm/cpu_features.o \
	libdeflate/lib/x86/cpu_features.o \

lib-$(CONFIG_LIBFDT) += \
	libfdt/fdt.o \
	libfdt/fdt_ro.o \
	libfdt/fdt_rw.o \
	libfdt/fdt_wip.o \
	libfdt/fdt_sw.o \
	libfdt/fdt_strerror.o

ccflags-y += -I$(srctree)/include/lib/libdeflate

ccflags-y += -I$(srctree)/include/lib/libfdt
