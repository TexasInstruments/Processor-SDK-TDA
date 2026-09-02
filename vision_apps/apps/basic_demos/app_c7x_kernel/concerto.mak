ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))
    include $(PRELUDE)
    TARGET      := vx_app_c7x_kernel
    CSOURCES    := main.c

    ifeq ($(TARGET_CPU),x86_64)
        TARGETTYPE  := exe
        CSOURCES    += main_x86.c

        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

        IDIRS += $(APP_KERNELS_IDIRS)
    endif

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
        ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

            TARGETTYPE  := exe
            CSOURCES    += main_linux_arm.c

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

            IDIRS += $(APP_KERNELS_IDIRS)

            STATIC_LIBS += $(APP_KERNELS_LIBS)
            STATIC_LIBS += $(APP_KERNELS_UTILS_LIBS)

        endif
    endif
    include $(FINALE)
endif


