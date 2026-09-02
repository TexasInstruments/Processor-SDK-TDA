ifeq ($(TARGET_CPU),R5F)

  include $(PRELUDE)
  TARGET      := app_utils_mpu
  TARGETTYPE  := library

  ifeq ($(RTOS),FREERTOS)
    ifeq ($(RTOS_SDK),pdk)
      CSOURCES    := app_mpu_freertos_pdk.c
    else
      CSOURCES    := app_mpu_freertos_mcu_plus.c
      IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
    endif
  endif

  ifeq ($(RTOS),SAFERTOS)
    ifeq ($(RTOS_SDK),pdk)
      CSOURCES    := app_mpu_safertos_pdk.c
    else
      CSOURCES    := app_mpu_safertos_mcu_plus.c
      IDIRS += $(PLATFORM_PATH)/platform/$(SOC)/rtos/$(CPU_ID)/generated
    endif
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/api/$(SAFERTOS_ISA_EXT_r5f)
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/api/PrivWrapperStd
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/config
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/kernel/include_api
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_r5f)
    IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_r5f)/$(SAFERTOS_COMPILER_EXT_r5f)
  endif

  include $(FINALE)

endif
