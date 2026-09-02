ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
  ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))

    3DSRV_ROOT          = $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv
    3DSRV_INCLUDES      = $(3DSRV_ROOT)/utils/include
    3DSRV_INCLUDES     += $(3DSRV_ROOT)/graphics/include

    UTILS_FILE_LIST    := $(call all-cpp-files-in,$(3DSRV_ROOT)/utils/src)
    GRAPHICS_FILE_LIST := $(call all-cpp-files-in,$(3DSRV_ROOT)/graphics/src)
    3DSRV_SOURCES      := $(foreach f, $(UTILS_FILE_LIST),utils/src/$(f))
    3DSRV_SOURCES      += $(foreach f, $(GRAPHICS_FILE_LIST),graphics/src/$(f))

    include $(PRELUDE)
    TARGET      := vx_srv_render_utils
    TARGETTYPE  := library
    ifeq ($(TARGET_OS),LINUX)
      IDIRS       += $(PSDK_PATH)/toolchain/sysroots/aarch64-oe-linux/usr/include
      CFLAGS      += -DEGL_NO_X11
    endif
    ifeq ($(TARGET_OS),QNX)
      IDIRS       += $(QNX_TARGET)/usr/include
      ifneq ($(QNX_SDP_VERSION),800)
        CPPFLAGS    += -std=c++17 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700
      endif
    endif
    IDIRS       += $(APP_KERNELS_PATH)/utils/include
    IDIRS       += $(3DSRV_ROOT)
    IDIRS       += $(GLM_PATH)
    IDIRS       += $(PSDK_PATH)/SRVDEPS/include/
    IDIRS       += $(PSDK_PATH)/SRVDEPS/include/stb
    CFLAGS      += -DGL_ES
    CFLAGS      += -Wno-pragmas
    CFLAGS      += -Wno-int-to-pointer-cast
    CFLAGS      += -Wno-uninitialized -Wno-maybe-uninitialized

    CSOURCES    := $(call all-c-files)
    CPPSOURCES  := $(call all-cpp-files)
    CPPSOURCES  += $(3DSRV_SOURCES)

    ifeq ($(SOC),$(filter $(SOC), am62a))
      SKIPBUILD=1
    endif

    include $(FINALE)

  endif
endif

ifeq ($(TARGET_CPU),x86_64)

  include $(PRELUDE)
  TARGET      := vx_srv_render_utils
  TARGETTYPE  := library
  IDIRS       += $(APP_KERNELS_PATH)/utils/include
  IDIRS       += $(3DSRV_ROOT)
  IDIRS       += $(GLM_PATH)
  IDIRS       += $(PSDK_PATH)/SRVDEPS/include/
  IDIRS       += $(PSDK_PATH)/SRVDEPS/include/stb
  CFLAGS      += -DGL_ES -DSTANDALONE
  CFLAGS      += -Wno-pragmas
  CFLAGS      += -Wno-int-to-pointer-cast
  CFLAGS      += -Wno-strict-aliasing

  CSOURCES    := $(call all-c-files)
  CPPSOURCES  := $(call all-cpp-files)
  CPPSOURCES  += $(3DSRV_SOURCES)

  ifeq ($(SOC),$(filter $(SOC), am62a))
    SKIPBUILD=1
  endif

  include $(FINALE)

endif
