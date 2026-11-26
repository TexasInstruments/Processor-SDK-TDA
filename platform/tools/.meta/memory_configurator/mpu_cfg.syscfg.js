let common   = system.getScript("/common");
const memHelper = system.getScript("/memory_configurator/helper");
let selfCoreName = common.getSelfSysCfgCoreName();
const device = system.deviceData.device;
let isSafeRTOS = (process.env.RTOS || "").toUpperCase() === "SAFERTOS";

/*
 * Returns the module path of the SoC-specific default MPU configuration
 * script for the currently active device.
 *
 * The script lives at platform/rtos/<soc>/.meta/mem_map_default_cfg/ and is
 * discoverable via the per-SoC includePaths entries in product.json. It
 * provides getMpuRegionCount() and populate_mpu_regions() functions used
 * by the "Add Default MPU Config" button to auto-populate mpu_cfg instances.
 *
 * product.json puts every SoC on the include path at once, so the script name
 * is suffixed with the SoC (default_mpu_config_<soc>). A bare shared name would
 * always resolve to the first SoC listed, regardless of the active device.
 *
 * Returns: string — e.g. "/mem_map_default_cfg/default_mpu_config_j721s2", or
 *          null when the active device is not a recognised SoC.
 * Side effects: none.
 */
function getSocScript() {
    let soc = common.getSocName();
    return soc ? "/mem_map_default_cfg/default_mpu_config_" + soc : null;
}

/* -------------------------------------------------------------------------
 * Size table: each entry has the byte count, a CSL enum string, and a
 * SafeRTOS-style byte-expression string.
 * ------------------------------------------------------------------------- */
const SIZE_TABLE = [
    { bytes: 32,         log2: 5,  csl: "CSL_ARM_R5_MPU_REGION_SIZE_32B",   safe: "(32U)",                      display: "32 B"   },
    { bytes: 64,         log2: 6,  csl: "CSL_ARM_R5_MPU_REGION_SIZE_64B",   safe: "(64U)",                      display: "64 B"   },
    { bytes: 128,        log2: 7,  csl: "CSL_ARM_R5_MPU_REGION_SIZE_128B",  safe: "(128U)",                     display: "128 B"  },
    { bytes: 256,        log2: 8,  csl: "CSL_ARM_R5_MPU_REGION_SIZE_256B",  safe: "(256U)",                     display: "256 B"  },
    { bytes: 512,        log2: 9,  csl: "CSL_ARM_R5_MPU_REGION_SIZE_512B",  safe: "(512U)",                     display: "512 B"  },
    { bytes: 1024,       log2: 10, csl: "CSL_ARM_R5_MPU_REGION_SIZE_1KB",   safe: "(1U * 1024U)",               display: "1 KB"   },
    { bytes: 2048,       log2: 11, csl: "CSL_ARM_R5_MPU_REGION_SIZE_2KB",   safe: "(2U * 1024U)",               display: "2 KB"   },
    { bytes: 4096,       log2: 12, csl: "CSL_ARM_R5_MPU_REGION_SIZE_4KB",   safe: "(4U * 1024U)",               display: "4 KB"   },
    { bytes: 8192,       log2: 13, csl: "CSL_ARM_R5_MPU_REGION_SIZE_8KB",   safe: "(8U * 1024U)",               display: "8 KB"   },
    { bytes: 16384,      log2: 14, csl: "CSL_ARM_R5_MPU_REGION_SIZE_16KB",  safe: "(16U * 1024U)",              display: "16 KB"  },
    { bytes: 32768,      log2: 15, csl: "CSL_ARM_R5_MPU_REGION_SIZE_32KB",  safe: "(32U * 1024U)",              display: "32 KB"  },
    { bytes: 65536,      log2: 16, csl: "CSL_ARM_R5_MPU_REGION_SIZE_64KB",  safe: "(64U * 1024U)",              display: "64 KB"  },
    { bytes: 131072,     log2: 17, csl: "CSL_ARM_R5_MPU_REGION_SIZE_128KB", safe: "(128U * 1024U)",             display: "128 KB" },
    { bytes: 262144,     log2: 18, csl: "CSL_ARM_R5_MPU_REGION_SIZE_256KB", safe: "(256U * 1024U)",             display: "256 KB" },
    { bytes: 524288,     log2: 19, csl: "CSL_ARM_R5_MPU_REGION_SIZE_512KB", safe: "(512U * 1024U)",             display: "512 KB" },
    { bytes: 1048576,    log2: 20, csl: "CSL_ARM_R5_MPU_REGION_SIZE_1MB",   safe: "(1U * 1024U * 1024U)",       display: "1 MB"   },
    { bytes: 2097152,    log2: 21, csl: "CSL_ARM_R5_MPU_REGION_SIZE_2MB",   safe: "(2U * 1024U * 1024U)",       display: "2 MB"   },
    { bytes: 4194304,    log2: 22, csl: "CSL_ARM_R5_MPU_REGION_SIZE_4MB",   safe: "(4U * 1024U * 1024U)",       display: "4 MB"   },
    { bytes: 8388608,    log2: 23, csl: "CSL_ARM_R5_MPU_REGION_SIZE_8MB",   safe: "(8U * 1024U * 1024U)",       display: "8 MB"   },
    { bytes: 16777216,   log2: 24, csl: "CSL_ARM_R5_MPU_REGION_SIZE_16MB",  safe: "(16U * 1024U * 1024U)",      display: "16 MB"  },
    { bytes: 33554432,   log2: 25, csl: "CSL_ARM_R5_MPU_REGION_SIZE_32MB",  safe: "(32U * 1024U * 1024U)",      display: "32 MB"  },
    { bytes: 67108864,   log2: 26, csl: "CSL_ARM_R5_MPU_REGION_SIZE_64MB",  safe: "(64U * 1024U * 1024U)",      display: "64 MB"  },
    { bytes: 134217728,  log2: 27, csl: "CSL_ARM_R5_MPU_REGION_SIZE_128MB", safe: "(128U * 1024U * 1024U)",     display: "128 MB" },
    { bytes: 268435456,  log2: 28, csl: "CSL_ARM_R5_MPU_REGION_SIZE_256MB", safe: "(256U * 1024U * 1024U)",     display: "256 MB" },
    { bytes: 536870912,  log2: 29, csl: "CSL_ARM_R5_MPU_REGION_SIZE_512MB", safe: "(512U * 1024U * 1024U)",     display: "512 MB" },
    { bytes: 1073741824, log2: 30, csl: "CSL_ARM_R5_MPU_REGION_SIZE_1GB",   safe: "(1U * 1024U * 1024U * 1024U)", display: "1 GB" },
    { bytes: 2147483648, log2: 31, csl: "CSL_ARM_R5_MPU_REGION_SIZE_2GB",   safe: "portmpuLARGEST_REGION_SIZE_ACTUAL", display: "2 GB" },
    { bytes: 4294967296, log2: 32, csl: "CSL_ARM_R5_MPU_REGION_SIZE_4GB",   safe: "portmpuLARGEST_REGION_SIZE_ACTUAL", display: "4 GB" },
];

/*
 * Maps a region size in bytes to the corresponding CSL ARMv7-R MPU region
 * size enum string.
 *
 * Looks up the given sizeBytes in SIZE_TABLE. If a matching entry is found,
 * returns its .csl field (e.g. "CSL_ARM_R5_MPU_REGION_SIZE_32KB"). If no
 * entry matches (size is not a power-of-2 in the supported range), falls back
 * to a hex string representation of the byte count.
 *
 * Parameters:
 *   sizeBytes - number — region size in bytes; must be a power-of-2 between
 *               32 and 4294967296 for a CSL enum to be returned.
 *
 * Returns: string — CSL enum name, or "0x<HEX>" fallback string.
 * Side effects: none.
 */
function getSizeEnum(sizeBytes) {
    let e = SIZE_TABLE.find(x => x.bytes === sizeBytes);
    return e ? e.csl : ("0x" + sizeBytes.toString(16).toUpperCase());
}

/*
 * Maps a region size in bytes to the SafeRTOS-style size expression string.
 *
 * Looks up sizeBytes in SIZE_TABLE and returns the .safe field which is a
 * human-readable C constant expression suitable for use in SafeRTOS MPU
 * configuration structures (e.g. "(32U * 1024U)" for 32 KB). Falls back to
 * a simple "(<bytes>U)" expression for sizes not in the table.
 *
 * Parameters:
 *   sizeBytes - number — region size in bytes.
 *
 * Returns: string — SafeRTOS size expression, or "(<bytes>U)" fallback.
 * Side effects: none.
 */
function getSafertosSize(sizeBytes) {
    let e = SIZE_TABLE.find(x => x.bytes === sizeBytes);
    return e ? e.safe : ("(" + sizeBytes + "U)");
}

/*
 * Returns true when this mpu_cfg instance was created by a memory_region_mpu
 * parent (Automate MPU mode), false when it is a standalone region added
 * directly by the user.
 *
 * Reads inst.isAutomated, a hidden configurable that is set to true via
 * requiredArgs when memory_region_mpu's addModuleInstances() spawns the child.
 * This flag is intentionally read directly from the instance rather than
 * checking $ownedBy, because $ownedBy access from onChange/getValue/validate
 * can trigger cross-context SysConfig errors.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.isAutomated (boolean).
 *
 * Returns: boolean — true if owned by a memory_region_mpu, false otherwise.
 * Side effects: none.
 */
function isOwnedByMemRegion(inst) {
    return inst.isAutomated === true;
}

/*
 * Reads the resolved base address from the owning memory_region_mpu instance
 * for use as the mpu_cfg's baseAddr_mr getValue handler.
 *
 * When a mpu_cfg is spawned by memory_region_mpu (Automate MPU mode), its
 * base address must track the owning region's start address. This function
 * returns 0x0 for standalone (non-automated) instances. For automated
 * instances it reads $ownedBy.autoStartAddress or $ownedBy.manualStartAddress
 * depending on whether the parent region is auto-placed.
 *
 * This function is safe to call from getValue (display time) because getValue
 * runs in the context of the owning core. It must NOT be called during
 * moduleInstances() — that would trigger autoStartAddress evaluation across
 * contexts and cause a SysConfig error.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.isAutomated, inst.$ownedBy.
 *
 * Returns: number — the effective base address in bytes, or 0x0 if the
 *          instance is not in Automate MPU mode.
 * Side effects: none.
 */
function readFromOwner(inst) {
    if (!isOwnedByMemRegion(inst)) return 0x0;
    if (inst.$ownedBy.auto) return inst.$ownedBy.autoStartAddress;
    return inst.$ownedBy.manualStartAddress;
}

/*
 * onChange handler for the sizeBytes configurable that toggles the visibility
 * of baseAddr vs baseAddr_mr depending on the instance's ownership mode.
 *
 * In Automate MPU mode (isOwnedByMemRegion == true), baseAddr is hidden and
 * baseAddr_mr (read-only, driven by the owning region) is shown. In
 * standalone mode the reverse applies.
 *
 * SysConfig does not provide an onOwnerChange event, so this handler is
 * attached to sizeBytes onChange as a proxy — sizeBytes is the only
 * configurable modified via requiredArgs when the child is first created,
 * so changeDisplay fires at creation time as well.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.isAutomated.
 *   ui   - SysConfig UI state object. Writes ui.baseAddr.hidden and
 *          ui.baseAddr_mr.hidden.
 *
 * Returns: void.
 * Side effects: mutates ui.baseAddr.hidden and ui.baseAddr_mr.hidden.
 */
function changeDisplay(inst, ui) {
    let owned = isOwnedByMemRegion(inst);
    ui.baseAddr.hidden    =  owned;
    ui.baseAddr_mr.hidden = !owned;
}

/*
 * onChange handler for the attributes (memory preset) configurable.
 * Shows or hides the advanced cache/shareable/memAttr fields depending on
 * whether the "ADVANCED" preset is selected.
 *
 * For all standard presets (StronglyOrdered, NonCached, Cached,
 * CachedShareable) the advanced fields are hidden because they are derived
 * automatically by resolvedAttrs(). Only when the user selects "ADVANCED" are
 * the individual CSL fields exposed for manual override.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.attributes.
 *   ui   - SysConfig UI state object. Writes hidden on cacheable,
 *          cachePolicy, shareable, and memAttr fields.
 *
 * Returns: void.
 * Side effects: mutates ui.cacheable.hidden, ui.cachePolicy.hidden,
 *              ui.shareable.hidden, ui.memAttr.hidden.
 */
function onAttributesChange(inst, ui) {
    let isAdvanced = (inst.attributes === "ADVANCED");
    ui.cacheable.hidden    = !isAdvanced;
    ui.cachePolicy.hidden  = !isAdvanced;
    ui.shareable.hidden    = !isAdvanced;
    ui.memAttr.hidden      = !isAdvanced;
}

/*
 * Finds all linker memory region instances across all cores whose address
 * range overlaps this MPU region's address span, and returns a structured
 * summary of the matching regions.
 *
 * Determines the MPU range from baseAddr_mr (Automate MPU mode) or baseAddr
 * (standalone mode) plus sizeBytes. Calls memHelper.util_function() with the
 * appropriate suffix ("_mpu" for automated, "" for standalone) to get all
 * per-core memory region instances, then checks each for address overlap.
 *
 * The returned object accumulates newline-separated strings for the name,
 * type, and cores fields to support multi-line display in the read-only
 * memory region info sub-group. The shared flag is set to true if any
 * overlapping region has isShared == true.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.isAutomated, inst.baseAddr,
 *          inst.baseAddr_mr, inst.sizeBytes.
 *
 * Returns: object — {
 *     name:   string  newline-separated "regionName_coreName" entries,
 *     type:   string  newline-separated unique memory type keys,
 *     shared: boolean true if at least one overlapping region is shared,
 *     cores:  string  newline-separated unique core name strings
 *   }
 *   All fields default to "" / false when no overlap is found.
 * Side effects: none.
 */
function getOverlappingMemoryRegions(inst) {
    let result = { name: "", type: "", shared: false, cores: "" };

    let mpuStart = isOwnedByMemRegion(inst) ? inst.baseAddr_mr : inst.baseAddr;
    let mpuEnd   = mpuStart + inst.sizeBytes - 1;

    /* Check both plain memory_region and memory_region_mpu (Automate MPU mode) */
    let allCoreMods = memHelper.util_function(isOwnedByMemRegion(inst) ? "_mpu" : "");

    for (let coreMod of allCoreMods) {
        if (!coreMod.core || coreMod.instances.length === 0) continue;

        for (let region of coreMod.instances) {
            let regionStart = region.auto ? region.autoStartAddress : region.manualStartAddress;
            let regionEnd   = regionStart + region.size - 1;

            if (regionEnd >= mpuStart && mpuEnd >= regionStart) {
                result.name = result.name.concat(region.$name, "_", coreMod.core, "\n");

                if (region.type && !result.type.split("\n").includes(region.type))
                    result.type = result.type.concat(region.type, "\n");

                if (!result.cores.split("\n").includes(coreMod.core))
                    result.cores = result.cores.concat(coreMod.core, "\n");

                if (region.isShared) {
                    result.shared = true;
                    if (region.shared_cores) {
                        for (let sc of region.shared_cores) {
                            if (!result.cores.split("\n").includes(sc))
                                result.cores = result.cores.concat(sc, "\n");
                        }
                    }
                }
            }
        }
    }
    return result;
}

/*
 * Formats the overlapping memory region names from getOverlappingMemoryRegions()
 * into a human-readable multi-line string grouped by core, suitable for the
 * read-only "Name" display field in the Memory Region info sub-group.
 *
 * Parses the raw "regionName_coreName\n..." string, groups region names by
 * their owning core, and produces output in the form:
 *   "CORENAME -->\n\nregion1\nregion2\n\n..."
 *
 * Returns "(none)" when there are no overlapping regions.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance passed through to
 *          getOverlappingMemoryRegions().
 *
 * Returns: string — formatted multi-line display string, or "(none)".
 * Side effects: none.
 */
function getMemoryRegionNames(inst) {
    let raw = getOverlappingMemoryRegions(inst).name;
    if (!raw || raw.trim() === "") return "(none)";

    /* Group region names by core: "CORE --> \nregion1\nregion2\n\n" */
    let lines = raw.split("\n").filter(l => l.length > 0);
    let map   = {};
    for (let line of lines) {
        let lastUnderscore = line.lastIndexOf("_");
        let regionName = line.substring(0, lastUnderscore);
        let coreName   = line.substring(lastUnderscore + 1);
        if (!map[coreName]) map[coreName] = [];
        map[coreName].push(regionName);
    }

    let out = "";
    for (let core in map) {
        out += core.toUpperCase() + " -->\n\n";
        for (let r of map[core]) out += r + "\n";
        out += "\n";
    }
    return out.trim() || "(none)";
}

/*
 * Derives the resolved cacheable, cachePolicy, and shareable values from the
 * selected attributes preset for use in XDT code generation templates.
 *
 * Standard presets map to fixed CSL values:
 *   StronglyOrdered  → cacheable=false, cachePolicy="0U",                        shareable=false
 *   NonCached        → cacheable=false, cachePolicy=NON_CACHEABLE,               shareable=false
 *   Cached           → cacheable=true,  cachePolicy=WB_WA,                       shareable=false
 *   CachedShareable  → cacheable=true,  cachePolicy=WB_WA,                       shareable=true
 *   ADVANCED         → reads inst.cacheable / inst.cachePolicy / inst.shareable
 *                      directly; cachePolicy is forced to "0U" when cacheable=false.
 *
 * Called by XDT templates via the module-exported getResolvedAttrs() helper,
 * not directly from SysConfig GUI callbacks.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.attributes and, for the
 *          ADVANCED preset, inst.cacheable, inst.cachePolicy, inst.shareable.
 *
 * Returns: object — { cacheable: boolean, cachePolicy: string, shareable: boolean }
 * Side effects: none.
 */
function resolvedAttrs(inst) {
    switch (inst.attributes) {
        case "StronglyOrdered":
            return { cacheable: false, cachePolicy: "0U",                                    shareable: false };
        case "NonCached":
            return { cacheable: false, cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE", shareable: false };
        case "Cached":
            return { cacheable: true,  cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WB_WA",        shareable: false };
        case "CachedShareable":
            return { cacheable: true,  cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WB_WA",        shareable: true  };
        case "ADVANCED":
            return {
                cacheable:   inst.cacheable,
                cachePolicy: inst.cacheable ? inst.cachePolicy : "0U",
                shareable:   inst.shareable,
            };
        default:
            return { cacheable: false, cachePolicy: "0U", shareable: false };
    }
}

/* -------------------------------------------------------------------------
 * config array — rendered by SysConfig as the per-region form panel
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

    /* --- Base Address ---------------------------------------------------- */
    {
        name: "baseAddr",
        displayName: "Base Address (hex)",
        default: 0x0,
        displayFormat: "hex",
        longDescription: "Physical base address of the MPU region. Must be aligned to the region size.",
    },
    {
        name: "baseAddr_mr",
        displayName: "Base Address (hex)",
        default: 0x0,
        hidden: true,
        displayFormat: "hex",
        getValue: readFromOwner,
        longDescription: "Base address read from the owning memory region (Automate MPU mode).",
    },

    /* --- Size ------------------------------------------------------------ */
    {
        name: "sizeBytes",
        displayName: "Region Size",
        default: 32768,
        options: SIZE_TABLE.map(e => ({ name: e.bytes, displayName: e.display })),
        onChange: changeDisplay,
        longDescription: "ARMv7-R MPU regions must be a power-of-2 size, minimum 32 bytes.",
    },

    /* --- Access ---------------------------------------------------------- */
    {
        name: "accessPermission",
        displayName: "Access Permission",
        default: "CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR",
        options: [
            { name: "CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR",  displayName: "Supervisor RD+WR, User RD+WR" },
            { name: "CSL_ARM_R5_ACC_PERM_PRIV_RD_WR",       displayName: "Supervisor RD+WR, User blocked" },
            { name: "CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_ONLY", displayName: "Supervisor RD, User RD" },
            { name: "CSL_ARM_R5_ACC_PERM_PRIV_RD_ONLY",     displayName: "Supervisor RD, User blocked" },
        ],
    },
    {
        name: "exeNeverControl",
        displayName: "Execute Never (XN)",
        default: false,
        longDescription: "Set for data-only regions (IPC buffers, log memory). Unset for code regions.",
    },

    /* --- Sub-regions ----------------------------------------------------- */
    {
        name: "subRegionEnable",
        displayName: "Sub-Region Enable Mask (hex)",
        default: 0xFF,
        displayFormat: "hex",
        longDescription: "0xFF enables all 8 sub-regions. Bit N=0 disables sub-region N (each 1/8 of the region).",
    },

    /* --- Memory Attributes preset --------------------------------------- */
    {
        name: "attributes",
        displayName: "Memory Attributes",
        default: "NonCached",
        onChange: onAttributesChange,
        options: [
            {
                name: "StronglyOrdered",
                displayName: "Strongly Ordered",
                description: "Cache disabled, strongly ordered. Use for peripheral MMRs.",
            },
            {
                name: "NonCached",
                displayName: "Non-Cached",
                description: "Cache disabled, non-cacheable. Use for IPC/shared data buffers.",
            },
            {
                name: "Cached",
                displayName: "Cached (WB-WA)",
                description: "Cache enabled, write-back write-allocate. Use for code and local data.",
            },
            {
                name: "CachedShareable",
                displayName: "Cached + Shareable (WB-WA)",
                description: "Cache enabled and region marked shareable. Use for Ethernet descriptor memory.",
            },
            {
                name: "ADVANCED",
                displayName: "Advanced (manual CSL fields)",
                description: "Directly set cacheable, cache policy, shareable, and memAttr CSL values.",
            },
        ],
        longDescription: "Choose a preset or 'Advanced' to set individual CSL fields.",
    },

    /* --- Advanced fields (hidden unless ADVANCED preset selected) -------- */
    {
        name: "cacheable",
        displayName: "Cacheable",
        default: false,
        hidden: true,
    },
    {
        name: "cachePolicy",
        displayName: "Cache Policy",
        default: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
        hidden: true,
        options: [
            { name: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE", displayName: "Non-Cacheable" },
            { name: "CSL_ARM_R5_CACHE_POLICY_WB_WA",         displayName: "Write-Back, Write-Allocate" },
            { name: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",      displayName: "Write-Through, No Write-Allocate" },
        ],
    },
    {
        name: "shareable",
        displayName: "Shareable",
        default: false,
        hidden: true,
    },
    {
        name: "memAttr",
        displayName: "Memory Attribute (memAttr)",
        default: "0U",
        hidden: true,
        options: [
            { name: "0U",                                       displayName: "0 (default)"          },
            { name: "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",     displayName: "Cached WT No-WA"      },
            { name: "CSL_ARM_R5_MEM_ATTR_CACHED_WB_NO_WA",     displayName: "Cached WB No-WA"      },
            { name: "CSL_ARM_R5_MEM_ATTR_STRONGLY_NON_CACHED", displayName: "Non-Cached (normal memory)" },
            { name: "CSL_ARM_R5_MEM_ATTR_CACHED_WB_WA",        displayName: "Cached WB WA"         },
        ],
    },

    /* --- Memory Region info (read-only, mirrors j722s2 style) ----------- */
    {
        name: "group_memory_region",
        displayName: "Memory Region",
        description: "Linker memory regions that overlap this MPU region's address range.",
        collapsed: false,
        config: [
            {
                name: "memoryRegion_name",
                displayName: "Name",
                multiline: true,
                default: "",
                description: "Memory region(s) that fall within this MPU region's address range, grouped by core.",
                readOnly: true,
                getValue: (inst) => getMemoryRegionNames(inst),
            },
            {
                name: "memoryRegion_type",
                displayName: "Type",
                multiline: true,
                default: "",
                description: "Memory type(s) of overlapping region(s).",
                readOnly: true,
                getValue: (inst) => {
                    let t = getOverlappingMemoryRegions(inst).type;
                    return (t && t.trim()) ? t.trim() : "(none)";
                },
            },
            {
                name: "memoryRegion_shared",
                displayName: "Shared",
                default: false,
                description: "True if at least one overlapping memory region is shared across cores.",
                readOnly: true,
                getValue: (inst) => getOverlappingMemoryRegions(inst).shared,
            },
            {
                name: "memoryRegion_core",
                displayName: "Cores Using It",
                multiline: true,
                default: "",
                description: "Core(s) that own or share overlapping memory regions.",
                readOnly: true,
                getValue: (inst) => {
                    let c = getOverlappingMemoryRegions(inst).cores;
                    return (c && c.trim()) ? c.trim() : "(none)";
                },
            },
        ],
    },
];

/*
 * Top-level validation function for mpu_cfg instances. Enforces hardware and
 * software constraints on MPU region configuration.
 *
 * Checks performed:
 *   1. Total region count: the number of mpu_cfg instances must not exceed
 *      the hardware maximum (16 for FreeRTOS/standard, 10 for SafeRTOS which
 *      reserves the upper regions for OS use).
 *   2. Minimum size: sizeBytes must be >= 32 (ARMv7-R MPU minimum).
 *   3. Address alignment: warns (logWarning) if the effective base address is
 *      not aligned to the region size. The hardware silently rounds down, so
 *      this is a warning rather than a hard error.
 *   4. 32-bit address space: the effective base address must fit in 32 bits.
 *   5. Sub-region enable mask: subRegionEnable must be in [0x00, 0xFF].
 *   6. Cache coherence note: logs an info message when ADVANCED preset has
 *      both cacheable and shareable enabled, reminding the user to ensure
 *      coherency across cores.
 *
 * The effective base address is read from baseAddr_mr for automated instances
 * and from baseAddr for standalone instances.
 *
 * Parameters:
 *   inst   - SysConfig mpu_cfg instance being validated.
 *   report - SysConfig report object used to call logError, logWarning, logInfo.
 *
 * Returns: void.
 * Side effects: calls report methods for each constraint violation or advisory.
 */
function validate(inst, report) {
    let allInstances = system.modules['/memory_configurator/mpu_cfg']?.$instances;
    let regionCount  = allInstances ? allInstances.length : 0;
    let maxRegions   = isSafeRTOS ? 10 : 16;
    if (regionCount > maxRegions) {
        report.logError(
            `${isSafeRTOS ? "SafeRTOS allows at most 10 user-configurable" : "ARMv7-R MPU supports at most 16"} regions. Currently configured: ${regionCount}.`,
            inst, "$name"
        );
    }

    if (inst.sizeBytes < 32) {
        report.logError("Region size must be at least 32 bytes.", inst, "sizeBytes");
    }

    let effectiveAddr  = isOwnedByMemRegion(inst) ? inst.baseAddr_mr : inst.baseAddr;
    let addrField      = isOwnedByMemRegion(inst) ? "baseAddr_mr"    : "baseAddr";
    if ((effectiveAddr % inst.sizeBytes) !== 0) {
        report.logWarning(
            `Base address 0x${effectiveAddr.toString(16).toUpperCase()} is not aligned to ` +
            `region size 0x${inst.sizeBytes.toString(16).toUpperCase()}. ` +
            `MPU hardware will round down to 0x${(effectiveAddr & ~(inst.sizeBytes - 1)).toString(16).toUpperCase()}.`,
            inst, addrField
        );
    }
    if (effectiveAddr > 0xFFFFFFFF) {
        report.logError("Base address must fit in 32 bits.", inst, addrField);
    }

    if (inst.subRegionEnable < 0 || inst.subRegionEnable > 0xFF) {
        report.logError("Sub-region enable mask must be between 0x00 and 0xFF.", inst, "subRegionEnable");
    }

    if (inst.attributes === "ADVANCED" && inst.cacheable && inst.shareable) {
        report.logInfo(
            "Cacheable + shareable: ensure this region is coherent across cores.",
            inst, "shareable"
        );
    }
}

/*
 * Exported helpers used by XDT templates
 */

/*
 * Wrapper that extracts sizeBytes from a mpu_cfg instance and returns the
 * corresponding CSL MPU region size enum string via getSizeEnum().
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.sizeBytes.
 *
 * Returns: string — CSL enum name (e.g. "CSL_ARM_R5_MPU_REGION_SIZE_32KB")
 *          or hex fallback string.
 * Side effects: none.
 */
function getSizeEnumFromInst(inst) {
    return getSizeEnum(inst.sizeBytes);
}

/*
 * Wrapper that extracts sizeBytes from a mpu_cfg instance and returns the
 * corresponding SafeRTOS size expression string via getSafertosSize().
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.sizeBytes.
 *
 * Returns: string — SafeRTOS-style size expression (e.g. "(32U * 1024U)")
 *          or "(<bytes>U)" fallback.
 * Side effects: none.
 */
function getSafertossSizeFromInst(inst) {
    return getSafertosSize(inst.sizeBytes);
}

/*
 * Returns the effective base address of a mpu_cfg instance as an uppercase
 * hex string with "0x" prefix and "U" suffix, suitable for direct emission
 * into C source as an unsigned literal.
 *
 * Selects baseAddr_mr for automated instances (Automate MPU mode) or baseAddr
 * for standalone instances.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.isAutomated, inst.baseAddr,
 *          inst.baseAddr_mr.
 *
 * Returns: string — e.g. "0x70000000U".
 * Side effects: none.
 */
function getBaseAddrStr(inst) {
    let addr = isOwnedByMemRegion(inst) ? inst.baseAddr_mr : inst.baseAddr;
    return "0x" + addr.toString(16).toUpperCase() + "U";
}

/*
 * Returns the memAttr CSL field value for a mpu_cfg instance to be emitted
 * in XDT templates.
 *
 * For the ADVANCED preset, returns the user-selected inst.memAttr value.
 * For all standard presets the memAttr field is not meaningful (it is only
 * relevant for certain ARMv7-R implementations), so "0U" is returned as a
 * safe default.
 *
 * Parameters:
 *   inst - SysConfig mpu_cfg instance. Reads inst.attributes and inst.memAttr.
 *
 * Returns: string — CSL memAttr enum string or "0U".
 * Side effects: none.
 */
function getMemAttr(inst) {
    if (inst.attributes === "ADVANCED") return inst.memAttr;
    return "0U";
}

/* -------------------------------------------------------------------------
 * Module export
 * ------------------------------------------------------------------------- */
let mpuCfgModule = {
    displayName: "MPU Region",
    defaultInstanceName: "MPU_REGION",
    config: config,
    validate: validate,

    moduleStatic: {
        config: [
            {
                name: "addDefaultMpuConfig",
                displayName: "Add Default MPU Config",
                buttonText: "CLICK",
                longDescription: "Populate MPU regions from the SoC default matching the current static mpu_cfg.c files.",
                scriptingOnComplete: (inst) => {
                    let socScript = getSocScript();
                    if (socScript === null) {
                        system.reportError("No default MPU config available for device: " + device);
                        return;
                    }
                    let rtos   = isSafeRTOS ? "safertos" : "freertos";
                    let mpuSoc = system.getScript(socScript);
                    let count  = mpuSoc.getMpuRegionCount(selfCoreName, rtos);

                    let mpuMod = scripting.addModule("/memory_configurator/mpu_cfg.syscfg.js", {}, false);
                    let instances = [];
                    for (let i = 0; i < count; i++) instances.push(mpuMod.addInstance());
                    let wrapper = { mpu_region: instances };
                    for (let i = 0; i < count; i++) mpuSoc.populate_mpu_regions(wrapper, i, device, selfCoreName);
                },
            },
        ],
    },

    /* Helpers called from XDT templates */
    getSizeEnum:         getSizeEnumFromInst,
    getSafertosSize:     getSafertossSizeFromInst,
    getBaseAddrStr:      getBaseAddrStr,
    getResolvedAttrs:    resolvedAttrs,
    getMemAttr:          getMemAttr,

};

exports = mpuCfgModule;
