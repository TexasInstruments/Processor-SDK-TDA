ifneq ($(TARGET_PLATFORM), PC)

include $(PRELUDE)
TARGET      := TI_DEVICE_armv8_test_dl_algo_host_rt
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)


# defines for host emulation
DEFS += _HOST_BUILD
DEFS += _A72_BUILD

LDIRS       += $(LINUX_FS_PATH)/usr/lib
LDIRS 		+= $(PLATFORM_PATH)/out/$(TARGET_PLATFORM)/$(TARGET_CPU)/LINUX/$(TARGET_BUILD)

ifeq ($(CODE_COVERAGE_ENABLED_FOR_TIDL),yes)
DEFS += CODE_COVERAGE_ENABLED_FOR_TIDL
endif

# This is not needed in HE as tiovx libs in HE include librt.so
# to emulate multiple cores
SHARED_LIBS += ti_rpmsg_char
SHARED_LIBS += rt

# get the common make flags from test/src/<plat>/../concerto_common.mak
include $($(_MODULE)_SDIR)/../concerto_common.mak

# This compiler keeps screaming about warnings
CFLAGS += -Wno-unknown-pragmas \
	  -Wno-format-overflow \
	  -Wno-maybe-uninitialized \
	  -Wno-unused-variable \
	  -Wno-unused-function \
	  -Wno-sign-compare \
	  -Wno-parentheses

include $(FINALE)

endif
