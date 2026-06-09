
let common = system.getScript("/common");

let timerClockSourceConfig = [
    {
        "name": "HFOSC0_CLKOUT",
    },
    {
        "name": "DEVICE_CLKOUT_32K",
    },
    {
        "name": "MAIN_PLL0_HSDIV7_CLKOUT",
    },
    {
        "name": "CLK_12M_RC",
    },
    {
        "name": "MCU_EXT_REFCLK0",
    },
    {
        "name": "EXT_REFCLK1",
    },
    {
        "name": "CP_GEMAC_CPTS0_RFT_CLK",
    },
    {
        "name": "MAIN_PLL1_HSDIV3_CLKOUT",
    },
    {
        "name": "MAIN_PLL2_HSDIV6_CLKOUT",
    },
    {
        "name": "CPSW0_CPTS_GENF0",
    },
    {
        "name": "CPSW0_CPTS_GENF1",
    },
];

let timerClockSourceConfig_wkup = [
    {
        "name": "HFOSC0_CLKOUT",
    },
    {
        "name": "DM_CLK_BY_2",
    },
    {
        "name": "CLK_12M_RC",
    },
    {
        "name": "MCU_PLL0_HSDIV5_CLKOUT",
    },
    {
        "name": "MCU_EXT_REFCLK0",
    },
    {
        "name": "DEVICE_CLKOUT_32K",
    },
    {
        "name": "CPSW0_CPTS_GENF0",
    },
    {
        "name": "CLK_32K_RC",
    },
];

let timerClockSourceConfig_main = [
    {
        "name": "HFOSC0_CLKOUT",
    },
    {
        "name": "DEVICE_CLKOUT_32K",
    },
    {
        "name": "MAIN_PLL0_HSDIV7_CLKOUT",
    },
    {
        "name": "CLK_12M_RC",
    },
    {
        "name": "MCU_EXT_REFCLK0",
    },
    {
        "name": "EXT_REFCLK1",
    },
    {
        "name": "CP_GEMAC_CPTS0_RFT_CLK",
    },
    {
        "name": "MAIN_PLL1_HSDIV3_CLKOUT",
    },
    {
        "name": "MAIN_PLL2_HSDIV6_CLKOUT",
    },
    {
        "name": "CPSW0_CPTS_GENF0",
    },
    {
        "name": "CPSW0_CPTS_GENF1",
    },
];

function getTimerClockSourceValue(instance) {
    let cpu = common.getSelfSysCfgCoreName();
    let clkSelMuxValue = 0;

    if(cpu.match(/wkup-r5fss0-0/)) {
        switch(instance.clkSource) {
            default:
            case "HFOSC0_CLKOUT":
                clkSelMuxValue = 0x0;
                break;
            case "DM_CLK_BY_2":
                clkSelMuxValue = 0x1;
                break;
            case "CLK_12M_RC":
                clkSelMuxValue = 0x2;
                break;
            case "MCU_PLL0_HSDIV5_CLKOUT":
                clkSelMuxValue = 0x3;
                break;
            case "MCU_EXT_REFCLK0":
                clkSelMuxValue = 0x4;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSelMuxValue = 0x5;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSelMuxValue = 0x6;
                break;
            case "CLK_32K_RC":
                clkSelMuxValue = 0x7;
                break;
        }
    }
    else if(cpu.match(/r5fss/)) {
        switch(instance.clkSource) {
            default:
            case "HFOSC0_CLKOUT":
                clkSelMuxValue = 0x0;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSelMuxValue = 0x1;
                break;
            case "MAIN_PLL0_HSDIV7_CLKOUT":
                clkSelMuxValue = 0x02;
                break;
            case "CLK_12M_RC":
                clkSelMuxValue = 0x3;
                break;
            case "MCU_EXT_REFCLK0":
                clkSelMuxValue = 0x4;
                break;
            case "EXT_REFCLK1":
                clkSelMuxValue = 0x5;
                break;
            case "CP_GEMAC_CPTS0_RFT_CLK":
                clkSelMuxValue = 0x7;
                break;
            case "MAIN_PLL1_HSDIV3_CLKOUT":
                clkSelMuxValue = 0x8;
                break;
            case "MAIN_PLL2_HSDIV6_CLKOUT":
                clkSelMuxValue = 0x9;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSelMuxValue = 0xA;
                break;
            case "CPSW0_CPTS_GENF1":
                clkSelMuxValue = 0xB;
                break;
        }
    }else if(cpu.match(/c75ss/)) {
        switch(instance.clkSource) {
            default:
            case "HFOSC0_CLKOUT":
                clkSelMuxValue = 0x0;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSelMuxValue = 0x1;
                break;
            case "MAIN_PLL0_HSDIV7_CLKOUT":
                clkSelMuxValue = 0x02;
                break;
            case "CLK_12M_RC":
                clkSelMuxValue = 0x3;
                break;
            case "MCU_EXT_REFCLK0":
                clkSelMuxValue = 0x4;
                break;
            case "EXT_REFCLK1":
                clkSelMuxValue = 0x5;
                break;
            case "CP_GEMAC_CPTS0_RFT_CLK":
                clkSelMuxValue = 0x7;
                break;
            case "MAIN_PLL1_HSDIV3_CLKOUT":
                clkSelMuxValue = 0x8;
                break;
            case "MAIN_PLL2_HSDIV6_CLKOUT":
                clkSelMuxValue = 0x9;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSelMuxValue = 0xA;
                break;
            case "CPSW0_CPTS_GENF1":
                clkSelMuxValue = 0xB;
                break;
        }
    }
    return clkSelMuxValue;
}

function getStaticConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();
    let staticConfigArr;

    if (cpu.match(/wkup-r5fss0-0/)){
        let staticConfig_wkup_r5f = [];
        for(let i=0; i<2; i++)
        {
            staticConfig_wkup_r5f.push(
                {
                    timerName: `WKUP_TIMER${i}`,
                    timerBaseAddr: 0x2B100000+ i*0x10000,
                    timerHwiIntNum: 138 + i,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x430081B0 + 4*i,
                    lockUnlockDomain: "SOC_DOMAIN_ID_WKUP",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_wkup_r5f;
    }
    else if(cpu.match(/r5fss/)) {
        let staticConfig_r5f = [];
        for(let i=0; i<6; i++)
        {
            staticConfig_r5f.push(
                {
                    timerName: `TIMER${i}`,
                    timerBaseAddr: 0x02400000 + i*0x10000,
                    timerHwiIntNum: 24 + i,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        for(let i=6; i<8; i++)
        {
            staticConfig_r5f.push(
                {
                    timerName: `TIMER${i}`,
                    timerBaseAddr: 0x02400000+ i*0x10000,
                    timerHwiIntNum: 22 + i - 6,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        for(let i=8; i<16; i++)
        {
            staticConfig_r5f.push(
                {
                    timerName: `TIMER${i}`,
                    timerBaseAddr: 0x02400000 + i*0x10000,
                    timerHwiIntNum: 264 + i - 8,
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        staticConfigArr = staticConfig_r5f;
    }
    else if (cpu.match(/c75ss0-0/)){
        let staticConfig_c75x = [];
        for(let i=6; i<8; i++)
        {
            staticConfig_c75x.push(
                {
                    timerName: `TIMER${i}`,
                    timerBaseAddr: 0x02400000+ i*0x10000,
                    timerHwiIntNum: 3 + i - 6,
                    eventId: 120 + 256 +  i, /* (256 - GIC SPI Intr start, ref: clec_spec am275_soc_event_out_mapping)*/
                    timerInputPreScaler: 1,
                    clkSelMuxAddr: 0x001081B0 + 4*i,
                    disableClkSourceConfig: false,
                    lockUnlockDomain: "SOC_DOMAIN_ID_MAIN",
                    lockUnlockPartition: 2,
                }
            )
        }
        for(let i=8; i<10; i++)
        {
            staticConfig_c75x.push(
                {
                    timerName: `TIMER${i}`,
                    timerBaseAddr: 0x02400000+ i*0x10000,
                    timerHwiIntNum: 5 + i - 8,
                    eventId: 32 + 256 +  i - 8, /* (256 - GIC SPI Intr start, ref: clec_spec am275_soc_event_out_mapping)*/
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
    else if (cpu.match(/c75ss1-0/)){
        let staticConfig_c75x = [];
        for(let i=10; i<14; i++)
            {
                staticConfig_c75x.push(
                    {
                        timerName: `TIMER${i}`,
                        timerBaseAddr: 0x02400000+ i*0x10000,
                        timerHwiIntNum: 3 + i - 10,
                        eventId: 34 + 256 +  i, /* (256 - GIC SPI Intr start, ref: clec_spec am275_soc_event_out_mapping)*/
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

function getInterfaceName(inst) {
    let cpu = common.getSelfSysCfgCoreName();
    if (cpu.match(/wkup-r5fss0-0/)){
        return "WKUP_TIMER";
    }
    else if(cpu.match(/r5fss/)) {
        return "TIMER";
    } else if (cpu.match(/c75ss/)){
        return "TIMER";
    }

    return "TIMER";
}

function getTimerClockSourceConfigArr() {
    let cpu = common.getSelfSysCfgCoreName();

    if(cpu.match(/wkup-r5fss0-0/)) {
        return timerClockSourceConfig_wkup;
    }
    else if(cpu.match(/r5fss/)) {
        return timerClockSourceConfig;
    }
    else if(cpu.match(/c75ss/)) {
        return timerClockSourceConfig_main;
    }

    return timerClockSourceConfig;
}

function getTimerClockSourceHz(clkSource) {
    let cpu = common.getSelfSysCfgCoreName();
    let clkSourceHz = 0;

    if(cpu.match(/wkup-r5fss0-0*/)) {
        /* DM R5*/
        switch(clkSource) {
            default:
            case "HFOSC0_CLKOUT":
                clkSourceHz = 25*1000000;
                break;
            case "DM_CLK_BY_2":
                clkSourceHz = 100*1000000;
                break;
            case "CLK_12M_RC":
                clkSourceHz = 12*1000000;
                break;
            case "MCU_PLL0_HSDIV5_CLKOUT":
                clkSourceHz = 200*1000000;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSourceHz = 100*1000000;
                break;
            case "MAIN_PLL0_HSDIV7_CLKOUT":
                clkSourceHz = 12*1000000;
                break;
            case "CLK_12M_RC":
                clkSourceHz = 200*1000000;
                break;
            case "MCU_EXT_REFCLK0":
                clkSourceHz = 1*1000000;
                break;
            case "CPSW2G_CPTS_RFT_CLK":
                clkSourceHz = 1*1000000;
                break;
            case "MAIN_PLL1_HSDIV3_CLKOUT":
                clkSourceHz = 32*1000;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSourceHz = 1*1000000; //to check
                break;
            case "DEVICE_CLKOUT_32K":
                clkSourceHz = 32*1000;
                break;
            case "CLK_32K_RC":
                clkSourceHz = 32*1000;
                break;
        }
    }
    else if(cpu.match(/r5fss*/)) {
        /* MCU R5*/
        switch(clkSource) {
            default:clkSourceHz
            case "HFOSC0_CLKOUT":
                clkSourceHz = 25*1000000;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSourceHz = 32*1000;
                break;
            case "MAIN_PLL0_HSDIV7_CLKOUT":
                clkSourceHz = 200*1000000;
                break;
            case "CLK_12M_RC":
                clkSourceHz = 12*1000000;
                break;
            case "MCU_EXT_REFCLK0":
                clkSourceHz = 1*1000000;
                break;
            case "EXT_REFCLK1":
                clkSourceHz = 32*1000;
                break;
            case "CP_GEMAC_CPTS0_RFT_CLK":
                clkSourceHz = 200*1000000; // to check
                break;
            case "MAIN_PLL1_HSDIV3_CLKOUT":
                clkSourceHz = 32*1000;
                break;
            case "MAIN_PLL2_HSDIV6_CLKOUT":
                clkSourceHz = 200*1000000;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSourceHz = 1*1000000;
                break;
            case "CPSW0_CPTS_GENF1":
                clkSourceHz = 1*1000000;
                break;
        }
    }
    else if(cpu.match(/c75ss*/)) {
        /* C75 */
        switch(clkSource) {
            default:clkSourceHz
            case "HFOSC0_CLKOUT":
                clkSourceHz = 25*1000000;
                break;
            case "DEVICE_CLKOUT_32K":
                clkSourceHz = 32*1000;
                break;
            case "MAIN_PLL0_HSDIV7_CLKOUT":
                clkSourceHz = 200*1000000;
                break;
            case "CLK_12M_RC":
                clkSourceHz = 12*1000000;
                break;
            case "MCU_EXT_REFCLK0":
                clkSourceHz = 1*1000000;
                break;
            case "EXT_REFCLK1":
                clkSourceHz = 32*1000;
                break;
            case "CP_GEMAC_CPTS0_RFT_CLK":
                clkSourceHz = 200*1000000; // to check
                break;
            case "MAIN_PLL1_HSDIV3_CLKOUT":
                clkSourceHz = 32*1000;
                break;
            case "MAIN_PLL2_HSDIV6_CLKOUT":
                clkSourceHz = 200*1000000;
                break;
            case "CPSW0_CPTS_GENF0":
                clkSourceHz = 1*1000000;
                break;
            case "CPSW0_CPTS_GENF1":
                clkSourceHz = 1*1000000;
                break;
        }
    }

    return clkSourceHz;
}

function getBlockedTimers() {

    let cpu = common.getSelfSysCfgCoreName();

    if(cpu.match(/wkup-r5fss0-0/)) {
        return ['WKUP_TIMER0'];
    }
    else if(cpu.match(/r5fss/)) {
        return ['TIMER0', 'TIMER1', 'TIMER2', 'TIMER3', 'TIMER4', 'TIMER5'];
    }
    else if(cpu.match(/c75ss0-0/)) {
        return ['TIMER0', 'TIMER1', 'TIMER2', 'TIMER3', 'TIMER4', 'TIMER5', 'TIMER10', 'TIMER11', 'TIMER12', 'TIMER13'];
    }
    else if(cpu.match(/c75ss1-0/)) {
        return ['TIMER0', 'TIMER1', 'TIMER2', 'TIMER3', 'TIMER4', 'TIMER5', 'TIMER6', 'TIMER7', 'TIMER8', 'TIMER9'];
    }

    return [];
}

exports = {
    getStaticConfigArr,
    getTimerClockSourceConfigArr,
    getTimerClockSourceValue,
    getTimerClockSourceHz,
    getInterfaceName,
    getBlockedTimers,
    dmaTriggerSupport: false,
};
