ifneq ($(TARGET_PLATFORM),PC)

  include $(PRELUDE)
  TARGET      := ipc_common
  TARGETTYPE  := library
  CSOURCES    := ipc.c

  ifeq ($(TARGET_OS),$(filter $(TARGET_OS), FREERTOS SAFERTOS))
    CSOURCES     += ipc_trace.c ipc_rsctable.c
  endif

  IDIRS+=$(PLATFORM_PATH)/utils/ipc/include

  include $(FINALE)
endif
