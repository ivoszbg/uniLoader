# SPDX-License-Identifier: GPL-2.0

#
# Compiler defines
#
CCPREFIX?=aarch64-linux-gnu-
CC=$(CCPREFIX)gcc
CPP=$(CCPREFIX)cpp
LD=$(CCPREFIX)ld
OBJCPY=$(CCPREFIX)objcopy

#
# Compiler flags
#
CFLAGS = -march=armv8-a -Wall -nodefaultlibs	\
         -nostdlib -nostartfiles -fno-builtin	\
         -nostdinc -Wstrict-prototypes -std=gnu11 -I'../include'

#
# Split BOARD definition into 2 parts - manufacturer and codename
#
split_board := $(subst -, ,$(BOARD:%=%))
board_manu := $(word 1,$(split_board))
board_codename := $(word 2,$(split_board))

#
# Supported SoCs
#

# Exynos
ifeq ($(SOC),exynos8895)
brand_name=exynos
else ifeq ($(SOC),exynos9810)
brand_name=exynos
endif

# Apple
ifeq ($(SOC),t7000)
brand_name=apple
endif

# Generic
ifeq ($(SOC),generic)
brand_name=generic
endif

all: $(brand_name)/$(SOC).o copy-soc

copy-soc:
	cp $(brand_name)/$(SOC).o ../soc.o
