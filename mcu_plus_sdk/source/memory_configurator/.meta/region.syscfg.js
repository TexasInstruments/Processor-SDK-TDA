let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();

let isC7XCore = selfCoreName && (selfCoreName.includes("c75ss0") || selfCoreName.includes("c75ss1"));

/* RAT is available on R5F and C66x cores */
let isRatCore = selfCoreName && (selfCoreName.includes("r5fss") || selfCoreName.includes("c66"));
let settingLabel = isC7XCore ? "Automate MMU Setting" : "Automate MPU Setting";
let settingDescription = isC7XCore ?
    "Checking this would save the user from manually configuring MMU for regions. Uncheck this to get flexibility in configuring MMU." :
    "Checking this would save the user from manually configuring MPU for regions. Uncheck this to get flexibility in configuring MPU.";

let rtos = (typeof process !== "undefined" && process.env.RTOS) ? process.env.RTOS.toLowerCase() : "freertos";

let config = [
    {
        name: "$name",
        hidden: false,
        isCIdentifier: false,
    },
    {
        name: "mpu_setting",
        displayName: settingLabel,
        default: false,
        longDescription: settingDescription,
    },
    {
        name: "rat_setting",
        displayName: "Automate RAT Setting",
        default: false,
        hidden: !isRatCore,
        longDescription: "Checking this lets every memory region own an optional RAT entry whose local address and " +
            "size follow the region. Only the translated system address needs to be entered. Independent of the " +
            "MPU setting. Uncheck this to configure RAT entries manually in the DPL RAT (addr_translate) module.",
    },
];

function addModuleInstances(inst) {
    let modInstances = new Array();
    let module_name = "";

    if (inst.mpu_setting) {
        if (selfCoreName && (selfCoreName.includes("c75ss0") || selfCoreName.includes("c75ss1"))) {
            module_name = "memory_configurator/memory_region_mmu";
        } else {
            module_name = "memory_configurator/memory_region_mpu";
        }
    } else {
        module_name = "memory_configurator/memory_region";
    }

    modInstances.push({
        name: "memory_region",
        displayName: "Region",
        moduleName: module_name,
        useArray: true,
        minInstanceCount: 0,
        collapsed: false,
        /* ratAutomated only exists on memory_region / memory_region_mpu, so it
         * must not be forced on cores that use memory_region_mmu (C7x). RAT
         * automation is independent of MPU: the same flag works with both
         * sub-module types. */
        ...(isRatCore ? { requiredArgs: { ratAutomated: inst.rat_setting } } : {}),
    });

    return modInstances;
}

function validate(inst, report) {
    let coreNames = common.getSysCfgCoreNames();
    let own_mpu = inst.mpu_setting;
    let settingName = isC7XCore ? "MMU" : "MPU";

    for (let core of coreNames) {
        if (core.includes(selfCoreName)) {
            continue;
        }

        let core_module = common.getModuleForCore('/memory_configurator/region', core);
        if (core_module !== undefined) {
            let others_mpu = core_module.$instances[0].mpu_setting;
            if (others_mpu !== own_mpu) {
                report.logError(`${settingName} setting differs from ${core}. All cores should have either Automate ${settingName} ON or OFF`, inst, "mpu_setting");
            }
        }
    }
}

exports = {
    defaultInstanceName: "MEMORY_REGION_CONFIGURATION",
    displayName: "Memory Region",
    config: config,
    maxInstances: 1,
    longDescription: "",
    moduleInstances: addModuleInstances,
    validate: validate,
    templates: {
        "/memory_configurator/templates/linker_mem_map_base.xdt": {
            linker_mem_map: "/memory_configurator/.meta/templates/linker_mem_map_sub.xdt",
        },
        "/memory_configurator/templates/app_mem_map_base.xdt": {
            app_mem_map: "/memory_configurator/.meta/templates/app_mem_map_sub.xdt",
        },
        "/memory_configurator/templates/linux_rtos_memory_map_base.xdt": {
            linux_rtos_memory_map: "/memory_configurator/.meta/templates/linux_rtos_memory_map_sub.xdt",
        },
    },
    views: [{
        name: "memory_configurator/Summary",
        displayName: "Memory Summary",
        viewType: "markdown",
        icon: "table-large",
        ignoreErrors: true,
    }],
};