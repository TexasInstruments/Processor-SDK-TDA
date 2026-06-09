include $(PRELUDE)

TARGET      := ethfw_estdemo
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 J742S2))
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
CSOURCES += src/ethfw_estdemo.c
endif

ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_EST_DEMO_TALKER),yes)
    DEFS += ETHFW_EST_DEMO_TALKER
  endif
endif

ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_EST_DEMO_LISTENER),yes)
    DEFS += ETHFW_EST_DEMO_LISTENER
  endif
endif

IDIRS := ${ETHFW_PATH}
IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/include/
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/examples/utils/include
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto
IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack
IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack/tsn_combase/tilld/jacinto

LDIRS += $(PDK_PATH)/packages/ti/transport/tsn/lib/${TARGET_CPU_FOLDER}/$(TARGET_BUILD)/
endif

# Feature flags: ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

include $(FINALE)
