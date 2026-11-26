#
# Copyright (c) 2013-2024, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Filename: rules_ti_cgt_arm.mk
#
# Make rules for TI ARM CGT - This file has all the common rules and defines
# required for Cortex-M4/R5F ISA
#
# This file needs to change when:
#     1. Code generation tool chain changes (currently it uses TI ARM CGT)
#     2. Internal switches (which are normally not touched) has to change
#     4. a rule common for M4/R5F ISA has to be added or modified

# Set compiler/archiver/linker commands and include paths
ifeq ($(CGT_ISA),$(filter $(CGT_ISA), M4F))
CGT_ISA_PATH_PRFX = M4
else
CGT_ISA_PATH_PRFX = $(CGT_ISA)
endif

CODEGEN_INCLUDE = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/include/c
CC = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/bin/tiarmclang
AR = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/bin/tiarmar
LNK = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/bin/tiarmclang
STRP = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/bin/tiarmstrip
SIZE = $(TOOLCHAIN_PATH_$(CGT_ISA_PATH_PRFX))/bin/tiarmsize

# Derive a part of RTS Library name based on ENDIAN: little/big
ifeq ($(ENDIAN),little)
  RTSLIB_ENDIAN = EL

else
  RTSLIB_ENDIAN = EB
endif

# Derive compiler switch and part of RTS Library name based on FORMAT: COFF/ELF
ifeq ($(FORMAT),COFF)
  CSWITCH_FORMAT = ti_arm9_abi
  RTSLIB_FORMAT = tiarm9
endif
ifeq ($(FORMAT),ELF)
  CSWITCH_FORMAT = eabi
  RTSLIB_FORMAT = eabi
endif

# OPTIMIZATION can take two values i.e. CODE and PERFORMANCE
# This is parameter is only valid for release mode and not for debug mode
# CODE is recommended for optimizing code size
# PERFORMANCE is recommended for optimizing performance, but it is likely to increase compiler generated code size
ifeq ($(OPTIMIZATION),PERFORMANCE)
  OPT_LEVEL = -O3
else
  OPT_LEVEL = -Os
endif

LNKFLAGS_INTERNAL_COMMON +=

SUPRESS_WARNINGS_FLAG = -Wno-extra -Wno-exceptions -ferror-limit=100 -Wno-unused-command-line-argument -Wno-unused-function -Wno-extern-initializer -Wno-excess-initializers -Wno-bitfield-constant-conversion -Wno-address-of-packed-member

ifeq ($(CPLUSPLUS_BUILD), yes)
  SUPRESS_WARNINGS_FLAG += -Wno-c99-designator -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-reorder-init-list -Wno-deprecated-register -Wno-writable-strings -Wno-enum-compare -Wno-reserved-user-defined-literal -Wno-unused-const-variable -Wno-missing-braces
endif

# Internal CFLAGS - normally doesn't change
ifeq ($(CGT_ISA),$(filter $(CGT_ISA),M4 R5 M3))
  CFLAGS_INTERNAL = -c -Wall $(SUPRESS_WARNINGS_FLAG) -fno-strict-aliasing -$(RTSLIB_ENDIAN) -g -mfloat-abi=hard
  ifeq ($(CGT_ISA),$(filter $(CGT_ISA), R5))
    CFLAGS_INTERNAL += -mfpu=vfpv3-d16 -mcpu=cortex-r5
    ifeq ($(COMPILE_MODE), thumb)
      # Enabling thumb2 mode
      CFLAGS_INTERNAL += -mthumb -march=thumbv7r
    # else
    #   # Enabling arm mode
    #   CFLAGS_INTERNAL += -marm -march=armv7-r
    endif
  else
    CFLAGS_INTERNAL += -mfpu=vfplib
  endif
else ifeq ($(CGT_ISA), Arm9)
  CFLAGS_INTERNAL = -c -Wall $(SUPRESS_WARNINGS_FLAG) --endian=$(ENDIAN) -mv5e --float_support=vfplib -eo.$(OBJEXT) -ea.$(ASMEXT) -g
endif

# Reset the CFLAGS_INTERNAL flag for M4F
ifeq ($(CGT_ISA), M4F)
  CFLAGS_INTERNAL = -mcpu=cortex-m4 -c -Wall $(SUPRESS_WARNINGS_FLAG) --endian=$(ENDIAN) -mv7M4 -eo.$(OBJEXT) -ea.$(ASMEXT) -g
  CFLAGS_INTERNAL += -mfpu=fpv4-sp-d16
endif

ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)
  CFLAGS_INTERNAL += -Werror
  LNKFLAGS_INTERNAL_COMMON += -Werror
else
  CFLAGS_INTERNAL += -Wno-all -Wno-error -Wno-unused-value -Wno-unused-variable -Wno-unused-function
endif

ifeq ($(CPLUSPLUS_BUILD), yes)
 CFLAGS_INTERNAL += -nostdlibinc
endif

CFLAGS_DIROPTS = -c

ifneq ($(EXTERNAL_LNKCMD_FILE_LOCAL),)
EXTERNAL_LNKCMD_FILE = $(EXTERNAL_LNKCMD_FILE_LOCAL)
else
EXTERNAL_LNKCMD_FILE = $(CONFIG_BLD_LNK_r5f)
endif

ifeq ($(CGT_ISA),$(filter $(CGT_ISA),M4F))
 ifneq ($(EXTERNAL_LNKCMD_FILE_LOCAL),)
 EXTERNAL_LNKCMD_FILE = $(EXTERNAL_LNKCMD_FILE_LOCAL)
 else
 EXTERNAL_LNKCMD_FILE = $(CONFIG_BLD_LNK_m4f)
 endif
endif

# CFLAGS based on profile selected
ifeq ($(BUILD_PROFILE_$(CORE)), debug)
 LNKFLAGS_INTERNAL_BUILD_PROFILE =
 CFLAGS_INTERNAL += -D_DEBUG_=1 -O1
endif
ifeq ($(BUILD_PROFILE_$(CORE)), release)
 ifeq ($(CGT_ISA),$(filter $(CGT_ISA),M4 R5 M3))
   LNKFLAGS_INTERNAL_BUILD_PROFILE = $(LNKFLAGS_GLOBAL_$(CORE))
   ifeq ($(CGT_ISA),$(filter $(CGT_ISA), R5))
     CFLAGS_INTERNAL += $(OPT_LEVEL)
   else
     CFLAGS_INTERNAL += $(OPT_LEVEL) -op0 -os --optimize_with_debug --inline_recursion_limit=20
   endif
 endif
 ifeq ($(CGT_ISA), Arm9)
    LNKFLAGS_INTERNAL_BUILD_PROFILE = --diag_warning=225 --diag_suppress=23000 $(LNKFLAGS_GLOBAL_$(CORE))
    CFLAGS_INTERNAL += -oe $(OPT_LEVEL) -op0 -os --optimize_with_debug --inline_recursion_limit=20
 endif
 ifeq ($(CGT_ISA),$(filter $(CGT_ISA), M4F))
	# LNKFLAGS_INTERNAL_BUILD_PROFILE = --opt='--float_support=FPv4SPD16 --endian=$(ENDIAN) -mv7M4 -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function $(CFLAGS_GLOBAL_$(CORE)) -oe -g -op2 $(OPT_LEVEL) -os --optimize_with_debug --inline_recursion_limit=20 --diag_suppress=23000' --strict_compatibility=on
     LNKFLAGS_INTERNAL_BUILD_PROFILE = --diag_warning=225 --diag_suppress=23000 $(LNKFLAGS_GLOBAL_$(CORE))
	 CFLAGS_INTERNAL += -oe $(OPT_LEVEL) -op0 -os --optimize_with_debug --inline_recursion_limit=20
 endif
endif

# Assemble CFLAGS from all other CFLAGS definitions
_CFLAGS = $(CFLAGS_GLOBAL_$(CORE)) $(CFLAGS_INTERNAL) $(CFLAGS_LOCAL_COMMON) $(CFLAGS_LOCAL_$(CORE)) $(CFLAGS_COMP_COMMON)
ifeq ($($(MODULE_NAME)_BOARD_DEPENDENCY),yes)
  _CFLAGS += $(CFLAGS_LOCAL_$(BOARD)) $(CFLAGS_GLOBAL_$(BOARD))
else
  ifeq ($($(APP_NAME)_BOARD_DEPENDENCY),yes)
    _CFLAGS += $(CFLAGS_LOCAL_$(BOARD)) $(CFLAGS_GLOBAL_$(BOARD))
  else
    _CFLAGS += $(CFLAGS_LOCAL_$(SOC)) $(CFLAGS_GLOBAL_$(SOC))
  endif
endif

ifeq ($(CPLUSPLUS_BUILD), yes)
  ifeq ($(CPLUSPLUS_ENET), yes)
	# For cpp files removed '-nostdlibinc' for Enet LLD libs
    _CFLAGS := $(filter-out -nostdlibinc,$(_CFLAGS))
  endif
endif

# Decide the compile mode
COMPILEMODE = -x c
ifeq ($(CPLUSPLUS_BUILD), yes)
  COMPILEMODE = -x c++
endif

# Object file creation
# The first $(CC) generates the dependency make files for each of the objects
# The second $(CC) compiles the source to generate object
$(OBJ_PATHS): $(OBJDIR)/%.$(OBJEXT): %.c $(GEN_FILE) | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE):$<
	$(MKDIR) -p $(dir $@)
	$(CC) -MMD $(_CFLAGS) $(INCLUDES) -c $(COMPILEMODE) $< -o $@ -MF $(DEPFILE).d
	$(CC) $(_CFLAGS) $(INCLUDES) -c $(COMPILEMODE) $< -o $@

$(OBJ_PATHS_CPP): $(OBJDIR)/%.$(OBJEXT): %.cpp $(GEN_FILE) | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE):$<
	$(MKDIR) -p $(dir $@)
	$(CC) -MMD $(_CFLAGS) $(INCLUDES) -c -x c++ $< -o $@ -MF $(DEPFILE).d
	$(CC) $(_CFLAGS) $(INCLUDES) -c -x c++ $< -o $@

#TODO: Check ASMFLAGS if really required
ASMFLAGS = -me -g -mthumb --diag_warning=225

ifeq ($(RB_ENABLE), true)
  _CFLAGS += -Xti-assembler --define=SM_READBACK_ENABLED=1
else
  _CFLAGS += -Xti-assembler --define=SM_READBACK_ENABLED=0
endif

# Object file creation
$(OBJ_PATHS_ASM): $(OBJDIR)/%.$(OBJEXT): %.asm $(GEN_FILE) | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE): $<
	$(CC) -MMD $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -x ti-asm $< -o $@
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -x ti-asm $< -o $@

# Object file creation
$(OBJ_PATHS_S): $(OBJDIR)/%.$(OBJEXT): %.S $(GEN_FILE) | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE): $<
	$(CC) -MMD -c $(_CFLAGS) $< -o $@ -MF $(DEPFILE).d
	$(CC) -c $(_CFLAGS) $< -o $@

$(PACKAGE_PATHS): $(PACKAGEDIR)/%: %
	$(ECHO) \# Copying to $(PACKAGE_RELPATH)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)/$<
	$(MKDIR) -p $(PACKAGE_ROOT)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)
	$(CP) --parents -rf $< $(PACKAGE_ROOT)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)

# Archive flags - normally doesn't change
ARFLAGS = rc

# Archive/library file creation
$(LIBDIR)/$(LIBNAME).$(LIBEXT) : $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP) $(GEN_FILE) | $(LIBDIR)
	$(ECHO) \#
	$(ECHO) \# Archiving $(PRINT_MESSAGE) into $@ ...
	$(ECHO) \#
	$(AR) $(ARFLAGS) $@ $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP)

$(LIBDIR)/$(LIBNAME).$(LIBEXT)_size: $(LIBDIR)/$(LIBNAME).$(LIBEXT)
	$(ECHO) \#
	$(ECHO) \# Computing sectti size $(PRINT_MESSAGE) info into $@.txt ...
	$(ECHO) \#
	$(SIZE) -x $(subst _size,,$@) > $@temp
	$(SECTTI) $@temp > $@.txt
	$(RM)   $@temp

# Linker options and rules
LNKFLAGS_INTERNAL_COMMON += -Xlinker -q -Xlinker -u -Xlinker _c_int00 -Xlinker --display_error_number -Xlinker --use_memcpy=fast -Xlinker --use_memset=fast
# Supress warning for "entry-point symbol other than "_c_int00" specified"
LNKFLAGS_INTERNAL_COMMON += -Xlinker --diag_suppress=10063-D
# Supress warning for "no matching section"
LNKFLAGS_INTERNAL_COMMON += -Xlinker --diag_suppress=10068-D
# Supress warning for " LOAD placement ignored for "":  object is uninitialized"
LNKFLAGS_INTERNAL_COMMON += -Xlinker --diag_suppress=10083-D

ifeq ($(BOARD),$(filter $(BOARD), qtJ7))
  LNKFLAGS_INTERNAL_COMMON += -Xlinker -cr -Xlinker --ram_model
else
  LNKFLAGS_INTERNAL_COMMON += -Xlinker -c
endif

ifeq ($(CGT_ISA), R5)
  LNKFLAGS_INTERNAL_COMMON += -mcpu=cortex-r5 -march=armv7-r
  #--diag_suppress=10063 supresses 'warning: entry point other than _c_int00 specified'
  # Supress warning for " linking in section which does not contain program data"
  LNKFLAGS_INTERNAL_COMMON += -Xlinker --diag_suppress=10230-D
else
ifeq ($(CGT_ISA), M4F)
  LNKFLAGS_INTERNAL_COMMON += --mcpu=cortex-m4
else
ifeq ($(CGT_ISA), Arm9)
  LNKFLAGS_INTERNAL_COMMON +=
else
  LNKFLAGS_INTERNAL_COMMON += --silicon_version=7$(CGT_ISA)
endif
endif
endif

# Assemble Linker flags from all other LNKFLAGS definitions
_LNKFLAGS = $(LNKFLAGS_INTERNAL_COMMON) $(LNKFLAGS_INTERNAL_BUILD_PROFILE) $(LNKFLAGS_GLOBAL_$(CORE)) $(LNKFLAGS_LOCAL_COMMON) $(LNKFLAGS_LOCAL_$(CORE))

# Path of the RTS library - normally doesn't change for a given tool-chain
#Let the linker choose the required library
RTSLIB_PATH = $(CGT_PATH)/lib/libc.a

LNK_LIBS = $(addprefix -l,$(LIB_PATHS))
LNK_LIBS += $(addprefix -l,$(EXT_LIB_PATHS))

# Linker - to create executable file
ifeq ($(LOCAL_APP_NAME),)
 EXE_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
 EXE_STRIP_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(BUILD_PROFILE_$(CORE))_strip.$(EXEEXT)
else
 EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
 EXE_STRIP_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(BUILD_PROFILE_$(CORE))_strip.$(EXEEXT)
endif

ifeq ($($(APP_NAME)_TRICORE),yes)
  EXE2_NAME = $(BINDIR)/$(APP_NAME)_mcu2_0_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
  EXE3_NAME = $(BINDIR)/$(APP_NAME)_mcu3_0_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
endif

NUM_PROCS = 1

ifeq ($(OS),Windows_NT)
  NUM_PROCS = $(NUMBER_OF_PROCESSORS)
else
  NUM_PROCS = $(shell grep -c ^processor /proc/cpuinfo)
endif

# Commenting this out as these are already available with compiler
#ifneq ($(findstring mcu,$(CORE)),)
#RTSLIB_NAME = rtsv7R4_T_le_v3D16_eabi.lib
#BUILD_LIB_ONCE = $(CGT_PATH)/lib/$(RTSLIB_NAME)
#$(BUILD_LIB_ONCE):
#	$(ECHO) \# $@ not found, building  $@ ...
#	$(CGT_PATH)/lib/mklib --pattern=$(RTSLIB_NAME) --parallel=$(NUM_PROCS) --compiler_bin_dir=$(CGT_PATH)/bin
#endif

$(EXE_NAME) : $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP) $(LIB_PATHS) $(LNKCMD_FILE) $(BUILD_LIB_ONCE)
ifeq ($(BUILD_OS_TYPE), freertos)
ifeq ($(wildcard $(BINDIR)/syscfg_c.rov.xs),)
	$(CP) -n $(pdk_PATH)/ti/kernel/freertos/rov/syscfg_c.rov.xs $(BINDIR)
endif
endif

	$(ECHO) \# Linking into $(EXE_NAME)...
	$(ECHO) \#
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP) -Xlinker $(LNKCMD_FILE) $(EXTERNAL_LNKCMD_FILE) $(APPEND_LNKCMD_FILE) -Xlinker --map_file=$@.map -Xlinker --output_file=$@ $(LNK_LIBS) $(RTSLIB_PATH)
	$(ECHO) \#
	$(ECHO) \# $@ created.
	$(ECHO) \#
ifeq ($(BUILD_PROFILE_$(CORE)), release)
	$(ECHO) \# Generating stripped image into $(EXE_STRIP_NAME)...
	$(ECHO) \#
	$(RM)  $(EXE_STRIP_NAME)
	$(STRP) -p $(EXE_NAME) -o $(EXE_STRIP_NAME)
endif
ifeq ($($(APP_NAME)_TRICORE),yes)
	$(ECHO) \# Linking into $(EXE2_NAME)...
	$(ECHO) \#
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP) -Xlinker $(LNKCMD_FILE) $(EXTERNAL_LNKCMD_FILE) $(APPEND2_LNKCMD_FILE) -Xlinker --map_file=$(EXE2_NAME).map -Xlinker --output_file=$(EXE2_NAME) $(LNK_LIBS) $(RTSLIB_PATH)
	$(ECHO) \#
	$(ECHO) \# $(EXE2_NAME) created.
	$(ECHO) \#
	$(ECHO) \# Linking into $(EXE3_NAME)...
	$(ECHO) \#
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS_ASM) $(OBJ_PATHS_S) $(OBJ_PATHS) $(OBJ_PATHS_CPP) -Xlinker $(LNKCMD_FILE) $(EXTERNAL_LNKCMD_FILE) $(APPEND3_LNKCMD_FILE) -Xlinker --map_file=$(EXE3_NAME).map -Xlinker --output_file=$(EXE3_NAME) $(LNK_LIBS) $(RTSLIB_PATH)
	$(ECHO) \#
	$(ECHO) \# $(EXE3_NAME) created.
	$(ECHO) \#
endif

ifeq ($(OS),Windows_NT)
  EVERYONE = $(word 1,$(shell whoami -groups | findstr "S-1-1-0"))
endif

# Include dependency make files that were generated by $(CC)
-include $(SRCS:%.c=$(DEPDIR)/%.d)

# Nothing beyond this point
