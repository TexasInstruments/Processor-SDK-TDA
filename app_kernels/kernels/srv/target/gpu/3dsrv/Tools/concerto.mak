ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 J722S))
  ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

      include $(PRELUDE)
      TARGET      := vx_srv_render_utils_tools
      TARGETTYPE  := library
      ifeq ($(TARGET_OS),LINUX)
        IDIRS       += $(LINUX_FS_PATH)/usr/include
        IDIRS       += $(LINUX_FS_PATH)/usr/include/drm
        CFLAGS      += -DEGL_NO_X11
      endif
      ifeq ($(TARGET_OS),QNX)
        IDIRS       += $(QNX_TARGET)/usr/include
      endif
      IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/gpu/3dsrv/Tools
      IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/gpu/3dsrv/Tools/OGLES2
      IDIRS       += $(APP_KERNELS_PATH)
      CFLAGS      += -DGL_ES
      #CFLAGS      += -DSTANDALONE
      CFLAGS      += -Wno-pragmas
      CFLAGS      += -Wno-int-to-pointer-cast
      CFLAGS      += -Wno-class-memaccess
      CFLAGS      += -Wno-int-in-bool-context
      CFLAGS      += -Wno-array-bounds
      CFLAGS      += -Wno-maybe-uninitialized

      CSOURCES    := $(call all-c-files)
      CPPSOURCES  := $(filter-out PVRTPrint3D.cpp, $(call all-cpp-files))

      SKIPBUILD=0

      include $(FINALE)

    endif
  endif
endif

ifeq ($(TARGET_CPU),x86_64)

  include $(PRELUDE)
  TARGET      := vx_srv_render_utils_tools
  TARGETTYPE  := library
  IDIRS       += $(APP_KERNELS_PATH)
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/gpu/3dsrv/Tools
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/gpu/3dsrv/Tools/OGLES2
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/gpu/3dsrv/Include
  CFLAGS      += -DGL_ES -DSTANDALONE
  CFLAGS      += -Wno-pragmas
  CFLAGS      += -Wno-int-to-pointer-cast

  CSOURCES    := $(call all-c-files)
  CPPSOURCES  := $(filter-out PVRTPrint3D.cpp, $(call all-cpp-files))

  include $(FINALE)

endif
