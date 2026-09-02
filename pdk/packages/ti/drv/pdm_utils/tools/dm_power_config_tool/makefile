CURRENT_DIR := $(shell pwd)
SYSCONFIG_VERSION := 1.24.2

HOME_DIR := $(HOME)
SYSCFG_PATH := $(HOME_DIR)/ti/sysconfig_$(SYSCONFIG_VERSION)

SDKPRODUCT_CMD := $(CURRENT_DIR)/../.metadata/product.json
SYSCFG_NODE = $(SYSCFG_PATH)/nodejs/node
NWJS_CMD := $(SYSCFG_PATH)/nw/nw
NWJS_CMD_ARGS := $(SYSCFG_PATH)
BOARD ?= j784s4_evm

BOARDLIST := j784s4_evm j721s2_evm j742s2_evm

ifeq ($(BOARD), j784s4_evm)
SYSCFG_DEVICE := J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x
endif

ifeq ($(BOARD), j721s2_evm)
SYSCFG_DEVICE := J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x
endif

ifeq ($(BOARD), j742s2_evm)
SYSCFG_DEVICE := J742S2
endif

check_board:
	@if [ "$(BOARD)" != "$(filter $(BOARD),$(BOARDLIST))" ]; then \
		echo "Board $(BOARD) is not supported. Please use one of the following boards: $(BOARDLIST)"; \
		echo "  "; \
		make help; \
		exit 1; \
	fi

syscfg_gui:check_board
	@echo Opening syscfg GUI...
	$(NWJS_CMD) $(NWJS_CMD_ARGS) --product $(SDKPRODUCT_CMD)  --device $(SYSCFG_DEVICE) --output ../../../../board/src/$(BOARD)/ out/$(BOARD).syscfg

syscfg:check_board
	@echo Generating syscfg...
	$(SYSCFG_NODE) $(SYSCFG_PATH)/dist/cli.js --product $(SDKPRODUCT_CMD) --device $(SYSCFG_DEVICE) --part Default --output ../../../../board/src/$(BOARD)/  out/$(BOARD).syscfg

help:
	@echo "Makefile help: "

	@echo "Supported boards: $(BOARDLIST)"
	@echo "Default board: j784s4_evm"
	@echo "It requires sysconfig to be installed in $(SYSCFG_PATH)"

	@echo "You can use \"make syscfg_gui BOARD=<board>\" or \"make syscfg BOARD=<board>\"";
	@echo "For Device in MCU+SDK make syscfg is not supported.";
	@echo "This tool is integrated into MCU+SDK.";

	@echo "  make syscfg_gui BOARD=<board> - Open syscfg GUI"
	@echo "  make syscfg BOARD=<board>     - Generate output code file in respective directory"
	@echo "  make help                     - Print this help message"


