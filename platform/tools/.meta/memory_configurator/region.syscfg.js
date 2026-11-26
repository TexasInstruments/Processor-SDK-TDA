let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();

let isC7XCore = selfCoreName && selfCoreName.includes("c71ss");

/* RAT is available on R5F and C66x cores (same condition as components.syscfg.js) */
let isRatCore = selfCoreName && (selfCoreName.includes("r5fss") || selfCoreName.includes("c66"));
let settingLabel = isC7XCore ? "Automate MMU Setting" : "Automate MPU Setting";
let settingDescription = isC7XCore ?
    "Checking this would save the user from manually configuring MMU for regions. Uncheck this to get flexibility in configuring MMU." :
    "Checking this would save the user from manually configuring MPU for regions. Uncheck this to get flexibility in configuring MPU.";

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
            "MPU setting. Uncheck this to configure RAT entries manually in the RAT Configurator.",
    },
];

/*
 * Returns the moduleInstances descriptor array that selects which memory
 * region sub-module to attach based on the mpu_setting toggle and the current
 * core type.
 *
 * Three possible sub-modules:
 *   - mpu_setting == false → "memory_configurator/memory_region" (manual
 *     linker regions, no MPU automation)
 *   - mpu_setting == true, C7x core (name includes "c71ss") →
 *     "memory_configurator/memory_region_mmu" (automated MMU regions)
 *   - mpu_setting == true, R5F or other core →
 *     "memory_configurator/memory_region_mpu" (automated MPU regions)
 *
 * The module type switch must happen at the region level (not per-region-
 * instance) because SysConfig requires all instances of a parent to use the
 * same sub-module type.
 *
 * RAT automation does not change the sub-module type. On R5F/C66x cores the
 * rat_setting toggle is forwarded to the selected sub-module as the hidden
 * ratAutomated flag (requiredArgs), which makes each region able to own an
 * optional rat_cfg child. This keeps RAT independent of MPU: the same flag
 * works with memory_region and memory_region_mpu.
 *
 * Parameters:
 *   inst - SysConfig region module instance. Reads inst.mpu_setting and
 *          inst.rat_setting. The module-level isC7XCore / isRatCore are also
 *          read to branch on core type.
 *
 * Returns: object[] — array containing one moduleInstances descriptor
 *          (useArray: true, minInstanceCount: 0) for the selected sub-module.
 * Side effects: none.
 */
function addModuleInstances(inst) {
    let modInstances = new Array();
    let module_name = "";

    if (inst.mpu_setting) {
        if (isC7XCore) {
            module_name = "memory_configurator/memory_region_mmu";
        } else {
            module_name = "memory_configurator/memory_region_mpu";
        }
    } else {
        module_name = "memory_configurator/memory_region";
    }

    let regionDescriptor = {
        name: "memory_region",
        displayName: "Region",
        moduleName: module_name,
        useArray: true,
        minInstanceCount: 0,
        collapsed: false,
    };

    /* ratAutomated only exists on memory_region / memory_region_mpu, so it
     * must not be forced on cores that use memory_region_mmu (C7x). */
    if (isRatCore) {
        regionDescriptor.requiredArgs = { ratAutomated: inst.rat_setting };
    }

    modInstances.push(regionDescriptor);

    return modInstances;
}

/*
 * Validates that all cores agree on the same mpu_setting value (all ON or all
 * OFF). Because mpu_setting determines which sub-module type is spawned,
 * mixing modes across cores would cause inconsistent linker file generation
 * and is therefore treated as a hard error.
 *
 * Iterates over all cores from getSysCfgCoreNames(), skips the current core,
 * looks up each remote core's region module, and compares its mpu_setting to
 * the current instance's value. Errors are reported on the mpu_setting field
 * of the current instance.
 *
 * The error message names both the conflicting remote core and the setting
 * name (MPU or MMU, depending on whether the current core is a C7x core) to
 * provide actionable context to the user.
 *
 * Parameters:
 *   inst   - SysConfig region module instance. Reads inst.mpu_setting.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for each core whose mpu_setting
 *              differs from the current instance.
 */
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
            linker_mem_map: "/memory_configurator/templates/linker_mem_map_sub.xdt",
        },
        "/memory_configurator/templates/app_mem_map_base.xdt": {
            app_mem_map: "/memory_configurator/templates/app_mem_map_sub.xdt",
        },
        "/memory_configurator/templates/linux_rtos_memory_map_base.xdt": {
            linux_rtos_memory_map: "/memory_configurator/templates/linux_rtos_memory_map_sub.xdt",
        },
        "/memory_configurator/templates/mmu_cfg_base.xdt": {
            mmu_cfg: "/memory_configurator/templates/mmu_cfg_sub.xdt",
        },
        "/memory_configurator/templates/mpu_cfg_freertos_base.xdt": {
            mpu_cfg_freertos: "/memory_configurator/templates/mpu_cfg_freertos_sub.xdt",
        },
        "/memory_configurator/templates/mpu_cfg_safertos_base.xdt": {
            mpu_cfg_safertos: "/memory_configurator/templates/mpu_cfg_safertos_sub.xdt",
        },
        "/memory_configurator/templates/rat_cfg_base.xdt": {
            rat_cfg: "/memory_configurator/templates/rat_cfg_sub.xdt",
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
