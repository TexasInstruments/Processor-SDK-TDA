# Copyright (C) 2013 Texas Instruments
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifndef TIARMCGT_LLVM_ROOT
$(error You must define TIARMCGT_LLVM_ROOT!)
endif

# check for the supported CPU types for this compiler
ifeq ($(filter $(TARGET_FAMILY),ARM),)
$(error TARGET_FAMILY $(TARGET_FAMILY) is not supported by this compiler)
endif

# check for the support OS types for this compiler
ifeq ($(filter $(TARGET_OS),SYSBIOS NO_OS FREERTOS SAFERTOS THREADX),)
$(error TARGET_OS $(TARGET_OS) is not supported by this compiler)
endif

CC=$(TIARMCGT_LLVM_ROOT)/bin/tiarmclang
CP=$(TIARMCGT_LLVM_ROOT)/bin/tiarmclang
AS=$(TIARMCGT_LLVM_ROOT)/bin/tiarmclang
AR=$(TIARMCGT_LLVM_ROOT)/bin/tiarmar
LD=$(TIARMCGT_LLVM_ROOT)/bin/tiarmclang

ifdef LOGFILE
LOGGING:=&>$(LOGFILE)
else
LOGGING:=
endif

OBJ_EXT=obj
LIB_PRE=
LIB_EXT=lib
# if an object is encountered as dsmo, make it a library.
ifeq ($(strip $($(_MODULE)_TYPE)),dsmo)
$(_MODULE)_TYPE := library
endif
ifeq ($(strip $($(_MODULE)_TYPE)),library)
	BIN_PRE=
	BIN_EXT=.$(LIB_EXT)
else ifeq ($(strip $($(_MODULE)_TYPE)),exe)
	BIN_PRE=
	BIN_EXT=.out
endif

$(_MODULE)_OUT  := $(BIN_PRE)$($(_MODULE)_TARGET)$(BIN_EXT)
$(_MODULE)_BIN  := $($(_MODULE)_TDIR)/$($(_MODULE)_OUT)
$(_MODULE)_OBJS := $(ASSEMBLY:%.asm=$($(_MODULE)_ODIR)/%.$(OBJ_EXT)) $(CPPSOURCES:%.cpp=$($(_MODULE)_ODIR)/%.$(OBJ_EXT)) $(CSOURCES:%.c=$($(_MODULE)_ODIR)/%.$(OBJ_EXT))
# Redefine the local static libs with REAL paths and pre/post-fixes
$(_MODULE)_STATIC_LIBS := $(foreach lib,$(STATIC_LIBS),$($(_MODULE)_TDIR)/$(LIB_PRE)$(lib).$(LIB_EXT))
$(_MODULE)_STATIC_LIBS += $(foreach lib,$(SYS_STATIC_LIBS),$($(_MODULE)_TDIR)/$(LIB_PRE)$(lib).$(LIB_EXT))

$(_MODULE)_DEP_HEADERS := $(foreach inc,$($(_MODULE)_HEADERS),$($(_MODULE)_SDIR)/$(inc).h)

$(_MODULE)_COPT := $(CFLAGS)
$(_MODULE)_LOPT := $(LDFLAGS)
$(_MODULE)_COPT += $(SUPRESS_WARNINGS_FLAG)
#$(_MODULE)_COPT += -Weverything -Wno-deprecated

ifeq ($(TREAT_WARNINGS_AS_ERROR),1)
$(_MODULE)_COPT +=-Werror
endif

ifeq ($(TARGET_BUILD),debug)
$(_MODULE)_COPT += -O0 -g
else ifneq ($(filter $(TARGET_BUILD),release production),)
$(_MODULE)_COPT += -Oz
endif

ifeq ($(TARGET_CPU),R5F)
TARGET_CPU_FLAGS := -mfloat-abi=hard -mfpu=vfpv3-d16 -mcpu=cortex-r5 -march=armv7-r -fno-strict-aliasing
endif

ifeq ($(TARGET_CPU),M55)
TARGET_CPU_FLAGS := \
	-mcpu=cortex-m55 \
	-mfloat-abi=hard \
	-fshort-enums \
	-DBUILD_M55 \
	-mthumb \

TARGET_LD_FLAGS := \
	-mcpu=cortex-m55 \
	--ram_model \
	--reread_libs \

endif

ifeq ($(TARGET_CPU),R52P)
TARGET_CPU_FLAGS := \
	-mcpu=cortex-r52 \
	-mfloat-abi=hard \
	-mfpu=neon-fp-armv8 \
	-mthumb \
	-Wno-gnu-variable-sized-type-not-at-end \
	-Wno-unused-function \

TARGET_LD_FLAGS := \
	--diag_suppress=10063 \
	--priority \
	--ram_model \
	--reread_libs \

endif

$(_MODULE)_COPT += $(TARGET_CPU_FLAGS)
$(_MODULE)_LOPT += $(TARGET_LD_FLAGS)

ifeq ($(BUILD_IGNORE_LIB_ORDER),yes)
LINK_START_GROUP=-Wl,--start-group
LINK_END_GROUP=-Wl,--end-group
else
LINK_START_GROUP=
LINK_END_GROUP=
endif

$(_MODULE)_MAP      := $($(_MODULE)_BIN).map
$(_MODULE)_INCLUDES := $(foreach inc,$($(_MODULE)_IDIRS),-I$(inc))
$(_MODULE)_DEFINES  := $(foreach def,$($(_MODULE)_DEFS),-D$(def))
$(_MODULE)_LIBRARIES:= $(foreach ldir,$($(_MODULE)_LDIRS),-L$(ldir)) \
                       $(foreach ldir,$($(_MODULE)_SYSLDIRS),-L$(ldir)) \
                       $(LINK_START_GROUP) \
                       $(foreach lib,$(STATIC_LIBS),-l$(LIB_PRE)$(lib).$(LIB_EXT)) \
                       $(foreach lib,$(SYS_STATIC_LIBS),-l$(LIB_PRE)$(lib).$(LIB_EXT)) \
                       $(foreach lib,$(ADDITIONAL_STATIC_LIBS),-l$(lib)) \
                       $(LINK_END_GROUP)
$(_MODULE)_AFLAGS   := $($(_MODULE)_INCLUDES)
$(_MODULE)_LDFLAGS  += $($(_MODULE)_LOPT) --use_memcpy=fast --use_memset=fast --diag_suppress=10063-D --diag_suppress=10068-D
$(_MODULE)_CPLDFLAGS := $(foreach ldf,$($(_MODULE)_LDFLAGS),-Wl,$(ldf)) $($(_MODULE)_LINKER_CMD_FILES)
$(_MODULE)_CFLAGS   := -c $($(_MODULE)_INCLUDES) $($(_MODULE)_DEFINES) $($(_MODULE)_COPT) $(CFLAGS)

ifdef DEBUG
$(_MODULE)_AFLAGS += --gdwarf-2
endif

###################################################
# COMMANDS
###################################################
EXPORT_FLAG:=--export-all-symbols
EXPORTER   :=

$(_MODULE)_LINK_LIB   := $(AR) -rscu $($(_MODULE)_BIN) $($(_MODULE)_OBJS) #$($(_MODULE)_STATIC_LIBS)
$(_MODULE)_LINK_EXE   := $(LD) $(EXPORTER) $($(_MODULE)_CPLDFLAGS) $($(_MODULE)_OBJS) $($(_MODULE)_LIBRARIES) -o $($(_MODULE)_BIN) -Wl,--map_file=$($(_MODULE)_MAP)

###################################################
# MACROS FOR COMPILING
###################################################

define $(_MODULE)_BUILD
build:: $($(_MODULE)_BIN)
endef

define $(_MODULE)_ANALYZER

analysis::$(CPPSOURCES:%.cpp=$(ODIR)/%.xml) $(CSOURCES:%.c=$(ODIR)/%.xml)

$(call PATH_CONV,$(ODIR)$(PATH_SEP)%.xml): $(SDIR)/%.c $(ODIR)/.gitignore
	@echo [TIARM] Analyzing C $$(notdir $$<)
	$(Q)$(CC) --analyze $($(_MODULE)_CFLAGS) $$< -o $$@

$(call PATH_CONV,$(ODIR)$(PATH_SEP)%.xml): $(SDIR)/%.cpp $(ODIR)/.gitignore
	@echo [TIARM] Analyzing C++ $$(notdir $$<)
	$(Q)$(CP) --analyze $($(_MODULE)_CFLAGS) $$< -o $$@
endef

define $(_MODULE)_COMPILE_TOOLS
$(call PATH_CONV,$(ODIR)$(PATH_SEP)%.$(OBJ_EXT)): $(SDIR)/%.c $($(_MODULE)_DEP_HEADERS)
	@echo [TIARM] Compiling C $$(notdir $$<)
	$(Q)$(CC) $($(_MODULE)_CFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.$(OBJ_EXT)' $$< -o $$@ $(LOGGING)

$(call PATH_CONV,$(ODIR)$(PATH_SEP)%.$(OBJ_EXT)): $(SDIR)/%.cpp $($(_MODULE)_DEP_HEADERS)
	@echo [TIARM] Compiling C++ $$(notdir $$<)
	$(Q)$(CP) $($(_MODULE)_CFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.$(OBJ_EXT)' $$< -o $$@ $(LOGGING)

$(call PATH_CONV,$(ODIR)$(PATH_SEP)%.$(OBJ_EXT)): $(SDIR)/%.asm
	@echo [TIARM] Assembling $$(notdir $$<)
	$(Q)$(AS) $($(_MODULE)_AFLAGS) -MD $(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)
endef
