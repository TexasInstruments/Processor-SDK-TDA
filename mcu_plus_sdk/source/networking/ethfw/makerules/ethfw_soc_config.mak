
ifndef $(ETHFW_SOC_CONFIG_MAK)
ETHFW_SOC_CONFIG_MAK = 1

get-num-cores = $(strip $(foreach core-list, $(1), \
                $(if $(findstring $(2):,$(core-list)), $(word 2,$(subst :, ,$(core-list))))))

is-valid-combo =  $(and $(call get-num-cores,$(5),$(1)), $(strip $(filter $(1):$(2):$(3),$(4))))

expand-target-combos =  $(foreach soc,$(1),\
                $(foreach os,$(2),\
                  $(foreach isa,$(3),\
                    $(foreach profile,$(4),\
                      $(foreach cgt,$(5),\
                         $(if $(call is-valid-combo,$(isa),$(cgt),$(os),$(6),${${soc}_ISA_CORE_COUNT}),$(soc):$(os):$(isa):$(strip $(call get-num-cores,${${soc}_ISA_CORE_COUNT},$(isa))):$(profile):$(cgt)) \
                      )\
                    )\
                  )\
                )\
              )

#J721E SOC Configuration
J721E_ISA_CORE_COUNT := R5F:3 R5Ft:3
J721E_ISA_CORE_COUNT += A72:1
J721E_ISA_CORE_COUNT += C66:2
J721E_ISA_CORE_COUNT += C71:1
J721E_CORE_LIST      := c6x_1 c6x_2 c7x_1 mcu1_0 mcu2_0 mcu2_1 mcu3_0 mpu1_0
J721E_BOARD          := j721e_evm

#J7200 SOC Configuration
J7200_ISA_CORE_COUNT := R5F:2 R5Ft:2
J7200_ISA_CORE_COUNT += A72:1
J7200_CORE_LIST      := mcu1_0 mcu2_0 mcu2_1 mpu1_0
J7200_BOARD          := j7200_evm

#J784S4 SOC Configuration
J784S4_ISA_CORE_COUNT := R5F:2 R5Ft:2
J784S4_ISA_CORE_COUNT += A72:1
J784S4_CORE_LIST      := mcu1_0 mcu2_0 mcu2_1 mcu3_0 mpu1_0
J784S4_BOARD          := j784s4_evm

#AM65XX SOC Configuration
AM65XX_ISA_CORE_COUNT := R5F:1 R5Ft:1
AM65XX_ISA_CORE_COUNT += A53:1
AM65XX_CORE_LIST      := mcu1_0 mpu1_0
AM65XX_BOARD          := am65xx_evm

#J742S2 SOC Configuration
J742S2_ISA_CORE_COUNT := R5F:2 R5Ft:2
J742S2_ISA_CORE_COUNT += A72:1
J742S2_CORE_LIST      := mcu1_0 mcu2_0 mcu2_1 mcu3_0 mpu1_0
J742S2_BOARD          := j742s2_evm

ISA_CGT_OS_VALID_TUPLE   += R5F:TIARMCGT:FREERTOS
ISA_CGT_OS_VALID_TUPLE   += R5Ft:TIARMCGT:FREERTOS
ISA_CGT_OS_VALID_TUPLE   += R5F:TIARMCGT_LLVM:FREERTOS
ISA_CGT_OS_VALID_TUPLE   += R5Ft:TIARMCGT_LLVM:FREERTOS
ISA_CGT_OS_VALID_TUPLE   += R5F:TIARMCGT:SAFERTOS
ISA_CGT_OS_VALID_TUPLE   += R5Ft:TIARMCGT:SAFERTOS
ISA_CGT_OS_VALID_TUPLE   += R5F:TIARMCGT_LLVM:SAFERTOS
ISA_CGT_OS_VALID_TUPLE   += R5Ft:TIARMCGT_LLVM:SAFERTOS
ISA_CGT_OS_VALID_TUPLE   += A72:GCC_LINUX_ARM:LINUX
ISA_CGT_OS_VALID_TUPLE   += A53:GCC_LINUX_ARM:LINUX

endif # ifndef $(ETHFW_SOC_CONFIG_MAK)
