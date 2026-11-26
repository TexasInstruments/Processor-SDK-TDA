/*
 * Linker-region module for C7x cores in Automate MMU mode.
 * Analogous to memory_region_mpu.syscfg.js for R5F MPU regions.
 *
 * Each instance represents one linker-section placement and automatically
 * spawns a child mmu_cfg instance that carries the MMU hardware settings
 * (virtual/physical address, size, attrIndx).
 */

let common = system.getScript("/common");
const { getMemoryLayout } = system.getScript("/memory_configurator/memoryLayoutSolver");
const physicalLayout = system.getScript("/memory_configurator/physicalLayout.json")[system.deviceData.device];
const memoryRegs = system.getScript("/memory_configurator/helper");

let g_memory_region_types = [];
let selfCoreName = common.getSelfSysCfgCoreName();

/*
 * Builds the list of memory type options visible to the current core in the
 * memory_region_mmu module's "Type" dropdown.
 *
 * Filters physicalLayout to entries whose core suffix matches the current core,
 * is "ALL" (accessible by every core), or has no underscore (legacy format).
 * Also updates the module-level g_memory_region_types cache used by the
 * isShared configurable's initial hidden state.
 *
 * Returns: object[] — array of {name, displayName} entries.
 * Side effects: updates module-level variable g_memory_region_types.
 */
function memoryRegions() {
    let memory_region_types = [];
    let selfCoreName = common.getSelfSysCfgCoreName();

    for (let key in physicalLayout) {
        let core = key.slice(key.lastIndexOf('_') + 1);

        if (core == "ALL" || selfCoreName.includes(core.toLowerCase()) || (key.indexOf('_') == -1)) {
            let displayName = "";
            if (key.indexOf('_') == -1) {
                displayName = key;
            } else {
                displayName = key.slice(0, key.lastIndexOf('_'));
            }
            memory_region_types.push({ name: key.toString(), displayName: displayName });
        }
    }

    g_memory_region_types = memory_region_types;
    return memory_region_types;
}

/*
 * onChange handler for the isShared toggle that shows or hides the
 * shared_cores multi-select field.
 */
function enableShared_core(inst, ui) {
    if (inst.isShared)
        ui.shared_cores.hidden = false;
    else {
        ui.shared_cores.hidden = true;
        inst.shared_cores = [];
    }
}

let config = [
    {
        name: "$name",
        hidden: false,
        isCIdentifier: false,
    },
    {
        name: "type",
        displayName: "Type",
        default: memoryRegions()[0]?.name ?? "",
        options: memoryRegions(),
        onChange: (inst) => {
            inst.$name = (inst.type.concat("_x")).toUpperCase();
            inst.manualStartAddress = physicalLayout[inst.type].start;
            inst.isShared = false;
            inst.$uiState.shared_cores.hidden = true;
            inst.shared_cores = [];
            if (physicalLayout[inst.type].access == "individual") {
                inst.$uiState.isShared.hidden = true;
            } else {
                inst.$uiState.isShared.hidden = false;
            }
        },
        longDescription: 'Choose CUSTOM if the memory address lies outside of the remaining types.'
    }, {
        name: "addressType",
        displayName: "Address Type",
        default: "Physical",
        hidden: false,
        options: [
            { name: "Physical", displayName: "Physical Address" },
            { name: "Virtual",  displayName: "Virtual Address"  }
        ],
        longDescription: 'Select Physical Address for automatic validation, or Virtual Address to bypass physical memory fit check. When using Virtual Address, configure corresponding MMU settings.'
    }, {
        name: "attributes",
        displayName: "Attributes",
        default: ["R", "W", "X", "I"],
        options: [
            { name: "R", displayName: "Read"       },
            { name: "W", displayName: "Write"      },
            { name: "X", displayName: "Execute"    },
            { name: "I", displayName: "Initialize" },
        ],
        longDescription: "R: memory can be read; W: memory can be written to; X: memory can contain executable code; I: memory can be initialized."
    }, {
        name: "auto",
        displayName: "Calculate Start Address",
        default: true,
        onChange: (inst) => {
            inst.$uiState.autoStartAddress.hidden  = !inst.auto;
            inst.$uiState.manualStartAddress.hidden = inst.auto;
        },
        longDescription: 'Check this if placement of the region does not matter. The region will be placed in the smallest hole that fits it.'
    }, {
        name: "autoStartAddress",
        displayName: "Start Address",
        default: 0x0,
        displayFormat: "hex",
        getValue: (inst) => {
            if (!inst.auto) return 0x0;
            let addr = getMemoryLayout()?.[system.context]?.[inst.$name] ?? 0;
            return (addr < 0) ? 0x0 : addr;
        },
        longDescription: 'Auto-calculated start address. Uncheck the auto option to enter a manual start address.'
    }, {
        name: "manualStartAddress",
        displayName: "Start Address",
        hidden: true,
        default: physicalLayout[Object.keys(physicalLayout)[0]].start,
        displayFormat: "hex",
    }, {
        name: "size",
        displayName: "Region Size (bytes)",
        default: 0x20000000,
        displayFormat: "hex",
        description: "In hexa-decimal",
        longDescription: "C7x MMU does not require power-of-2 sizes. The child mmu_cfg entry will inherit this size.",
    }, {
        name: "endAddress",
        displayName: "End Address",
        default: 0x0,
        displayFormat: "hex",
        getValue: (inst) => {
            let start = inst.auto ? inst.autoStartAddress : inst.manualStartAddress;
            return start + inst.size - 0x1;
        }
    }, {
        name: "alignment",
        displayName: "Alignment",
        default: 8,
        hidden: true,
        displayFormat: "dec",
    }, {
        name: "last_sym",
        displayName: "Last Symbol",
        default: "",
        longDescription: "Optionally specifies a symbol for the last allocated byte in this region."
    }, {
        name: "isShared",
        displayName: "Shared",
        default: false,
        hidden: !(g_memory_region_types[0].name.includes("_ALL") || (physicalLayout[g_memory_region_types[0].name].access === "all")),
        onChange: enableShared_core,
        longDescription: 'Check if this region is shared among multiple cores.'
    }, {
        name: "shared_cores",
        displayName: "Share With Cores",
        default: [],
        hidden: true,
        options: (inst) => { return memoryRegs.coreList(inst); },
    },
];

/*
 * Validates the size and manual start address of a memory_region_mmu instance
 * against the bounds defined in physicalLayout for the selected memory type.
 *
 * C7x MMU does not require power-of-2 sizes, so only bounds are checked.
 */
function valueCheck(inst, report) {
    if (inst.size < 0) {
        report.logError("Size can't be less than 0", inst, "size");
    }

    let mem_params_size = physicalLayout[inst.type].size;

    if (inst.size > mem_params_size) {
        report.logError(`Size can't be greater than ${mem_params_size} B`, inst, "size");
    }

    if (inst.size == 0) {
        report.logError(`Size can't be 0`, inst, "size");
    }

    if (!inst.auto) {
        let isPhysicalAddr = inst.addressType !== "Virtual";
        if (isPhysicalAddr) {
            let typeStart = physicalLayout[inst.type].start;
            let typeEnd   = typeStart + physicalLayout[inst.type].size - 1;
            let regionEnd = inst.manualStartAddress + inst.size - 1;

            if (inst.manualStartAddress < typeStart || regionEnd > typeEnd) {
                report.logError(
                    `Physical address must be within the range of selected memory type ` +
                    `(0x${typeStart.toString(16).toUpperCase()} - 0x${typeEnd.toString(16).toUpperCase()})`,
                    inst, "manualStartAddress"
                );
            }
        }
    }
}

/*
 * Detects address range overlaps between manually placed memory_region_mmu
 * instances within the same core context.
 *
 * Auto-placed instances are skipped because the layout solver guarantees
 * non-overlap. Cross-core overlap is not checked here — shared DDR regions
 * are handled by the layout solver.
 */
function checkOverlap(inst, report) {
    if (inst.auto) return;

    let myStart = inst.manualStartAddress;
    let myEnd   = myStart + inst.size - 1;

    for (let other of inst.$module.$instances) {
        if (other === inst) continue;

        let otherStart = other.auto ? other.autoStartAddress : other.manualStartAddress;
        if (otherStart < 0) continue;
        let otherEnd = otherStart + other.size - 1;

        if (myStart <= otherEnd && otherStart <= myEnd) {
            report.logError(
                `Address range overlaps with region "${other.$name}" ` +
                `(0x${otherStart.toString(16).toUpperCase()} - 0x${otherEnd.toString(16).toUpperCase()})`,
                inst, "manualStartAddress"
            );
            return;
        }
    }
}

/*
 * Validates that the $name of the current memory_region_mmu instance is unique
 * both within this core and across all cores that share regions with this core.
 *
 * Calls util_function("_mmu") to enumerate remote cores' memory_region_mmu
 * instances for cross-core name conflict detection.
 */
function checkSameFieldName(instance, report) {
    let regions_this_core = instance.$module.$instances;
    let regions_shared_with_this_core = memoryRegs.util_function("_mmu");

    for (let i = 0; i < regions_this_core.length; i++) {
        if (instance.$name === regions_this_core[i].$name && instance !== regions_this_core[i]) {
            report.logError(`Same name cannot be used`, instance, "$name");
        }
    }

    let selfCoreName = common.getSelfSysCfgCoreName();
    _.each(regions_shared_with_this_core, (remote_core) => {
        if (remote_core.core === selfCoreName) return;
        _.each(remote_core.instances, (remote_region) => {
            if (instance.$name === remote_region.$name) {
                report.logError(`Region with same name exists in core "${remote_core.core}"`, instance, "$name");
            }
        });
    });
}

/*
 * Top-level validation function for memory_region_mmu instances. Orchestrates
 * all sub-checks and enforces shared_cores selection constraints.
 */
function validate(inst, report) {
    let isVirtualAddr = inst.addressType === "Virtual";

    if (!isVirtualAddr && !inst.isShared && -1 === getMemoryLayout()[system.context][inst.$name]) {
        report.logError(
            "This region does not fit in the physical memory provided",
            inst, inst.auto ? "autoStartAddress" : "manualStartAddress"
        );
    }

    checkSameFieldName(inst, report);
    checkOverlap(inst, report);
    valueCheck(inst, report);

    if (inst.isShared && inst.shared_cores.length == 0) {
        report.logError(`Select at least one core.`, inst, "shared_cores");
    }

    memoryRegs.validateSharedCores(inst, report);
}

/*
 * Returns the moduleInstances descriptor that spawns one mmu_cfg child per
 * memory_region_mmu instance in Automate MMU mode.
 *
 * sizeBytes is passed as a requiredArg so the child inherits the linker region
 * size and changeDisplay fires at creation time. The base addresses (vAddr_mr,
 * pAddr_mr) are derived lazily via getValue on the child to avoid
 * autoStartAddress evaluation across contexts during moduleInstances().
 *
 * isAutomated: true tells the child mmu_cfg that it is in Automate MMU mode
 * and should read its addresses from the owning memory_region_mmu.
 */
function addModuleInstances(inst) {
    return [{
        name: "mmu_config",
        displayName: "MMU Region",
        moduleName: "/memory_configurator/mmu_cfg",
        useArray: true,
        minInstanceCount: 0,
        maxInstanceCount: 1,
        collapsed: false,
        requiredArgs: {
            size:        inst.size,
            isAutomated: true,
        },
    }];
}

let regionModule = {
    displayName: "Memory Region",
    defaultInstanceName: "MEMORY_REGION",
    config: config,
    validate: validate,
    moduleInstances: addModuleInstances,
};

exports = regionModule;
