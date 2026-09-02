ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86 x86_64))
  include $(PRELUDE)

  TARGET      := app_init_pc_common
  TARGETTYPE  := library

  CSOURCES    := app_init_pc.c

  IDIRS       += $(TIOVX_PATH)/source/platform/pc

  ifeq ($(VDK), yes)
    DEFS += VDK
  endif

  include $(FINALE)

endif
