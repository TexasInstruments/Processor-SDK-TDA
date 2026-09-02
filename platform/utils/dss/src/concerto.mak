ifeq ($(BUILD_MCU_BOARD_DEPENDENCIES),yes)
  ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS))
    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),R5F M55))

      include $(PRELUDE)
      TARGET      := app_utils_dss
      TARGETTYPE  := library

      ifeq ($(BUILD_ENABLE_ETHFW),yes)
        DEFS+=ENABLE_ETHFW
      endif

      ifeq ($(RTOS_SDK), mcu_plus_sdk)
        IDIRS       += $(MCU_PLUS_SDK_PATH)/source
        IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
        IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
        ifeq ($(SOC), j722s)
          IDIRS       += $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/generated/
        endif
        CSOURCES    := app_dss_mcu_plus.c
      else ifeq ($(RTOS_SDK), mcu_sdk)
        IDIRS       += $(MCU_SDK_PATH)/source/drivers
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Dss/v0
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Dss/v0/soc
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Dss/v0/soc/$(SOC)
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Dss/v0/include
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Dss/v0/hw_include/V4
        IDIRS       += $(MCU_SDK_PATH)/source/compatibility/dpl/include
        IDIRS       += $(MCU_SDK_PATH)/source/drivers/Fvid2/v0/include
        IDIRS       += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
        IDIRS       += $(MCU_SDK_PATH)/source/arch/include

        CSOURCES    := app_dss_mcu_sdk.c
        CSOURCES    += dss_config_tda54.c
      else
        IDIRS       += $(PDK_PATH)/packages
        IDIRS       += $(PDK_PATH)/packages/ti/drv
        IDIRS       += $(PDK_PATH)/packages/ti/drv/dss

        CSOURCES    := app_dss.c app_dss_soc.c app_dctrl.c app_dss_defaults.c app_dss_dual_display_defaults.c
      endif

      DEFS+=$(BUILD_PDK_BOARD)

      include $(FINALE)

    endif
  endif
endif
