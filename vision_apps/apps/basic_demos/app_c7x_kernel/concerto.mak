ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))
    include $(PRELUDE)
    TARGET      := vx_app_c7x_kernel
    CSOURCES    := main.c app_c7x_kernel_img_add.c

    ifeq ($(TARGET_CPU),x86_64)
        TARGETTYPE  := exe
        CSOURCES    += main_x86.c
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    endif

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
        ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

            TARGETTYPE  := exe
            CSOURCES    += main_linux_arm.c

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

        endif
    endif
    include $(FINALE)
endif


