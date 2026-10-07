ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
  ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A720 A72 A53))
    include $(PRELUDE)

    TARGET      := app_init_hlos_common
    TARGETTYPE  := library

    CSOURCES    += app_init.c

    IDIRS+=$(PLATFORM_PATH)

    include $(FINALE)
  endif
endif
