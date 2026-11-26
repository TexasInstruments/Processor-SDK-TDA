let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();
const device = system.deviceData.device;

/*
 * Returns the module path of the SoC-specific default MMU configuration script.
 * Provides getMmuRegionCount() and populate_mmu_regions() used by the
 * "Add Default MMU Config" button.
 *
 * product.json puts every SoC on the include path at once, so the script name
 * is suffixed with the SoC (default_mmu_config_<soc>). A bare shared name would
 * always resolve to the first SoC listed, regardless of the active device.
 *
 * Returns the script path, or null when the active device is not a recognised SoC.
 */
function getSocScript() {
    let soc = common.getSocName();
    return soc ? "/mem_map_default_cfg/default_mmu_config_" + soc : null;
}

/* -------------------------------------------------------------------------
 * attrIndx options — map directly to Mmu_AttrIndx_MAIR<n> CSL enums
 * -------------------------------------------------------------------------
 * MAIR attribute byte encoding (see ti/csl/arch/c7x/src/Mmu.c):
 *   Device memory: special encodings for peripheral access ordering/gathering
 *   Normal memory: [7:4]=outer cache policy, [3:0]=inner cache policy
 *
 * CSL default values:
 *   MAIR0 = 0x00 (Device-nGnRnE)
 *   MAIR1 = 0x04 (Device-nGnRE)
 *   MAIR2 = 0x08 (Device-nGRE)
 *   MAIR3 = 0x0C (Device-GRE)
 *   MAIR4 = 0x44 (Normal non-cacheable inner/outer)
 *   MAIR5 = 0x4F (Normal outer non-cacheable, inner write-back)
 *   MAIR6 = 0xBB (Normal write-through inner/outer)
 *   MAIR7 = 0x7D (Normal write-back outer, write-through inner)
 * ------------------------------------------------------------------------- */
const ATTR_INDX_OPTIONS = [
    { name: "Mmu_AttrIndx_MAIR0", displayName: "Attr Index 0: Non-gathering, non-reordering and no early write acknowledgement" },
    { name: "Mmu_AttrIndx_MAIR1", displayName: "Attr Index 1: Non-gathering, non-reordering and early write acknowledgement" },
    { name: "Mmu_AttrIndx_MAIR2", displayName: "Attr Index 2: Non-gathering, reordering and early write acknowledgement" },
    { name: "Mmu_AttrIndx_MAIR3", displayName: "Attr Index 3: Gathering, reordering and early write acknowledgement" },
    { name: "Mmu_AttrIndx_MAIR4", displayName: "Attr Index 4: Normal inner & outer non-cacheable" },
    { name: "Mmu_AttrIndx_MAIR5", displayName: "Attr Index 5: Normal outer non-cacheable, inner write-back cacheable non-transient memory" },
    { name: "Mmu_AttrIndx_MAIR6", displayName: "Attr Index 6: Normal outer & inner write-through cacheable non-transient memory" },
    { name: "Mmu_AttrIndx_MAIR7", displayName: "Attr Index 7: Normal outer and inner write-back cacheable non-transient memory" },
];

/*
 * Returns true when this mmu_cfg instance was created by a memory_region_mmu
 * parent (Automate MMU mode), false when it is a standalone region added
 * directly by the user.
 *
 * Reads inst.isAutomated, set to true via requiredArgs when memory_region_mmu's
 * addModuleInstances() spawns the child. Checked directly rather than via
 * $ownedBy to avoid cross-context SysConfig errors in onChange/getValue.
 */
function isOwnedByMemRegion(inst) {
    return inst.isAutomated === true;
}

/*
 * Reads the resolved start address from the owning memory_region_mmu instance.
 *
 * Used as the getValue handler for vAddr_mr and pAddr_mr in Automate MMU mode.
 * Returns 0x0 for standalone instances. Safe to call from getValue (display
 * time) but must NOT be called during moduleInstances() to avoid cross-context
 * errors.
 */
function readFromOwner(inst) {
    if (!isOwnedByMemRegion(inst)) return 0x0;
    if (inst.$ownedBy.auto) return inst.$ownedBy.autoStartAddress;
    return inst.$ownedBy.manualStartAddress;
}

/*
 * onChange handler for size that toggles address field visibility based
 * on ownership mode.
 *
 * In Automate MMU mode (isAutomated == true): hides vAddr/pAddr and shows
 * vAddr_mr/pAddr_mr (read-only, driven by owner). In standalone mode the
 * reverse applies.
 *
 * Attached to size because that is the only configurable set via
 * requiredArgs when the child is spawned, so it fires at creation time —
 * SysConfig provides no onOwnerChange event.
 */
function changeDisplay(inst, ui) {
    let owned = isOwnedByMemRegion(inst);
    ui.vAddr.hidden    =  owned;
    ui.vAddr_mr.hidden = !owned;
    ui.pAddr.hidden    =  owned;
    ui.pAddr_mr.hidden = !owned;
}

/* -------------------------------------------------------------------------
 * config array
 * ------------------------------------------------------------------------- */
let config = [
    {
        name: "$name",
        hidden: false,
        isCIdentifier: false,
    },
    {
        name: "isAutomated",
        default: false,
        hidden: true,
    },

    /* --- Virtual Address ------------------------------------------------- */
    {
        name: "vAddr",
        displayName: "Virtual Address (hex)",
        description: "Virtual address for the mapping. Usually identical to pAddr for identity mappings.",
        default: 0x0,
        displayFormat: "hex",
        longDescription: "Virtual address used in code. For identity-mapped regions (DDR, MSMC) this equals pAddr. For C7x local memory above 4 GB, set this to the virtual address your linker script places the section at.",
    },
    {
        name: "vAddr_mr",
        displayName: "Virtual Address (hex)",
        default: 0x0,
        hidden: true,
        displayFormat: "hex",
        getValue: readFromOwner,
        longDescription: "Virtual address derived from the owning linker memory region (Automate MMU mode). For non-identity mappings use standalone mmu_cfg instances instead.",
    },

    /* --- Physical Address ------------------------------------------------ */
    {
        name: "pAddr",
        displayName: "Physical Address (hex)",
        description: "Physical address to map to. Differs from vAddr only for non-identity mappings.",
        default: 0x0,
        displayFormat: "hex",
        longDescription: "Physical address the MMU maps the virtual range to. Set equal to vAddr for identity mappings.",
    },
    {
        name: "pAddr_mr",
        displayName: "Physical Address (hex)",
        default: 0x0,
        hidden: true,
        displayFormat: "hex",
        getValue: readFromOwner,
        longDescription: "Physical address derived from the owning linker memory region (identity mapping assumed). For non-identity mappings use standalone mmu_cfg instances.",
    },

    /* --- Size ------------------------------------------------------------ */
    {
        name: "size",
        displayName: "Size (Bytes hex)",
        description: "Size of the mapped region in bytes. Does not need to be a power of 2 for C7x MMU.",
        default: 0x20000000,
        displayFormat: "hex",
        onChange: changeDisplay,
        longDescription: "C7x MMU accepts any non-zero size. In Automate MMU mode this is set automatically from the owning linker region's size.",
    },

    /* --- Memory Attribute ------------------------------------------------ */
    {
        name: "attrIndx",
        displayName: "Memory Attribute Index",
        description: "MAIR index defining memory type and caching behavior.",
        default: "Mmu_AttrIndx_MAIR0",
        options: ATTR_INDX_OPTIONS,
        longDescription: "Selects one of the 8 MAIR attribute bytes programmed during MMU initialisation. Use MAIR0-3 for device memory (peripherals) and MAIR4-7 for normal memory (DDR, MSMC, L3).",
    },
];

/*
 * Validates an mmu_cfg instance.
 *
 * Checks:
 *   1. size must be > 0.
 *   2. Effective virtual address must be non-negative (defensive; getValue
 *      should never return negative).
 */
function validate(inst, report) {
    if (inst.size === 0) {
        report.logError("Size must be greater than 0.", inst, "size");
    }

    let effectiveVAddr = isOwnedByMemRegion(inst) ? inst.vAddr_mr : inst.vAddr;
    if (effectiveVAddr < 0) {
        let addrField = isOwnedByMemRegion(inst) ? "vAddr_mr" : "vAddr";
        report.logError("Virtual address must be non-negative.", inst, addrField);
    }
}

/* -------------------------------------------------------------------------
 * Helpers called from the XDT template
 * ------------------------------------------------------------------------- */

/*
 * Returns the effective virtual address of an mmu_cfg instance as an uppercase
 * hex string with "0x" prefix, suitable for emission into C source.
 *
 * Selects vAddr_mr for automated instances (Automate MMU mode) or vAddr for
 * standalone instances.
 */
function getVAddrStr(inst) {
    let addr = isOwnedByMemRegion(inst) ? inst.vAddr_mr : inst.vAddr;
    return "0x" + addr.toString(16).toUpperCase();
}

/*
 * Returns the effective physical address of an mmu_cfg instance as an uppercase
 * hex string with "0x" prefix.
 *
 * Selects pAddr_mr for automated instances or pAddr for standalone instances.
 */
function getPAddrStr(inst) {
    let addr = isOwnedByMemRegion(inst) ? inst.pAddr_mr : inst.pAddr;
    return "0x" + addr.toString(16).toUpperCase();
}

/*
 * Returns the size of an mmu_cfg instance as an uppercase hex string with
 * "0x" prefix, suitable for emission into C source.
 */
function getSizeStr(inst) {
    return "0x" + inst.size.toString(16).toUpperCase();
}

/* -------------------------------------------------------------------------
 * Module export
 * ------------------------------------------------------------------------- */
let mmuCfgModule = {
    displayName: "MMU Region",
    defaultInstanceName: "MMU_REGION",
    config: config,
    validate: validate,

    moduleStatic: {
        config: [
            {
                name: "addDefaultMmuConfig",
                displayName: "Add Default MMU Config",
                buttonText: "CLICK",
                longDescription: "Populate MMU regions from the SoC default matching the current static main.c MMU setup.",
                scriptingOnComplete: (inst) => {
                    let socScript = getSocScript();
                    if (socScript === null) {
                        system.reportError("No default MMU config available for device: " + device);
                        return;
                    }
                    let mmuSoc = system.getScript(socScript);
                    let count  = mmuSoc.getMmuRegionCount(selfCoreName);

                    let mmuMod = scripting.addModule("/memory_configurator/mmu_cfg.syscfg.js", {}, false);
                    let instances = [];
                    for (let i = 0; i < count; i++) instances.push(mmuMod.addInstance());
                    let wrapper = { mmu_region: instances };
                    for (let i = 0; i < count; i++) mmuSoc.populate_mmu_regions(wrapper, i, device, selfCoreName);
                },
            },
        ],
    },

    /* Helpers called from XDT templates */
    getVAddrStr: getVAddrStr,
    getPAddrStr:  getPAddrStr,
    getSizeStr:   getSizeStr,
};

exports = mmuCfgModule;
