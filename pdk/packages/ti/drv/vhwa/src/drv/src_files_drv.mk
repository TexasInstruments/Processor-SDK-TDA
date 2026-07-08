SRCS_COMMON += vhwa_utils.c

ifeq ($(SOC),$(filter $(SOC), j721e j784s4 j721s2 j742s2))
SRCS_COMMON += vhwa_m2mFcDrvApi.c vhwa_m2mFcResManages.c \
               vhwa_m2mFcDrvEvtManager.c vhwa_m2mFcConfig.c
endif

ifeq ($(CORE),$(filter $(CORE), $(vhwa_vpac_$(SOC)_CORELIST)))
SRCS_COMMON += vhwa_m2mMscApi.c vhwa_m2mMscUdma.c vhwa_m2mMscIntr.c \
               vhwa_m2mLdcApi.c vhwa_m2mLdcUdma.c vhwa_m2mLdcIntr.c \
               vhwa_m2mNfApi.c vhwa_m2mNfUdma.c vhwa_m2mNfIntr.c \
               vhwa_m2mVissPriv.c vhwa_m2mVissCsl.c vhwa_m2mVissIntr.c \
               vhwa_m2mVissUdma.c vhwa_m2mVissApi.c \
               vhwa_m2mBistDrv.c
endif

ifeq ($(CORE),$(filter $(CORE), $(vhwa_dmpac_$(SOC)_CORELIST)))
SRCS_COMMON += vhwa_m2mDofApi.c vhwa_m2mDofUdma.c vhwa_m2mDofIntr.c \
               vhwa_m2mSdeApi.c vhwa_m2mSdeUdma.c vhwa_m2mSdeIntr.c \
               vhwa_m2mBistDrv.c
endif
