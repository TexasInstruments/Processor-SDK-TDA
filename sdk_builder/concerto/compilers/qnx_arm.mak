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

ifeq ($(TARGET_CPU),$(HOST_CPU))
  $(error $(TARGET_CPU) same as $(HOST_CPU) not supported for QNX)
else ifeq ($(TARGET_CPU),X86)
  $(error $(TARGET_CPU) not supported for QNX)
else ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU),A72 A53 A720))
  CROSS_COMPILE_QNX:=$(QNX_CROSS_COMPILER_TOOL)
else ifeq ($(TARGET_CPU),A15)
  CROSS_COMPILE_QNX:=ntoarmv7-
endif

ifneq ($(HOST_FAMILY),$(TARGET_FAMILY))
$(if $(CROSS_COMPILE_QNX),,$(error Cross Compiling is not enabled! TARGET_FAMILY != HOST_FAMILY))
endif

ifeq ($(HOST_OS),Windows_NT)
#$(if $(GCC_LINUX_ARM_ROOT),,$(error GCC_LINUX_ARM_ROOT must be defined!))
$(error HOST_OS $(HOST_OS) is not supported)
endif

# check for the supported CPU types for this compiler
ifeq ($(filter $(TARGET_FAMILY),ARM X86 x86_64),)
$(error TARGET_FAMILY $(TARGET_FAMILY) is not supported by this compiler)
endif

# check for the support OS types for this compiler
ifeq ($(filter $(TARGET_OS), QNX),)
$(error TARGET_OS $(TARGET_OS) is not supported by this compiler)
endif

ifneq ($(GCC_QNX_ARM_ROOT),)
CC = $(GCC_QNX_ARM_ROOT)/qcc
CP = $(GCC_QNX_ARM_ROOT)/qcc
AS = $(GCC_QNX_ARM_ROOT)/qcc
AR = $(GCC_QNX_ARM_ROOT)/$(CROSS_COMPILE_QNX)ar
LD = $(GCC_QNX_ARM_ROOT)/qcc
else
$(error GCC_QNX_ARM_ROOT $(GCC_QNX_ARM_ROOT) is not supported by this compiler)
endif

ifdef LOGFILE
LOGGING:=&>$(LOGFILE)
else
LOGGING:=
endif

DSO_EXT := .so

ifeq ($(strip $($(_MODULE)_TYPE)),library)
	BIN_PRE=lib
	BIN_EXT=.a
else ifeq ($(strip $($(_MODULE)_TYPE)),dsmo)
	BIN_PRE=lib
	BIN_EXT=$(DSO_EXT)
else
	BIN_PRE=
	BIN_EXT=.out
endif

$(_MODULE)_OUT  := $(BIN_PRE)$($(_MODULE)_TARGET)$(BIN_EXT)
$(_MODULE)_BIN  := $($(_MODULE)_TDIR)/$($(_MODULE)_OUT)
$(_MODULE)_OBJS := $(ASSEMBLY:%.S=$($(_MODULE)_ODIR)/%.o) $(CPPSOURCES:%.cpp=$($(_MODULE)_ODIR)/%.o) $(CSOURCES:%.c=$($(_MODULE)_ODIR)/%.o)
# Redefine the local static libs and shared libs with REAL paths and pre/post-fixes
$(_MODULE)_STATIC_LIBS := $(foreach lib,$(STATIC_LIBS),$($(_MODULE)_TDIR)/lib$(lib).a)
$(_MODULE)_SHARED_LIBS := $(foreach lib,$(SHARED_LIBS),$($(_MODULE)_TDIR)/lib$(lib)$(DSO_EXT))
ifeq ($(BUILD_MULTI_PROJECT),1)
$(_MODULE)_STATIC_LIBS += $(foreach lib,$(SYS_STATIC_LIBS),$($(_MODULE)_TDIR)/lib$(lib).a)
$(_MODULE)_SHARED_LIBS += $(foreach lib,$(SYS_SHARED_LIBS),$($(_MODULE)_TDIR)/lib$(lib)$(DSO_EXT))
$(_MODULE)_PLATFORM_LIBS := $(foreach lib,$(PLATFORM_LIBS),$($(_MODULE)_TDIR)/lib$(lib)$(DSO_EXT))
else
$(_MODULE)_PLATFORM_LIBS := $(PLATFORM_LIBS)
endif
$(_MODULE)_DEP_HEADERS := $(foreach inc,$($(_MODULE)_HEADERS),$($(_MODULE)_SDIR)/$(inc).h)

$(_MODULE)_COPT += -fPIC
$(_MODULE)_COPT += -Wall
$(_MODULE)_COPT += -Vgcc_ntoaarch64le

ifeq ($(TARGET_BUILD),debug)
$(_MODULE)_COPT += -ggdb -ggdb3 -gdwarf-2 -D_DEBUG_=1 -g -O0
else ifneq ($(filter $(TARGET_BUILD),release production),)
$(_MODULE)_COPT += -O3 -DNDEBUG
endif

ifeq ($(TARGET_BUILD),production)
# Remove all symbols.
$(_MODULE)_LOPT += -s
endif

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
					   -Wl,-Bstatic \
					   $(LINK_START_GROUP) \
					   $(foreach lib,$(STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(ADDITIONAL_STATIC_LIBS),-l:$(lib)) \
					   $(LINK_END_GROUP) \
					   -Wl,-Bdynamic \
					   $(foreach lib,$(SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(PLATFORM_LIBS),-l$(lib))
$(_MODULE)_AFLAGS   := $($(_MODULE)_INCLUDES)
$(_MODULE)_LDFLAGS  += $($(_MODULE)_LOPT) -M
$(_MODULE)_CPLDFLAGS := $(foreach ldf,$($(_MODULE)_LDFLAGS),-Wl,$(ldf)) $($(_MODULE)_COPT)
$(_MODULE)_CFLAGS   := -c $($(_MODULE)_INCLUDES) $($(_MODULE)_DEFINES) $($(_MODULE)_COPT) $(CFLAGS) -Wc,-MT,$(ODIR)/$*.o -Wc,-MMD -Wc,-MP
$(_MODULE)_CPPFLAGS := $(CPPFLAGS) -Vgcc_ntoaarch64le -Wc,-MMD -Wc,-MP

ifeq ($(TREAT_WARNINGS_AS_ERROR),1)
$(_MODULE)_CFLAGS += -Werror
endif

ifdef DEBUG
$(_MODULE)_AFLAGS += --gdwarf-2
endif

###################################################
# COMMANDS
###################################################
EXPORT_FLAG:=--export-dynamic
EXPORTER   :=-rdynamic

$(_MODULE)_LN_DSO     := $(LINK) $(notdir $($(_MODULE)_BIN).$($(_MODULE)_VERSION)) $($(_MODULE)_BIN)
$(_MODULE)_LN_INST_DSO:= $(LINK) $($(_MODULE)_INSTALL_LIB)/$($(_MODULE)_OUT).$($(_MODULE)_VERSION) $($(_MODULE)_INSTALL_LIB)/$($(_MODULE)_OUT)
$(_MODULE)_LINK_LIB   := $(AR) -rscu $($(_MODULE)_BIN) $($(_MODULE)_OBJS)

$(_MODULE)_LINK_DSO   := $(LD) $($(_MODULE)_CPLDFLAGS) -shared -Wl,$(EXPORT_FLAG) -Wl,-soname,$(notdir $($(_MODULE)_BIN)).$($(_MODULE)_VERSION) $($(_MODULE)_OBJS) -Wl,--whole-archive $($(_MODULE)_LIBRARIES) -lm -Wl,--no-whole-archive -o $($(_MODULE)_BIN).$($(_MODULE)_VERSION) -Wl,-Map=$($(_MODULE)_MAP)
$(_MODULE)_LINK_EXE   := $(LD) $(EXPORTER) -Wl,--cref $($(_MODULE)_CPLDFLAGS) $($(_MODULE)_OBJS) $($(_MODULE)_LIBRARIES) -o $($(_MODULE)_BIN) -Wl,-Map=$($(_MODULE)_MAP)

###################################################
# MACROS FOR COMPILING
###################################################

define $(_MODULE)_BUILD
build:: $($(_MODULE)_BIN)
endef

define $(_MODULE)_COMPILE_TOOLS
$(ODIR)/%.o: $(SDIR)/%.c $($(_MODULE)_DEP_HEADERS)
	@echo [QNX GCC] Compiling C99 $$(notdir $$<)
	$(Q)$(CC) $($(_MODULE)_CFLAGS) $(call $(_MODULE)_GCC_DEPS,$$*) -Wc,-MF,$(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.cpp $($(_MODULE)_DEP_HEADERS)
	@echo [QNX GCC] Compiling C++ $$(notdir $$<)
	$(Q)$(CP) $($(_MODULE)_CFLAGS) $($(_MODULE)_CPPFLAGS) $(call $(_MODULE)_GCC_DEPS,$$*) -Wc,-MF,$(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.S
	@echo [QNX GCC] Assembling $$(notdir $$<)
	$(Q)$(AS) $($(_MODULE)_AFLAGS) $(call $(_MODULE)_ASM_DEPS,$$*) -Wc,-MD,$(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)
endef
