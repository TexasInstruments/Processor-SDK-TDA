
let common = system.getScript("/common");

let epwm_func_clk = 250 * 1000 * 1000;

const epwm_config_r5 = [
    {
        name: "EPWM0",
        baseAddr: "CSL_EPWM0_EPWM_BASE",
        intrNum: 0,
        tripIntrNum: 80,
        isSyncoPresent: true,
        isSynciPresent: true,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM0_VBUSP_CLK" ],
    },
    {
        name: "EPWM1",
        baseAddr: "CSL_EPWM1_EPWM_BASE",
        intrNum: 0,
        tripIntrNum: 81,
        isSyncoPresent: false,
        isSynciPresent: false,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM1_VBUSP_CLK" ],
    },
    {
        name: "EPWM2",
        baseAddr: "CSL_EPWM2_EPWM_BASE",
        intrNum: 0,
        tripIntrNum: 82,
        isSyncoPresent: false,
        isSynciPresent: false,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM2_VBUSP_CLK" ],
    },
];

const epwm_config_c75 = [
    {
        name: "EPWM0",
        baseAddr: "CSL_EPWM0_EPWM_BASE",
        eventId: 197 + 256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        intrNum: 49,
        tripIntrNum: 198,
        isSyncoPresent: true,
        isSynciPresent: true,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM0_VBUSP_CLK" ],
    },
    {
        name: "EPWM1",
        baseAddr: "CSL_EPWM1_EPWM_BASE",
        eventId: 199 + 256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        intrNum: 50,
        tripIntrNum: 201,
        isSyncoPresent: false,
        isSynciPresent: false,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM1_VBUSP_CLK" ],
    },
    {
        name: "EPWM2",
        baseAddr: "CSL_EPWM2_EPWM_BASE",
        eventId: 202 + 256, /* (256 - GIC SPI Intr start, ref: clec_spec am62a_soc_event_out_mapping)*/
        intrNum: 51,
        tripIntrNum: 203,
        isSyncoPresent: false,
        isSynciPresent: false,
        funcClk: epwm_func_clk,
        clockIds: [ "TISCI_DEV_EPWM2_VBUSP_CLK" ],
    },
];

function getStaticConfigArr() {
    let epwm_config;

    if(common.getSelfSysCfgCoreName().includes("r5f")) {
        epwm_config = epwm_config_r5;
    }
    else {
        epwm_config = epwm_config_c75;
    }

    return epwm_config;
}

let soc = {
    getStaticConfigArr,
};

exports = soc;