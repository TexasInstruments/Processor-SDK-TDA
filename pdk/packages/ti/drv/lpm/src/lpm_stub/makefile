ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

MODULE_NAME = lpm_stub

SRCDIR = src/lpm_stub
SRCDIR += src
PACKAGE_SRCS_COMMON = $(SRCDIR)
PACKAGE_SRCS_COMMON += include/lpm_stub.h

INCLUDE_EXTERNAL_INTERFACES = pdk
INCLUDE_INTERNAL_INTERFACES = board

SRCS_COMMON = lpm_stub_main.c lpm_ddr.c lpm_uart_baremetal.c lpm_i2c_baremetal.c lpm_pmic_i2c_baremetal.c lpm_stub_utils.c

# To enable debug logs add -DLPM_DEBUG to CFLAGS_LOCAL_COMMON
#
# -nostdlib           : Do not link against the standard C library or its startup files.
#                       The stub runs entirely in TCM with no OS/runtime support,
#                       so libc must be absent to prevent unresolved symbol errors.
# -fomit-frame-pointer: Free the frame-pointer register for general use. We never return back
#                       from the LPM stub, so frame pointer is not needed.
# -fno-builtin        : Disable compiler substitution of standard library functions with
#                       built-in equivalents (e.g. memset, memcpy). Prevents hidden
#                       optimizations from libc that would break the -nostdlib constraint.
CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) -nostdlib -fomit-frame-pointer -fno-builtin

# Include common make files
ifeq ($(MAKERULEDIR), )
Makerule path not defined, define this and assume relative path from ROOTDIR
  MAKERULEDIR := $(ROOTDIR)/ti/build/makerules
  export MAKERULEDIR
endif
include $(MAKERULEDIR)/common.mk

