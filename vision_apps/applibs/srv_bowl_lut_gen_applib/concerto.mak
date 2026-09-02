ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)
TARGET      := vx_applib_srv_bowl_lut_gen
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include

include $(FINALE)

endif
