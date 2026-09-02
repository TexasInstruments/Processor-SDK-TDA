ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_fileio
  TARGETTYPE  := library

  CSOURCES    := vx_kernels_fileio_target.c
  CSOURCES    += vx_fileio_write_array_target.c
  CSOURCES    += vx_fileio_write_image_target.c
  CSOURCES    += vx_fileio_write_raw_image_target.c
  CSOURCES    += vx_fileio_write_tensor_target.c
  CSOURCES    += vx_fileio_write_user_data_object_target.c

  IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/host
  IDIRS       += $(VXLIB_PATH)/packages

  include $(FINALE)

endif
