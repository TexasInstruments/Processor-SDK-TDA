let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");

let mcasp_input_clk_freq = 49152000;

const mcasp_config = [
    {
        name                        : "MCASP0",
        regBaseAddr                 : "CSL_MCASP0_CFG_BASE",
        dataRegBaseAddr             : "CSL_MCASP0_DMA_BASE",
        numSerializers              : 16,
        inputClkFreq                : mcasp_input_clk_freq,
        wkupR5RxIntr                : 120,
        wkupR5TxIntr                : 121,
        c7xRxIntr                   : 39,
        c7xTxIntr                   : 40,
        c7xRxEvent                  : 267,
        c7xTxEvent                  : 268,
        clockIds                    : ["TISCI_DEV_MCASP0"],
        clockFrequencies            : [
            {
                moduleId: "TISCI_DEV_MCASP0_LOCAL_AUXCLK_SEL_DEV_VD ",
                clkId   : "TISCI_DEV_MCASP0_LOCAL_AUXCLK_SEL_DEV_VD_CLK",
                clkRate : mcasp_input_clk_freq,
            },
            {
                moduleId: "TISCI_DEV_MCASP0",
                clkId   : "TISCI_DEV_MCASP0_AUX_CLK",
                clkRate : mcasp_input_clk_freq,
            },
        ],
        udmaPdmaChannels            : [
            {
                txCh : "UDMA_PDMA_CH_MAIN0_MCASP0_TX",
                rxCh : "UDMA_PDMA_CH_MAIN0_MCASP0_RX",
            }
        ],
    },
    {
        name                        : "MCASP1",
        regBaseAddr                 : "CSL_MCASP1_CFG_BASE",
        dataRegBaseAddr             : "CSL_MCASP1_DMA_BASE",
        numSerializers              : 16,
        inputClkFreq                : mcasp_input_clk_freq,
        wkupR5RxIntr                : 122,
        wkupR5TxIntr                : 123,
        c7xRxIntr                   : 41,
        c7xTxIntr                   : 42,
        c7xRxEvent                  : 269,
        c7xTxEvent                  : 270,
        clockIds                    : ["TISCI_DEV_MCASP1"],
        clockFrequencies            : [
            {
                moduleId: "TISCI_DEV_MCASP1_LOCAL_AUXCLK_SEL_DEV_VD ",
                clkId   : "TISCI_DEV_MCASP1_LOCAL_AUXCLK_SEL_DEV_VD_CLK",
                clkRate : mcasp_input_clk_freq,
            },
            {
                moduleId: "TISCI_DEV_MCASP1",
                clkId   : "TISCI_DEV_MCASP1_AUX_CLK",
                clkRate : mcasp_input_clk_freq,
            }
        ],
        udmaPdmaChannels            : [
            {
                txCh : "UDMA_PDMA_CH_MAIN0_MCASP1_TX",
                rxCh : "UDMA_PDMA_CH_MAIN0_MCASP1_RX",
            }
        ],
    },
    {
        name                        : "MCASP2",
        regBaseAddr                 : "CSL_MCASP2_CFG_BASE",
        dataRegBaseAddr             : "CSL_MCASP2_DMA_BASE",
        numSerializers              : 16,
        inputClkFreq                : mcasp_input_clk_freq,
        wkupR5RxIntr                : 124,
        wkupR5TxIntr                : 125,
        c7xRxIntr                   : 43,
        c7xTxIntr                   : 44,
        c7xRxEvent                  : 271,
        c7xTxEvent                  : 272,
        clockIds                    : ["TISCI_DEV_MCASP2"],
        clockFrequencies            : [
            {
                moduleId: "TISCI_DEV_MCASP2_LOCAL_AUXCLK_SEL_DEV_VD ",
                clkId   : "TISCI_DEV_MCASP2_LOCAL_AUXCLK_SEL_DEV_VD_CLK",
                clkRate : mcasp_input_clk_freq,
            },
            {
                moduleId: "TISCI_DEV_MCASP2",
                clkId   : "TISCI_DEV_MCASP2_AUX_CLK",
                clkRate : mcasp_input_clk_freq,
            }
        ],
        udmaPdmaChannels            : [
            {
                txCh : "UDMA_PDMA_CH_MAIN0_MCASP2_TX",
                rxCh : "UDMA_PDMA_CH_MAIN0_MCASP2_RX",
            }
        ],
    },
    {
        name                        : "MCASP3",
        regBaseAddr                 : "CSL_MCASP3_CFG_BASE",
        dataRegBaseAddr             : "CSL_MCASP3_DMA_BASE",
        numSerializers              : 16,
        inputClkFreq                : mcasp_input_clk_freq,
        wkupR5RxIntr                : 126,
        wkupR5TxIntr                : 127,
        c7xRxIntr                   : 45,
        c7xTxIntr                   : 46,
        c7xRxEvent                  : 273,
        c7xTxEvent                  : 274,
        clockIds                    : ["TISCI_DEV_MCASP3"],
        clockFrequencies            : [
            {
                moduleId: "TISCI_DEV_MCASP3_LOCAL_AUXCLK_SEL_DEV_VD ",
                clkId   : "TISCI_DEV_MCASP3_LOCAL_AUXCLK_SEL_DEV_VD_CLK",
                clkRate : mcasp_input_clk_freq,
            },
            {
                moduleId: "TISCI_DEV_MCASP3",
                clkId   : "TISCI_DEV_MCASP3_AUX_CLK",
                clkRate : mcasp_input_clk_freq,
            }
        ],
        udmaPdmaChannels            : [
            {
                txCh : "UDMA_PDMA_CH_MAIN0_MCASP3_TX",
                rxCh : "UDMA_PDMA_CH_MAIN0_MCASP3_RX",
            }
        ],
    },
    {
        name                        : "MCASP4",
        regBaseAddr                 : "CSL_MCASP4_CFG_BASE",
        dataRegBaseAddr             : "CSL_MCASP4_DMA_BASE",
        numSerializers              : 16,
        inputClkFreq                : mcasp_input_clk_freq,
        wkupR5RxIntr                : 130,
        wkupR5TxIntr                : 131,
        c7xRxIntr                   : 47,
        c7xTxIntr                   : 48,
        c7xRxEvent                  : 275,
        c7xTxEvent                  : 276,
        clockIds                    : ["TISCI_DEV_MCASP4"],
        clockFrequencies            : [
            {
                moduleId: "TISCI_DEV_MCASP4_LOCAL_AUXCLK_SEL_DEV_VD ",
                clkId   : "TISCI_DEV_MCASP4_LOCAL_AUXCLK_SEL_DEV_VD_CLK",
                clkRate : mcasp_input_clk_freq,
            },
            {
                moduleId: "TISCI_DEV_MCASP4",
                clkId   : "TISCI_DEV_MCASP4_AUX_CLK",
                clkRate : mcasp_input_clk_freq,
            }
        ],
        udmaPdmaChannels            : [
            {
                txCh : "UDMA_PDMA_CH_MAIN0_MCASP4_TX",
                rxCh : "UDMA_PDMA_CH_MAIN0_MCASP4_RX",
            }
        ],
    }
];

let mcasp_ext_rxhclk_src = [
    { name: 0, displayName: "EXT_REFCLK1"},
    { name: 1, displayName: "HFOSC0_CLKOUT"},
    { name: 2, displayName: "AUDIO_EXT_REFCLK0"},
    { name: 3, displayName: "AUDIO_EXT_REFCLK1"},
    { name: 4, displayName: "AUDIO_EXT_REFCLK2"},
    { name: 6, displayName: "MLB_IO_CLK"},
    { name: 8, displayName: "ATCLK0"},
    { name: 9, displayName: "ATCLK1"},
    { name: 10, displayName: "ATCLK2"},
    { name: 11, displayName: "ATCLK3"},
    { name: 12, displayName: "CPSW_CPTS_GENF0"},
    { name: 13, displayName: "CPSW_CPTS_GENF1"},
    { name: 16, displayName: "Invalid Clock"},
];

let mcasp_ext_txhclk_src = [
    { name: 0, displayName: "EXT_REFCLK1"},
    { name: 1, displayName: "HFOSC0_CLKOUT"},
    { name: 2, displayName: "AUDIO_EXT_REFCLK0"},
    { name: 3, displayName: "AUDIO_EXT_REFCLK1"},
    { name: 4, displayName: "AUDIO_EXT_REFCLK2"},
    { name: 6, displayName: "MLB_IO_CLK"},
    { name: 8, displayName: "ATCLK0"},
    { name: 9, displayName: "ATCLK1"},
    { name: 10, displayName: "ATCLK2"},
    { name: 11, displayName: "ATCLK3"},
    { name: 12, displayName: "CPSW_CPTS_GENF0"},
    { name: 13, displayName: "CPSW_CPTS_GENF1"},
    { name: 16, displayName: "Invalid Clock"},
];

function getConfigArr() {
    return mcasp_config;
}

let mcas_ext_hclk_src_list = [
    "EXT_REFCLK1", "CLKOUT0", "AUDIO_EXT_REFCLK0", "AUDIO_EXT_REFCLK1", "AUDIO_EXT_REFCLK2"
];

function getExtClkPins() {
    return mcas_ext_hclk_src_list;
}

function getExtRxHclkSrc() {
    return mcasp_ext_rxhclk_src;
}

function getExtTxHclkSrc() {
    return mcasp_ext_txhclk_src;
}

function getPinmuxReq(txHclkSourceMux, rxHclkSourceMux)
{
    let systemResources = [];
    let pinResource = {}

    if(txHclkSourceMux == 0 || rxHclkSourceMux == 0)
    {
        pinResource = pinmux.getPinRequirements("CLOCKING", "EXT_REFCLK1", "External ref clk 1");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        systemResources.push(pinResource);
    }
    if(txHclkSourceMux == 1 || rxHclkSourceMux == 1)
    {
        pinResource = pinmux.getPinRequirements("CLOCKING", "CLKOUT0", "High Frequency Oscillator clk out 0");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        systemResources.push(pinResource);
    }
    if(txHclkSourceMux == 2 || rxHclkSourceMux == 2)
    {
        pinResource = pinmux.getPinRequirements("CLOCKING", "AUDIO_EXT_REFCLK0", "Audio external ref clk 0");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        systemResources.push(pinResource);
    }
    if(txHclkSourceMux == 3 || rxHclkSourceMux == 3)
    {
        pinResource = pinmux.getPinRequirements("CLOCKING", "AUDIO_EXT_REFCLK1", "Audio external ref clk 1");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        systemResources.push(pinResource);
    }
    if(txHclkSourceMux == 4 || rxHclkSourceMux == 4)
    {
        pinResource = pinmux.getPinRequirements("CLOCKING", "AUDIO_EXT_REFCLK2", "Audio external ref clk 1");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        systemResources.push(pinResource);
    }

    return systemResources;
}

function getSystemPinmux(systemResources)
{
    let clockingPinmux = {
        name: "CLOCKING",
        displayName: "Clocking",
        interfaceName: "CLOCKING",
        resources: systemResources
    };

    return clockingPinmux;
}

exports = {
    getConfigArr,
    mcasp_input_clk_freq,
    getExtRxHclkSrc,
    getExtTxHclkSrc,
    getExtClkPins,
    getPinmuxReq,
    getSystemPinmux,
};
