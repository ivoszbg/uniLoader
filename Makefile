# SPDX-License-Identifier: GPL-2.0

# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";

ARCH ?= aarch64
MAKEFLAGS += -rR --no-print-directory

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

# kbuild supports saving output files in a separate directory.
# To locate output files in a separate directory two syntaxes are supported.
# In both cases the working directory must be the root of the kernel src.
# 1) O=
# Use "make O=dir/to/store/output/files/"
#
# 2) Set KBUILD_OUTPUT
# Set the environment variable KBUILD_OUTPUT to point to the directory
# where the output files shall be placed.
# export KBUILD_OUTPUT=dir/to/store/output/files/
# make
#
# The O= assignment takes precedence over the KBUILD_OUTPUT environment
# variable.

# Our default target
PHONY := _all
_all:

# KBUILD_SRC is set on invocation of make in OBJ directory
# KBUILD_SRC is not intended to be used by the regular user (for now)
ifeq ($(KBUILD_SRC),)

# OK, Make called in directory where kernel src resides
# Do we want to locate output files in a separate directory?
ifeq ("$(origin O)", "command line")
  KBUILD_OUTPUT := $(O)
endif

ifeq ("$(origin W)", "command line")
  export KBUILD_ENABLE_EXTRA_GCC_CHECKS := $(W)
endif

# Cancel implicit rules on top Makefile
$(CURDIR)/Makefile Makefile: ;

ifneq ($(KBUILD_OUTPUT),)
# Invoke a second make in the output directory, passing relevant variables
# check that the output directory actually exists
saved-output := $(KBUILD_OUTPUT)
KBUILD_OUTPUT := $(shell cd $(KBUILD_OUTPUT) && /bin/pwd)
$(if $(KBUILD_OUTPUT),, \
     $(error output directory "$(saved-output)" does not exist))

PHONY += $(MAKECMDGOALS) sub-make

$(filter-out _all sub-make $(CURDIR)/Makefile, $(MAKECMDGOALS)) _all: sub-make
	$(Q)@:

sub-make: FORCE
	$(if $(KBUILD_VERBOSE:1=),@)$(MAKE) -C $(KBUILD_OUTPUT) \
	KBUILD_SRC=$(CURDIR) \
	-f $(CURDIR)/Makefile \
	$(filter-out _all sub-make,$(MAKECMDGOALS))

# Leave processing to above invocation of make
skip-makefile := 1
endif # ifneq ($(KBUILD_OUTPUT),)
endif # ifeq ($(KBUILD_SRC),)

# We process the rest of the Makefile if this is the final invocation of make
ifeq ($(skip-makefile),)

# If building an external module we do not care about the all: rule
# but instead _all depend on modules
PHONY += all
_all: all

srctree		:= $(if $(KBUILD_SRC),$(KBUILD_SRC),$(CURDIR))
objtree		:= $(CURDIR)
src		:= $(srctree)
obj		:= $(objtree)

VPATH		:= $(srctree)

export srctree objtree VPATH

# Configuration variables
CROSS_COMPILE	?= $(CONFIG_CROSS_COMPILE:"%"=%)
KERNEL_PATH	?= $(CONFIG_KERNEL_PATH:"%"=%)
DT_PATH		?= $(CONFIG_DT_PATH:"%"=%)
RAMDISK_PATH	?= $(CONFIG_RAMDISK_PATH:"%"=%)
TEXT_BASE	?= $(CONFIG_TEXT_BASE:"%"=%)

KCONFIG_CONFIG	?= .config
export KCONFIG_CONFIG

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

# Host tools
HOSTCC       = gcc
HOSTCXX      = g++
HOSTCFLAGS   = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 \
               -fomit-frame-pointer -Wno-builtin-declaration-mismatch
HOSTCXXFLAGS = -O2

# Beautify output
# ---------------------------------------------------------------------------
#
# Normally, we echo the whole command before executing it. By making
# that echo $($(quiet)$(cmd)), we now have the possibility to set
# $(quiet) to choose other forms of output instead, e.g.
#
#         quiet_cmd_cc_o_c = Compiling $(RELDIR)/$@
#         cmd_cc_o_c       = $(CC) $(c_flags) -c -o $@ $<
#
# If $(quiet) is empty, the whole command will be printed.
# If it is set to "quiet_", only the short version will be printed. 
# If it is set to "silent_", nothing will be printed at all, since
# the variable $(silent_cmd_cc_o_c) doesn't exist.
#
# A simple variant is to prefix commands with $(Q) - that's useful
# for commands that shall be hidden in non-verbose mode.
#
#	$(Q)ln $@ :<
#
# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(findstring s,$(MAKEFLAGS)),)
  quiet=silent_
endif

export quiet Q KBUILD_VERBOSE

# Look for make include files relative to root of kernel src
MAKEFLAGS += --include-dir=$(srctree)

# We need some generic definitions (do not try to remake the file).
$(srctree)/scripts/Kbuild.include: ;
include $(srctree)/scripts/Kbuild.include

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
INSTALLKERNEL  := installkernel
PERL		= perl

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
		  -Wbitwise -Wno-return-void $(CF)
CFLAGS_KERNEL	=
AFLAGS_KERNEL	=

# Use LINUXINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
LINUXINCLUDE    := -Iinclude \
                   $(if $(KBUILD_SRC), -I$(srctree)/include) \
                   -include include/generated/autoconf.h

KBUILD_CPPFLAGS := -D__KERNEL__

KBUILD_CFLAGS   := -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs \
		   -fno-strict-aliasing -fno-common \
		   -fno-stack-protector \
		   -Werror-implicit-function-declaration \
		   -Wno-format-security \
		   -fno-delete-null-pointer-checks \
		   -Wno-builtin-declaration-mismatch -Wno-main -nostdinc \
		   -I$(srctree)/lib/unic

# Try to get the version from Git has
GIT_VERSION_TAG	?= $(shell git rev-parse --short HEAD 2>/dev/null)
BUILD_DATE	?= $(shell date "+%Y-%m-%d %H:%M:%S" 2>/dev/null)
KBUILD_CFLAGS	+= -DVER_TAG="\"($(GIT_VERSION_TAG)) - date $(BUILD_DATE)\""

KBUILD_AFLAGS_KERNEL :=
KBUILD_CFLAGS_KERNEL :=
KBUILD_AFLAGS   := -D__ASSEMBLY__

# Architecture-specific flags
ifeq ($(ARCH), arm)
KBUILD_CFLAGS += -march=armv7-a -mfloat-abi=soft -mtune=cortex-a9
KBUILD_AFLAGS += -march=armv7-a -mfloat-abi=soft -mtune=cortex-a9
else
KBUILD_CFLAGS += -march=armv8-a -mtune=cortex-a53 -mcpu=cortex-a53
KBUILD_AFLAGS += -march=armv8-a -mtune=cortex-a53 -mcpu=cortex-a53
endif

# Read KERNELRELEASE from include/config/kernel.release (if it exists)
KERNELRELEASE = $(shell cat include/config/kernel.release 2> /dev/null)
KERNELVERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION)

export ARCH SRCARCH CONFIG_SHELL HOSTCC HOSTCFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP
export MAKE AWK GENKSYMS INSTALLKERNEL PERL UTS_MACHINE
export HOSTCXX HOSTCXXFLAGS

export KBUILD_CPPFLAGS NOSTDINC_FLAGS LINUXINCLUDE OBJCOPYFLAGS LDFLAGS
export KBUILD_CFLAGS CFLAGS_KERNEL
export KBUILD_AFLAGS AFLAGS_KERNEL
export KBUILD_AFLAGS_KERNEL KBUILD_CFLAGS_KERNEL
export KBUILD_ARFLAGS

# Files to ignore in find ... statements
RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS \
                    -o -name .pc -o -name .hg -o -name .git \) -prune -o

# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

PHONY += outputmakefile
# outputmakefile generates a Makefile in the output directory, if using a
# separate output directory. This allows convenient use of make in the
# output directory.
outputmakefile:
ifneq ($(KBUILD_SRC),)
	$(Q)ln -fsn $(srctree) source
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/mkmakefile \
	    $(srctree) $(objtree) $(VERSION) $(PATCHLEVEL)
endif

# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

no-dot-config-targets := clean mrproper distclean \
			 cscope gtags TAGS tags help %docs check% coccicheck \
			 include/linux/version.h headers_% \
			 kernelversion %src-pkg

config-targets := 0
mixed-targets  := 0
dot-config     := 1

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifneq ($(filter config %config,$(MAKECMDGOALS)),)
        config-targets := 1
        ifneq ($(filter-out config %config,$(MAKECMDGOALS)),)
                mixed-targets := 1
        endif
endif

ifeq ($(mixed-targets),1)
# ===========================================================================
# We're called with mixed targets (*config and build targets).
# Handle them one by one.

%:: FORCE
	$(Q)$(MAKE) -C $(srctree) KBUILD_SRC= $@

else
ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Read arch specific Makefile to set KBUILD_DEFCONFIG as needed.
# KBUILD_DEFCONFIG may point out an alternative default configuration
# used for 'make defconfig'
export KBUILD_DEFCONFIG KBUILD_KCONFIG

config: scripts_basic outputmakefile FORCE
	$(Q)mkdir -p include/linux include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

%config: scripts_basic outputmakefile FORCE
	$(Q)mkdir -p include/linux include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

else

ifeq ($(dot-config),1)
# Read in config
-include include/config/auto.conf

# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
-include include/config/auto.conf.cmd

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
	$(Q)$(MAKE) -f $(srctree)/Makefile silentoldconfig

else
# Dummy target needed, because used as prerequisite
include/config/auto.conf: ;
endif # $(dot-config)

# ===========================================================================
# uniLoader build configuration

# Main object files and architecture-specific libraries
uniloader-main-y := arch/built-in.o main/built-in.o

# Directory structure
uniloader-obj-dirs := main arch
uniloader-lib-dirs := soc board drivers lib

# All directories that need to be built
uniloader-all-dirs := $(uniloader-obj-dirs) $(uniloader-lib-dirs)

# Build products
uniloader-objs := $(patsubst %,%/built-in.o, $(uniloader-obj-dirs))
uniloader-libs := $(patsubst %,%/lib.a, $(uniloader-lib-dirs))
uniloader-all := $(uniloader-objs) $(uniloader-libs)

# Linker script dependencies
linker-script-deps := $(KERNEL_PATH) $(RAMDISK_PATH)

# ===========================================================================
# Build targets

# The all: target is the default when no target is given on the command line.
# This allows a user to issue only 'make' to build uniLoader
all: arch/$(ARCH)/linker.lds uniLoader

# Generate linker script from template
quiet_cmd_cpp_lds = LDS     $@
      cmd_cpp_lds = $(CPP)  -include include/generated/autoconf.h $< \
                            -DTEXT_BASE=$(TEXT_BASE) \
                            -DKERNEL_PATH=$(KERNEL_PATH) \
                            -DDTB_PATH=$(DT_PATH) \
                            -DRAMDISK_PATH=$(RAMDISK_PATH) \
                            -P -o $@

arch/$(ARCH)/linker.lds: arch/$(ARCH)/linker.lds.S $(linker-script-deps)
	$(call if_changed,cpp_lds)

#
# CMDs for binary formats
#
LIBGCC := $(shell $(CC) $(KBUILD_CFLAGS) -print-libgcc-file-name)

quiet_cmd_uniloader_link = LD      $@.o
      cmd_uniloader_link = $(LD) $(uniloader-main-y) \
                                 --start-group $(uniloader-libs) $(LIBGCC) --end-group \
                                 -o $@.o \
                                 --script=arch/$(ARCH)/linker.lds

quiet_cmd_uniloader_objcopy = OBJCOPY $@
      cmd_uniloader_objcopy = $(OBJCOPY) -O binary $@.o $@

quiet_cmd_gzip_uniloader = GZIP    $@.gz
      cmd_gzip_uniloader = gzip -c uniLoader > uniLoader.gz

quiet_cmd_lz4_uniloader = LZ4     $@.lz4
      cmd_lz4_uniloader = lz4 uniLoader -l -f -q

uniLoader: $(uniloader-all) arch/$(ARCH)/linker.lds FORCE
	$(call if_changed,uniloader_link)
	$(call if_changed,uniloader_objcopy)
ifdef CONFIG_COMPRESS_GZIP
	$(call if_changed,gzip_uniloader)
endif
ifdef CONFIG_COMPRESS_LZ4
	$(call if_changed,lz4_uniloader)
endif

# The actual objects are generated when descending, 
# make sure no implicit rule kicks in
$(uniloader-all): $(uniloader-all-dirs) ;

# Handle descending into subdirectories
# Preset locale variables to speed up the build process. Limit locale
# tweaks to this spot to avoid wrong language settings when running
# make menuconfig etc.
# Error messages still appears in the original language
PHONY += $(uniloader-all-dirs)
$(uniloader-all-dirs): scripts_basic
	$(Q)$(MAKE) $(build)=$@

# ===========================================================================
# Cleaning targets

# Directories & files removed with 'make clean'
CLEAN_DIRS  +=
CLEAN_FILES += uniLoader uniLoader.o uniLoader.gz arch/$(ARCH)/linker.lds

# Directories & files removed with 'make mrproper'
MRPROPER_DIRS  += include/config include/generated
MRPROPER_FILES += .config .config.old tags TAGS cscope* GPATH GTAGS GRTAGS GSYMS

# clean - Delete most, but leave enough to build external modules
clean: rm-dirs  := $(CLEAN_DIRS)
clean: rm-files := $(CLEAN_FILES)
clean-dirs      := $(addprefix _clean_, $(uniloader-all-dirs))

PHONY += $(clean-dirs) clean archclean
$(clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _clean_%,%,$@)

clean: $(clean-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find . $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name modules.builtin -o -name '.tmp_*.o.*' \
		-o -name '*.gcno' \) -type f -print | xargs rm -f

# mrproper - Delete all generated files, including .config
mrproper: rm-dirs  := $(wildcard $(MRPROPER_DIRS))
mrproper: rm-files := $(wildcard $(MRPROPER_FILES))
mrproper-dirs      := $(addprefix _mrproper_, scripts)

PHONY += $(mrproper-dirs) mrproper
$(mrproper-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _mrproper_%,%,$@)

mrproper: clean $(mrproper-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)

# distclean
PHONY += distclean
distclean: mrproper
	@find $(srctree) $(RCS_FIND_IGNORE) \
		\( -name '*.orig' -o -name '*.rej' -o -name '*~' \
		-o -name '*.bak' -o -name '#*#' -o -name '.*.orig' \
		-o -name '.*.rej' -o -size 0 \
		-o -name '*%' -o -name '.*.cmd' -o -name 'core' \) \
		-type f -print | xargs rm -f

# ===========================================================================
# Helper commands

quiet_cmd_rmdirs = $(if $(wildcard $(rm-dirs)),CLEAN   $(wildcard $(rm-dirs)))
      cmd_rmdirs = rm -rf $(rm-dirs)

quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),CLEAN   $(wildcard $(rm-files)))
      cmd_rmfiles = rm -f $(rm-files)

# Shorthand for $(Q)$(MAKE) -f scripts/Makefile.clean obj=dir
# Usage:
# $(Q)$(MAKE) $(clean)=dir
clean := -f $(if $(KBUILD_SRC),$(srctree)/)scripts/Makefile.clean obj

# ===========================================================================
# Help

help:
	@echo  'Cleaning targets:'
	@echo  '  clean		  - Remove most generated files but keep the config and'
	@echo  '                    enough build support to build external modules'
	@echo  '  mrproper	  - Remove all generated files + config + various backup files'
	@echo  '  distclean	  - mrproper + remove editor backup and patch files'
	@echo  ''
	@echo  'Configuration targets:'
	@$(MAKE) -f $(srctree)/scripts/kconfig/Makefile help
	@echo  ''
	@echo  'Other generic targets:'
	@echo  '  all		  - Build all targets marked with [*]'
	@echo  '* uniLoader	  - Build the uniLoader application'
	@echo  '  dir/            - Build all files in dir and below'
	@echo  '  dir/file.[oisS] - Build specified target only'
	@echo  '  dir/file.lst    - Build specified mixed source/assembly target only'
	@echo  '                    (requires a recent binutils and recent build (System.map))'
	@echo  '  tags/TAGS	  - Generate tags file for editors'
	@echo  '  cscope	  - Generate cscope index'
	@echo  '  gtags           - Generate GNU GLOBAL index'
	@echo  '  kernelrelease	  - Output the release version string'
	@echo  '  kernelversion	  - Output the version stored in Makefile'
	@echo  ''
	@echo  'Static analysers:'
	@echo  '  checkstack      - Generate a list of stack hogs'
	@echo  '  namespacecheck  - Name space analysis on compiled kernel'
	@echo  '  versioncheck    - Sanity check on version.h usage'
	@echo  '  includecheck    - Check for duplicate included header files'
	@echo  '  export_report   - List the usages of all exported symbols'
	@echo  '  headers_check   - Sanity check on exported headers'
	@echo  ''
	@echo  '  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build'
	@echo  '  make V=2   [targets] 2 => give reason for rebuild of target'
	@echo  '  make O=dir [targets] Locate all output files in "dir", including .config'
	@echo  '  make W=n   [targets] Enable extra gcc checks, n=1,2,3 where'
	@echo  '		1: warnings which may be relevant and do not occur too often'
	@echo  '		2: warnings which occur quite often but may still be relevant'
	@echo  '		3: more obscure warnings, can most likely be ignored'
	@echo  '		Multiple levels can be combined with W=12 or W=123'
	@echo  '  make RECORDMCOUNT_WARN=1 [targets] Warn about ignored mcount sections'
	@echo  ''
	@echo  'Execute "make" or "make all" to build all targets marked with [*] '
	@echo  'For further info see the ./README file'

endif #ifeq ($(config-targets),1)
endif #ifeq ($(mixed-targets),1)

endif	# skip-makefile

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
