/* linker options */
--retain="*(.bootCode)"
--retain="*(.startupCode)"
--retain="*(.startupData)"
--fill_value=0
--stack_size=0x2000
--heap_size=0x1000

#define ATCM_START 0x00000000

-e __VECS_ENTRY_POINT
--retain="*(.mcuCopyVecsToExc)"

SECTIONS
{
    .vecs : {
         *(.vecs)
    } palign(8) > R5F_TCMB0_VECS
    .vecs       : {
        __VECS_ENTRY_POINT = .;
    } palign(8) > R5F_TCMB0_VECS
    xdc.meta (COPY): { *(xdc.meta) } > R5F_TCMB0
    .init_text  : {
                     boot.*(.text)
                     *(.text:ti_sysbios_family_arm_MPU_*)
                     *(.text:ti_sysbios_family_arm_v7r_Cache_*)
                  }  > R5F_TCMB0
    .text:xdc_runtime_Startup_reset__I     : {} palign(8) > R5F_TCMB0
    .bootCode    	: {} palign(8) 		> R5F_TCMB0
    .startupCode 	: {} palign(8) 		> R5F_TCMB0
    .startupData 	: {} palign(8) 		> R5F_TCMB0, type = NOINIT
    .mcuCopyVecsToExc                   : {} palign(8) > R5F_TCMB0

    .text           : {} palign(8)   > DDR_MCU1_0
    .const          : {} palign(8)   > DDR_MCU1_0
    .cinit          : {} palign(8)   > DDR_MCU1_0
    .pinit          : {} palign(8)   > DDR_MCU1_0
    .bss            : {} align(4)    > DDR_MCU1_0
    .far            : {} align(8)    > DDR_MCU1_0
    .data           : {} palign(128) > DDR_MCU1_0
    .sysmem         : {} align(8)    > DDR_MCU1_0
    .stack          : {} align(4)    > DDR_MCU1_0
    .data_buffer    : {} palign(128) > DDR_MCU1_0
    .boardcfg_data  : {} palign(8)   > DDR_MCU1_0

    .resource_table : {
        __RESOURCE_TABLE = .;
    } > DDR_MCU1_0_RESOURCE_TABLE

    .const.devgroup.MCU_WAKEUP    : {} align(4)      > DDR_MCU1_0
    .const.devgroup.MAIN          : {} align(4)      > DDR_MCU1_0
    .const.devgroup.DMSC_INTERNAL : {} align(4)      > DDR_MCU1_0
    .bss.devgroup.MAIN            : {} palign(4)     > DDR_MCU1_0
    .bss.devgroup.MCU_WAKEUP      : {} palign(4)     > DDR_MCU1_0
    .bss.devgroup.DMSC_INTERNAL   : {} palign(4)     > DDR_MCU1_0
    .bss.devgroup*                : {} align(4)      > DDR_MCU1_0
    .const.devgroup*              : {} align(4)      > DDR_MCU1_0

    .bss:taskStackSection            : {} > DDR_MCU1_0
    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_MCU1_0_LOCAL_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM
}
