/*=========================*/
/*     Linker Settings     */
/*=========================*/

--retain="*(.bootCode)"
--retain="*(.startupCode)"
--retain="*(.startupData)"
--retain="*(.irqStack)"
--retain="*(.fiqStack)"
--retain="*(.abortStack)"
--retain="*(.undStack)"
--retain="*(.svcStack)"

--fill_value=0
--stack_size=0x8000
--heap_size=0x10000
--entry_point=_freertosresetvectors

-stack  0x8000  /* SOFTWARE STACK SIZE */
-heap   0x10000 /* HEAP AREA SIZE      */

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__IRQ_STACK_SIZE   = 0x1000;
__FIQ_STACK_SIZE   = 0x0100;
__ABORT_STACK_SIZE = 0x0100;
__UND_STACK_SIZE   = 0x0100;
__SVC_STACK_SIZE   = 0x0100;

/*--------------------------------------------------------------*/
/*                     Section Configuration                    */
/*--------------------------------------------------------------*/
SECTIONS
{
    .freertosrstvectors : {} palign(8)      > R5F_TCMA_VECS

    .bootCode           : {} palign(8)      > R5F_TCMA
    .startupCode        : {} palign(8)      > R5F_TCMA
    .startupData        : {} palign(8)      > R5F_TCMA, type = NOINIT
    GROUP
    {
        .text.hwi       : palign(8)
        .text.cache     : palign(8)
        .text.mpu       : palign(8)
        .text.boot      : palign(8)
    }                                       > R5F_TCMA

    .text               : {} palign(8)      > DDR_MCU2_0
    .const              : {} palign(8)      > DDR_MCU2_0
    .rodata             : {} palign(8)      > DDR_MCU2_0
    .mpu_cfg                                > DDR_MCU2_0
    .cinit              : {} palign(8)      > DDR_MCU2_0
    .pinit              : {} palign(8)      > R5F_TCMA
    .bss                : {} align(4)       > DDR_MCU2_0
    RUN_START(__BSS_START)
    RUN_END(__BSS_END)
    .data               : {} palign(128)    > DDR_MCU2_0
    .sysmem             : {} align(8)       > DDR_MCU2_0
    .stack              : {} align(4)       > DDR_MCU2_0
    .bss.devgroup     : {*(.bss.devgroup*)} align(4)       > DDR_MCU2_0
    .bss:taskStackSection > DDR_MCU2_0

    .far:CPSW_DMA_DESC_MEMPOOL  (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .far:CPSW_DMA_RING_MEMPOOL (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .far:CPSW_DMA_PKT_MEMPOOL (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .bss:NDK_MMBUFFER  (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .bss:NDK_PACKETMEM (NOLOAD) {} ALIGN (128) > DDR_MCU2_0

    .data_buffer        : {} palign(128)    > DDR_MCU2_0
    .const.devgroup     : {*(.const.devgroup*)} align(4)       > DDR_MCU2_0
    .boardcfg_data      : {} align(4)       > DDR_MCU2_0

    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_MCU2_0_RESOURCE_TABLE

    intercore_eth_desc_mem (NOLOAD) : {} palign(128) > INTERCORE_ETH_DESC_MEM
    intercore_eth_data_mem (NOLOAD) : {} palign(128) > INTERCORE_ETH_DATA_MEM

    .tracebuf                : {} align(1024)   > DDR_MCU2_0

    .bss:l3mem              (NOLOAD) : {} > MAIN_OCRAM_MCU2_0
    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_MCU2_0_LOCAL_HEAP
    .bss:ddr_cache_wt_mem   (NOLOAD) : {} > DDR_MCU2_0_VISS_CONFIG_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    .irqStack   : {. = . + __IRQ_STACK_SIZE;} align(4)      > DDR_MCU2_0  (HIGH)
    RUN_START(__IRQ_STACK_START)
    RUN_END(__IRQ_STACK_END)

    .fiqStack   : {. = . + __FIQ_STACK_SIZE;} align(4)      > DDR_MCU2_0  (HIGH)
    RUN_START(__FIQ_STACK_START)
    RUN_END(__FIQ_STACK_END)

    .abortStack : {. = . + __ABORT_STACK_SIZE;} align(4)    > DDR_MCU2_0  (HIGH)
    RUN_START(__ABORT_STACK_START)
    RUN_END(__ABORT_STACK_END)

    .undStack   : {. = . + __UND_STACK_SIZE;} align(4)      > DDR_MCU2_0  (HIGH)
    RUN_START(__UND_STACK_START)
    RUN_END(__UND_STACK_END)

    .svcStack   : {. = . + __SVC_STACK_SIZE;} align(4)      > DDR_MCU2_0  (HIGH)
    RUN_START(__SVC_STACK_START)
    RUN_END(__SVC_STACK_END)
}
