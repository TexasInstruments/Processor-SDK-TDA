/* make sure below retain is there in your linker command file, it keeps the vector table in the final binary */
/* This is the stack that is used by code running within main()
 * In case of NORTOS,
 * - This means all the code outside of ISR uses this stack
 * In case of FreeRTOS
 * - This means all the code until vTaskStartScheduler() is called in main()
 *   uses this stack.
 * - After vTaskStartScheduler() each task created in FreeRTOS has its own stack
 */
--stack_size=16384
/* This is the heap size for malloc() API in NORTOS and FreeRTOS
 * This is also the heap used by pvPortMalloc in FreeRTOS
 */
--heap_size=32768
--entry_point=_c_int00

SECTIONS
{
    /* This has the M55 entry point and vector table, this MUST be at 0x0 */
    .vectors : {}, RUN_START(__INT_VECS_START)  > MCU2_TCMA_VECS

    /* This has the M55 boot code until MPU is enabled,  this MUST be at a address < 0xA0000000
     * i.e this cannot be placed in DDR
     */
    GROUP {
        .text:_c_int00        : {} palign(8)
        .text:_c_int00_noargs : {} palign(8)
        .text:boot            : {} palign(8)
        .text:__mpu_init      : {} palign(8)
    } load = MCU2_TCMA

    .text:   {} palign(8) > DDR_MCU2     /* This is where code resides */

    .bss:    {} palign(8) > DDR_MCU2     /* This is where uninitialized globals go */
    /* This is allocated for IPC */
    .bss.ipcctrl: {} palign(8) > DDR_MCU2
    RUN_START(__BSS_START)
    RUN_END(__BSS_END)

    .data:   {} palign(8) > DDR_MCU2     /* This is where initialized globals and static go */
    .rodata: {} palign(8) > DDR_MCU2     /* This is where const's go */
    .sysmem: {} palign(8) > DDR_MCU2     /* This is where the malloc heap goes */
    .stack:  {} palign(8) > DDR_MCU2     /* This is where the main() stack goes */
    .bss:taskStackSection > DDR_MCU2

    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_MCU2_LOCAL_HEAP
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    .resource_table          :
    {
        __RESOURCE_TABLE = .;
    }                                           > DDR_MCU2_RESOURCE_TABLE
    /* This IPC log can be viewed via ROV in CCS and when linux is enabled, this log can also be viewed via linux debugfs */
    .bss.debug_mem_trace_buf    : {} palign(128)    > DDR_MCU2_IPC_TRACE

    /* Sections needed for C++ projects */
    .ARM.exidx:     {} palign(8) > DDR_MCU2  /* Needed for C++ exception handling */
    .init_array:    {} palign(8) > DDR_MCU2  /* Contains function pointers called before main */
    .fini_array:    {} palign(8) > DDR_MCU2  /* Contains function pointers called after main */
}
