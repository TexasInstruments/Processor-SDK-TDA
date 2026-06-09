/* linker options */

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
--heap_size=0x3E800
--entry_point=_freertosresetvectors

-stack  0x8000  /* SOFTWARE STACK SIZE */
-heap   0x3E800 /* HEAP AREA SIZE      */

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__IRQ_STACK_SIZE   = 0x1000;
__FIQ_STACK_SIZE   = 0x0100;
__ABORT_STACK_SIZE = 0x0100;
__UND_STACK_SIZE   = 0x0100;
__SVC_STACK_SIZE   = 0x0100;

SECTIONS
{
    .freertosrstvectors : {} palign(8)      > R5F_TCMA
    
    .bootCode           : {} palign(8)      > R5F_TCMB0
    .startupCode        : {} palign(8)      > R5F_TCMB0
    .startupData        : {} palign(8)      > R5F_TCMB0, type = NOINIT
    GROUP 
    {
        .text.hwi       : palign(8)
        .text.cache     : palign(8)
        .text.mpu       : palign(8)
        .text.boot      : palign(8)
    }                                       > R5F_TCMB0

    .text_fast {
/* TODO: Commenting due to linking error reported for missing sections
         need to be removed when the enet-lld + lwip is integrated

        *(.text:EnetDma_retrieveRxPktQ*)
        *(.text:EnetDma_retrieveTxPktQ*)
        *(.text:EnetUdma_ringDequeue*)
        *(.text:EnetUdma_ringEnqueue*)
        *(.text:EnetUdma_submitPkts*)
        *(.text:EnetDma_submitRxPktQ*)
        *(.text:EnetDma_submitTxPktQ*)
        *(.text:EnetQueue_append*)
        *(.text:EnetQueue_copyQ*)
        *(.text:EnetQueue_deq*)
        *(.text:EnetQueue_enq*)
        *(.text:EnetQueue_enqHead*)
        *(.text:EnetQueue_getQCount*)
        *(.text:EnetUtils_physToVirt*)
        *(.text:EnetUtils_virtToPhys*)
        *(.text:Udma_ringQueueRaw*)
        *(.text:Udma_ringDequeueRaw*)
        *(.text:Udma_virtToPhyFxn*)
*/
     }     > DDR_MCU2_0

    .text               : {} palign(8)      > DDR_MCU2_0

    .const              : {} palign(8)      > DDR_MCU2_0
    .rodata             : {} palign(8)      > DDR_MCU2_0
    .cinit              : {} palign(8)      > DDR_MCU2_0
    .pinit              : {} palign(8)      > R5F_TCMB0
    .bss                : {} align(4)       > DDR_MCU2_0
    .far                : {} align(4)       > DDR_MCU2_0
    .data               : {} palign(128)    > DDR_MCU2_0
    .sysmem             : {} align(8)       > DDR_MCU2_0
    .data_buffer        : {} palign(128)    > DDR_MCU2_0
    .bss.devgroup       : {*(.bss.devgroup*)} align(4)   > DDR_MCU2_0
    .const.devgroup     : {*(.const.devgroup*)} align(4) > DDR_MCU2_0
    .boardcfg_data      : {} align(4)       > DDR_MCU2_0

    ipc_data_buffer (NOINIT) : {} palign(128)   > DDR_MCU2_0
    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_MCU2_0_RESOURCE_TABLE

    intercore_eth_desc_mem (NOLOAD) : {} palign(128) > INTERCORE_ETH_DESC_MEM
    intercore_eth_data_mem (NOLOAD) : {} palign(128) > INTERCORE_ETH_DATA_MEM

    .tracebuf                : {} align(1024)   > DDR_MCU2_0

    .bss:ENET_DMA_DESC_MEMPOOL  (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .bss:ENET_DMA_RING_MEMPOOL  (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .bss:ENET_DMA_PKT_MEMPOOL   (NOLOAD) {} ALIGN (128) > DDR_MCU2_0
    .bss:ENET_DMA_OBJ_MEM       (NOLOAD) {} ALIGN (128) > DDR_MCU2_0

    /* Used in Switch configuration tool */
    .serialContext     (NOLOAD) {} ALIGN (128) > DDR_MCU2_0

    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM
    .bss:ddr_shared_mem     (NOLOAD) : {} > DDR_MCU2_0

    .stack                   : {} align(4)      > DDR_MCU2_0  (HIGH)

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
}  /* end of SECTIONS */

