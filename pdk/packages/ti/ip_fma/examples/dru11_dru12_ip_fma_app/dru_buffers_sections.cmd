SECTIONS
{
    /* DR 55-pattern buffer at absolute address */
    .udma_ddr_55 0x95555555 : { *(.udma_ddr_55) } type = NOINIT

    /* DR AA-pattern buffer at absolute address */
    .udma_ddr_aa 0xAAAAAAAA : { *(.udma_ddr_aa) } type = NOINIT

    /* OCMC/SRAM buffer at absolute address */
    .udma_sram 0x41CE3200 : { *(.udma_sram) } type = NOINIT
}