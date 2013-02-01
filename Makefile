APP_NAME := hypervisor

# Don't use built-in rules and variables for subsequent invocations of make.
# For makefile debugging, add: --warn-undefined-variables
MAKEFLAGS += -rR --no-print-directory

SOURCE_PATH  := src
SCRIPT_PATH  := scripts
OUTPUT_PATH  := build

COMMA        := ,
EMPTY        :=
SPACE        := $(EMPTY) $(EMPTY)

VPATH        :=

HOSTCC       := gcc

HOSTCFLAGS   := -O2 -fomit-frame-pointer -Wall
HOSTCPPFLAGS :=


CC            = $(CROSS_COMPILE)gcc
AS            = $(CROSS_COMPILE)as
LD            = $(CROSS_COMPILE)ld
OBJCOPY       = $(CROSS_COMPILE)objcopy
OBJDUMP       = $(CROSS_COMPILE)objdump

AFLAGS       := --fatal-warnings
CFLAGS       := -marm -mabi=aapcs-linux -mno-thumb-interwork -msoft-float \
                -ggdb3 -std=gnu99 \
                -ffreestanding -fno-common -fno-stack-protector \
                -Wall -Wextra -Wcast-qual -Wformat=2 -Winit-self -Winline -Wlogical-op \
                -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpacked \
                -Wredundant-decls -Wshadow -Wstrict-prototypes -Wundef -Wvla -Wwrite-strings \
                -Wno-empty-body -Wno-unused-label -Wno-unused-parameter \
                -Werror=format-extra-args \
                -Werror=implicit-function-declaration -Werror=implicit-int \
                -Werror=init-self -Werror=parentheses -Werror=return-type -Werror=uninitialized
CPPFLAGS     := -iquote $(SOURCE_PATH) -nostdinc
LDDEPS       :=
LDFLAGS      := --cref --error-unresolved-symbols -Map $(OUTPUT_PATH)/$(APP_NAME).map
OBJDUMPFLAGS := -lS -M reg-names-std
ADFLAGS      := --decoder-stages 1 --decoder-type g --exhaustive-test


CLEAN_GOALS  := clean clean_% distclean
CONFIG_GOALS := %config
AD_GOALS     := autodecoder_arm autodecoder_t16 autodecoder_t32
HELP_GOALS   := help

# List of all goals for which no dependency files will be generated and no build goals will be included.
# As %_GOALS variables may change later, delay their expansion.
NO_BUILD_GOALS = $(CLEAN_GOALS) $(CONFIG_GOALS) $(HELP_GOALS)


# Do not allow cleaning and doing anything else at once; in the current setup this would cause dependency tracking to be disabled.
# Fixing this issue is far harder than using "make clean && make".
ifneq ($(filter $(CLEAN_GOALS),$(MAKECMDGOALS)),)
  ifneq ($(filter-out $(CLEAN_GOALS),$(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(CLEAN_GOALS),$(MAKECMDGOALS)))
  endif
endif

# For the same reason, do not allow updates to the configuration file to interfere with the build process.
ifneq ($(filter $(CONFIG_GOALS),$(MAKECMDGOALS)),)
  ifneq ($(filter-out $(CONFIG_GOALS),$(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(CONFIG_GOALS),$(MAKECMDGOALS)))
  endif
endif

# Make autodecoder stuff run on its own.
ifneq ($(filter $(AD_GOALS),$(MAKECMDGOALS)),)
  ifneq ($(filter-out $(AD_GOALS),$(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(AD_GOALS),$(MAKECMDGOALS)))
  endif
endif

# And also ... let help come on its own. Why on earth would you combine it with something else anyway?
ifneq ($(filter $(HELP_GOALS),$(MAKECMDGOALS)),)
  ifneq ($(filter-out $(HELP_GOALS),$(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(HELP_GOALS),$(MAKECMDGOALS)))
  endif
endif


.PHONY: all

all: _all


# Cancel implicit rules on makefiles (reduces output of make -d)
Makefile: ;
%/makefile.in: ;
%/makefile.mk: ;


# Include Kconfig makefile
include $(SCRIPT_PATH)/kconfig/makefile.mk


# Check if we are building
ifneq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)

.PHONY: _all

_all: ;

else

.PHONY: _all

_all: $(KCONFIG_CONFIG) binary dump

$(KCONFIG_CONFIG):
	@echo 'MAKE     defconfig'
	+@$(MAKE) defconfig

  # Include makefile generated by Kconfig
  include $(KCONFIG_AUTOCONFIG)

  # Crash and burn if no cross compiler is configured
  ifeq ($(CROSS_COMPILE),)
    $(error You need to set up a cross compiler toolchain)
  endif


  AFLAGS-y :=
  CFLAGS-y :=
  CPPFLAGS-y := -imacros $(KCONFIG_AUTOHEADER)
  LDDEPS-y :=
  LDFLAGS-y :=


  CFLAGS-$(CONFIG_OLEVEL_0) += -O0
  CFLAGS-$(CONFIG_OLEVEL_S) += -Os -Wno-inline
  CFLAGS-$(CONFIG_OLEVEL_1) += -O1
  CFLAGS-$(CONFIG_OLEVEL_2) += -O2
  CFLAGS-$(CONFIG_OLEVEL_3) += -O3
  CFLAGS-$(CONFIG_OLEVEL_FAST) += -Ofast


  CFLAGS-$(CONFIG_BUILD_ANALYZE_FUNCTIONS) += -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn

  CFLAGS-$(CONFIG_BUILD_NO_UNALIGNED_ACCESS) += -mno-unaligned-access

  CFLAGS-$(CONFIG_BUILD_SSP) += -fstack-protector-all -Wstack-protector

  CFLAGS-$(CONFIG_BUILD_LTO) += -flto=jobserver -fuse-linker-plugin

  ifeq ($(CONFIG_BUILD_SAVE_TEMPS),y)
    CPPFLAGS-y += -save-temps=obj
  else
    CFLAGS-y += -pipe
  endif


  AFLAGS-$(CONFIG_CPU_CORTEX_A8) += -mcpu=cortex-a8
  CFLAGS-$(CONFIG_CPU_CORTEX_A8) += -mcpu=cortex-a8

  AFLAGS-$(CONFIG_CPU_CORTEX_A9) += -mcpu=cortex-a9
  CFLAGS-$(CONFIG_CPU_CORTEX_A9) += -mcpu=cortex-a9

  LDDEPS-$(CONFIG_SOC_TI_OMAP_3530) += $(SCRIPT_PATH)/omap3530.lds
  LDFLAGS-$(CONFIG_SOC_TI_OMAP_3530) += -T $(SCRIPT_PATH)/omap3530.lds


  AFLAGS       += $(AFLAGS-y)
  CFLAGS       += $(CFLAGS-y)
  CPPFLAGS     += $(CPPFLAGS-y)
  LDDEPS       += $(LDDEPS-y)
  LDFLAGS      += $(LDFLAGS-y)
  OBJDUMPFLAGS += $(OBJDUMPFLAGS-y)


  # Some assembly files are preprocessed by the C preprocessor (.S extension) and assembler flags
  # are passed through the C compiler rather than directly to the assembler. Adapt AFLAGS to a
  # format understood by the compiler.
  CPP_AFLAGS := $(foreach AFLAG,$(AFLAGS),$(AFLAG))
  ifneq ($(CPP_AFLAGS),)
    CPP_AFLAGS := -Wa,$(subst $(SPACE),$(COMMA),$(CPP_AFLAGS))
  endif

  # With LTO, the linker is invoked by the compiler. Hence, linker flags are passed through the
  # compiler rather than directly to the linker, so we need to adapt LDFLAGS to a format understood
  # by the compiler.
  ifeq ($(CONFIG_BUILD_LTO),y)
    LDFLAGS := $(foreach LDFLAG,$(LDFLAGS),$(LDFLAG))
    ifneq ($(LDFLAGS),)
      LDFLAGS := -Wl,$(subst $(SPACE),$(COMMA),$(LDFLAGS))
    endif
  endif

endif # ifneq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)


HYPARM_DIRS-y := common common/memoryAllocator cpuArch drivers/beagle exceptions guestBoot \
                 guestManager instructionEmu instructionEmu/decoder instructionEmu/interpreter \
                 instructionEmu/translator memoryManager vm/omap35xx

# sd-card
HYPARM_DIRS-$(CONFIG_MMC_LOG) += io io/fs
HYPARM_DIRS-$(CONFIG_MMC_GUEST_ACCESS) += io io/fs


# Hacks
HYPARM_DIRS-$(CONFIG_CLI) += cli

HYPARM_SRCS_C-y  := main.c
HYPARM_SRCS_S-y  :=
HYPARM_SRCS_SX-y := startup.S debug.S


# Include all makefile.mk files from HYPARM_DIRS-y
include $(foreach DIR,$(HYPARM_DIRS-y),$(SOURCE_PATH)/$(DIR)/makefile.mk)


HYPARM_DEPS    := $(foreach SRC,$(HYPARM_SRCS_C-y),$(SOURCE_PATH)/$(SRC).d)
HYPARM_DEPS    += $(foreach SRC,$(HYPARM_SRCS_SX-y), $(SOURCE_PATH)/$(SRC).d)

HYPARM_OBJS_C  := $(foreach SRC,$(HYPARM_SRCS_C-y),$(SOURCE_PATH)/$(SRC).o)
HYPARM_OBJS_S  := $(foreach SRC,$(HYPARM_SRCS_S-y),$(SOURCE_PATH)/$(SRC).o)
HYPARM_OBJS_SX := $(foreach SRC,$(HYPARM_SRCS_SX-y),$(SOURCE_PATH)/$(SRC).o)
HYPARM_OBJS    := $(HYPARM_OBJS_C) $(HYPARM_OBJS_S) $(HYPARM_OBJS_SX)


# Check if we are building
ifeq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)

  # Include automatically generated dependency files.
  -include $(HYPARM_DEPS)


.PHONY: binary dump

binary: $(OUTPUT_PATH)/$(APP_NAME).bin

dump: $(OUTPUT_PATH)/$(APP_NAME).dump


# Generate an assembly listing of the final linker output, possible intermixed with source code
$(OUTPUT_PATH)/$(APP_NAME).dump: $(OUTPUT_PATH)/$(APP_NAME).elf
	@echo 'OBJDUMP  $@'
	@$(OBJDUMP) -d $(OBJDUMPFLAGS) $< > $@

# Create a binary image
$(OUTPUT_PATH)/$(APP_NAME).bin: $(OUTPUT_PATH)/$(APP_NAME).elf
	@echo 'OBJCOPY  $@'
	@$(OBJCOPY) -O binary $< $@

# Link and optionally run link-time optimization (LTO).
#  * When not using LTO, we can just invoke cross-ld.
#  * With LTO, cross-cc has to be invoked once again to perform the optimizations, and when done it
#    will automatically invoke the linker. Hence, linker flags must be passed to the compiler.
$(OUTPUT_PATH)/$(APP_NAME).elf: $(HYPARM_OBJS) $(KCONFIG_CONFIG) $(LDDEPS)
ifneq ($(CONFIG_BUILD_LTO),y)
	@echo 'LD       $@'
	@$(LD) -o $@ $(LDFLAGS) $(HYPARM_OBJS)
else
	@echo 'CC/LD    $@'
	+@$(CC) $(CFLAGS) -nostartfiles -nostdlib -o $@ $(LDFLAGS) $(HYPARM_OBJS)
endif


# Generate dependency files for all C source files. The C compiler scans a source file to identify
# all the files it includes and generates a dependency file that can be included from this Makefile.
#
# For any filename.c, the compiler by default outputs a rule
#   filename.o : includedFile1.h ... includedFileN.h
#
# Since headers might be deleted (or renamed) in between builds, it is a good idea to prevent make
# from trying to build them (make will always try to build files it cannot find). Using the -MP
# switch, the compiler will generate empty rules for all included headers:
#  includedFile1.h:
#  includedFile2.h:
#
# Object files compiled from C source files are named filename.c.o rather than filename.o to prevent
# collisions with object files compiled from assembly source files, and to be able to easily
# distinguish between the two. Hence, the rule for filename.o must be changed to filename.c.o. This
# can be achieved with the -MT switch. However, the dependency file for filename.c should also be
# regenerated whenever any of the headers changes (it may include a different set of other headers).
# The easiest way to achieve this is to add the dependency file to the rule for filename.c.o.
#
# Solution:
# 1) We pass -MT __out__ to the compiler so the first rule will always read
#      __out__ : includedFile1.h ... includedFileN.h
# 2) We use sed to replace '__out__' with 'filename.c.o filename.c.d'
$(SOURCE_PATH)/%.c.d: $(SOURCE_PATH)/%.c $(KCONFIG_OK)
ifneq ($(VERBOSE),)
	@echo 'DEP      $<'
endif
	@$(CC) -M $(CPPFLAGS) -MP $< -MT __out__ | sed 's,__out__[ :]*,$<.o $@ : ,g' > $@

# Generate dependency files for all assembly source files that require preprocessing. This rule is
# exactly the same as the rule above, except that it generates *.S.d files from *.S files.
$(SOURCE_PATH)/%.S.d: $(SOURCE_PATH)/%.S $(KCONFIG_OK)
ifneq ($(VERBOSE),)
	@echo 'DEP      $<'
endif
	@$(CC) -M $(CPPFLAGS) -MP $< -MT __out__ | sed 's,__out__[ :]*,$<.o $@ : ,g' > $@


$(SOURCE_PATH)/%.c.o: $(SOURCE_PATH)/%.c
	@echo 'CC       $<'
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(SOURCE_PATH)/%.s.o: $(SOURCE_PATH)/%.s $(KCONFIG_CONFIG)
	@echo 'AS       $<'
	@$(AS) $(AFLAGS) -o $@ $<

$(SOURCE_PATH)/%.S.o: $(SOURCE_PATH)/%.S $(KCONFIG_CONFIG)
	@echo 'CPP/AS   $<'
	@$(CC) $(CPP_AFLAGS) $(CPPFLAGS) -c -o $@ $<


  # Check if we are invoking autodecoder
  ifneq ($(filter $(AD_GOALS),$(MAKECMDGOALS)),)

    # Configured path to autodecoder overrides environment
    ifneq ($(CONFIG_PATH_AUTODECODER),)
      CONFIG_PATH_AUTODECODER := $(CONFIG_PATH_AUTODECODER:"%"=%)
      ifneq ($(CONFIG_PATH_AUTODECODER),)
        PATH_AUTODECODER := $(CONFIG_PATH_AUTODECODER:"%"=%)
      endif
    endif

    # Crash and burn if the path to autodecoder is not configured
    ifeq ($(PATH_AUTODECODER),)
      $(error You need to set up the path to autodecoder)
    endif

.PHONY: $(AD_GOALS)

.SECONDEXPANSION:
AD_SRC_autodecoder_arm := arm
AD_SRC_autodecoder_t16 := t16
AD_SRC_autodecoder_t32 := t32

$(AD_GOALS): $(SOURCE_PATH)/instructionEmu/decoder/$$(AD_SRC_$$@)/decoder.xml $(KCONFIG_CONFIG) $(KCONFIG_OK)
	cd $(PATH_AUTODECODER) && $(PATH_AUTODECODER)/autodecoder --generate-decoder \
	  --decoder-spec $(abspath $<) $(ADFLAGS) \
	  --out $(abspath $(patsubst %decoder.xml,%graph.inc.c,$<))

  endif # ifneq ($(filter $(AD_GOALS),$(MAKECMDGOALS)),)

endif # ifeq ($(filter $(NO_BUILD_GOALS)),)


.PHONY: clean distclean

clean:
	@find $(SOURCE_PATH) -name '*.c.[dis]' -exec rm {} +
	@find $(SOURCE_PATH) -name '*.c.d.*' -exec rm {} +
	@find $(SOURCE_PATH) -name '*.o' -exec rm {} +
	@find $(SOURCE_PATH) -name '*.S.[ds]' -exec rm {} +
	@rm $(KCONFIG_AUTOCONFIG) $(KCONFIG_AUTOHEADER) $(KCONFIG_OK) \
	  $(OUTPUT_PATH)/$(APP_NAME).{elf,map,bin,dump} 2> /dev/null || :

distclean: clean clean_kconfig
	@rm -r $(KCONFIG_CONFIG) $(KCONFIG_CONFIG).old $(OUTPUT_PATH) 2> /dev/null || :


.PHONY: help

help:
	@echo 'Cleaning targets:'
	@echo '  clean             Remove files generated during hypervisor build'
	@echo '  clean_kconfig     Remove files generated during KConfig build'
	@echo '  distclean         Remove all generated files including $(KCONFIG_CONFIG)'
	@echo
	@echo 'Configuration targets:'
	@echo '  config            Create or update $(KCONFIG_CONFIG)'
	@echo '  defconfig         Create $(KCONFIG_CONFIG) with defaults from $(SCRIPT_PATH)/defconfig'
	@echo
	@echo 'Build targets:'
	@echo '  all               Build all targets marked with [*]'
	@echo '* binary            Build $(APP_NAME).bin'
	@echo '* dump              Build $(APP_NAME).dump'
	@echo
	@echo 'Autodecoder targets:'
	@echo '  autodecoder_arm   Create C source for ARM decoder'
	@echo '  autodecoder_t16   Create C source for Thumb 16-bit decoder'
	@echo '  autodecoder_t32   Create C source for Thumb 32-bit decoder'
	@echo
	@echo 'Some influential environment variables:'
	@echo '  CROSS_COMPILE     Cross-compiler toolchain prefix'
	@echo '  PATH_AUTODECODER  Path to autodecoder'
	@echo
	@echo 'Values for these variables specified in $(KCONFIG_CONFIG) override environment values.'

