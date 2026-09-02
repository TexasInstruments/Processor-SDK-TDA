ifeq ($(BUILD_APP_RTOS_LINUX),yes)
  ifeq ($(BUILD_CPU_RMCU2_0),yes)
    ifeq ($(TARGET_CPU),R52P)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=linux

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_linux_rmcu2_0
      TARGETTYPE  := exe
      CSOURCES    := main.c

      include $($(_MODULE)_SDIR)/concerto_rmcu2_0_inc.mak

      STATIC_LIBS += app_rtos_linux

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      include $(FINALE)

    endif
  endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
  ifeq ($(BUILD_CPU_RMCU2_0),yes)
    ifeq ($(TARGET_CPU),R52P)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=qnx

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_qnx_rmcu2_0
      TARGETTYPE  := exe
      CSOURCES    := main.c

      include $($(_MODULE)_SDIR)/concerto_rmcu2_0_inc.mak

      STATIC_LIBS += app_rtos_qnx

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      include $(FINALE)

    endif
  endif
endif
