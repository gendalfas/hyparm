APP_NAME := hypervisor

# Don't use built-in rules and variables for subsequent invocations of make.
# For makefile debugging, add: --warn-undefined-variables
MAKEFLAGS += -rR

SOURCE_PATH  := src
SCRIPT_PATH  := scripts
OUTPUT_PATH  := build

VPATH :=

HOSTCC       := gcc

HOSTCFLAGS   := -O2 -fomit-frame-pointer -Wall
HOSTCPPFLAGS :=


CC            = $(CROSS_COMPILE)gcc
AS            = $(CROSS_COMPILE)as
LD            = $(CROSS_COMPILE)ld
OBJCOPY       = $(CROSS_COMPILE)objcopy
OBJDUMP       = $(CROSS_COMPILE)objdump

AFLAGS       := --fatal-warnings
CFLAGS       := -pipe \
                -marm -mabi=aapcs-linux -mno-thumb-interwork -msoft-float \
		-O3 -ffixed-r8 -ffreestanding \
		-fno-builtin -fno-common -fno-stack-protector -fno-toplevel-reorder \
		-Wall -Wextra -Winline -Wstrict-prototypes -Wwrite-strings \
		-Wno-empty-body -Wno-unused-label -Wno-unused-parameter
CPPFLAGS     := -iquote $(SOURCE_PATH) -nostdinc
LDFLAGS      :=


CLEAN_GOALS  := clean clean_%
CONFIG_GOALS := %config
HELP_GOALS   := help

# List of all goals for which no dependency files will be generated and no build goals will be included.
# As %_GOALS variables may change later, delay their expansion.
NO_BUILD_GOALS = $(CLEAN_GOALS) $(CONFIG_GOALS) $(HELP_GOALS)


# Do not allow cleaning and doing anything else at once; in the current setup this would cause dependency tracking to be disabled.
# Fixing this issue is far harder than using "make clean && make".
ifneq ($(filter $(CLEAN_GOALS), $(MAKECMDGOALS)),)
  ifneq ($(filter-out $(CLEAN_GOALS), $(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(CLEAN_GOALS), $(MAKECMDGOALS)))
  endif
endif

# For the same reason, do not allow updates to the configuration file to interfere with the build process.
ifneq ($(filter $(CONFIG_GOALS), $(MAKECMDGOALS)),)
  ifneq ($(filter-out $(CONFIG_GOALS), $(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(CONFIG_GOALS), $(MAKECMDGOALS)))
  endif
endif

# And also ... let help come on its own. Why on earth would you combine it with something else anyway?
ifneq ($(filter $(HELP_GOALS), $(MAKECMDGOALS)),)
  ifneq ($(filter-out $(HELP_GOALS), $(MAKECMDGOALS)),)
    $(error The following goals must be specified separately: $(filter $(HELP_GOALS), $(MAKECMDGOALS)))
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
	@echo 'Error: configuration file missing'; /bin/false

  # Include makefile generated by Kconfig
  include $(KCONFIG_AUTOCONFIG)

  # Configured cross-compiler toolchain overrides environment
  ifneq ($(CONFIG_CROSS_COMPILE),)
    CONFIG_CROSS_COMPILE := $(CONFIG_CROSS_COMPILE:"%"=%)
    ifneq ($(CONFIG_CROSS_COMPILE),)
      CROSS_COMPILE := $(CONFIG_CROSS_COMPILE:"%"=%)
    endif
  endif

  # Crash and burn if no cross compiler is configured
  ifeq ($(CROSS_COMPILE),)
    $(error You need to set up a cross compiler toolchain)
  endif


  AFLAGS-y :=
  CFLAGS-y :=
  CPPFLAGS-y := -imacros $(KCONFIG_AUTOHEADER)
  LDFLAGS-y :=


  # TODO AFLAGS IS A HACK
  AFLAGS-$(CONFIG_ARCH_V7_A) += --defsym CONFIG_ARCH_V7_A=1


  # TODO AFLAGS IS A HACK
  AFLAGS-$(CONFIG_ARCH_EXT_SECURITY) += --defsym CONFIG_ARCH_EXT_SECURITY=1


  AFLAGS-$(CONFIG_CPU_CORTEX_A8) += -mcpu=cortex-a8
  CFLAGS-$(CONFIG_CPU_CORTEX_A8) += -mcpu=cortex-a8

  AFLAGS-$(CONFIG_CPU_CORTEX_A9) += -mcpu=cortex-a9
  CFLAGS-$(CONFIG_CPU_CORTEX_A9) += -mcpu=cortex-a9


  # TODO AFLAGS IS A HACK
  AFLAGS-$(CONFIG_SOC_TI_OMAP_35XX) += --defsym CONFIG_SOC_TI_OMAP_35XX=1


  LDFLAGS-$(CONFIG_SOC_TI_OMAP_3530) += -T $(SCRIPT_PATH)/omap3530.lds


  CFLAGS-$(CONFIG_DEBUG)  += -g -fstack-usage -Wframe-larger-than=256
  LDFLAGS-$(CONFIG_DEBUG) += -g -Map $(OUTPUT_PATH)/$(APP_NAME).map --cref


  AFLAGS   += $(AFLAGS-y)
  CFLAGS   += $(CFLAGS-y)
  CPPFLAGS += $(CPPFLAGS-y)
  LDFLAGS  += $(LDFLAGS-y)

endif # ifneq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)


HYPARM_DIRS-y := common cpuArch drivers/beagle exceptions guestManager instructionEmu linuxBoot memoryManager vm/omap35xx

HYPARM_DIRS-$(CONFIG_CLI) += cli

HYPARM_DIRS-$(CONFIG_MMC) += io io/fs

HYPARM_SRCS_C-y := main.c
HYPARM_SRCS_S-y := startup.s


# Include all makefile.mk files from HYPARM_DIRS-y
include $(foreach DIR, $(HYPARM_DIRS-y), $(SOURCE_PATH)/$(DIR)/makefile.mk)


HYPARM_DEPS   := $(foreach SRC, $(HYPARM_SRCS_C-y), $(SOURCE_PATH)/$(SRC).d)

HYPARM_OBJS_C := $(foreach SRC, $(HYPARM_SRCS_C-y), $(SOURCE_PATH)/$(SRC).o)
HYPARM_OBJS_S := $(foreach SRC, $(HYPARM_SRCS_S-y), $(SOURCE_PATH)/$(SRC).o)
HYPARM_OBJS   := $(HYPARM_OBJS_C) $(HYPARM_OBJS_S)


# Check if we are building
ifeq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)

  # Include automatically generated dependency files.
  -include $(HYPARM_DEPS)


.PHONY: binary dump

binary: $(OUTPUT_PATH)/$(APP_NAME).bin

dump: $(OUTPUT_PATH)/$(APP_NAME).dump


$(OUTPUT_PATH)/$(APP_NAME).dump: $(OUTPUT_PATH)/$(APP_NAME).elf
	@echo OBJDUMP $@
	@$(OBJDUMP) -d $< > $@

$(OUTPUT_PATH)/$(APP_NAME).bin: $(OUTPUT_PATH)/$(APP_NAME).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $< $@

$(OUTPUT_PATH)/$(APP_NAME).elf: $(HYPARM_OBJS) $(KCONFIG_CONFIG)
	@echo LD $@
	@$(LD) -o $@ $(LDFLAGS) $(filter-out $(KCONFIG_CONFIG), $+)


$(SOURCE_PATH)/%.c.d: $(SOURCE_PATH)/%.c $(KCONFIG_OK)
	@echo DEP $<
	@$(CC) -M $(CPPFLAGS) -MP -MT $(patsubst %.c,%.c.o,$<) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm $@.$$$$

$(SOURCE_PATH)/%.c.o: $(SOURCE_PATH)/%.c
	@echo CC  $<
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(SOURCE_PATH)/%.s.o: $(SOURCE_PATH)/%.s $(KCONFIG_CONFIG)
	@echo AS  $<
	@$(AS) $(AFLAGS) -o $@ $<

endif # ifeq ($(filter $(NO_BUILD_GOALS)),)


.PHONY: clean

clean: clean_kconfig
	@for file in $(sort $(HYPARM_DEPS) $(HYPARM_OBJS)) $(OUTPUT_PATH)/$(APP_NAME).{elf,map,bin,dump}; do \
		if [ -f $$file ]; then \
			echo RM $$file; \
			rm $$file || :; \
		fi; \
	done


.PHONY: help

help:
	@echo 'Cleaning targets:'
	@echo '  clean          Remove all generated files but keep $(KCONFIG_CONFIG)'
	@echo '  clean_kconfig  Remove files generated during KConfig build'
	@echo
	@echo 'Configuration targets:'
	@echo '  config         Create or update $(KCONFIG_CONFIG)'
	@echo '  defconfig      Create $(KCONFIG_CONFIG) with defaults from $(SCRIPT_PATH)/defconfig'
	@echo
	@echo 'Build targets:'
	@echo '  all            Build all targets marked with [*]'
	@echo '* binary         Build $(APP_NAME).bin'
	@echo '* dump           Build $(APP_NAME).dump'
	@echo
	@echo 'Some influential environment variables:'
	@echo '  CROSS_COMPILE  Cross-compiler toolchain prefix'
	@echo
	@echo 'Values for these variables specified in $(KCONFIG_CONFIG) override environment values.'

