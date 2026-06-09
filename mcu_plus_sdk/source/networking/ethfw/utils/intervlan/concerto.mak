include $(PRELUDE)

TARGET      := eth_intervlan
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 AM65XX J742S2))

CSOURCES := src/eth_hwintervlan.c
CSOURCES += src/eth_swintervlan.c

IDIRS := ${ETHFW_PATH}
IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/csl
IDIRS += $(PDK_PATH)/packages/ti/drv/udma/src
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/examples
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

endif

include $(FINALE)
