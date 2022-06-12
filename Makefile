# SPDX-License-Identifier: GPL-2.0

#
# Defaut values
#
KERNEL_PATH?=blob/Image
DTB_PATH?=blob/dtb

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
         -nostdinc -Wstrict-prototypes -std=gnu11 \
         -Wno-main -I'./include'

#
# Split BOARD definition into 2 parts - manufacturer and codename
#
split_board := $(subst -, ,$(BOARD:%=%))
board_manu := $(word 1,$(split_board))
board_codename := $(word 2,$(split_board))

OBJ  =	main.o \
	asm/Start.o

.PHONY: clean

#
# Syntax:
# name: dependencies-in-order
#	action-to-do
#
uniLoader: uniLoader.o
	$(OBJCPY) -O binary $< $@

uniLoader.o: copy-board-config $(OBJ) asm/linker.lds build-board build-soc
	$(LD) $(OBJ) board.o soc.o -o $@ --script=asm/linker.lds

asm/linker.lds: asm/linker.lds.S $(KERNEL_PATH)
	$(CPP) $< -DKERNEL_PATH=$(KERNEL_PATH) -DDTB_PATH=$(DTB_PATH) -P -o $@

build-board:
	cd board && make

build-soc:
	cd soc && make

copy-board-config:
	cp include/board/board-$(board_codename).h include/board-config.h

clean:
	-rm *.o asm/linker.lds asm/Start.o uniLoader board/*/*.o include/board-config.h soc/*/*.o
