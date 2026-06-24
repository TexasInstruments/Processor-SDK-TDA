ifeq ($(BUILD_ENABLE_ETHFW),yes)
    ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E J784S4 J742S2 J722S))
        ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS))
            ifeq ($(TARGET_CPU),R5F)
                ifeq ($(BUILD_CPU_MCU2_0),yes)

                include $(PRELUDE)

                TARGET      := app_utils_ethfw
                TARGETTYPE  := library

                IDIRS += $(ETHFW_PATH)
                IDIRS += $(VISION_APPS_PATH)

                ifeq ($(TARGET_OS),FREERTOS)

                    ifeq ($(RTOS_SDK),pdk)

                        IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/src/include
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/freertos/include
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config/$(SOC)
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/contrib
                        IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/r5f
                        ifeq ($(TARGET_PLATFORM),J742S2)
                            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/r5f
                        else
                            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/r5f
                        endif
                        IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/FreeRTOS-LTS/FreeRTOS-Kernel/include
                        ifeq ($(ETHFW_GPTP_BUILD_SUPPORT),yes)
                            IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack
                            IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack/tsn_combase/tilld/jacinto
                        endif
                        IDIRS += $(PDK_PATH)/packages/ti/drv/enet
                        IDIRS += $(PDK_PATH)/packages/ti/drv/enet/lwipif/inc

                    else ifeq ($(RTOS_SDK),mcu_plus_sdk)

                        IDIRS += $(ETHFW_PATH)/utils/ethfw_abstract/$(SOC)
                        IDIRS += $(VISION_APPS_PATH)/platform/$(SOC)/rtos/mcu2_0/generated
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-stack/src/include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-stack/contrib
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-port/include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-port/freertos/include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-config/$(SOC)

                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/tsn/tsn-stack
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/tsn/tsn-stack/tsn_combase/tilld/sitara

                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/include/core
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/utils/include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/soc/k3/$(SOC)
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/lwipif/inc
                        IDIRS += ${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipific_tap/inc

                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/hw_include
                        IDIRS += $(MCU_PLUS_SDK_PATH)/source/board/ethphy/enet/rtos_drivers/include

                    endif # ifeq ($(RTOS_SDK),pdk)

                else ifeq ($(TARGET_OS),SAFERTOS)

                    ifeq ($(RTOS_SDK),pdk)
                        IDIRS += $(ETHFW_PATH)/utils/ethfw_abstract/jacinto
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/src/include
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/safertos/include
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config/$(SOC)
                        IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/contrib
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/api/$(SAFERTOS_ISA_EXT_r5f)
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/api/PrivWrapperStd
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/config
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/kernel/include_api
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/kernel/include_prv
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_r5f)
                        IDIRS += $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_r5f)/$(SAFERTOS_COMPILER_EXT_r5f)
                        IDIRS += $(PDK_PATH)/packages/ti/drv/enet
                        IDIRS += $(PDK_PATH)/packages/ti/drv/enet/lwipif/inc

                    else ifeq ($(RTOS_SDK),mcu_plus_sdk)
                        SKIPBUILD=1
                    endif # ifeq ($(RTOS_SDK),pdk)

                endif # ifeq ($(TARGET_OS),FREERTOS)

                CSOURCES    := app_ethfw_rtos.c

                ifeq ($(TARGET_OS),$(filter $(TARGET_OS),FREERTOS SAFERTOS))
                    ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
                        DEFS += ETHFW_VEPA_SUPPORT
                        DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
                    else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
                        DEFS += ETHAPP_ENABLE_INTERCORE_ETH
                    endif
                    ifeq ($(RTOS_SDK),pdk)
                        DEFS += ENABLE_QSGMII_PORTS
                    endif
                endif

                ifeq ($(ETHFW_IET_ENABLE),yes)
                    DEFS += ETHFW_IET_ENABLE
                endif

                # iperf server support
                ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
                    DEFS += ETHAPP_ENABLE_IPERF_SERVER
                endif

                # ETHFW gPTP stack - for now, supported in FreeRTOS only
                ifeq ($(ETHFW_GPTP_SUPPORT),yes)
                    ifeq ($(TARGET_OS),FREERTOS)
                        DEFS += ETHFW_GPTP_SUPPORT
                    endif
                endif

                # Ethfw Intervlan demo
                ifeq ($(ETHFW_DEMO_SUPPORT),yes)
                    DEFS += ETHFW_DEMO_SUPPORT
                endif

                # Feature flags: ETHFW EST demo - should be supported with gPTP
                ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
                    ifeq ($(ETHFW_GPTP_SUPPORT),yes)
                        DEFS += ETHFW_EST_DEMO_SUPPORT
                    endif
                endif

                include $(FINALE)

                endif
            endif
        endif
    endif
endif
