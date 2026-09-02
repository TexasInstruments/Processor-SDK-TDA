#
# Copyright (c) {2015 - 2018} Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
# *       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# *       any redistribution and use are licensed by TI for use only with TI Devices.
#
# *       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# *       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# *       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#


##############################################################

# compiler specifc rules and flags to build library or executable. Primirly to be used for DSPs

include $(ALGBASE_PATH)/makerules/config.mk

BUILD_FOLDER      := $(ALGBASE_PATH)\out

ifdef SystemRoot
#Windows OS
    ifndef UTILS_PATH
      $(error You must define UTILS_PATH!)
    endif
    RM_CMD = $(UTILS_PATH)\rm -rf
    MKDIR_CMD = $(UTILS_PATH)\mkdir -p $(1)
    CHANGE_PATHSEP=$(subst /,\,$(1))
    CAT_CMD = $(UTILS_PATH)\cat
    MAKE = gmake
else
    ifeq ($(shell uname), Linux)
#Linux
      RM_CMD = rm -rf
      MKDIR_CMD = mkdir -p $(1)
      CHANGE_PATHSEP=$(subst \,/,$(1))
      CAT_CMD = cat
      MAKE = make
    endif
endif

ifeq ($(SHOW_COMMANDS),1)
Q:=
else
Q:=@
endif

ALGBASE_PATH:= $(call  CHANGE_PATHSEP,$(ALGBASE_PATH))
EMPTY:=
SPACE:=$(EMPTY) $(EMPTY)
COMMA:=$(EMPTY),$(EMPTY)

_MAKEPATH := $(abspath .)
_MAKEPATH:= $(call  CHANGE_PATHSEP,$(_MAKEPATH))


ifdef SystemRoot
_MODPATH  := $(subst $(ALGBASE_PATH)\,,$(_MAKEPATH))
else
_MODPATH  := $(subst $(ALGBASE_PATH)/,,$(_MAKEPATH))
endif
_MODPATH:= $(call CHANGE_PATHSEP,$(_MODPATH))



BUILD_MODE ?=OBJ

ifeq ($(TARGET_BUILD), )
ifeq ($(TARGET_PLATFORM) , PC)
	TARGET_BUILD := debug
else
	TARGET_BUILD := release
endif
endif

ifeq ($(TARGET_PLATFORM) , PC)
CURR_BUILD_FOLDER:= $(BUILD_FOLDER)/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_BUILD)
else
CURR_BUILD_FOLDER:= $(BUILD_FOLDER)/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_BUILD)
endif

CURR_BUILD_FOLDER:= $(call  CHANGE_PATHSEP,$(CURR_BUILD_FOLDER))

##############################################################

##############################################################
# common things
ifeq ($(TARGET_CPU),C71)
  CFLAGS += -D__C7100__
  CFLAGS += -DSOC_J721E
else ifeq ($(TARGET_CPU),C7120)
  CFLAGS += -D__C7120__
  ifeq ($(SOC),j721s2)
    CFLAGS += -DSOC_J721S2
  else ifeq ($(SOC),j784s4)
    CFLAGS += -DSOC_J784S4
  else ifeq ($(SOC),j742s2)
    CFLAGS += -DSOC_J742S2
  endif
else
  CFLAGS += -D__C6600__
endif

ifeq ($(TARGET_PLATFORM) , PC)

ifdef SystemRoot
  #Window Host Emulation
  CC = CL
  CP = CL
  AS = $(TARGET_CPU)ASM
  AR = LIB
  LD = LINK
  COMPILER_FLAGS += /EHsc /TP /W4  /DHOST_EMULATION /c /nologo /MT /FS /Zi /D_HAS_ITERATOR_DEBUGGING=0 /D_ITERATOR_DEBUG_LEVEL=0 /D_HOST_BUILD /D_CRT_SECURE_NO_WARNINGS
  LDDEBUG := /DEBUG
  ARFLAGS := /nologo

  ifeq ($(TARGET_BUILD), release)
    COMPILER_FLAGS += /Ox
  else
    COMPILER_FLAGS += /Od
  endif

 else
  #Linux Host Emulation
  COMPILER_FLAGS += -DHOST_EMULATION -D_HOST_BUILD -DGCC_BUILD -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-result -Wno-maybe-uninitialized -fPIC -DCORE_DSP -DLITTLE_ENDIAN_HOST -D__aarch64__
  ARFLAGS := rvs
  LDFLAGS +=

  ifeq ($(TARGET_BUILD), debug)
    COMPILER_FLAGS +=  -ggdb -ggdb3 -gdwarf-2
  else
    COMPILER_FLAGS += -O3
  endif

  # Set the default gcc based on the version of Ubuntu
  OS_VERSION := $(shell cat /etc/os-release | grep VERSION_ID= | sed -e "s|VERSION_ID=\"||" | sed -e "s|\"||")
ifeq ($(OS_VERSION),18.04)
	TIADALG_GCC_VERSION?=5
else ifeq  ($(OS_VERSION),22.04)
	TIADALG_GCC_VERSION?=11
else ifeq  ($(OS_VERSION),24.04)
	TIADALG_GCC_VERSION?=13
endif


  CC=g++-$(TIADALG_GCC_VERSION) -c
  AR=gcc-ar-$(TIADALG_GCC_VERSION)
  LD=g++-$(TIADALG_GCC_VERSION)

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120))
    COMPILER_FLAGS +=  --std=c++14
  else
    COMPILER_FLAGS +=  --std=c++11
  endif

endif

# for host emulation, intrinsic flow. TMS320C66X in host emulation flow, and _TMS320C6600 in acctual c66x flow
# need to revisit this once we have c7x host emulation
ifeq ($(TARGET_CPU),C71)
    CFLAGS += -I $(CGT7X_ROOT)/host_emulation/include/C7100/
else ifeq ($(TARGET_CPU),C7120)
    CFLAGS += -I $(CGT7X_ROOT)/host_emulation/include/C7120/
else
    CFLAGS += -DTMS320C66X
    CFLAGS += -DLITTLE_ENDIAN_HOST
endif

else
#TI_DEVICE
    CFLAGS += -DUSE_LEVEL_1_MACROS --abi=eabi --rtti -DLITTLE_ENDIAN_HOST

ifeq ($(TIADALG_CPP_BUILD),yes)
    CFLAGS += --cpp_default
endif

ifeq ($(TARGET_CPU),C66)
    CFLAGS+= -I $(CGT6X_ROOT)\include
else
    CFLAGS+= -I $(CGT7X_ROOT)\include
endif

    ARFLAGS = r
    LDFLAGS += -cr -x

    ifeq ($(TARGET_BUILD), release)
      CFLAGS += -O3  -mo -k -mw --fp_not_associative
    else
      CFLAGS += -g
    endif

    ifeq ($(TARGET_CPU),C7120)
      CFLAGS += -mv7120
    else ifeq ($(TARGET_CPU),C71)
      CFLAGS += -mv7100
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120))
      CC = $(CGT7X_ROOT)\bin\cl7x
      LD = $(CGT7X_ROOT)\bin\lnk7x
      AR = $(CGT7X_ROOT)\bin\ar7x
      CFLAGS += --advice:performance=none
      RTS_LIB := -l $(CGT7X_ROOT)//lib//libc.a
    endif

    ifeq ($(TARGET_CPU),C66)
      CC = $(CGT6X_ROOT)\bin\cl6x
      LD = $(CGT6X_ROOT)\bin\lnk6x
      AR = $(CGT6X_ROOT)\bin\ar6x
      CFLAGS += -D_TMS320C6600
      CFLAGS += --silicon_version=6600
      RTS_LIB := -l $(CGT6X_ROOT)/lib/rts6600_elf_mt.lib
    endif
endif


##############################################################


##############################################################
ifeq ($(LIBDIR),)
LIBDIR =  .\lib\$(TARGET_BUILD)
endif
ifeq ($(OUTDIR),)
OUTDIR =  .\elf_out
endif


##############################################################

# In case there are no sources, it is better to not generate any lib
ifeq (,$(wildcard $(CFILES)))
OUTFILE=
endif
##############################################################
CFILES := $(call  CHANGE_PATHSEP,$(CFILES))
HFILES := $(call  CHANGE_PATHSEP,$(HFILES))

CFILESK:= $(filter %.k,$(CFILES))
CFILESC:= $(filter %.c,$(CFILES))
CFILESCPP:= $(filter %.cpp,$(CFILES))
CFILESCC:= $(filter %.cc,$(CFILES))
CFILESASM:= $(filter %.asm,$(CFILES))
CFILES:= $(CFILESK) $(CFILESC) $(CFILESASM) $(CFILESCPP) $(CFILESCC)

# VPATH lets make locate source files by basename when CFILES has absolute paths
VPATH := $(sort $(dir $(CFILES)))

OFILES:= $(addsuffix .obj, $(basename $(notdir $(CFILESC))))
OFILES+= $(addsuffix .obj, $(basename $(notdir $(CFILESCPP))))
OFILES+= $(addsuffix .obj, $(basename $(notdir $(CFILESCC))))
ifneq ($(TARGET_PLATFORM) , PC)
OFILES+= $(addsuffix .obj, $(basename $(notdir $(CFILESASM))))
endif
DEPILES:= $(addsuffix .dep, $(basename $(notdir $(CFILESC))))
DEPILES+= $(addsuffix .dep, $(basename $(notdir $(CFILESCPP))))
DEPILES+= $(addsuffix .dep, $(basename $(notdir $(CFILESCC))))
KOFILES:= $(addsuffix .obj, $(basename $(notdir $(CFILESK))))

BUILDDIR := $(CURR_BUILD_FOLDER)/$(_MODPATH)/
BUILDDIR := $(call  CHANGE_PATHSEP,$(BUILDDIR))

OFILES:= $(addprefix  $(BUILDDIR), $(OFILES))
KOFILES:= $(addprefix $(BUILDDIR), $(KOFILES))
DEPILES:= $(addprefix $(BUILDDIR), $(DEPILES))

OFILES := $(call  CHANGE_PATHSEP,$(OFILES))
FIRST_OFILES := $(word 1,$(OFILES))
RESTOF_OFILES := $(filter-out $(FIRST_OFILES), $(OFILES))

BUILDDIR := $(call  CHANGE_PATHSEP,$(BUILDDIR))
OUTFILE := $(call  CHANGE_PATHSEP,$(OUTFILE))
OBJDIRS := $(foreach file, $(OFILES), $(dir $(file)))
OBJDIRS += $(foreach file, $(KOFILES), $(dir $(file)))
OBJDIRS += $(foreach file, $(OUTFILE), $(dir $(file)))
OBJDIRS += $(BUILDDIR)
OBJDIRS := $(sort $(OBJDIRS))
OBJDIRS := $(addsuffix \.gitignore, $(OBJDIRS))

##############################################################
CC := $(call  CHANGE_PATHSEP,$(CC))
LD := $(call  CHANGE_PATHSEP,$(LD))
AR := $(call  CHANGE_PATHSEP,$(AR))

CFLAGS := $(call  CHANGE_PATHSEP,$(CFLAGS))
OUTFILE := $(call  CHANGE_PATHSEP,$(OUTFILE))
LIBDIR := $(call  CHANGE_PATHSEP,$(LIBDIR))
OUTDIR := $(call  CHANGE_PATHSEP,$(OUTDIR))

OBJDIRS := $(call  CHANGE_PATHSEP,$(OBJDIRS))

CFLAGS := $(COMPILER_FLAGS) $(CFLAGS)

#$(info OBJDIRS $(OBJDIRS))

##############################################################
# 'all' rules
all : dir $(OUTFILE)

# Define a ".gitignore" file which will help in making sure the module's output
# folder always exists.
%.gitignore:
#	@echo creating $@
	$(Q) $(MKDIR_CMD) $(dir $@)
	$(Q)echo .> $@
dir: $(OBJDIRS)

$(OUTFILE): $(KOFILES) $(OFILES)

ifdef SystemRoot
$(BUILDDIR)\%.obj: .\%.c
else
$(BUILDDIR)%.obj: %.c
endif
ifeq ($(TARGET_PLATFORM) , PC)
	$(Q)echo compiling $<
ifdef SystemRoot
  #Window Host Emulation
	$(Q)$(CC) $(CFLAGS) $< /Fo$@ /Fd$(BUILDDIR)$*.pdb
else
	$(Q)$(CC) $(CFLAGS) $< -o $@
endif
else
	$(Q)echo compiling $<
	$(Q)$(CC) $(CFLAGS) --preproc_with_compile -fr=$(dir $@) -fs=$(dir $@) -ppd="$(BUILDDIR)$*.dep" $<
endif


ifdef SystemRoot
$(BUILDDIR)\%.obj: ./%.cpp
else
$(BUILDDIR)%.obj: .%.cpp
endif
	$(Q)echo compiling $<
ifeq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
  #Window Host Emulation
	$(Q)$(CC) $(CFLAGS) $< /Fo$@ /Fd$(BUILDDIR)$*.pdb
else
	$(Q)$(CC) $(CFLAGS) $< -o $@
endif
else
	$(Q)$(CC) $(CFLAGS) --preproc_with_compile -fr=$(dir $@) -fs=$(dir $@) -ppd="$(BUILDDIR)$*.dep" $<
endif

ifneq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
$(BUILDDIR)\%.obj : ./%.asm
else
$(BUILDDIR)%.obj : %.asm
endif
	$(Q)echo compiling $<
	$(Q)$(CC) $(CFLAGS) -c $^ --output_file=$@
endif
##############################################################

print :
	echo $(LDFLAGS)
	echo $(OFILES)
	echo $(OUTFILE)
	echo $(CFILES)
	echo $(HFILES)
	echo @$(AR) $(ARFLAGS) $(OUTFILE) $(KOFILES) $(OFILES) $(ARFILES)
##############################################################
# clean rules
.IGNORE: clean
clean:
ifdef SystemRoot
	@echo echo OFF > temp_pps.bat
	@FOR %%i IN ($(OFILES) $(KOFILES)) DO echo del %%i >> temp_pps.bat
ifneq ($(TARGET_PLATFORM) , PC)
	@FOR %%i IN ( $(DEPILES)) DO echo del %%i >> temp_pps.bat
endif
	@echo echo ON >> temp_pps.bat
	@temp_pps.bat
	@-del temp_pps.bat
else
	$(Q)$(RM_CMD) $(OFILES) $(KOFILES) $(DEPILES)
endif
ifneq ($(BUILD_MODE),OBJ)
	$(Q)$(RM_CMD) $(LIBDIR)
endif
#$(Q)$(RM_CMD) $(OUTDIR) # no need to complete delete the out dir as it has obj of alg also
##############################################################


##############################################################
.PHONY: libfile
libfile: $(LIBDIR) $(KOFILES) $(OFILES) $(ARFILES)
ifeq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
  #Window Host Emulation
	$(Q)echo Linking $(OUTFILE)
	$(Q)echo $(ARFLAGS) /OUT:$(OUTFILE) > ar_cmd.txt
	$(Q)FOR %%i IN ($(OFILES) $(KOFILES) $(ARFILES)) DO echo %%i >> ar_cmd.txt
	$(Q)$(AR) @ar_cmd.txt
	$(Q)-del ar_cmd.txt
else
	$(Q)$(AR) $(ARFLAGS) -o $(OUTFILE) $(KOFILES) $(OFILES) $(ARFILES)
endif
else
	$(Q)$(RM_CMD) $(OUTFILE)
ifdef SystemRoot
	$(Q)echo $(ARFLAGS) $(OUTFILE) > ar_cmd.txt
	$(Q)FOR %%i IN ($(OFILES) $(KOFILES) $(ARFILES)) DO echo %%i >> ar_cmd.txt
	$(Q)$(AR) @ar_cmd.txt
	$(Q)$(RM_CMD) ar_cmd.txt
else
	@$(AR) $(ARFLAGS) $(OUTFILE) $(KOFILES) $(OFILES) $(ARFILES)
endif
endif
$(LIBDIR):
	$(Q) $(MKDIR_CMD) $(LIBDIR) || cd $(LIBDIR)

##############################################################
.PHONY: pllibfile
pllibfile: $(LIBDIR) $(OFILES) $(LDFILES)
ifeq ($(TARGET_PLATFORM) , PC)
	echo Linking $(OUTFILE)
	ifdef SystemRoot
	  #Window Host Emulation
		$(Q)echo $(ARFLAGS) /OUT:$(OUTFILE) > ar_cmd.txt
		$(Q)FOR %%i IN ($(OFILES) $(ARFILES)) DO echo %%i >> ar_cmd.txt
		$(Q)$(AR) @ar_cmd.txt
		$(Q)-del ar_cmd.txt
	else
		$(Q)$(AR) $(ARFLAGS) -o $(OUTFILE) $(KOFILES) $(OFILES) $(ARFILES)
	endif
else
ifdef SystemRoot
	$(Q)echo $(ARFLAGS) temp_pl.lib > ar_cmd.txt
	$(Q)FOR %%i IN ($(RESTOF_OFILES) $(KOFILES) $(ARFILES)) DO echo %%i >> ar_cmd.txt
	$(Q)$(AR) @ar_cmd.txt
	$(Q)-del ar_cmd.txt
	$(Q)if exist $(OUTFILE) del $(OUTFILE)
	$(Q)$(LD) -r $(LDFLAGS) --output_file="plink.out"  $(FIRST_OFILES) $(LDFILES) -m "plink.map" -l temp_pl.lib
	$(Q)echo $(ARFLAGS) $(OUTFILE) > ar_cmd.txt
	$(Q)echo plink.out >> ar_cmd.txt
	$(Q)$(AR) @ar_cmd.txt
	$(Q)-del temp_pl.lib
	$(Q)-del ar_cmd.txt
	$(Q)-del plink.map
	$(Q)-del plink.out
else
	$(Q)$(LD) -r --output_file="plink.out"  $(FIRST_OFILES) $(RESTOF_OFILES) $(KOFILES) -m "plink.map"
	$(Q)$(AR) $(ARFLAGS) $(OUTFILE) plink.out
endif
endif

##############################################################
PC_LDFLAGS1 = $(filter-out %rtsarp32_v200.lib,$(LDFLAGS))
PC_LDFLAGS = $(filter-out %rtsarp32_v200.lib",$(PC_LDFLAGS1))
ALL_LIBS1 = $(filter %.lib,$(PC_LDFLAGS))
ALL_LIBS1 += $(filter %.lib",$(PC_LDFLAGS))
ALL_LIBS1 += $(filter %.lib"",$(PC_LDFLAGS))
ALL_LIBS1 += $(filter %.a86",$(PC_LDFLAGS))

ALL_LIBS  = $(subst .lib,.lib,$(ALL_LIBS1))

##############################################################
.PHONY: outfile
outfile: $(OUTDIR) $(KOFILES) $(OFILES) $(LDFILES)
ifeq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
  #Window Host Emulation
	$(Q)if exist $(OUTFILE).exe del $(OUTFILE).exe
	$(Q)$(LD) $(ALL_LIBS) $(LDDEBUG) /nologo /OUT:$(OUTFILE).exe $(KOFILES) $(OFILES) $(LDFILES)
else
	$(Q)$(RM_CMD) $(OUTFILE)
	$(Q)$(LD) -o $(OUTFILE) $(KOFILES) $(OFILES) $(LDFILES) $(LDFLAGS)
endif
else
	$(Q)$(RM_CMD) $(OUTFILE)
	$(Q)$(LD) $(LDFLAGS) --output_file=$(OUTFILE) $(KOFILES) $(OFILES) $(LDFILES) $(RTS_LIB) -m "$(OUTFILE)".map
endif
$(OUTDIR):
	$(Q)$(MKDIR_CMD) $(OUTDIR) || cd $(OUTDIR)

##############################################################



define INCLUDE_DEPEND
-include $(1).dep
endef

$(foreach obj,$(OFILES),$(eval $(call INCLUDE_DEPEND,$(basename $(obj)))))

