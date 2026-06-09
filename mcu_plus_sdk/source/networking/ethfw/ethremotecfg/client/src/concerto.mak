include $(PRELUDE)
TARGET      := ethfw_remotecfg_client
TARGETTYPE  := library

CSOURCES    += cpsw_proxy.c

ifeq ($(ETHFW_MTS_DEMO_TEST),yes)
  DEFS += ETHFW_MTS_DEMO_TEST
endif

ifeq ($(ETHFW_MTS_SUPPORT),yes)
CSOURCES += ts_coupler_client.c
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)
