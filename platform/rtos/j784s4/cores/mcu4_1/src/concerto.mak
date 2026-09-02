ifeq ($(BUILD_APP_RTOS_LINUX),yes)
  ifeq ($(BUILD_CPU_MCU4_1),yes)
    ifeq ($(TARGET_CPU),R5F)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=linux

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_linux_mcu4_1
      TARGETTYPE  := exe
      CSOURCES    := main.c

      include $($(_MODULE)_SDIR)/concerto_mcu4_1_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      STATIC_LIBS += app_rtos_linux

      include $(FINALE)

    endif
  endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
  ifeq ($(BUILD_CPU_MCU4_1),yes)
    ifeq ($(TARGET_CPU),R5F)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=qnx

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_qnx_mcu4_1
      TARGETTYPE  := exe
      CSOURCES    := main.c

      include $($(_MODULE)_SDIR)/concerto_mcu4_1_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include

      STATIC_LIBS += app_rtos_qnx

      include $(FINALE)

    endif
  endif
endif
