/*=========================*/
/*     Linker Settings     */
/*=========================*/

--retain="*(.rstvectors)"
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
--stack_size=0x8000
--heap_size=0x10000
--entry_point=_axSafeRTOSresetVectors     /* C RTS boot.asm with added SVC handler	*/

-stack  0x8000  /* SOFTWARE STACK SIZE */
-heap   0x10000 /* HEAP AREA SIZE      */

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

SECTIONS
{
    GROUP
    {
        .safeRTOSrstvectors                                 : {} palign(8)
        .rstvectors                                         : {} palign(8)
    } > R5F_TCMA_VECS
    GROUP
    {
        .bootCode                                               : {} palign( 8 )
        .startupCode                                            : {} palign( 8 )
        .pinit                                                  : {} align( 32 )
        .MPU_INIT_FUNCTION                                      : {} palign( 8 )
        .startupData                                            : {} palign( 8 ), type = NOINIT
    } > R5F_TCMA

    .cinit                                                  : {} align( 32 ) > DDR_MCU2_0

    .data_buffer     : {} palign(128)    > DDR_MCU2_0

    /* USB or any other LLD buffer for benchmarking */
    .benchmark_buffer (NOLOAD) {} ALIGN (8) > DDR_MCU2_0
    
    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_MCU2_0_RESOURCE_TABLE

    GROUP LOAD_START( lnkFlashStartAddr ), LOAD_END( lnkFlashEndAddr )
    {
        .KERNEL_FUNCTION LOAD_START( lnkKernelFuncStartAddr ),
                         LOAD_END( lnkKernelFuncEndAddr )       : {} palign( 0x10000 )
    } > DDR_MCU2_0

    .unpriv_flash palign( 0x10000 ) :
    {
        *(.text)
        *(.rodata)
    } > DDR_MCU2_0

    .tracebuf                : {} align(1024)   > DDR_MCU2_0

    .bss:l3mem              (NOLOAD) : {} > MAIN_OCRAM_MCU2_0
    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_MCU2_0_LOCAL_HEAP
    .bss:ddr_cache_wt_mem   (NOLOAD) : {} > DDR_MCU2_0_VISS_CONFIG_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

   /* Data sections. */
    GROUP  palign( 0x10000 ), LOAD_START( lnkRamStartAddr ), LOAD_END( lnkRamEndAddr )
    {
        .bss                                                    : {} align( 4 )
        RUN_START(__BSS_START)
        RUN_END(__BSS_END)
        .far                                                    : {} align( 4 )
        .data                                                   : {} palign( 128 )
        .boardcfg_data                                          : {} palign( 128 )
        .sysmem                                                 : {}
        .bss.devgroup                                           : {*(.bss.devgroup*)} align(4)
        .bss:taskStackSection                                   : {}
        .const.devgroup                                         : {*(.const.devgroup*)} align(4)
        .KERNEL_DATA LOAD_START( lnkKernelDataStartAddr ),
                     LOAD_END( lnkKernelDataEndAddr )           : {} palign( 0x800 )
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
    } > DDR_MCU2_0   (HIGH)
}
