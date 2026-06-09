let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let soc = system.getScript(`/drivers/rtc/soc/rtc_${common.getSocName()}`);
let hwi = system.getScript("/kernel/dpl/hwi.js");

function getConfigArr() {
    return soc.getConfigArr();
}

function getDefaultConfig() {
    return soc.getDefaultConfig();
}

function getInstanceConfig(moduleInstance) {

    let config = getDefaultConfig();

    let clkSelMuxValue = soc.getTimerClockSelectMaskValue(moduleInstance);

    return {
        ...config,
        ...moduleInstance,
        clkSelMuxValue : clkSelMuxValue,
    }
}

function onChangeClkSource(instance, ui)
{
    instance.timerInputClkHz = soc.getTimerClockSourceHz(instance.clkSource);
}

function validate(instance, report) {
    common.validate.checkNumberRange(instance, report, "timerInputClkHz", 32000, 32768, "dec");

    if(instance.enableIntr) {
        if (instance.on_offCallback && (instance.ON_OFF_CallbackFxn == "NULL" || instance.ON_OFF_CallbackFxn == "")) {
            report.logError("ON_OFF Callback function must be provided if 'on_offCallback' is enabled.", instance, "ON_OFF_CallbackFxn");
        }

        if (instance.off_onCallback && (instance.OFF_ON_CallbackFxn == "NULL" || instance.OFF_ON_CallbackFxn == "")) {
            report.logError("OFF_ON Callback function must be provided if 'off_onCallback' is enabled.", instance, "OFF_ON_CallbackFxn");
        }
    }
}

let rtc_module_name = "/drivers/rtc/rtc";

let rtc_module = {
    displayName: "RTC",
    templates: {
        "/drivers/system/system_config.c.xdt": {
			driver_config: "/drivers/rtc/templates/rtc_v0_config.c.xdt",
			driver_init: "/drivers/rtc/templates/rtc_init.c.xdt",
			driver_deinit: "/drivers/rtc/templates/rtc_deinit.c.xdt",
		},
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/rtc/templates/rtc.h.xdt",
        },

        "/drivers/system/drivers_open_close.h.xdt": {
            driver_open_close_config: "/drivers/rtc/templates/rtc_open_close.h.xdt",
        },
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: rtc_module_name,
        },
        "/drivers/system/power_clock_config.c.xdt": {
            moduleName: rtc_module_name,
        },
    },
    maxInstances: getConfigArr().length,

    defaultInstanceName: "CONFIG_RTC",

    config: [
        {
            name: "timerInputClkHz",
            displayName: "Input Clock Frequency (Hz)",
            default: soc.getTimerClockSourceHz( soc.getTimerClockSourceConfigArr()[0].name ),
            description: "Make sure the value put here matches the actual input clock frequency provided on the board/SOC",
            hidden: false,
        },
        {
            name: "clkSource",
            displayName: "Input Clock Source",
            default: soc.getTimerClockSourceConfigArr()[0].name,
            options: soc.getTimerClockSourceConfigArr(),
            hidden: false,
            onChange: onChangeClkSource,
        },
        {
            name: "enableIntr",
            displayName: "Enable Interrupt",
            default: false,
            onChange: function(inst, ui) {
                ui.intrPriority.hidden = !inst.enableIntr;
                ui.ON_OFF_CallbackFxn.hidden = !inst.enableIntr || !inst.on_offCallback;
                ui.ON_OFF_CallbackFxn.hidden = !inst.enableIntr || !inst.off_onCallback;

                if (!inst.enableIntr) {
                    inst.ON_OFF_CallbackFxn = "NULL";
                    inst.OFF_ON_CallbackFxn = "NULL";
                }
            },
            description: "Enable or Disable interrupts for RTC",
        },
        {
            name: "intrPriority",
            displayName: "Interrupt Priority",
            default: 4,
            description: `Interrupt Priority: 0 (highest) to ${hwi.getHwiMaxPriority()} (lowest)`,
            hidden: true,
        },
        {
            name: "isFreezeMode",
            displayName: "Enable Freeze Mode",
            default: true,
            description: "Enable or disable Freeze Mode.",
        },
        {
            name: "on_offCallback",
            displayName: "Enable ON_OFF Timer Callback",
            default: false,
            onChange: function(inst, ui) {
                ui.ON_OFF_CallbackFxn.hidden = !inst.on_offCallback;
                if (!inst.on_offCallback) {
                    inst.ON_OFF_CallbackFxn = "NULL";
                }
            },
        },
        {
            name: "ON_OFF_CallbackFxn",
            displayName: "ON_OFF Timer Callback Function",
            default: "NULL",
            hidden: true,
            description: "Callback function for ON_OFF interrupt",
        },
        {
            name: "off_onCallback",
            displayName: "Enable OFF_ON Timer Callback",
            default: false,
            onChange: function(inst, ui) {
                ui.OFF_ON_CallbackFxn.hidden = !inst.off_onCallback;
                if (!inst.off_onCallback) {
                    inst.OFF_ON_CallbackFxn = "NULL";
                }
            },
        },
        {
            name: "OFF_ON_CallbackFxn",
            displayName: "OFF_ON Timer Callback Function",
            default: "NULL",
            hidden: true,
            description: "Callback function for OFF_ON interrupt",
        },
    ],
    validate: validate,
    getInstanceConfig,
};

exports = rtc_module;