#
# Copyright (c) 2017-2024, Texas Instruments Incorporated
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

# Filename: rules_c7x.mk
#
# Make rules for c7x - This file has all the common rules and defines required
#                     for c7x ISA
#
# This file needs to change when:
#     1. Code generation tool chain changes (currently it uses CG600_7.2.0.B2)
#     2. Internal switches (which are normally not touched) has to change
#     3. XDC specific switches change
#     4. a rule common for C7x ISA has to be added or modified

# Set compiler/archiver/linker commands and include paths
CODEGEN_INCLUDE = $(C7X_GEN_INSTALL_PATH)/include
CC = $(C7X_GEN_INSTALL_PATH)/bin/cl7x
AR = $(C7X_GEN_INSTALL_PATH)/bin/ar7x
LNK = $(C7X_GEN_INSTALL_PATH)/bin/cl7x --silicon_version=$(SI_VER) -z
STRP = $(C7X_GEN_INSTALL_PATH)/bin/strip7x
SIZE = $(C7X_GEN_INSTALL_PATH)/bin/ofd7x

# Derive a part of RTS Library name based on ENDIAN: little/big
ifeq ($(ENDIAN),little)
  RTSLIB_ENDIAN =
else
  RTSLIB_ENDIAN = e
endif

# Derive compiler switch and part of RTS Library name based on FORMAT: COFF/ELF
ifeq ($(FORMAT),COFF)
  CSWITCH_FORMAT = ti_arm9_abi
  RTSLIB_FORMAT = _tiarm9
  CSWITCH_FORMAT =
endif
ifeq ($(FORMAT),ELF)
  CSWITCH_FORMAT = --abi=eabi
  RTSLIB_FORMAT = _elf
endif

ifneq ($(EXTERNAL_LNKCMD_FILE_LOCAL),)
  EXTERNAL_LNKCMD_FILE = $(EXTERNAL_LNKCMD_FILE_LOCAL)
  LNKCMD_FILE :=
else
  EXTERNAL_LNKCMD_FILE = $(CONFIG_BLD_LNK_c7x)
endif

# Internal CFLAGS - normally doesn't change
CFLAGS_INTERNAL = -mv$(SI_VER) $(CSWITCH_FORMAT) -q -mo -pden -pds=238 -pds=880 -pds1110 --endian=$(ENDIAN) -eo.$(OBJEXT) -ea.$(ASMEXT)
ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)
  CFLAGS_INTERNAL += --emit_warnings_as_errors
  LNKFLAGS_INTERNAL_COMMON += --emit_warnings_as_errors
endif
ifeq ($(SI_VER), 7100)
  CFLAGS_INTERNAL += --silicon_errata_i2117
endif
CFLAGS_DIROPTS = -fr=$(OBJDIR) -fs=$(OBJDIR)

#
# Suppress this warning, 10063-D: entry-point symbol other than "_c_int00"
# c7x boots in secure mode and to switch to non-secure mode we need to start at
# a special entry point '_c_int00_secure' and later after switching to
# non-secure mode, sysbios jumps to usual entry point of _c_int00. Hence we need
# to suppress this warning
#
LNKFLAGS_INTERNAL_COMMON += --diag_suppress=10063

#C7x should be in RAM model so that constants are set at load time and not
#extracted from cinit post Cinit . This is because early boot routines like
#MMu_init depends on constants being set and they are executed before cinit
LNKFLAGS_INTERNAL_COMMON += --ram_model

# CFLAGS based on profile selected
ifeq ($(BUILD_PROFILE_$(CORE)), debug)
 CFLAGS_INTERNAL +=
 CFLAGS_INTERNAL += -D_DEBUG_=1
 LNKFLAGS_INTERNAL_BUILD_PROFILE =
endif

ifeq ($(BUILD_PROFILE_$(CORE)), release)
 LNKFLAGS_INTERNAL_BUILD_PROFILE =
 CFLAGS_INTERNAL += --opt_level=3
 CFLAGS_INTERNAL += -D_DEBUG_=1
 ifeq ($(BUILD_OS_TYPE), safertos)
  # SafeRTOS package uses .asmfunc and .endasmfunc directives.
  # This will work only with --symdebug:dwarf
  CFLAGS_INTERNAL += --symdebug:dwarf
 else
  CFLAGS_INTERNAL += --symdebug:none
 endif
  CFLAGS_INTERNAL += --advice:performance=none
else
 CFLAGS_INTERNAL += --opt_level=0 --symdebug:dwarf
endif

########################################

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

# Decide the compile mode
COMPILEMODE = -fc
ifeq ($(CPLUSPLUS_BUILD), yes)
  COMPILEMODE = -fg
endif

# Object file creation
# The first $(CC) generates the dependency make files for each of the objects
# The second $(CC) compiles the source to generate object
$(OBJ_PATHS): $(OBJDIR)/%.$(OBJEXT): %.c | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE): $<
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) $(COMPILEMODE) $<
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) $(COMPILEMODE) $<

# Object file creation
$(OBJ_PATHS_ASM): $(OBJDIR)/%.$(OBJEXT): %.asm | $(OBJDIR) $(DEPDIR)
	$(ECHO) \# Compiling $(PRINT_MESSAGE): $<
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fa $<
	$(CC) $(_CFLAGS) $(ASMFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fa $<

$(PACKAGE_PATHS): $(PACKAGEDIR)/%: %
	$(ECHO) \# Copying to $(PACKAGE_RELPATH)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)/$<
	$(MKDIR) -p $(PACKAGE_ROOT)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)
	$(CP) --parents -rf $< $(PACKAGE_ROOT)/$($(APP_NAME)$(MODULE_NAME)_RELPATH)

# Archive flags - normally doesn't change
ARFLAGS = rq

# Archive/library file creation
$(LIBDIR)/$(LIBNAME).$(LIBEXT) : $(OBJ_PATHS_ASM) $(OBJ_PATHS) | $(LIBDIR)
	$(ECHO) \#
	$(ECHO) \# Archiving $(PRINT_MESSAGE) into $@ ...
	$(ECHO) \#
	$(AR) $(ARFLAGS) $@ $(OBJ_PATHS_ASM) $(OBJ_PATHS)

$(LIBDIR)/$(LIBNAME).$(LIBEXT)_size: $(LIBDIR)/$(LIBNAME).$(LIBEXT)
	$(ECHO) \#
	$(ECHO) \# Computing sectti size $(PRINT_MESSAGE) info into $@.txt ...
	$(ECHO) \#
	$(SIZE) -x $(subst _size,,$@) > $@temp
	$(SECTTI) $@temp > $@.txt
	$(RM)   $@temp

# Linker options and rules
LNKFLAGS_INTERNAL_COMMON += --warn_sections -q

# Assemble Linker flags from all other LNKFLAGS definitions
_LNKFLAGS = $(LNKFLAGS_INTERNAL_COMMON) $(LNKFLAGS_INTERNAL_BUILD_PROFILE) $(LNKFLAGS_GLOBAL_$(CORE)) $(LNKFLAGS_LOCAL_COMMON) $(LNKFLAGS_LOCAL_$(CORE))

# Path of the RTS library - normally doesn't change for a given tool-chain
#Let the linker choose the required library
RTSLIB_PATH = $(C7X_GEN_INSTALL_PATH)/lib/libc.a

LNK_LIBS = $(addprefix -l,$(LIB_PATHS))
LNK_LIBS += $(addprefix -l,$(EXT_LIB_PATHS))
LNK_LIBS += $(addprefix -l,$(RTSLIB_PATH))

# Linker - to create executable file
ifeq ($(LOCAL_APP_NAME),)
 EXE_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
 EXE_STRIP_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(BUILD_PROFILE_$(CORE))_strip.$(EXEEXT)
else
 ifeq ($(BUILD_PROFILE_$(CORE)),prod_release)
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME).$(EXEEXT)
  EXE_STRIP_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_strip.$(EXEEXT)
 else
  EXE_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(BUILD_PROFILE_$(CORE)).$(EXEEXT)
  EXE_STRIP_NAME = $(BINDIR)/$(LOCAL_APP_NAME)_$(BUILD_PROFILE_$(CORE))_strip.$(EXEEXT)
 endif
endif

$(EXE_NAME) : $(OBJ_PATHS_ASM) $(OBJ_PATHS) $(LIB_PATHS) $(LNKCMD_FILE)
ifeq ($(BUILD_OS_TYPE), freertos)
ifeq ($(wildcard $(BINDIR)/syscfg_c.rov.xs),)
	$(CP) -n $(pdk_PATH)/ti/kernel/freertos/rov/syscfg_c.rov.xs $(BINDIR)
endif
endif
	$(ECHO) \# Linking into $(EXE_NAME)...
	$(ECHO) \#
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS_ASM) $(OBJ_PATHS) -l$(LNKCMD_FILE) $(EXTERNAL_LNKCMD_FILE) $(APPEND_LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS)
	$(ECHO) \#
	$(ECHO) \# $@ created.
	$(ECHO) \#
ifeq ($(BUILD_PROFILE_$(CORE)), release)
	$(ECHO) \# Generating stripped image into $(EXE_STRIP_NAME)...
	$(ECHO) \#
	$(STRP) -p $(EXE_NAME) -o $(EXE_STRIP_NAME)
endif

ifeq ($(BUILD_PROFILE_$(CORE)),prod_release)
  CONFIGURO_BUILD_PROFILE = release
else
  CONFIGURO_BUILD_PROFILE = $(BUILD_PROFILE_$(CORE))
endif

ifeq ($(OS),Windows_NT)
  EVERYONE = $(word 1,$(shell whoami -groups | findstr "S-1-1-0"))
endif

# Include dependency make files that were generated by $(CC)
-include $(SRCS:%.c=$(DEPDIR)/%.P)

# Nothing beyond this point