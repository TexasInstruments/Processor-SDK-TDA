
include $(PRELUDE)
TARGET      := app_utils_console_io
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 AM65XX J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
CSOURCES    := src/app_log_writer.c src/app_log_rtos.c src/app_log_reader.c src/app_cli_rtos.c
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R5Ft C66 C71))
CSOURCES += src/app_log_printf_ticgt_rtos.c
endif
endif
ifeq ($(TARGET_OS),LINUX)
CSOURCES    := src/app_log_writer.c src/app_log_reader.c src/app_log_linux.c
endif
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

IDIRS       := ${ETHFW_PATH}
IDIRS       += $(PDK_PATH)/packages

include $(FINALE)

