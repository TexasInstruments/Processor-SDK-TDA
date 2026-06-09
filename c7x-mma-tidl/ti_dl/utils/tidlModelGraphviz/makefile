#******************************************************************************
#*      Copyright (C) 2009-2012 Texas Instruments Incorporated.               *
#*                      All Rights Reserved                                   *
#******************************************************************************

##############################################################
ALGBASE_PATH ?= $(abspath ../../../)
PSDK_INSTALL_PATH ?= $(abspath ../../../../)
override BUILD_WITH_OPENACC=no
COMMON_DIR= ../../../common
CFILES := tidl_graph_model.c
CFILES += ../tidlModelDump/tidl_dump.cpp
CFILES += ../tidlModelDump/tidl_strings.cpp
CFILES += ../../utils/perfsim/gc_helper.c

HFILES := tidl_graph_model_template.h

OUTFILE= ./out/tidl_graphVisualiser.out
##############################################################
TIDL_IMPORT_TOOL := 1
TARGET_PLATFORM = PC
BUILD_WITH_CUDA := no
override BUILD_WITH_CUDA=no
override BUILD_WITH_OPENACC=no

include $(ALGBASE_PATH)/makerules/rules.mk
include $(ALGBASE_PATH)/makerules/common.mk

#used inside makerules, but okay to define it afterwards
CFLAGS+= -I ../../inc
CFLAGS+= -I ../tidlModelDump
CFLAGS+= -I $(COMMON_DIR)
CFLAGS+= -I $(TIDL_GRAPHVIZ_PATH)/include/graphviz

ifdef SystemRoot
CFLAGS+= /DTIDL_IMPORT_TOOL
LDFLAGS+=-l $(TIDL_GRAPHVIZ_PATH)/lib/cgraph.lib
LDFLAGS+=-l $(TIDL_GRAPHVIZ_PATH)/lib/gvc.lib
else
CFLAGS+= -DTIDL_IMPORT_TOOL
CFLAGS += -Werror=return-type -Wno-unused-result -Wno-format-overflow
LDFLAGS+= -L $(TIDL_GRAPHVIZ_PATH)/lib

LDFLAGS+= -lcgraph -lgvc
CFLAGS+= -I $(TIDL_GRAPHVIZ_PATH)/
endif
CFLAGS += -I ../perfsim

##############################################################
#LDFLAGS+= -lcdt -lgvc -lcgraph
##############################################################
##############################################################
OUTDIR =  "./out"

$(OUTFILE) : outfile
##############################################################

