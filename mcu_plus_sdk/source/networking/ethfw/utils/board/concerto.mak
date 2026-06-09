ifeq ($(BUILD_CPU_MCU2_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

include $(PRELUDE)

TARGET      := ethfw_board
TARGETTYPE  := library

# Needed for board library header files
DEFS        += $(SOC_DIR)_evm

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J721E)
    CSOURCES    += src/j721e/board_j721e_evm.c
    CSOURCES    += src/j721e/board_pinmux_data.c
  else ifeq ($(TARGET_PLATFORM),J7200)
    CSOURCES    += src/j7200/board_j7200_evm.c
    CSOURCES    += src/j7200/board_pinmux_data.c
  else ifeq ($(TARGET_PLATFORM),J784S4)
    CSOURCES    += src/j784s4/board_j784s4_evm.c
    CSOURCES    += src/j784s4/board_pinmux_data.c
  else ifeq ($(TARGET_PLATFORM),J742S2)
    CSOURCES    += src/j742s2/board_j742s2_evm.c
    CSOURCES    += src/j742s2/board_pinmux_data.c
  endif
endif

IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(ETHFW_PATH)
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

ifeq ($(ETHFW_RAND_MACADDR_GEN),yes)
  DEFS += ETHFW_RAND_MACADDR_GEN
endif

# Feature flags: PPS Signal Gen
ifeq ($(ETHFW_PPS_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    DEFS += ETHFW_PPS_DEMO_SUPPORT
  endif
endif

include $(FINALE)

endif
endif

########################################################

ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

include $(PRELUDE)

TARGET      := ethfw_board
TARGETTYPE  := library

# Needed for board library header files
DEFS        += $(SOC_DIR)_evm

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J721E)
    CSOURCES    += src/j721e/board_j721e_evm.c
    CSOURCES    += src/j721e/board_pinmux_data.c
  else ifeq ($(TARGET_PLATFORM),J784S4)
    CSOURCES    += src/j784s4/board_j784s4_evm.c
    CSOURCES    += src/j784s4/board_pinmux_data.c
  endif
endif

IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(ETHFW_PATH)
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

ifeq ($(ETHFW_RAND_MACADDR_GEN),yes)
  DEFS += ETHFW_RAND_MACADDR_GEN
endif

# Feature flags: PPS Signal Gen
ifeq ($(ETHFW_PPS_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    DEFS += ETHFW_PPS_DEMO_SUPPORT
  endif
endif

include $(FINALE)

endif
endif
