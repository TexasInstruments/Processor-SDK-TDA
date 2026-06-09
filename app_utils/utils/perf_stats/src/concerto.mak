ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64))

    include $(PRELUDE)

    TARGET      := app_utils_perf_stats
    TARGETTYPE  := library

    CSOURCES    += app_perf_stats_api_x86.c

    include $(FINALE)

endif

ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 AM62A J722S TDA54))

    include $(PRELUDE)
    TARGET       := app_utils_perf_stats
    TARGETTYPE   := library

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), FREERTOS SAFERTOS THREADX))
        CSOURCES := app_perf_stats_freertos.c
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        CSOURCES := app_perf_stats_hlos.c
    endif

    CSOURCES += app_perf_stats_api.c

    ifeq ($(SOC),am62a)
        ifeq ($(TARGET_CPU),C7504)
            DEFS+=CPU_c7504
        endif
    endif

    ifeq ($(SOC),tda54)
        ifeq ($(TARGET_CPU),C7604)
            DEFS+=CPU_c7604
        endif
    endif

    ifeq ($(SOC),j722s)
        ifeq ($(TARGET_CPU),C7524)
            DEFS+=CPU_c7524
        endif
    endif

    ifeq ($(VDK), yes)
        DEFS+=VDK
    endif

    include $(FINALE)

endif
