ifeq ($(BUILD_APP_RTOS_LINUX),yes)
  ifeq ($(BUILD_CPU_C7x_1),yes)
    ifeq ($(TARGET_CPU),C71)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=linux

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      DEFS+=CPU_c7x_1

      TARGET      := vx_app_rtos_linux_c7x_1
      TARGETTYPE  := exe
      CSOURCES    := $(call all-c-files)

      include $($(_MODULE)_SDIR)/concerto_c7x_1_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common_linux

      STATIC_LIBS += app_rtos_linux


      include $(FINALE)

    endif
  endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
  ifeq ($(BUILD_CPU_C7x_1),yes)
    ifeq ($(TARGET_CPU),C71)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=qnx

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      DEFS+=CPU_c7x_1

      TARGET      := vx_app_rtos_qnx_c7x_1
      TARGETTYPE  := exe
      CSOURCES    := $(call all-c-files)

      include $($(_MODULE)_SDIR)/concerto_c7x_1_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common_qnx

      STATIC_LIBS += app_rtos_qnx

      include $(FINALE)

    endif
  endif
endif
