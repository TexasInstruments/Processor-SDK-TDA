
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 R5F M55))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_img_proc_mcu
  TARGETTYPE  := library

  CSOURCES    := vx_kernels_img_proc_target.c

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F M55))
    CSOURCES    += vx_img_mosaic_msc_target.c
  else
    CSOURCES    += vx_img_mosaic_msc_target_sim.c
    IDIRS       += $(VPAC_C_MODELS_PATH)/include
    IDIRS       += $(IMAGING_PATH)/kernels/hwa/include
  endif

  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/host
  IDIRS       += $(TIADALG_PATH)/include
  IDIRS       += $(IVISION_PATH)
  IDIRS       += $(TIDL_PATH)/arm-tidl/rt/inc
  IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
  IDIRS       += $(VXLIB_PATH)/packages

  ifeq ($(RTOS_SDK), mcu_plus_sdk)
    IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa
  else
    IDIRS       += $(PDK_PATH)/packages/ti/drv/vhwa
  endif

  ifeq ($(SOC), $(filter $(SOC), tda54))
    ifneq ($(TARGET_CPU), x86_64)
      SKIPBUILD=1
    endif
  endif

  ifeq ($(TARGET_CPU), x86_64)
    ifeq ($(SOC),am62a)
      SKIPBUILD=1
    endif
  endif

  include $(FINALE)

endif
