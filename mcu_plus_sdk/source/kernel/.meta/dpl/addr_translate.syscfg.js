
let common = system.getScript("/common");
let soc = system.getScript(`/kernel/dpl/addr_translate_${common.getSocName()}`);

/*
 * Returns true when this addr_translate instance was created by a memory region
 * parent (Automate RAT mode), false when it is a standalone entry added
 * directly in the DPL RAT module.
 *
 * Reads inst.isAutomated, set to true via requiredArgs by the parent's
 * moduleInstances() (see memory_configurator/helper.getRatModuleInstances).
 * Checked directly rather than via $ownedBy to avoid cross-context SysConfig
 * errors in onChange/validate.
 */
function isOwnedByMemRegion(inst) {
    return inst.isAutomated === true;
}

/*
 * Reads the resolved start address from the owning memory region instance.
 *
 * Used as the getValue handler for localAddr_mr in Automate RAT mode. Returns
 * 0x0 for standalone instances. Safe to call from getValue (display time) but
 * must NOT be called during moduleInstances(), which would evaluate
 * autoStartAddress across contexts and raise a SysConfig error.
 */
function readFromOwner(inst) {
    if (!isOwnedByMemRegion(inst)) return 0x0;
    if (inst.$ownedBy.auto) return inst.$ownedBy.autoStartAddress;
    return inst.$ownedBy.manualStartAddress;
}

/*
 * onChange handler that toggles localAddr / localAddr_mr visibility based on
 * ownership mode: standalone instances edit localAddr, automated instances
 * show the read-only localAddr_mr that follows the owning region.
 *
 * Attached to both isAutomated and size. SysConfig has no onOwnerChange
 * event, so this relies on the requiredArgs the parent sets when it spawns
 * the child. isAutomated is the dependable trigger because the required value
 * (true) always differs from its default (false); size is kept as well to
 * mirror mpu_armv7, but its required value can coincide with its default.
 */
function changeDisplay(inst, ui) {
    let owned = isOwnedByMemRegion(inst);
    ui.localAddr.hidden    =  owned;
    ui.localAddr_mr.hidden = !owned;
}

/*
 * Returns the effective local address of an addr_translate instance as a
 * number: localAddr_mr for automated instances, localAddr for standalone ones.
 *
 * Deliberately not truncated with ">>> 0". The raw value is needed so that
 * validate() can reject addresses above 32 bits instead of silently wrapping
 * them to a valid-looking 32-bit address.
 */
function getLocalAddr(inst) {
    return isOwnedByMemRegion(inst) ? inst.localAddr_mr : inst.localAddr;
}

/*
 * Returns the owning memory region size in bytes. memory_region stores size in
 * bytes while memory_region_mpu stores it as a log2 exponent, so the owner
 * module decides the interpretation.
 */
function getOwnerSizeBytes(inst) {
    let owner = inst.$ownedBy;
    if (owner.$module === system.modules["/memory_configurator/memory_region_mpu"]) {
        return Math.pow(2, owner.size);
    }
    return owner.size;
}

let addr_translate_module = {
    displayName: "RAT",
    longDescription: soc.getLongDescription() +
`
With "Automate RAT Setting" enabled in the Memory Region tab, an instance can
instead be added under a memory region. Its local address and size then follow
that region, and only the translated system address has to be entered.`,
    maxInstances: 16,
    templates: {
        "/kernel/dpl/dpl_config.c.xdt": {
            dpl_config: "/kernel/dpl/addr_translate.c.xdt",
            dpl_init: "/kernel/dpl/addr_translate_init.c.xdt",
        },
        "/kernel/dpl/dpl_config.h.xdt": "/kernel/dpl/addr_translate.h.xdt",
    },
    defaultInstanceName: "CONFIG_ADDR_TRANSLATE_REGION",
    config: [
        {
            name: "isAutomated",
            default: false,
            hidden: true,
            onChange: changeDisplay,
        },
        {
            name: "localAddr",
            displayName: "Local Address (hex)",
            longDescription: "Local Address to the CPU. MUST be <= 32 bits and MUST be region size aligned and MUST NOT overlap with other regions",
            default: 0x0,
            displayFormat: "hex",
        },
        {
            name: "localAddr_mr",
            displayName: "Local Address (hex)",
            longDescription:
"Local address read from the owning memory region (Automate RAT mode). Must be " +
"aligned to the RAT region size, which is the memory region size rounded up to " +
"a power of 2.",
            default: 0x0,
            hidden: true,
            displayFormat: "hex",
            getValue: readFromOwner,
        },
        {
            name: "systemAddr",
            displayName: "Translated System Address (hex)",
            longDescription: "System Address common to the SoC. MUST be <= 48 bits and MUST be region size aligned",
            default: 0x0,
            displayFormat: "hex",
        },
        {
            name: "size",
            displayName: "Region Size",
            description: "Size of region(in bytes) that is to be translated",
            longDescription:
"In Automate RAT mode this is set automatically to the owning memory region " +
"size rounded up to a power of 2.",
            default: 29,
            onChange: changeDisplay,
            options: [
                {
                    name: 5,
                    displayName: "32 B"
                },
                {
                    name: 6,
                    displayName: "64 B"
                },
                {
                    name: 7,
                    displayName: "128 B"
                },
                {
                    name: 8,
                    displayName: "256 B"
                },
                {
                    name: 9,
                    displayName: "512 B"
                },
                {
                    name: 10,
                    displayName: "1 KB"
                },
                {
                    name: 11,
                    displayName: "2 KB"
                },
                {
                    name: 12,
                    displayName: "4 KB"
                },
                {
                    name: 13,
                    displayName: "8 KB"
                },
                {
                    name: 14,
                    displayName: "16 KB"
                },
                {
                    name: 15,
                    displayName: "32 KB"
                },
                {
                    name: 16,
                    displayName: "64 KB"
                },
                {
                    name: 17,
                    displayName: "128 KB"
                },
                {
                    name: 18,
                    displayName: "256 KB"
                },
                {
                    name: 19,
                    displayName: "512 KB"
                },
                {
                    name: 20,
                    displayName: "1 MB"
                },
                {
                    name: 21,
                    displayName: "2 MB"
                },
                {
                    name: 22,
                    displayName: "4 MB"
                },
                {
                    name: 23,
                    displayName: "8 MB"
                },
                {
                    name: 24,
                    displayName: "16 MB"
                },
                {
                    name: 25,
                    displayName: "32 MB"
                },
                {
                    name: 26,
                    displayName: "64 MB"
                },
                {
                    name: 27,
                    displayName: "128 MB"
                },
                {
                    name: 28,
                    displayName: "256 MB"
                },
                {
                    name: 29,
                    displayName: "512 MB"
                },
                {
                    name: 30,
                    displayName: "1 GB"
                },
                {
                    name: 31,
                    displayName: "2 GB"
                },
                {
                    name: 32,
                    displayName: "4 GB"
                },
            ],
        },
    ],

    validate : function (instance, report) {
        /* JS numbers are doubles, so addresses >= 0x80000000 stay positive
         * for % and comparisons. Do not use ">>> 0" here: it wraps values
         * above 32 bits and would defeat the maxLocalAddr check below. */
        let owned = isOwnedByMemRegion(instance);
        let addrField = owned ? "localAddr_mr" : "localAddr";
        let localAddr = getLocalAddr(instance);
        let systemAddr = instance.systemAddr;
        let size = 2**instance.size;
        let maxLocalAddr = 0x100000000;
        let maxSystemAddr = 0x1000000000000;

        if ( (localAddr % size) != 0)
        {
            report.logError( `Region start address must be aligned to 0x${size.toString(16).toUpperCase()}` +
                    (owned ? " (memory region size rounded up to a power of 2). " +
                             "Align the memory region start address or reduce its size." : ""),
                    instance, addrField);
        }
        if ( (systemAddr % size) != 0)
        {
            report.logError( `Translated Region start address must be aligned to 0x${size.toString(16).toUpperCase()}`,
                    instance, "systemAddr");
        }
        if (localAddr >= maxLocalAddr)
        {
            report.logError( `Region start address must be <= 0x${(maxLocalAddr-1).toString(16).toUpperCase()}`,
                    instance, addrField);
        }
        if (systemAddr >= maxSystemAddr)
        {
            report.logError( `Translated Region start address must be <= 0x${(maxSystemAddr-1).toString(16).toUpperCase()}`,
                    instance, "systemAddr");
        }
        /* The rounded-up size is clamped to 4 GB, so a larger memory region
         * would otherwise be only partly covered without any message. */
        if (owned && size < getOwnerSizeBytes(instance))
        {
            report.logError( `RAT region size 0x${size.toString(16).toUpperCase()} does not cover the ` +
                    `memory region size 0x${getOwnerSizeBytes(instance).toString(16).toUpperCase()}`,
                    instance, "size");
        }
        /* check overlap across instances */
        let module = instance.$module;

        for( let curInstance of module.$instances)
        {
            if(curInstance!=instance)
            {
                let curSize = 2**curInstance.size;
                let curLocalAddr = getLocalAddr(curInstance);
                if( (    localAddr >= curLocalAddr
                      && localAddr < (curLocalAddr+curSize)
                    )
                    ||
                    (    curLocalAddr >= localAddr
                        && curLocalAddr < (localAddr+size)
                      )
                    )
                {
                    report.logError( `Overlapping regions not allowed`,
                        instance, addrField);
                }
            }
        }
    },
    moduleStatic: {
        modules: function(inst) {
            return [{
                name: "system_common",
                moduleName: "/system_common",
            }]
        },
    },
    getSizeString(size) {
        size = 2**size;
        if(size < 1024) {
            return size.toString();
        }
        else
        if(size < 1024*1024) {
            size = size/1024;
            return size.toString()+"K";
        }
        else
        if(size < 1024*1024*1024) {
            size = size/(1024*1024);
            return size.toString()+"M";
        }
        else {
            size = size/(1024*1024*1024);
            return size.toString()+"G";
        }
    },
    getRatBaseAddr() {
        return soc.getRatBaseAddr();
    },
    getLocalAddr,
};

exports = addr_translate_module;

