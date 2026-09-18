#
# This file is the makefile for building IPC multi-core invalid endpt test app
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

APP_NAME = ipc_invalid_endpt_test_$(BUILD_OS_TYPE)

SRCS_COMMON += ipc_invalid_endpt_test.c ipc_invalid_endpt_config.c
PACKAGE_SRCS_COMMON = .
CFLAGS_LOCAL_COMMON += -DIPC_INVALID_ENDPT_TEST

#Path for including common files
IPC_COMMON_PATH = ../../common

include $(IPC_COMMON_PATH)/makefile.mk
