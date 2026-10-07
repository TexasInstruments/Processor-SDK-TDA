ifeq ($(SOC),$(filter $(SOC), am275x am283x))
PACKAGE_SRCS_COMMON += cslr_rl2_of.h src/ip/rl2_of/src_files_rl2_of.mk src/ip/rl2_of/V0
INCDIR += src/ip/rl2_of/V0
endif
