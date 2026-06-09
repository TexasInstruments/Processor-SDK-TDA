ifeq ($(TARGET_PLATFORM), PC)
ifeq ($(TARGET_OS), LINUX)

include $(PRELUDE)

TARGET      := PC_dsp_test_dl_algo.out
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

#STATIC BUILD C SOURCES
ifeq ($(BUILD_WITH_STATIC_ARRAYS), 1)
CSOURCES    += ../../../../ti_dl/test/tidl_tb_net.c
CSOURCES    += ../../../../ti_dl/test/tidl_tb_pIn_tensor.c
CSOURCES    += ../../../../ti_dl/test/tidl_tb_pOutx_tensor.c
CSOURCES    += ../../../../ti_dl/test/tidl_tb_gIOParams.c
CSOURCES    += ../../../../ti_dl/test/tidl_tb_tidl_net_config.c
DEFS += BUILD_WITH_STATIC_ARRAYS
endif

# host emulation needs HE includes
IDIRS += $(CGT7X_ROOT)/host_emulation/include/$(TARGET_C7X_VERSION)$(C7x_HOSTEMU_COMPILER_STRING)
ifeq ($(BUILD_LIDAR_PREPROC), 1)
IDIRS += $(PSDK_INSTALL_PATH)/tiadalg/include
endif

# library search dirs are always platform specific
LDIRS += $(CGT7X_ROOT)/host_emulation
LDIRS += $(PDK_PATH)/ti/csl/lib/$(SOC)/c7x-hostemu/$(TARGET_BUILD)

LDIRS += $(MMALIB_PATH)/lib/$(TARGET_C7X_VERSION)/$(TARGET_BUILD)
ifeq ($(BUILD_LIDAR_PREPROC), 1)
LDIRS += $(PSDK_INSTALL_PATH)/tiadalg/lib/x86_64/$(TARGET_BUILD)
endif
# path to tidl_algo and tidl_priv
ifeq ($(TIDL_BUILD_PATHS), LEGACY)
LDIRS += $($(_MODULE)_SDIR)/../../../lib/PC/dsp/algo/$(TARGET_BUILD)
else
LDIRS += $($(_MODULE)_SDIR)/../../../lib/$(TARGET_SOC)/PC/algo/$(TARGET_BUILD)
endif

# AVV build needs runtime support library, not libc
#ifeq ($(TARGET_C7X_VERSION), C7100)
#ADDITIONAL_STATIC_LIBS += rts7100_le.lib
#else
#ADDITIONAL_STATIC_LIBS += rts7120_le.lib
#endif

ifeq ($(RTOS_SDK),mcu_sdk)
  DEFS += MCU_SDK
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
  DEFS += MCU_PLUS_SDK
endif

# External libraries: The order in which they are defined ins important
ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_sdk mcu_plus_sdk))
  LDIRS += $(DMA_UTILS_PATH)/lib
endif
ifeq ($(RTOS_SDK),mcu_sdk)
  # Add MCU_SDK library path for system libraries
  ifeq ($(TARGET_BUILD),debug)
    LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Debug
  else
    LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Release
  endif
endif

ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_plus_sdk mcu_sdk))
  ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
    ADDITIONAL_STATIC_LIBS += dmautils.am62ax.c75x.ti-c7x-hostemu.$(TARGET_BUILD).lib
  else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
    ADDITIONAL_STATIC_LIBS += dmautils.j722s.c75ssx-0.ti-c7x-hostemu.$(TARGET_BUILD).lib
  else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
    ADDITIONAL_STATIC_LIBS += dmautils.tda54.c76x.ti-c7x-hostemu.$(TARGET_BUILD).lib
    ADDITIONAL_STATIC_LIBS += libdrivers-ti_sdk_cfg_default_hostemu_gcc-linux.a
  endif
else
  ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), am62a AM62A j722s J722S TDA54 tda54))
    DEFS += DMA_UTILS_STANDALONE
  else
    ifndef DMA_UTILS_STANDALONE
    ADDITIONAL_STATIC_LIBS += udma.lib
    ADDITIONAL_STATIC_LIBS += sciclient.lib
    ADDITIONAL_STATIC_LIBS += ti.osal.lib
    endif
	LDIRS += $(PDK_PATH)/ti/osal/lib/nonos/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
	LDIRS += $(PDK_PATH)/ti/drv/sciclient/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
  endif
  ADDITIONAL_STATIC_LIBS += dmautils.lib
  ADDITIONAL_STATIC_LIBS += ti.csl.lib

  ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
  else
    ADDITIONAL_STATIC_LIBS += ti.csl.init.lib
  endif

  LDIRS += $(PDK_PATH)/ti/drv/udma/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
  LDIRS += $(PDK_PATH)/ti/csl/lib/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
endif

# Custom Library
STATIC_LIBS += tidl_custom

# internal libraries
STATIC_LIBS += tidl_algo
STATIC_LIBS += tidl_ref
STATIC_LIBS += tidl_kernels
STATIC_LIBS += tidl_priv


# External libraries: The order in which they are defined ins important
# Also, must be defined after the internal libraries
STATIC_LIBS += mmalib_cn_x86_64
STATIC_LIBS += perfEst_x86_64
STATIC_LIBS += mmalib_x86_64
STATIC_LIBS += common_x86_64
STATIC_LIBS += $(TARGET_C7X_VERSION)$(C7x_HOSTEMU_COMPILER_STRING)-host-emulation
ifeq ($(BUILD_LIDAR_PREPROC), 1)
STATIC_LIBS += tiadalg_voxelization
DEFS += BUILD_LIDAR_PREPROC
endif
# CUDA
# TODO may have LDIRS, IDIRS, {STATIC, SHARED}_LIBS
ifeq ($(BUILD_WITH_CUDA),yes)
DEFS += BUILD_WITH_CUDA
endif

# OPENCV
ifeq ($(BUILD_WITH_OPENCV), 1)
DEFS += BUILD_WITH_OPENCV

# search path for opencv includes
IDIRS += $(TIDL_OPENCV_PATH)/modules/core/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/highgui/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/imgcodecs/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/videoio/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/imgproc/include
IDIRS += $(TIDL_OPENCV_PATH)/cmake

# search path for opencv library
LDIRS += $(TIDL_OPENCV_PATH)/cmake/lib
LDIRS += $(TIDL_OPENCV_PATH)/cmake/3rdparty/lib

# opencv libraries
STATIC_LIBS += opencv_imgproc
STATIC_LIBS += opencv_imgcodecs
STATIC_LIBS += opencv_core
STATIC_LIBS += libtiff
STATIC_LIBS += libwebp
STATIC_LIBS += libpng
STATIC_LIBS += libjpeg-turbo
STATIC_LIBS += IlmImf
STATIC_LIBS += zlib
STATIC_LIBS += libjasper

# opencv needs libdl.so
SHARED_LIBS += dl
endif

# AVX
# TODO may have LDIRS, IDIRS, {STATIC, SHARED}_LIBS
STATIC_LIBS += tidl_avx_kernels

# defines for host emulation
DEFS += HOST_EMULATION
DEFS += _HOST_BUILD

ifeq ($(TARGET_C7X_VERSION), C7100)
else
DEFS += _TEMP_REF_MODELS
endif

ifeq ($(DMA_UTILS_STANDALONE), 1)
DEFS += DMA_UTILS_STANDALONE
endif

ifeq ($(GCOV_ENABLED), 1)
  $(_MODULE)_COPT += -fprofile-arcs
  $(_MODULE)_COPT += -ftest-coverage
endif

# get the common make flags from test/src/<plat>/../concerto_common.mak
include $($(_MODULE)_SDIR)/../concerto_common.mak
# This compiler keeps screaming about warnings
ifeq ($(CODE_COVERAGE_ENABLED_FOR_TIDL),yes)
CFLAGS += -std=c++11 --no-warnings
else
CFLAGS += -std=c++11
endif

# override CC so that build uses g++-5 (for Ubuntu 18.04) or g++-11 (for Ubuntu 22.04)
override CC := g++-$(TIDL_GCC_VERSION)

include $(FINALE)

endif
endif
