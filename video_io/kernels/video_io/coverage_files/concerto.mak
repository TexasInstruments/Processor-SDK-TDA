ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F A72 A53))
ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
ifeq ($(BUILD_CAPTURE), yes)
ifeq ($(LDRA_COVERAGE_ENABLED_VIDEO_IO), yes)

include $(PRELUDE)

TARGET      := video_io_coverage
TARGETTYPE  := library


CSOURCES    := ldra_remote_core_coverage_main.c


include $(FINALE)

endif
endif
endif
endif
