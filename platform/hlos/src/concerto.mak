ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
  ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A720 A72 A53))
    include $(PRELUDE)

    TARGET      := app_init_hlos_common
    TARGETTYPE  := library

    ifeq ($(TARGET_OS), LINUX)
      CSOURCES    += app_init_linux.c
    else
      CSOURCES    += app_init_qnx.c
    endif

    IDIRS+=$(PLATFORM_PATH)

    include $(FINALE)
  endif
endif
