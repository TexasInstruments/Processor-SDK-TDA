let common = system.getScript("/common");

let mcan_func_clk = 80 * 1000 * 1000;

const mcan_config_r5fss = [
    {
        name            : "MCAN0",
        baseAddr        : "CSL_MCAN0_MSGMEM_RAM_BASE",
        intrNum         : 230,
        clockIds        : [ "TISCI_DEV_MCAN0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCAN0",
                clkId   : "TISCI_DEV_MCAN0_MCANSS_CCLK_CLK",
                clkRate : mcan_func_clk,
            },
        ],
    },
    {
        name            : "MCAN1",
        baseAddr        : "CSL_MCAN1_MSGMEM_RAM_BASE",
        intrNum         : 232,
        clockIds        : [ "TISCI_DEV_MCAN1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCAN1",
                clkId   : "TISCI_DEV_MCAN1_MCANSS_CCLK_CLK",
                clkRate : mcan_func_clk,
            },
        ],
    },
    {
        name            : "MCAN2",
        baseAddr        : "CSL_MCAN2_MSGMEM_RAM_BASE",
        intrNum         : 234,
        clockIds        : [ "TISCI_DEV_MCAN2" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCAN2",
                clkId   : "TISCI_DEV_MCAN2_MCANSS_CCLK_CLK",
                clkRate : mcan_func_clk,
            },
        ],
    },
    {
        name            : "MCAN3",
        baseAddr        : "CSL_MCAN3_MSGMEM_RAM_BASE",
        intrNum         : 236,
        clockIds        : [ "TISCI_DEV_MCAN3" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCAN3",
                clkId   : "TISCI_DEV_MCAN3_MCANSS_CCLK_CLK",
                clkRate : mcan_func_clk,
            },
        ],
    },
    {
        name            : "MCAN4",
        baseAddr        : "CSL_MCAN4_MSGMEM_RAM_BASE",
        intrNum         : 238,
        clockIds        : [ "TISCI_DEV_MCAN4" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCAN4",
                clkId   : "TISCI_DEV_MCAN4_MCANSS_CCLK_CLK",
                clkRate : mcan_func_clk,
            },
        ],
    },
];

function getConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let mcan_config;
    if(cpu.match(/r5f*/))
    {
        mcan_config = mcan_config_r5fss;
    }

    return mcan_config;
}

function getInterfaceName(instance) {
    let cpu = common.getSelfSysCfgCoreName();
    if(cpu.match(/r5f*/))
    {
        return "MCAN";
    }

    return "MCAN";

}

exports = {
    getConfigArr,
    getInterfaceName,
};
