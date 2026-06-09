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
	CROSS_COMPILE:=
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
	CROSS_COMPILE:=
endif

ifneq ($(HOST_FAMILY),$(TARGET_FAMILY))
#$(if $(CROSS_COMPILE),,$(error Cross Compiling is not enabled! TARGET_FAMILY != HOST_FAMILY))
endif

ifeq ($(HOST_OS),Windows_NT)
$(if $(GCC_LINUX_ROOT),,$(error GCC_LINUX_ROOT must be defined!))
$(if $(filter $(subst ;,$(SPACE),$(PATH)),$(GCC_LINUX_ROOT)),,$(error GCC_LINUX_ROOT must be in PATH as well as secondary directories))
endif

# check for the supported CPU types for this compiler
ifeq ($(filter $(TARGET_FAMILY),ARM X86 x86_64),)
$(error TARGET_FAMILY $(TARGET_FAMILY) is not supported by this compiler)
endif

# check for the support OS types for this compiler
ifeq ($(filter $(TARGET_OS),LINUX CYGWIN DARWIN NO_OS SYSBIOS),)
$(error TARGET_OS $(TARGET_OS) is not supported by this compiler)
endif

ifneq ($(GCC_LINUX_ROOT),)
CC = $(GCC_LINUX_ROOT)/bin/gcc-$(CONCERTO_GCC_VERSION)
CP = $(GCC_LINUX_ROOT)/bin/g++-$(CONCERTO_GCC_VERSION)
AS = $(GCC_LINUX_ROOT)/bin/as-$(CONCERTO_GCC_VERSION)
AR = $(GCC_LINUX_ROOT)/bin/gcc-ar-$(CONCERTO_GCC_VERSION)
LD = $(GCC_LINUX_ROOT)/bin/g++-$(CONCERTO_GCC_VERSION)
else
CC = gcc-$(CONCERTO_GCC_VERSION)
CP = g++-$(CONCERTO_GCC_VERSION)
AS = as-$(CONCERTO_GCC_VERSION)
AR = gcc-ar-$(CONCERTO_GCC_VERSION)
LD = g++-$(CONCERTO_GCC_VERSION)
endif
NVCC= $(CUDA_PATH)/bin/nvcc

ifdef LOGFILE
LOGGING:=&>$(LOGFILE)
else
LOGGING:=
endif

ifeq ($(TARGET_OS),DARWIN)
DSO_EXT := .dylib
else ifeq ($(TARGET_OS),CYGWIN)
DSO_EXT := .dll.a
else
DSO_EXT := .so
endif

ifeq ($(strip $($(_MODULE)_TYPE)),library)
	BIN_PRE=lib
	BIN_EXT=.a
else ifeq ($(strip $($(_MODULE)_TYPE)),dsmo)
	BIN_PRE=lib
	BIN_EXT=$(DSO_EXT)
else
	BIN_PRE=
	BIN_EXT=
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

ifneq ($(TARGET_OS),CYGWIN)
$(_MODULE)_COPT += -fPIC
endif
$(_MODULE)_COPT += -Wall -fms-extensions -Wno-write-strings -Wno-format-security

ifeq ($(TARGET_OS),SYSBIOS)
$(_MODULE)_COPT += -Dxdc_target_types__=gnu/targets/arm/std.h -Dxdc_target_name__=A15F -DCGT_GCC -c -mcpu=cortex-a15 -g -mfpu=neon -mfloat-abi=hard -mabi=aapcs -mapcs-frame  -ffunction-sections -fdata-sections
$(_MODULE)_COPT += -Wno-unknown-pragmas -Wno-missing-braces -Wno-format -Wno-unused-variable
endif

$(_MODULE)_COPT += -Wno-unknown-pragmas

ifeq ($(TARGET_BUILD),debug)
$(_MODULE)_COPT += -ggdb -ggdb3 -gdwarf-2 -D_DEBUG_=1
else ifneq ($(filter $(TARGET_BUILD),release production),)
$(_MODULE)_COPT += -O3 -DNDEBUG
endif

ifeq ($(TARGET_BUILD),production)
# Remove all symbols.
$(_MODULE)_LOPT += -s
endif

ifeq ($(TARGET_FAMILY),ARM)
ifeq ($(TARGET_ENDIAN),LITTLE)
$(_MODULE)_COPT += -mlittle-endian
else
$(_MODULE)_COPT += -mbig-endian
endif
endif

ifeq ($(TARGET_FAMILY),ARM)
$(_MODULE)_COPT += -mapcs -mno-sched-prolog -mno-thumb-interwork
ifeq ($(TARGET_OS),LINUX)
$(_MODULE)_COPT += -mabi=aapcs-linux
endif
endif

ifeq ($(HOST_CPU),$(TARGET_CPU))
$(_MODULE)_COPT += -pthread
else ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
$(_MODULE)_COPT += -pthread
else ifeq ($(TARGET_CPU),M3)
$(_MODULE)_COPT += -mcpu=cortex-m3
else ifeq ($(TARGET_CPU),M4)
$(_MODULE)_COPT += -mcpu=cortex-m4
else ifneq ($(filter $(TARGET_CPU),A8 A8F),)
$(_MODULE)_COPT += -mcpu=cortex-a8
else ifneq ($(filter $(TARGET_CPU),A9 A9F),)
$(_MODULE)_COPT += -mcpu=cortex-a9
else ifneq ($(filter $(TARGET_CPU),A15 A15F),)
$(_MODULE)_COPT += -mcpu=cortex-a15
endif

ifeq ($(TARGET_ARCH),32)
ifneq ($(TARGET_CPU),ARM)
$(_MODULE)_COPT += -m32 -fno-stack-protector
endif
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
					   $(LINK_START_GROUP) \
					   $(foreach lib,$(STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(ADDITIONAL_STATIC_LIBS),-l:$(lib)) \
					   $(LINK_END_GROUP) \
					   $(foreach lib,$(SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(PLATFORM_LIBS),-l$(lib)) \
					   -lcudart_static -lcublas -lstdc++ -lm -ldl -lpthread -lrt

$(_MODULE)_LIBRARIES_SO:= $(foreach ldir,$($(_MODULE)_LDIRS),-L$(ldir)) \
					   $(foreach lib,$(STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_STATIC_LIBS),-l$(lib)) \
					   $(foreach lib,$(ADDITIONAL_STATIC_LIBS),-l:$(lib)) \
					   $(foreach lib,$(SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(SYS_SHARED_LIBS),-l$(lib)) \
					   $(foreach lib,$(PLATFORM_LIBS),-l$(lib)) \
					   -lcudart_static -lcublas -lstdc++ -lm -ldl -lpthread -lrt

$(_MODULE)_AFLAGS   := $($(_MODULE)_INCLUDES)
ifeq ($(HOST_OS),DARWIN)
$(_MODULE)_LDFLAGS  := -arch $(TARGET_CPU) $(LDFLAGS)
endif
$(_MODULE)_LDFLAGS  += $($(_MODULE)_LOPT)
$(_MODULE)_CPLDFLAGS := $(foreach ldf,$($(_MODULE)_LDFLAGS),-Wl,$(ldf)) $($(_MODULE)_COPT)
$(_MODULE)_CFLAGS   := -c $($(_MODULE)_INCLUDES) $($(_MODULE)_DEFINES) $($(_MODULE)_COPT) $(CFLAGS)
$(_MODULE)_CPPFLAGS := $(CPPFLAGS)

ifdef DEBUG
$(_MODULE)_AFLAGS += --gdwarf-2
endif

###################################################
# COMMANDS
###################################################
ifneq ($(TARGET_OS),CYGWIN)
EXPORT_FLAG:=--export-dynamic
EXPORTER   :=
else
EXPORT_FLAG:=--export-all-symbols
EXPORTER   :=
endif

$(_MODULE)_LN_DSO     := $(LINK) $(notdir $($(_MODULE)_BIN).$($(_MODULE)_VERSION)) $($(_MODULE)_BIN)
$(_MODULE)_LN_INST_DSO:= $(LINK) $($(_MODULE)_INSTALL_LIB)/$($(_MODULE)_OUT).$($(_MODULE)_VERSION) $($(_MODULE)_INSTALL_LIB)/$($(_MODULE)_OUT)
$(_MODULE)_LINK_LIB   := $(AR) -rsc $($(_MODULE)_BIN) $($(_MODULE)_OBJS)

ifeq ($(HOST_OS),DARWIN)
$(_MODULE)_LINK_DSO   := $(LD) -shared $($(_MODULE)_LDFLAGS) -all_load $($(_MODULE)_LIBRARIES) -lm -o $($(_MODULE)_BIN).$($(_MODULE)_VERSION) $($(_MODULE)_OBJS)
$(_MODULE)_LINK_EXE   := $(LD) -rdynamic $($(_MODULE)_CPLDFLAGS) $($(_MODULE)_OBJS) $($(_MODULE)_LIBRARIES) -o $($(_MODULE)_BIN)
else
$(_MODULE)_LINK_DSO   := $(NVCC) -ccbin $(LD) -shared  $($(_MODULE)_OBJS) $($(_MODULE)_LIBRARIES_SO) -lm -o $($(_MODULE)_BIN).$($(_MODULE)_VERSION)
$(_MODULE)_LINK_EXE   := $(NVCC) -ccbin $(LD) $($(_MODULE)_OBJS) $($(_MODULE)_LIBRARIES_SO) -Xlinker --unresolved-symbols=ignore-in-shared-libs -o $($(_MODULE)_BIN)
endif

###################################################
# MACROS FOR COMPILING
###################################################

define $(_MODULE)_BUILD
build:: $($(_MODULE)_BIN)
endef

SMS ?= 30 35 37 50 52 60 70
ifeq ($(GENCODE_FLAGS),)
# Generate SASS code for each SM architecture listed in $(SMS)
$(foreach sm,$(SMS),$(eval GENCODE_FLAGS += -gencode arch=compute_$(sm),code=sm_$(sm)))

# Generate PTX code from the highest SM architecture in $(SMS) to guarantee forward-compatibility
HIGHEST_SM := $(lastword $(sort $(SMS)))
ifneq ($(HIGHEST_SM),)
GENCODE_FLAGS += -gencode arch=compute_$(HIGHEST_SM),code=compute_$(HIGHEST_SM)
endif
endif



define $(_MODULE)_COMPILE_TOOLS
$(ODIR)/%.o: $(SDIR)/%.c $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C99 $$(notdir $$<)
	$(Q)$(CC) -std=c99 $($(_MODULE)_CFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.o' $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.cpp $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C++ $$(notdir $$<)
	$(Q)$(CP) $($(_MODULE)_CFLAGS) $($(_MODULE)_CPPFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.o' $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.cu $($(_MODULE)_DEP_HEADERS)
	@echo [NVCC] Compiling CU $$(notdir $$<)
	$(Q)$(NVCC) -ccbi $(CP) $($(_MODULE)_CFLAGS) $($(_MODULE)_CPPFLAGS) $(GENCODE_FLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.o' $$< -o $$@ $(LOGGING)


$(ODIR)/%.o: $(SDIR)/%.S
	@echo [GCC] Assembling $$(notdir $$<)
	$(Q)$(AS) $($(_MODULE)_AFLAGS) -MD $(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)
endef


