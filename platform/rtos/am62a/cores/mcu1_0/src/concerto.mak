ifeq ($(BUILD_APP_RTOS_LINUX),yes)
  ifeq ($(BUILD_CPU_MCU1_0),yes)
    ifeq ($(TARGET_CPU),R5F)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=linux

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_linux_mcu1_0
      TARGETTYPE  := exe
      CSOURCES    := main.c

      ifeq ($(RTOS),SYSBIOS)
        ASSEMBLY    := mcuCopyVecs2Exc.asm
      endif

      include $($(_MODULE)_SDIR)/concerto_mcu1_0_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common_linux

      STATIC_LIBS += app_rtos_linux

      include $(FINALE)

    endif
  endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
  ifeq ($(BUILD_CPU_MCU1_0),yes)
    ifeq ($(TARGET_CPU),R5F)

      # OS_ID must be set before include $(PRELUDE)
      OS_ID=qnx

      _MODULE=$(OS_ID)
      include $(PRELUDE)

      TARGET      := vx_app_rtos_qnx_mcu1_0
      TARGETTYPE  := exe
      CSOURCES    := main.c

      ifeq ($(RTOS),SYSBIOS)
        ASSEMBLY    := mcuCopyVecs2Exc.asm
      endif

      include $($(_MODULE)_SDIR)/concerto_mcu1_0_inc.mak

      IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common_qnx

      STATIC_LIBS += app_rtos_qnx

      include $(FINALE)

    endif
  endif
endif
