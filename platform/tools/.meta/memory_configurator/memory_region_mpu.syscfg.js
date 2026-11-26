let common = system.getScript("/common");
const { getMemoryLayout } = system.getScript("/memory_configurator/memoryLayoutSolver");
const physicalLayout = system.getScript("/memory_configurator/physicalLayout.json")[system.deviceData.device];
const memoryRegs = system.getScript("/memory_configurator/helper");

let g_memory_region_types = [];
let selfCoreName = common.getSelfSysCfgCoreName();
let isC7XCore = selfCoreName && selfCoreName.includes("c71ss");

/*
 * Builds the list of memory type options that are visible to the current core
 * in the memory_region_mpu module's "Type" dropdown.
 *
 * Iterates over the physicalLayout JSON object (keyed by memory type strings
 * such as "DDR_ALL" or "OCMC_R5F") and includes a type when its core-suffix
 * matches the current core name, when the suffix is "ALL" (accessible to every
 * core), or when the type key contains no underscore at all (legacy entries
 * treated as universally accessible). Display names strip the trailing "_CORE"
 * portion so the GUI shows just "DDR" instead of "DDR_ALL".
 *
 * Also updates the module-level g_memory_region_types cache, which is read by
 * the isShared configurable's initial hidden state.
 *
 * Returns: object[] — array of {name, displayName} entries for the SysConfig
 *          options list. At least one entry is always present because the
 *          physicalLayout always includes at least one universally accessible
 *          memory type.
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
 *
 * When isShared is set to true the shared_cores field is made visible so the
 * user can pick which other cores share ownership of this region. When
 * isShared is cleared the field is hidden and shared_cores is reset to an
 * empty array to avoid stale selections persisting invisibly.
 *
 * Parameters:
 *   inst - SysConfig module instance. Reads inst.isShared; writes
 *          inst.shared_cores when hiding.
 *   ui   - SysConfig UI state object. Reads/writes ui.shared_cores.hidden.
 *
 * Returns: void.
 * Side effects: mutates inst.shared_cores and ui.shared_cores.hidden.
 */
function enableShared_core(inst, ui) {
    if (inst.isShared)
        ui.shared_cores.hidden = false;
    else {
        ui.shared_cores.hidden = true;
        inst.shared_cores = [];
    }
}

/*
 * Rounds a byte count up to the smallest power-of-2 value that satisfies both
 * the MPU hardware minimum size constraint (32 bytes) and is >= the requested
 * size.
 *
 * The ARMv7-R MPU requires region sizes to be powers of 2 and at least 32
 * bytes. This function is used by addModuleInstances() to compute the
 * requiredArgs.sizeBytes passed to the child mpu_cfg instance when spawning
 * it for "Automate MPU" mode, ensuring the MPU entry always covers at least
 * the full linker region.
 *
 * Parameters:
 *   sz - number — the desired region size in bytes (any non-negative integer).
 *
 * Returns: number — the smallest power-of-2 value that is >= max(sz, 32).
 *          Examples: mpuSizeBytes(0) → 32, mpuSizeBytes(33) → 64,
 *          mpuSizeBytes(32768) → 32768.
 * Side effects: none.
 */
function mpuSizeBytes(sz) {
    let p = 32;
    while (p < sz) p *= 2;
    return p;
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
        hidden: !isC7XCore,
        options: [
            { name: "Physical", displayName: "Physical Address" },
            { name: "Virtual",  displayName: "Virtual Address"  }
        ],
    }, {
        name: "attributes",
        displayName: "Attributes",
        default: ["R", "W", "X", "I"],
        options: [
            { name: "R", displayName: "Read"        },
            { name: "W", displayName: "Write"       },
            { name: "X", displayName: "Execute"     },
            { name: "I", displayName: "Initialize"  },
        ],
        longDescription: "R: memory can be read; W: memory can be written to; X: memory can contain executable code, I: memory can be initialized."
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
        longDescription: 'Auto-calculated start address.'
    }, {
        name: "manualStartAddress",
        displayName: "Start Address",
        hidden: true,
        default: physicalLayout[Object.keys(physicalLayout)[0]].start,
        displayFormat: "hex",
    }, {
        name: "size",
        displayName: "Region Size",
        default: 32768,
        options: [
            { name: 32,         displayName: "32 B"    },
            { name: 64,         displayName: "64 B"    },
            { name: 128,        displayName: "128 B"   },
            { name: 256,        displayName: "256 B"   },
            { name: 512,        displayName: "512 B"   },
            { name: 1024,       displayName: "1 KB"    },
            { name: 2048,       displayName: "2 KB"    },
            { name: 4096,       displayName: "4 KB"    },
            { name: 8192,       displayName: "8 KB"    },
            { name: 16384,      displayName: "16 KB"   },
            { name: 32768,      displayName: "32 KB"   },
            { name: 65536,      displayName: "64 KB"   },
            { name: 131072,     displayName: "128 KB"  },
            { name: 262144,     displayName: "256 KB"  },
            { name: 524288,     displayName: "512 KB"  },
            { name: 1048576,    displayName: "1 MB"    },
            { name: 2097152,    displayName: "2 MB"    },
            { name: 4194304,    displayName: "4 MB"    },
            { name: 8388608,    displayName: "8 MB"    },
            { name: 16777216,   displayName: "16 MB"   },
            { name: 33554432,   displayName: "32 MB"   },
            { name: 67108864,   displayName: "64 MB"   },
            { name: 134217728,  displayName: "128 MB"  },
            { name: 268435456,  displayName: "256 MB"  },
            { name: 536870912,  displayName: "512 MB"  },
            { name: 1073741824, displayName: "1 GB"    },
            { name: 2147483648, displayName: "2 GB"    },
            { name: 4294967296, displayName: "4 GB"    },
        ],
        longDescription: "MPU region size must be a power of 2. The linker will place sections within this region.",
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
    }, {
        /* Forced by region.syscfg.js (requiredArgs) from its "Automate RAT
         * Setting" toggle. When true, each region can own an optional rat_cfg
         * child alongside its mpu_cfg child. */
        name: "ratAutomated",
        default: false,
        hidden: true,
    },
];

/*
 * Validates the size and manual start address of a memory_region_mpu instance
 * against the bounds defined in physicalLayout for the selected memory type.
 *
 * Checks performed:
 *   1. size must be > 0 and <= the physical size of the selected memory type.
 *   2. When auto == false and addressType is not "Virtual", the manually
 *      entered start address plus region size must fall entirely within the
 *      physical address range of the selected memory type.
 *
 * Physical address validation is skipped for virtual address regions (C7x
 * cores that map virtual addresses above 4 GB to physical addresses via MMU).
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for each constraint violation found.
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
 * Detects address range overlaps between manually placed memory_region_mpu
 * instances within the same core context.
 *
 * Skips the check entirely for auto-placed regions (the layout solver handles
 * their non-overlap guarantee). For manually placed regions, computes [start,
 * end] for both the current instance and every other instance in the same
 * module, and reports an error on the first overlap found.
 *
 * Cross-core overlap detection is intentionally not performed here: shared DDR
 * regions are handled by the layout solver, and per-core memories (TCM, C7x
 * virtual addresses) are not comparable across cores.
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated. Reads .auto,
 *            .manualStartAddress, .autoStartAddress, and .size.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() on the first overlap found, then
 *              returns immediately (only one error per validation pass).
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
 * Validates that the $name of the current memory_region_mpu instance is
 * unique both within the current core and across all other cores that may
 * share regions with this one.
 *
 * Two passes are made:
 *   1. Intra-core: walks instance.$module.$instances for the current context
 *      and reports an error if any other instance has the same $name.
 *   2. Cross-core: calls util_function("_mpu") to enumerate every core's
 *      memory_region_mpu instances, skips the current core, and checks each
 *      remote region for the same $name. Remote shared regions must also have
 *      unique names because they appear in the same generated linker file.
 *
 * Parameters:
 *   instance - SysConfig module instance being validated. Reads .$name and
 *              .$module.$instances.
 *   report   - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for each naming conflict found.
 */
function checkSameFieldName(instance, report) {
    let regions_this_core = instance.$module.$instances;
    let regions_shared_with_this_core = memoryRegs.util_function("_mpu");

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
 * Top-level validation function for memory_region_mpu instances. Orchestrates
 * all sub-checks and enforces shared_cores selection constraints.
 *
 * Checks performed in order:
 *   1. Physical memory fit: unless the region uses a virtual address or is
 *      shared, verifies that getMemoryLayout() was able to place it (i.e. the
 *      resolved start address is not -1).
 *   2. Name uniqueness: delegates to checkSameFieldName().
 *   3. Address overlap: delegates to checkOverlap().
 *   4. Size/address bounds: delegates to valueCheck().
 *   5. Shared cores selection: if isShared is true, at least one core must be
 *      selected in shared_cores, and every selected core must appear in the
 *      device's known core list (rejects stale options from a prior device).
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls logError() for each constraint violation found via the
 *              sub-validators and directly for shared_cores issues.
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
 * Returns the moduleInstances descriptor array that spawns one mpu_cfg child
 * instance for each memory_region_mpu instance in "Automate MPU" mode.
 *
 * Only sizeBytes is passed as a requiredArg; the base address is intentionally
 * omitted here and instead derived lazily at display time via the getValue
 * function on mpu_cfg.baseAddr_mr (which reads inst.$ownedBy). This avoids
 * triggering autoStartAddress's getValue during the moduleInstances call,
 * which would access system.modules and cause a SysConfig cross-context error.
 *
 * The isAutomated flag is also set to true on the child so that mpu_cfg knows
 * it is operating in Automate MPU mode and should read its base address from
 * the owning memory_region_mpu rather than the standalone baseAddr field.
 *
 * When "Automate RAT Setting" is also enabled (inst.ratAutomated), a second
 * descriptor for the rat_cfg child is appended, so MPU and RAT automation can
 * be used together on the same region.
 *
 * Parameters:
 *   inst - SysConfig memory_region_mpu instance. Reads inst.size to compute
 *          the required MPU power-of-2 size via mpuSizeBytes(), and
 *          inst.ratAutomated for the optional RAT child.
 *
 * Returns: object[] — array containing the mpu_cfg moduleInstances descriptor
 *          (useArray: true, maxInstanceCount: 1), followed by the rat_cfg
 *          descriptor when RAT automation is enabled.
 * Side effects: none (descriptor construction only; SysConfig performs the
 *               actual child spawning).
 */
function addModuleInstances(inst) {
    return [{
        name: "mpu_config",
        displayName: "MPU Region",
        moduleName: "/memory_configurator/mpu_cfg",
        useArray: true,
        minInstanceCount: 0,
        maxInstanceCount: 1,
        collapsed: false,
        requiredArgs: {
            sizeBytes:   mpuSizeBytes(inst.size),
            isAutomated: true,
        },
    }, ...memoryRegs.getRatModuleInstances(inst)];
}

let regionModule = {
    displayName: "Memory Region",
    defaultInstanceName: "MEMORY_REGION",
    config: config,
    validate: validate,
    moduleInstances: addModuleInstances,
};

exports = regionModule;
