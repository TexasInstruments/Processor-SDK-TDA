ifeq ($(RTOS),SYSBIOS)
  XDC_BLD_FILE = $($(_MODULE)_SDIR)/../bios_cfg/config_c71.bld
  XDC_IDIRS    = $($(_MODULE)_SDIR)/../bios_cfg/
  XDC_CFG_FILE = $($(_MODULE)_SDIR)/c7x_2.cfg
  XDC_PLATFORM = "ti.platforms.tms320C7x:J7ES"
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker.cmd
endif
ifeq ($(RTOS),FREERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif
ifeq ($(RTOS),SAFERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_safertos.cmd
endif

ifeq ($(RTOS),FREERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd
endif

ifeq ($(RTOS),SAFERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map_safertos.cmd
endif

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)


ifeq ($(RTOS),SAFERTOS)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/api/$(SAFERTOS_ISA_EXT_c7x)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/api/PrivWrapperStd
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/config
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/kernel/include_api
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/kernel/include_prv
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_c7x)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_c7x)/$(SAFERTOS_COMPILER_EXT_c7x)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/api/NoWrapper
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/demo_projects/SafeRTOS_TDA4VH_C7x_Demo/TI_c7x_Support
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/demo_projects/SafeRTOS_TDA4VH_C7x_Demo
endif

ifeq ($(RTOS),FREERTOS)
  LDIRS += $(PDK_PATH)/packages/ti/kernel/lib/$(SOC)/c7x_2/$(TARGET_BUILD)/
endif

ifeq ($(RTOS),SAFERTOS)
  LDIRS += $(PDK_PATH)/packages/ti/kernel/safertos/lib/$(SOC)/c7x_2/$(TARGET_BUILD)/
endif

LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/c7x_2/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/c7x_2/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/c7x_2/$(TARGET_BUILD)/
LDIRS += $(TIDL_PATH)/ti_dl/lib/$(TARGET_PLATFORM)/dsp/algo/$(TARGET_BUILD)

TIDL_LIBS =
TIDL_LIBS += common_C7120
TIDL_LIBS += mmalib_C7120
TIDL_LIBS += mmalib_cn_C7120
TIDL_LIBS += perfEst_C7120
TIDL_LIBS += tidl_algo
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
  TIDL_LIBS += tidl_priv
  TIDL_LIBS += tidl_kernels
  TIDL_LIBS += tidl_ref
else
  TIDL_LIBS += tidl_priv_algo
  TIDL_LIBS += tidl_obj_algo
endif
TIDL_LIBS += tidl_custom

SYS_STATIC_LIBS += $(TIDL_LIBS)

ADDITIONAL_STATIC_LIBS += dmautils.ae71
ADDITIONAL_STATIC_LIBS += libtiadalg_fisheye_transformation.a
ADDITIONAL_STATIC_LIBS += libtiadalg_image_preprocessing.a
ADDITIONAL_STATIC_LIBS += libtiadalg_dof_plane_seperation.a
ADDITIONAL_STATIC_LIBS += libtiadalg_select_top_feature.a
ADDITIONAL_STATIC_LIBS += libtiadalg_sparse_upsampling.a
ADDITIONAL_STATIC_LIBS += libtiadalg_visual_localization.a
ADDITIONAL_STATIC_LIBS += libtiadalg_solve_pnp.a
ADDITIONAL_STATIC_LIBS += libtiadalg_image_color_blending.a
ADDITIONAL_STATIC_LIBS += libtiadalg_image_recursive_nms.a

include $($(_MODULE)_SDIR)/../../../concerto_c7x_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_c7x_2
ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_c7x_2
endif

#
# Suppress this warning, 10063-D: entry-point symbol other than "_c_int00" specified
# c7x boots in secure mode and to switch to non-secure mode we need to start at a special entry point '_c_int00_secure'
# and later after switching to non-secure mode, sysbios jumps to usual entry point of _c_int00
# Hence we need to suppress this warning
CFLAGS+=--diag_suppress=10063
