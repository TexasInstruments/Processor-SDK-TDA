
let common = system.getScript("/common");

let timerClockSourceConfig_r5f = {
    name: "clkSource",
    displayName: "Input Clock Source",
    default: "MCU_HFOSC0",
    options: [
        {
            "name": "MCU_HFOSC0",
        },
    ],
};

function getTimerClockSourceValue(instance) {
    let clkSelMuxValue = 0;

    switch(instance.clkSource) {
        default:
        case "MCU_HFOSC0":
            clkSelMuxValue = 0x0;
            break;
    }
    return clkSelMuxValue;
}

function makeInstanceConfig() {
    let config = {};
    let staticConfigArr = getStaticConfigArr();
    let defaultInstanceIndex = getDefaultInstance();

    if(staticConfigArr.length == 0)
        return undefined;

    config.name = "instance";
    config.displayName = "Instance";
    config.description = "Select Instance";
    config.default = staticConfigArr[defaultInstanceIndex].name;
    config.options = [];

    for (let i = 0; i < staticConfigArr.length; i++) {
        let option = {};

        option.name = staticConfigArr[i].name;
        config.options.push(option);
    }

    return config;
}

function getDefaultInstance() {
    let cpu = common.getSelfSysCfgCoreName();
    let defaultInstanceMap = {
        "wkup-r5fss0-0": 0,
        "r5fss0-0": 0,
        "r5fss0-1": 1,
        "r5fss1-0": 2,
        "r5fss1-1": 3,
        "c75ss0-0": 0,
        "c75ss1-0": 0,
    }
    return defaultInstanceMap[cpu];
}

function getStaticConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let staticConfigArr;

    if (cpu.match(/wkup-r5fss0-0/)){
        let staticConfig_wkup_r5f = [];
        for(let i=0; i<1; i++)
        {
            staticConfig_wkup_r5f.push(
                {
                    name: `WKUP_TIMER${i}`,
                    timerBaseAddr: 0x2B100000+ i*0x10000,
                    timerHwiIntNum: 138 + i,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x430081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_WKUP",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_wkup_r5f;
    }
    else if(cpu.match(/r5fss/)) {
        let staticConfig_r5f = [];

        for(let i=0; i<4; i++)
        {
            staticConfig_r5f.push(
                {
                    name: `TIMER${i}`,
                    timerBaseAddr: 0x02400000 + i*0x10000,
                    timerHwiIntNum: 24 + i,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_r5f;
    }
    else if(cpu.match(/c75ss0-0/)){
        let staticConfig_c75x = [];

        for(let i=4; i<5; i++)
        {
            staticConfig_c75x.push(
                {
                    name: `TIMER${i}`,
                    timerBaseAddr: 0x02400000 + i*0x10000,
                    timerHwiIntNum: 2 + i - 4,
                    eventId: 120 + 256 +  i, /* (256 - GIC SPI Intr start, ref: clec_spec am275_soc_event_out_mapping)*/
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_c75x;

    }
    else if(cpu.match(/c75ss1-0/)){
        let staticConfig_c75x = [];

        for(let i=5; i<6; i++)
        {
            staticConfig_c75x.push(
                {
                    name: `TIMER${i}`,
                    timerBaseAddr: 0x02400000 + i*0x10000,
                    timerHwiIntNum: 2 + i - 5,
                    eventId: 120 + 256 +  i, /* (256 - GIC SPI Intr start, ref: clec_spec am275_soc_event_out_mapping)*/
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_c75x;

    }

    return staticConfigArr;
}

function getTimerClockSourceConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let timerClockSourceConfig = timerClockSourceConfig_r5f;

    if(cpu.match(/wkup-r5fss0-0/) || cpu.match(/r5fss/) || cpu.match(/c75ss/)) {
        timerClockSourceConfig = timerClockSourceConfig_r5f;
    }

    return timerClockSourceConfig;
}

function getDefaultTimerClockSourceMhz() {
    let cpu = common.getSelfSysCfgCoreName();

    return 25000000;
}

exports = {
    getStaticConfigArr,
    getTimerClockSourceConfigArr,
    getTimerClockSourceValue,
    getDefaultTimerClockSourceMhz,
    makeInstanceConfig,
};
