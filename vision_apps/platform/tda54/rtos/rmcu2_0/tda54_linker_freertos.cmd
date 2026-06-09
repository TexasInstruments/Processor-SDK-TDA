/*=========================*/
/*     Linker Settings     */
/*=========================*/

--fill_value=0
--entry_point=__EL2_entry

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__HEAP_SIZE  = 2048;

/* This is the size of stack when R52 is in System (privileged user) mode */
__SYS_STACK_SIZE = 2048;

/* This is the size of stack when R52 is in IRQ mode */
__IRQ_STACK_SIZE = 512;

/* This is the size of stack when R52 is in Fast Interrupt mode */
__FIQ_STACK_SIZE = 512;

/* This is the size of stack when R52 is in Supervisor mode */
__SVC_STACK_SIZE = 4096;

/* This is the size of stack when R52 is in Abort mode */
__ABORT_STACK_SIZE = 256;

/* This is the size of stack when R52 is in Undefined instruction mode */
__UNDEFINED_STACK_SIZE = 256;

/* This is the size of stack when R52 is in Hyp mode (EL2) */
__HYP_STACK_SIZE = 256;

/*--------------------------------------------------------------*/
/*                     Section Configuration                    */
/*--------------------------------------------------------------*/
SECTIONS
{
    .vectors : { *(VECTORS) }               > R52F_TCMB0

    GROUP
    {
        .text.arch_r52       : palign(8)
        .text.boot           : palign(8)
    }                                       > R52F_TCMB0

    .text               : {} palign(8)      > DDR_RMCU2_0
    .const              : {} palign(8)      > DDR_RMCU2_0
    .rodata             : {} palign(8)      > DDR_RMCU2_0
    .cinit              : {} palign(8)      > DDR_RMCU2_0
    .bss                : {} align(8), RUN_START(__BSS_START), RUN_END(__BSS_END) > DDR_RMCU2_0
    .far                : {} align(4)       > DDR_RMCU2_0
    .data               : {} palign(128)    > DDR_RMCU2_0
    .sysmem             : { . = . + __HEAP_SIZE;  } > DDR_RMCU2_0
    .data_buffer        : {} palign(128)    > DDR_RMCU2_0
    .boardcfg_data      : {} align(4)       > DDR_RMCU2_0
    .bss:taskStackSection            : {}   > DDR_RMCU2_0

    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_RMCU2_0_RESOURCE_TABLE

    .tracebuf                : {} align(1024)   > DDR_RMCU2_0

    .stack                   : {} align(4)      > DDR_RMCU2_0  (HIGH)

    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_RMCU2_0_LOCAL_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    /* This IPC log can be viewed via ROV in CCS and when linux is enabled, this log can also be viewed via linux debugfs */
    .bss.debug_mem_trace_buf    : {} palign(128)    > DDR_RMCU2_0_IPC_TRACE

    GROUP :
    {
        .stack : align(8), RUN_START(__SYS_STACK_START), RUN_END(__SYS_STACK_END)
        {
            . = . + __SYS_STACK_SIZE;
        }
        .irqstack : align(8), RUN_START(__IRQ_STACK_START), RUN_END(__IRQ_STACK_END)
        {
            . = . + __IRQ_STACK_SIZE;
        }
        .fiqstack : align(8), RUN_START(__FIQ_STACK_START), RUN_END(__FIQ_STACK_END)
        {
            . = . + __FIQ_STACK_SIZE;
        }
        .svcstack : align(8), RUN_START(__SVC_STACK_START), RUN_END(__SVC_STACK_END)
        {
            . = . + __SVC_STACK_SIZE;
        }
        .abortstack : align(8), RUN_START(__ABORT_STACK_START), RUN_END(__ABORT_STACK_END)
        {
            . = . + __ABORT_STACK_SIZE;
        }
        .undefinedstack : align(8), RUN_START(__UNDEFINED_STACK_START), RUN_END(__UNDEFINED_STACK_END)
        {
            . = . + __UNDEFINED_STACK_SIZE;
        }
        .hypstack : align(8), RUN_START(__HYP_STACK_START), RUN_END(__HYP_STACK_END)
        {
            . = . + __HYP_STACK_SIZE;
        }
    } > DDR_RMCU2_0 (HIGH)
}
