/*=========================*/
/*     Linker Settings     */
/*=========================*/

--retain="*(.bootCode)"
--retain="*(.startupCode)"
--retain="*(.startupData)"
--retain="*(.irqStack)"
--retain="*(.fiqStack)"
--retain="*(.abortStack)"
--retain="*(.undefinedstack)"
--retain="*(.svcStack)"

--fill_value=0
--stack_size=0x8000
--heap_size=0x10000
--entry_point=_self_reset_start

-stack  0x8000  /* SOFTWARE STACK SIZE */
-heap   0x10000 /* HEAP AREA SIZE      */

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__IRQ_STACK_SIZE   = 0x1000;
__FIQ_STACK_SIZE   = 0x0100;
__ABORT_STACK_SIZE = 0x0100;
__UNDEFINED_STACK_SIZE   = 0x0100;
__SVC_STACK_SIZE   = 0x0100;

/*--------------------------------------------------------------*/
/*                     Section Configuration                    */
/*--------------------------------------------------------------*/
SECTIONS
{
    .vectors            : {} palign(8)      > DDR_MCU1_0
    .bootCode           : {} palign(8), load = R5F_TCMB0, run = R5F_TCMA
    .startupCode        : {} palign(8), load = R5F_TCMB0, run = R5F_TCMA
    .startupData        : {} palign(8), load = R5F_TCMB0 , run = R5F_TCMA, type = NOINIT
    GROUP
    {
        .text.hwi       : palign(8)
        .text.cache     : palign(8)
        .text.mpu       : palign(8)
        .text.boot      : palign(8)
        .text:abort     : palign(8) /* this helps in loading symbols when using XIP mode */
    } load = R5F_TCMB0, run = R5F_TCMA
    .mpu_cfg    : align = 8, load = R5F_TCMB0, run = R5F_TCMA

    .text               : {} palign(8)      > DDR_MCU1_0
    .const              : {} palign(8)      > DDR_MCU1_0
    .rodata             : {} palign(8)      > DDR_MCU1_0
    .cinit              : {} palign(8)      > DDR_MCU1_0
    .far                : {} align(4)       > DDR_MCU1_0
    .data               : {} palign(128)    > DDR_MCU1_0
    .sysmem             : {}                > DDR_MCU1_0
    .data_buffer        : {} palign(128)    > DDR_MCU1_0
    .const.devgroup     : {*(.const.devgroup*)} align(4)       > DDR_MCU1_0
    .boardcfg_data      : {} align(4)       > DDR_MCU1_0
    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_MCU1_0_RESOURCE_TABLE

    .tracebuf                : {} align(1024)   > DDR_MCU1_0_IPC_TRACE
    .stack                   : {} align(4)      > DDR_MCU1_0  (HIGH)

    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_MCU1_0_LOCAL_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    GROUP {
        .bss:    {} palign(4)   /* This is where uninitialized globals go */
        RUN_START(__BSS_START)
	.bss.devgroup     : {*(.bss.devgroup*)} align(4)  
	.bss:taskStackSection  : {} align(4)  
        RUN_END(__BSS_END)
    } > DDR_MCU1_0

    .irqStack   : {. = . + __IRQ_STACK_SIZE;} align(4)      > DDR_MCU1_0  (HIGH)
    RUN_START(__IRQ_STACK_START)
    RUN_END(__IRQ_STACK_END)

    .fiqStack   : {. = . + __FIQ_STACK_SIZE;} align(4)      > DDR_MCU1_0  (HIGH)
    RUN_START(__FIQ_STACK_START)
    RUN_END(__FIQ_STACK_END)

    .abortStack : {. = . + __ABORT_STACK_SIZE;} align(4)    > DDR_MCU1_0  (HIGH)
    RUN_START(__ABORT_STACK_START)
    RUN_END(__ABORT_STACK_END)

    .undStack   : {. = . + __UNDEFINED_STACK_SIZE;} align(4)      > DDR_MCU1_0  (HIGH)
    RUN_START(__UNDEFINED_STACK_START)
    RUN_END(__UNDEFINED_STACK_END)

    .svcStack   : {. = . + __SVC_STACK_SIZE;} align(4)      > DDR_MCU1_0  (HIGH)
    RUN_START(__SVC_STACK_START)
    RUN_END(__SVC_STACK_END)
}
