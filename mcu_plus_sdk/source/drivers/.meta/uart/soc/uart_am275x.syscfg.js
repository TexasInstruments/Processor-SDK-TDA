let common = system.getScript("/common");

let uart_input_clk_freq = 48000000;

const uart_config_r5 = [
    {
        name            : "WKUP_UART",
        baseAddr        : "CSL_WKUP_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 219,
        clockIds        : [ "TISCI_DEV_WKUP_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_WKUP_UART0",
                clkId   : "TISCI_DEV_WKUP_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_WKUP0_UART1_TX",
                rxCh    : "UDMA_PDMA_CH_WKUP0_UART1_RX",
            },
        ],
    },
    {
        name            : "UART0",
        baseAddr        : "CSL_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 210,
        clockIds        : [ "TISCI_DEV_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART0",
                clkId   : "TISCI_DEV_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART0_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART0_RX",
            },
        ],
    },
    {
        name            : "UART1",
        baseAddr        : "CSL_UART1_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 211,
        clockIds        : [ "TISCI_DEV_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART1",
                clkId   : "TISCI_DEV_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART1_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART1_RX",
            },
        ],
    },
    {
        name            : "UART2",
        baseAddr        : "CSL_UART2_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 212,
        clockIds        : [ "TISCI_DEV_UART2" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART2",
                clkId   : "TISCI_DEV_UART2_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART2_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART2_RX",
            },
        ],
    },
    {
        name            : "UART3",
        baseAddr        : "CSL_UART3_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 213,
        clockIds        : [ "TISCI_DEV_UART3" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART3",
                clkId   : "TISCI_DEV_UART3_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART3_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART3_RX",
            },
        ],
    },
    {
        name            : "UART4",
        baseAddr        : "CSL_UART4_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 214,
        clockIds        : [ "TISCI_DEV_UART4" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART4",
                clkId   : "TISCI_DEV_UART4_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART4_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART4_RX",
            },
        ],
    },
    {
        name            : "UART5",
        baseAddr        : "CSL_UART5_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 215,
        clockIds        : [ "TISCI_DEV_UART5" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART5",
                clkId   : "TISCI_DEV_UART5_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART5_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART5_RX",
            },
        ],
    },
    {
        name            : "UART6",
        baseAddr        : "CSL_UART6_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 216,
        clockIds        : [ "TISCI_DEV_UART6" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART6",
                clkId   : "TISCI_DEV_UART6_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART6_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART6_RX",
            },
        ],
    },
];

const uart_config_c75 = [
    {
        name            : "WKUP_UART",
        baseAddr        : "CSL_WKUP_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 22,
        eventId         : 186 + 256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_WKUP_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_WKUP_UART0",
                clkId   : "TISCI_DEV_WKUP_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_WKUP0_UART1_TX",
                rxCh    : "UDMA_PDMA_CH_WKUP0_UART1_RX",
            },
        ],
    },
    {
        name            : "UART0",
        baseAddr        : "CSL_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 32,
        eventId         : 178+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART0",
                clkId   : "TISCI_DEV_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART0_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART0_RX",
            },
        ],
    },
    {
        name            : "UART1",
        baseAddr        : "CSL_UART1_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 33,
        eventId         : 179+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART1",
                clkId   : "TISCI_DEV_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART1_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART1_RX",
            },
        ],
    },
    {
        name            : "UART2",
        baseAddr        : "CSL_UART2_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 34,
        eventId         : 180+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART2" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART2",
                clkId   : "TISCI_DEV_UART2_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART2_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART2_RX",
            },
        ],
    },
    {
        name            : "UART3",
        baseAddr        : "CSL_UART3_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 35,
        eventId         : 181+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART3" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART3",
                clkId   : "TISCI_DEV_UART3_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART3_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART3_RX",
            },
        ],
    },
    {
        name            : "UART4",
        baseAddr        : "CSL_UART4_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 36,
        eventId         : 182+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART4" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART4",
                clkId   : "TISCI_DEV_UART4_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART4_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART4_RX",
            },
        ],
    },
    {
        name            : "UART5",
        baseAddr        : "CSL_UART5_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 37,
        eventId         : 183+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART5" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART5",
                clkId   : "TISCI_DEV_UART5_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART5_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART5_RX",
            },
        ],
    },
    {
        name            : "UART6",
        baseAddr        : "CSL_UART6_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 38,
        eventId         : 184+256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        clockIds        : [ "TISCI_DEV_UART6" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART6",
                clkId   : "TISCI_DEV_UART6_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
        udmaPdmaChannels: [
            {
                txCh    : "UDMA_PDMA_CH_MAIN0_UART6_TX",
                rxCh    : "UDMA_PDMA_CH_MAIN0_UART6_RX",
            },
        ],
    },
];

function getConfigArr() {
    let uart_config;

    if(common.getSelfSysCfgCoreName().match(/r5f*/)) {
        uart_config = uart_config_r5;
    }
    else if(common.getSelfSysCfgCoreName().match(/c75*/)) {
        uart_config = uart_config_c75;
    }

    return uart_config;
}

function getDefaultClkRate() {
    return uart_input_clk_freq;
}

exports = {
    getConfigArr,
    getDefaultClkRate
};
