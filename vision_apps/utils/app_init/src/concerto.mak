ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
    include $(PRELUDE)

    TARGET      := app_utils_init
    TARGETTYPE  := library

    CSOURCES    := app_init.c

    ifeq ($(TARGET_CPU), x86_64)
        CFLAGS      := -DTARGET_X86_64
        IDIRS       := $(TIOVX_PATH)/source/platform/pc
    endif

    ifeq ($(VDK), yes)
        DEFS += VDK
    endif

    include $(FINALE)
endif
