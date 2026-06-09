
let common = system.getScript("/common");

let timerClockSourceConfig_r5f_a53 = {
    name: "clkSource",
    displayName: "Input Clock Source",
    default: "MCU_HFOSC0",
    options: [
        {
            "name": "MCU_HFOSC0",
        },
    ],
};
let staticConfig_m4f = [
    {
        name: "M4_SYSTICK",
        timerBaseAddr: 0xE000E010, /* Setting to SYST_CSR as defined by ARMv7-M */
        timerHwiIntNum: 15,
        timerInputPreScaler: 1, /* NOT USED */
        clkSelMuxAddr: 0, /* NOT USED */
        disableClkSourceConfig: true,
    }
];

let timerClockSourceConfig_m4f = {
    name: "clkSource",
    displayName: "Input Clock Source",
    default: "M4_CPU_CLK",
    options: [
        {
            "name": "M4_CPU_CLK",
        },
    ],
    readOnly: true,
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
        "r5fss0-0": 0,
        "m4fss0-0": 0,
        "a53ss0-0": 0,
        "a53ss0-1": 0,
        "a53ss1-0": 0,
        "a53ss1-1": 0,
    }
    return defaultInstanceMap[cpu];
}

function getStaticConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let staticConfigArr;

    if(cpu.match(/r5f*/)) {
        let staticConfig_r5f = [];

        for(let i=0; i<2; i++)
        {
            staticConfig_r5f.push(
                {
                    name: `TIMER${i}`,
                    timerBaseAddr: 0x2b100000 + i*0x10000,
                    timerHwiIntNum: 138 + i,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x430081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_WKUP",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_r5f;
    }
    else if(cpu.match(/m4f*/))
    {
        staticConfigArr = staticConfig_m4f;
    }
    else if(cpu.match(/a53*/))
    {
        let staticConfig_a53 = [];
        let i = 0;
        switch(cpu)
        {
            case "a53ss0-0":
                i = 6;
                break;
            case "a53ss0-1":
                i = 7;
                break;
            case "a53ss1-0":
                i = 4;
                break;
            case "a53ss1-1":
                i = 5;
                break;
            default  :
               break;

        }
        staticConfig_a53.push(
            {
                name: `TIMER${i}`,
                timerBaseAddr: 0x02400000 + i*0x10000,
                timerHwiIntNum: 152 + i,
                timerInputPreScaler: 1,
                clkSelMuxAddr: 0x001081B0 + 4*i,
                disableClkSourceConfig: false,
                lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                lockUnlockPartition: 2,
            }
        )
        staticConfigArr = staticConfig_a53;
    }

    return staticConfigArr;
}

function getTimerClockSourceConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let timerClockSourceConfig = timerClockSourceConfig_r5f_a53;

    if(cpu.match(/r5f*/)) {
        timerClockSourceConfig = timerClockSourceConfig_r5f_a53;
    }
    if(cpu.match(/a53*/)) {
        timerClockSourceConfig = timerClockSourceConfig_r5f_a53;
    }
    if(cpu.match(/m4f*/)) {
        timerClockSourceConfig = timerClockSourceConfig_m4f;
    }

    return timerClockSourceConfig;
}

function getDefaultTimerClockSourceMhz() {
    let cpu = common.getSelfSysCfgCoreName();

    if(cpu.match(/m4f*/)) {
        return 400*1000000;
    }

    return 25000000;
}

exports = {
    getStaticConfigArr,
    getTimerClockSourceConfigArr,
    getTimerClockSourceValue,
    getDefaultTimerClockSourceMhz,
    makeInstanceConfig,
};
