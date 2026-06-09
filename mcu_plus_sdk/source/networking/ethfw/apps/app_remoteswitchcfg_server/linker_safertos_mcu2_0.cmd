/*=========================*/
/*     Linker Settings     */
/*=========================*/

--retain="*(.bootCode)"
--retain="*(.startupCode)"
--retain="*(.startupData)"
--retain="*(.intvecs)"
--retain="*(.intc_text)"
--retain="*(.rstvectors)"
--retain="*(.safeRTOSrstvectors)"
--retain="*(.irqStack)"
--retain="*(.fiqStack)"
--retain="*(.abortStack)"
--retain="*(.undStack)"
--retain="*(.svcStack)"

--fill_value=0
--diag_suppress=10063                   /* entry point not _c_int00 */
--stack_size=0x4000
--heap_size=0x8000
--entry_point=_axSafeRTOSresetVectors   /* C RTS boot.asm with added SVC handler */

-stack  0x4000  /* SOFTWARE STACK SIZE */
-heap   0x8000  /* HEAP AREA SIZE      */

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__IRQ_STACK_SIZE   = 0x1000;
__FIQ_STACK_SIZE   = 0x1000;
__ABORT_STACK_SIZE = 0x1000;
__UND_STACK_SIZE   = 0x1000;
__SVC_STACK_SIZE   = 0x1000;

/*----------------------------------------------------------------------------*/
/* Section Configuration                                                      */
/*----------------------------------------------------------------------------*/
SECTIONS
{

/* Vector sections. */
    GROUP
    {
        .safeRTOSrstvectors                                 : {} palign(8)
        .rstvectors                                         : {} palign(8)
    } > R5F_TCMA

/* Startup code sections. */
    GROUP
    {
        .bootCode                                               : {} palign( 8 )
        .startupCode                                            : {} palign( 8 )
        .cinit                                                  : {} align( 32 )
        .pinit                                                  : {} align( 32 )
        .MPU_INIT_FUNCTION                                      : {} palign( 8 )
        .startupData                                            : {} palign( 8 ), type = NOINIT
    } > R5F_TCMA

    GROUP 
    {
        .text.hwi       : palign(8)
        .text.cache     : palign(8)
        .text.mpu       : palign(8)
        .text.boot      : palign(8)
    }                                       > R5F_TCMB0


/* Code sections. */
    GROUP LOAD_START( lnkFlashStartAddr ), LOAD_END( lnkFlashEndAddr )
    {
        .KERNEL_FUNCTION LOAD_START( lnkKernelFuncStartAddr ),
                         LOAD_END( lnkKernelFuncEndAddr )       : {} palign( 0x10000 )
    } > DDR_MCU2_0

    .text : align(8)    >  DDR_MCU2_0
    .rodata : align(8)  > DDR_MCU2_0



    .text_fast {

    }     > DDR_MCU2_0

    .text                    : {} palign(8)     > DDR_MCU2_0

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

/* Data sections. */
    GROUP  palign( 0x10000 ), LOAD_START( lnkRamStartAddr ), LOAD_END( lnkRamEndAddr )
    {
        .bss                                                    : {} align( 4 )
        .far                                                    : {} align( 4 )
        .data                                                   : {} palign( 128 )
        .boardcfg_data                                          : {} palign( 128 )
        .sysmem                                                 : {}
        .bss.devgroup                                           : {*(.bss.devgroup*)} align(4)
        .const.devgroup                                         : {*(.const.devgroup*)} align(4)
        .KERNEL_DATA LOAD_START( lnkKernelDataStartAddr ),
                     LOAD_END( lnkKernelDataEndAddr )           : {} palign( 0x800 )


    /* Additional sections settings. */

        .data_buffer                                            : {} palign(128)
        /* USB or any other LLD buffer for benchmarking */
        .benchmark_buffer (NOLOAD)                              : {} ALIGN (8)

    /* Stack sections. */
        .stack  RUN_START( lnkStacksStartAddr ) : {}                            align(4)
        .irqStack                               : {. = . + __IRQ_STACK_SIZE;}   align(4)
        RUN_START(__IRQ_STACK_START)
        RUN_END(__IRQ_STACK_END)
        .fiqStack                               : {. = . + __FIQ_STACK_SIZE;}   align(4)
        RUN_START(__FIQ_STACK_START)
        RUN_END(__FIQ_STACK_END)
        .abortStack                             : {. = . + __ABORT_STACK_SIZE;} align(4)
        RUN_START(__ABORT_STACK_START)
        RUN_END(__ABORT_STACK_END)
        .undStack                               : {. = . + __UND_STACK_SIZE;}   align(4)
        RUN_START(__UND_STACK_START)
        RUN_END(__UND_STACK_END)
        .svcStack    END( lnkStacksEndAddr )    : {. = . + __SVC_STACK_SIZE;}   align(4)
        RUN_START(__SVC_STACK_START)
        RUN_END(__SVC_STACK_END)
    } > DDR_MCU2_0
}

/*-------------------------------- END ---------------------------------------*/
