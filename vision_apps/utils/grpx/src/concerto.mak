ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720 R5F))

    include $(PRELUDE)
    TARGET      := app_utils_grpx
    TARGETTYPE  := library

    CSOURCES    := app_grpx.c
    CSOURCES    += app_grpx_cpu_load.c
    CSOURCES    += app_grpx_hwa_load.c
    CSOURCES    += app_grpx_ddr_load.c

    ifeq ($(VDK), yes)
        DEFS += VDK
    endif

    include $(FINALE)
endif

ifeq ($(TARGET_PLATFORM),PC)
    ifeq ($(TARGET_OS),LINUX)

    include $(PRELUDE)
    TARGET      := app_utils_grpx
    TARGETTYPE  := library

    CSOURCES    := app_grpx_null.c

    include $(FINALE)
    endif
endif
