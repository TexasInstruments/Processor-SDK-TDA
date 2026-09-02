ifeq ($(BUILD_APP_RTOS_LINUX),yes)
  ifeq ($(BUILD_CPU_C7x_4),yes)
    ifeq ($(TARGET_CPU),C7604)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=linux

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_linux_c7x_4
      TARGETTYPE  := exe
      CSOURCES    := $(call all-c-files)

      include $($(_MODULE)_SDIR)/concerto_c7x_4_inc.mak

      STATIC_LIBS += app_rtos_linux

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      include $(FINALE)

    endif
  endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
  ifeq ($(BUILD_CPU_C7x_4),yes)
    ifeq ($(TARGET_CPU),C7604)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=qnx

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_qnx_c7x_4
      TARGETTYPE  := exe
      CSOURCES    := $(call all-c-files)

      include $($(_MODULE)_SDIR)/concerto_c7x_4_inc.mak

      STATIC_LIBS += app_rtos_qnx

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      include $(FINALE)

    endif
  endif
endif
