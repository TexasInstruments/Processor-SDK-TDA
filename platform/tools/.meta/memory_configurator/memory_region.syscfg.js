let common = system.getScript("/common");
let general_module = system.modules['/memory_configurator/general'];
const { getMemoryLayout } = system.getScript("/memory_configurator/memoryLayoutSolver");
const physicalLayout = system.getScript("/memory_configurator/physicalLayout.json")[system.deviceData.device];
const memoryRegs = system.getScript("/memory_configurator/helper");

let g_memory_region_types = [];
let selfCoreName = common.getSelfSysCfgCoreName();
let isC7XCore = selfCoreName && selfCoreName.includes("c71ss");

/*
 * Builds the list of memory type options visible to the current core in the
 * memory_region module's "Type" dropdown.
 *
 * Iterates over the physicalLayout JSON (keyed by memory type strings) and
 * includes an entry when the type's core suffix matches the current core name,
 * the suffix is "ALL" (accessible by every core), or the key has no underscore
 * (legacy format treated as universally accessible). Display names have the
 * trailing "_CORE" portion stripped for readability in the GUI.
 *
 * Also updates the module-level g_memory_region_types cache, which drives the
 * initial hidden state of the isShared configurable.
 *
 * Returns: object[] — array of {name, displayName} entries. Always non-empty
 *          because physicalLayout always contains at least one _ALL type.
 * Side effects: updates module-level variable g_memory_region_types.
 */
function memoryRegions(){

    let memory_region_types = [];
    let selfCoreName = common.getSelfSysCfgCoreName();

    for(let key in physicalLayout){
        // value = physicalLayout[key];
        core = key.slice(key.lastIndexOf('_')+1);

        if( core == "ALL" || selfCoreName.includes(core.toLowerCase()) || (key.indexOf('_')==-1) ) {
            let displayName=""
            if (key.indexOf('_')==-1){
                displayName = key;
            }
            else {
                displayName = key.slice(0,key.lastIndexOf('_'));
            }
            memory_region_types.push({name: key.toString(), displayName: displayName})
        }
    }

    g_memory_region_types = memory_region_types
    return memory_region_types;
}


/*
 * onChange handler for the isShared toggle in the memory_region module.
 * Shows or hides the shared_cores multi-select field based on the toggle state.
 *
 * When isShared becomes true, shared_cores is revealed so the user can select
 * which other cores share this region. When isShared is cleared, shared_cores
 * is hidden and reset to [] so no stale selections remain.
 *
 * Parameters:
 *   inst - SysConfig module instance. Reads inst.isShared; writes
 *          inst.shared_cores when hiding.
 *   ui   - SysConfig UI state object. Writes ui.shared_cores.hidden.
 *
 * Returns: void.
 * Side effects: mutates inst.shared_cores and ui.shared_cores.hidden.
 */
function enableShared_core(inst, ui) {

    if(inst.isShared) {
        ui.shared_cores.hidden = false;
        ui.allowOverlap.hidden = false;
    }
    else{
        ui.shared_cores.hidden = true;
        inst.shared_cores = []
        ui.allowOverlap.hidden = true;
        inst.allowOverlap = false;
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
    default: memoryRegions()[0]?.name??"",
    options: memoryRegions(),
    onChange: (inst) => {
        inst.$name = (inst.type.concat("_x")).toUpperCase()
        inst.manualStartAddress = physicalLayout[inst.type].start
        inst.isShared = false;
        inst.$uiState.shared_cores.hidden = true;
        inst.shared_cores = [];
        inst.$uiState.allowOverlap.hidden = true;
        inst.allowOverlap = false;
        if(physicalLayout[inst.type].access == "individual"){
            inst.$uiState.isShared.hidden = true;
            //inst.shared_cores = [];
        }
        else {
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
        {name: "Physical", displayName: "Physical Address"},
        {name: "Virtual", displayName: "Virtual Address"}
    ],
    longDescription: 'For C7X cores: Select Physical Address for automatic validation, or Virtual Address to bypass physical memory fit check. Note: When using Virtual Address, ensure to configure corresponding MMU settings.'
}, {
    name: "attributes",
    displayName: "Attributes",
    default: ["R", "W", "X", "I"],
    options: [{name:"R", displayName:"Read"}, {name:"W", displayName:"Write"}, {name:"X", displayName:"Execute"}, {name:"I", displayName:"Initialize"}],
    longDescription:"R: memory can be read; W: memory can be written to; X: memory can contain executable code, I: memory can be initialized."
},{
    name: "auto",
    displayName: "Calculate Start Address",
    default: true,
    onChange: (inst) => {
        inst.$uiState.autoStartAddress.hidden = !inst.auto;
        inst.$uiState.manualStartAddress.hidden = inst.auto;
    },
    longDescription:'Check this if placement of the region does not matter. The region will be placed in the smallest hole that fits it.'
}, {
    name: "autoStartAddress",
    displayName: "Start Address",
    default: 0x0,
    displayFormat: "hex",
    getValue: (inst) => {
        if(!inst.auto) return 0x0
        let addr = getMemoryLayout()?.[system.context]?.[inst.$name] ?? 0;
        return (addr < 0) ? 0x0 : addr;
    },
    longDescription:'This is auto calculated start address. User may uncheck the auto option if manual start address needs to be put.'
}, {
    name: "manualStartAddress",
    displayName: "Start Address",
    hidden: true,
    default:  physicalLayout[Object.keys(physicalLayout)[0]].start,
    displayFormat: "hex",
 },
{
    name: "size",
    displayName: "Region Size (bytes)",
    default: 0x1,
    displayFormat: "hex",
    description: "Accepts hex format",
    description: 'In hexa-decimal'
},
{
    name: "endAddress",
    displayName: "End Address",
    default: 0x0,
    displayFormat: "hex",
    getValue: (inst) => {
        if(inst.auto) {
            return (inst.autoStartAddress + inst.size - 0x1 )
        }
        else {
            return (inst.manualStartAddress + inst.size - 0x1 )
        }
    }
},
{
    name: "alignment",
    displayName: "Alignment",
    description: "",
    longDescription: "",
    default: 8,
    hidden: true,
    displayFormat: "dec",
},
{
    name: "last_sym",
    displayName: "Last Symbol",
    default: "",
    longDescription:"Optionally specifies a Symbol that can be used at run-time to find the address of the last allocated byte in the memory \
    range"
},{
    name: "isShared",
    displayName: "Shared",
    default: false,
    hidden: !(g_memory_region_types[0].name.includes("_ALL") || ( physicalLayout[g_memory_region_types[0].name].access === "all")) ,
    onChange: enableShared_core,
    longDescription: 'Check if this region has to be shared among multiple cores. Once done, no need to select the same in other cores.'
 }, {
    name: "shared_cores",
    displayName: "Share With Cores",
    description: "",
    longDescription: "",
    default: [],
    hidden: true,
    options: (inst) => {return memoryRegs.coreList(inst)},
}, {
    name: "allowOverlap",
    displayName: "Allow Overlap",
    default: false,
    hidden: true,
    longDescription: 'Only for shared regions with a manual start address. Marks this region as an aggregate view over other regions ' +
        '(e.g. one region covering several per-host slices). It is reported in app_mem_map.h and the Linux dtsi, is exempt ' +
        'from overlap checks and address placement, and is NOT emitted in the linker MEMORY block of any core.',
}, {
    /* Forced by region.syscfg.js (requiredArgs) from its "Automate RAT Setting"
     * toggle. When true, each region can own an optional rat_cfg child. */
    name: "ratAutomated",
    default: false,
    hidden: true,
},
]

/*
 * Validates the size and manual start address of a memory_region instance
 * against the bounds defined in physicalLayout for the selected memory type.
 *
 * Checks performed:
 *   1. size must be > 0 (negative or zero sizes are invalid).
 *   2. size must not exceed the total physical size of the selected memory type.
 *   3. When auto == false and addressType is not "Virtual", the manually
 *      entered start address plus region size must lie entirely within the
 *      physical address range of the selected memory type.
 *
 * Physical address bounds validation is skipped when the addressType is
 * "Virtual" (used for C7x cores that access virtual addresses above 4 GB).
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated. Reads .size, .auto,
 *            .addressType, .manualStartAddress, and .type.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for each constraint violation found.
 */
function valueCheck(inst, report){

    if(inst.size < 0) {
        report.logError("Size can't be less than 0", inst, "size")
    }

    let mem_params_size = physicalLayout[inst.type].size

    if(inst.size > mem_params_size) {
        report.logError(`Size can't be greater than ${mem_params_size} B`, inst, "size")
    }

    if(inst.size == 0) {
        report.logError(`Size can't be 0`, inst, "size")
    }

    if(!inst.auto){
        let isPhysicalAddr = inst.addressType !== "Virtual";
        if(isPhysicalAddr){
            let typeStart = physicalLayout[inst.type].start;
            let typeEnd = typeStart + physicalLayout[inst.type].size - 1;
            let regionEnd = inst.manualStartAddress + inst.size - 1;

            if(inst.manualStartAddress < typeStart || regionEnd > typeEnd){
                report.logError(`Physical address must be within the range of selected memory type (0x${typeStart.toString(16).toUpperCase()} - 0x${typeEnd.toString(16).toUpperCase()})`, inst, "manualStartAddress")
            }
        }
    }

}


/*
 * Detects address range overlaps between manually placed memory_region
 * instances within the same core context.
 *
 * Auto-placed instances are skipped because their addresses are resolved by
 * the layout solver, which guarantees non-overlap for auto regions. For manual
 * regions, the function computes [start, end] spans and checks every other
 * instance in the same module for intersection. Returns on the first overlap
 * found to avoid reporting the same conflict twice.
 *
 * Cross-core overlap detection is intentionally not done here: shared DDR
 * regions are handled by the layout solver, and per-core memories (TCM,
 * C7x virtual addresses) are not comparable across core boundaries.
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated. Reads .auto,
 *            .manualStartAddress, .autoStartAddress, and .size.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() on the first overlap found, then
 *              returns immediately.
 */
function checkOverlap(inst, report) {
    if (inst.auto || inst.allowOverlap) return;

    let myStart = inst.manualStartAddress;
    let myEnd   = myStart + inst.size - 1;

    // Only check within the same core — cross-core overlap is either handled by the
    // layout solver (shared DDR) or invalid to compare (per-core TCM, C7x virtual addresses)
    for (let other of inst.$module.$instances) {
        if (other === inst || other.allowOverlap) continue;

        let otherStart = other.auto ? other.autoStartAddress : other.manualStartAddress;
        if (otherStart < 0) continue;
        let otherEnd = otherStart + other.size - 1;

        if (myStart <= otherEnd && otherStart <= myEnd) {
            report.logError(
                `Address range overlaps with region "${other.$name}" (0x${otherStart.toString(16).toUpperCase()} - 0x${otherEnd.toString(16).toUpperCase()})`,
                inst, "manualStartAddress"
            );
            return;
        }
    }
}


/*
 * Validates that the $name of the current memory_region instance is unique
 * within the current core context and across all cores that share regions
 * with this core.
 *
 * Two passes:
 *   1. Intra-core: walks instance.$module.$instances to check for any other
 *      instance with the same $name.
 *   2. Cross-core: calls util_function("") to get all memory_region instances
 *      from every core, and reports an error if any remote region shares the
 *      same $name. This is necessary because shared region names must be
 *      unique across cores to avoid symbol collisions in generated linker
 *      files.
 *
 * Parameters:
 *   instance - SysConfig module instance being validated.
 *   report   - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls report.logError() for each naming conflict found.
 */
function checkSameFieldName(instance, report)
{
    let regions_this_core = instance.$module.$instances;
    let regions_shared_with_this_core = memoryRegs.util_function("")

    for (let i = 0; i < regions_this_core.length; i++) {
        if (instance.$name === regions_this_core[i].$name &&
            instance !== regions_this_core[i]) {
            report.logError(`Same name cannot be used`, instance, "$name");
        }
    }

    _.each(regions_shared_with_this_core, (remote_region) => {
            if(instance.$name === remote_region.$name) {
                report.logError(`Region with same name being shared by other core with this one`, instance, "$name");
            }
    })
}


/*
 * Top-level validation function for memory_region instances. Orchestrates all
 * sub-checks and enforces shared_cores selection constraints.
 *
 * Checks performed in order:
 *   1. Physical memory fit: unless addressType is "Virtual" or isShared is
 *      true, verifies that getMemoryLayout() placed the region successfully
 *      (resolved start address != -1).
 *   2. Name uniqueness: delegates to checkSameFieldName().
 *   3. Address overlap: delegates to checkOverlap().
 *   4. Size/address bounds: delegates to valueCheck().
 *   5. Shared cores selection: if isShared is true, at least one core must
 *      be selected in shared_cores and every selected entry must be a valid
 *      core for the current device. A device-specific fallback list is used
 *      when getSysCfgCoreNames() returns empty (early GUI load).
 *
 * Parameters:
 *   inst   - SysConfig module instance being validated.
 *   report - SysConfig report object used to call logError.
 *
 * Returns: void.
 * Side effects: calls logError() for each constraint violation found via the
 *              sub-validators and directly for shared_cores issues.
 */
function validate (inst, report) {
    let isVirtualAddr = inst.addressType === "Virtual";

    if (!isVirtualAddr && !inst.isShared && -1 === getMemoryLayout()[system.context][inst.$name]) {
        report.logError("This region does not fit in the physical memory provided", inst, inst.auto ? "autoStartAddress" : "manualStartAddress")
    }

    checkSameFieldName(inst, report)
    checkOverlap(inst, report)
    valueCheck(inst, report)

    if (inst.isShared && inst.shared_cores.length == 0) {
        report.logError(`Select at least one core.`, inst, "shared_cores")
    }

    if (inst.allowOverlap && !inst.isShared) {
        report.logError(`Allow Overlap is only supported on shared regions.`, inst, "allowOverlap")
    }
    if (inst.allowOverlap && inst.auto) {
        report.logError(`Allow Overlap requires a manual start address.`, inst, "auto")
    }

    memoryRegs.validateSharedCores(inst, report)
}

/*
 * Debug helper that returns the string representation of the instance's
 * autoStartAddress field.
 *
 * This function appears to be a diagnostic utility retained from early
 * development. It calls the global toString() (not String()) on the numeric
 * autoStartAddress value. Note: toString() without a receiver in strict mode
 * will coerce to "[object Object]" rather than the numeric string — the
 * useful form would be String(inst.autoStartAddress) or
 * inst.autoStartAddress.toString(). Use with caution.
 *
 * Parameters:
 *   inst - SysConfig module instance. Reads inst.autoStartAddress.
 *
 * Returns: string — string form of autoStartAddress (behaviour depends on
 *          the JavaScript environment's global toString).
 * Side effects: none.
 */
function func(inst) {
    return toString(inst.autoStartAddress)
}

let regionModule = {
    displayName: "Memory Region",
    defaultInstanceName: "MEMORY_REGION",
    config : config,
	validate: validate,
    moduleInstances: memoryRegs.getRatModuleInstances,
};

exports = regionModule