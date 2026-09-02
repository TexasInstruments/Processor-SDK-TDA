ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 R5F A720))

  include $(PRELUDE)
  TARGET      := vx_kernels_fileio
  TARGETTYPE  := library
  CSOURCES    := vx_kernels_fileio_host.c
  CSOURCES    += tivx_fileio_node_api.c

  CSOURCES    += tivx_fileio_write_array_host.c
  CSOURCES    += tivx_fileio_write_image_host.c
  CSOURCES    += tivx_fileio_write_raw_image_host.c
  CSOURCES    += tivx_fileio_write_tensor_host.c
  CSOURCES    += tivx_fileio_write_user_data_object_host.c

  IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/arm

  include $(FINALE)

endif
