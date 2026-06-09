
let common = system.getScript("/common");

let timerClockSourceConfig = [
        {
            "name": "HFOSC0_CLKOUT_32K",
        },
        {
            "name": "CLK_32K_RC",
        },
];

const rtc_config_r5f = [
    {
        name: `WKUP_RTCSS0`,
        baseAddr: 0x2B1F0000,
        intrNum: 97,
        eventId: 0,
        timerInputPreScaler: 1,
        clkSelMuxAddr: 0x43008600 ,
        lockUnlockDomain: "SOC_DOMAIN_ID_WKUP",
        lockUnlockPartition: 2,
    },
];

function getTimerClockSourceValue(instance) {

    let clkSelMuxValue = 0;

    switch(instance.clkSource) {
        default:
        case "HFOSC0_CLKOUT_32K":
            clkSelMuxValue = 0x0;
            break;
        case "CLK_32K_RC":
            clkSelMuxValue = 0x1;
            break;
    }

    return clkSelMuxValue;
}

function getTimerClockSelectMaskValue(instance) {

    return getTimerClockSourceValue(instance);
}



function getStaticConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let staticConfigArr;

    if(cpu.match(/r5f*/))
    {
        staticConfigArr = rtc_config_r5f;
    }

    return staticConfigArr;
}

function getDefaultConfig()
{
    let cpu = common.getSelfSysCfgCoreName();
    let staticConfigArr;
    if(cpu.match(/r5f*/))
    {
        staticConfigArr = rtc_config_r5f;
    }

    return staticConfigArr[0];
}

function getConfigArr() {

    return getStaticConfigArr();
}

function getTimerClockSourceConfigArr() {

    return timerClockSourceConfig;
}

function getTimerClockSourceHz(clkSource) {

    let clkSourceHz = 0;

    switch(clkSource) {
        default:
        case "HFOSC0_CLKOUT_32K":
            clkSourceHz = 32*1000;
            break;
        case "CLK_32K_RC":
            clkSourceHz = 32*1000;
            break;
    }

    return clkSourceHz;
}

exports = {
    getConfigArr,
    getDefaultConfig,
    getTimerClockSourceConfigArr,
    getTimerClockSelectMaskValue,
    getTimerClockSourceHz,
};
