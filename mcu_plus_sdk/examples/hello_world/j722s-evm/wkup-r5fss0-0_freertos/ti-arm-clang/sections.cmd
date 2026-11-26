
--retain="*(.bootCode)";
--retain="*(.startupCode)";
--retain="*(.startupData)";
--retain="*(.irqStack)";
--retain="*(.fiqStack)";
--retain="*(.abortStack)";
--retain="*(.undefinedstack)";
--retain="*(.svcStack)";
--fill_value=0
--stack_size=0x4000
--heap_size=0x8000
--entry_point=_self_reset_start

__IRQ_STACK_SIZE = 0x1000;
__FIQ_STACK_SIZE = 0x0100;
__SVC_STACK_SIZE = 0x0100;
__ABORT_STACK_SIZE = 0x0100;
__UNDEFINED_STACK_SIZE = 0x0100;

SECTIONS
{
    .vectors            : {} palign(8) > DDR
    .bootCode           : align = 8, load = R5F_TCMB, run = R5F_TCMA
    .startupCode        : align = 8, load = R5F_TCMB, run = R5F_TCMA
    .startupData        : align = 8, load = R5F_TCMB, run = R5F_TCMA, type = NOINIT

    GROUP {
        .text.hwi: palign(8)
        .text.cache: palign(8)
        .text.mpu: palign(8)
        .text.boot: palign(8)
        .text:abort: palign(8)
    } load = R5F_TCMB, run = R5F_TCMA


    .text   : {} palign(8) > DDR
    .const  : {} palign(8) > DDR
    .rodata : {} palign(8) > DDR
    .cinit         : {} palign(8)   > DDR
    .far           : {} align(4)    > DDR
    .data          : {} palign(128) > DDR
    .sysmem        : {}             > DDR
    .data_buffer   : {} palign(128) > DDR
    .boardcfg_data : {} align(4)    > DDR

    GROUP {
        .bss: {} palign(4)
        RUN_START(__BSS_START)
        RUN_END(__BSS_END)
    } > DDR

    .benchmark_buffer (NOLOAD) {} ALIGN(8) > DDR
    .stack : {} align(4) > DDR (HIGH)

    GROUP {
        .irqstack: {. = . + __IRQ_STACK_SIZE;} align(4)
        RUN_START(__IRQ_STACK_START)
        RUN_END(__IRQ_STACK_END)
        .fiqstack: {. = . + __FIQ_STACK_SIZE;} align(4)
        RUN_START(__FIQ_STACK_START)
        RUN_END(__FIQ_STACK_END)
        .svcstack: {. = . + __SVC_STACK_SIZE;} align(4)
        RUN_START(__SVC_STACK_START)
        RUN_END(__SVC_STACK_END)
        .abortstack: {. = . + __ABORT_STACK_SIZE;} align(4)
        RUN_START(__ABORT_STACK_START)
        RUN_END(__ABORT_STACK_END)
        .undefinedstack: {. = . + __UNDEFINED_STACK_SIZE;} align(4)
        RUN_START(__UNDEFINED_STACK_START)
        RUN_END(__UNDEFINED_STACK_END)
    } > DDR (HIGH)

    GROUP {
        .ARM.exidx:  {} palign(8)
        .init_array: {} palign(8)
        .fini_array: {} palign(8)
    } > DDR

}
