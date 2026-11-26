let common = system.getScript("/common");
let selfCoreName = common.getSelfSysCfgCoreName();

/*
 * RAT CFG hardware base addresses, consistent across J721E/J721S2/J742S2/J784S4.
 * Source: CSL per-core base address headers
 *   MCU R5F (mcu-r5fss0-*) : CSL_MCU_ARMSS_RAT_CFG_BASE  / CSL_MCU_R5FSS0_RAT_CFG_BASE  = 0x40F90000
 *   Main R5F (r5fss*-*)    : CSL_ARMSS_RAT_CFG_BASE       / CSL_R5FSS*_RAT_CFG_BASE       = 0x0FF90000
 *   C66x (c66ss*-*)        : CSL_C66_COREPAC_C66_RATCFG_BASE                              = 0x07FF0000
 */
function getRatBaseAddr() {
    if (selfCoreName && selfCoreName.includes("mcu-r5fss"))
        return 0x40F90000;
    if (selfCoreName && selfCoreName.includes("r5fss"))
        return 0x0FF90000;
    if (selfCoreName && selfCoreName.includes("c66"))
        return 0x07FF0000;
    return 0x0;
}

/*
 * Returns true when this rat_cfg instance was created by a memory region
 * parent (Automate RAT mode), false when it is a standalone entry added
 * directly in the RAT Configurator.
 *
 * Reads inst.isAutomated, set to true via requiredArgs by the parent's
 * moduleInstances() (see helper.getRatModuleInstances). Checked directly
 * rather than via $ownedBy to avoid cross-context SysConfig errors in
 * onChange/validate.
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
 * mirror mpu_cfg, but its required value can coincide with its default.
 */
function changeDisplay(inst, ui) {
    let owned = isOwnedByMemRegion(inst);
    ui.localAddr.hidden    =  owned;
    ui.localAddr_mr.hidden = !owned;
}

/*
 * Returns the effective local address of a rat_cfg instance as a number:
 * localAddr_mr for automated instances, localAddr for standalone ones.
 *
 * Deliberately not truncated with ">>> 0". The raw value is needed so that
 * validate() can reject addresses above 32 bits (for example a DDR_HIGH
 * region, which an R5F cannot address directly) instead of silently wrapping
 * them to a valid-looking 32-bit address.
 */
function getLocalAddr(inst) {
    return isOwnedByMemRegion(inst) ? inst.localAddr_mr : inst.localAddr;
}

let rat_cfg_module = {
    displayName: "RAT",
    longDescription:
`R5F and C66x cores have a Region Address Translator (RAT) to access memory and
peripheral regions beyond the 32-bit address space. Add one instance per region
that requires translation. The RAT hardware base address is derived automatically
from the core context. Refer to the device TRM for regions that need translation.

With "Automate RAT Setting" enabled in the Memory Region tab, an instance can
instead be added under a memory region. Its local address and size then follow
that region, and only the translated system address has to be entered.`,
    maxInstances: 16,
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
            longDescription:
"32-bit local address as seen by the CPU. Must be aligned to the region size " +
"and must not overlap with other configured regions.",
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
            longDescription:
"48-bit SoC-level physical address this region maps to. Must be aligned to the region size.",
            default: 0x0,
            displayFormat: "hex",
        },
        {
            name: "size",
            displayName: "Region Size",
            description: "Size of the region to translate (must be a power of 2)",
            longDescription:
"In Automate RAT mode this is set automatically to the owning memory region " +
"size rounded up to a power of 2.",
            default: 29,
            onChange: changeDisplay,
            options: [
                { name: 5,  displayName: "32 B"   },
                { name: 6,  displayName: "64 B"   },
                { name: 7,  displayName: "128 B"  },
                { name: 8,  displayName: "256 B"  },
                { name: 9,  displayName: "512 B"  },
                { name: 10, displayName: "1 KB"   },
                { name: 11, displayName: "2 KB"   },
                { name: 12, displayName: "4 KB"   },
                { name: 13, displayName: "8 KB"   },
                { name: 14, displayName: "16 KB"  },
                { name: 15, displayName: "32 KB"  },
                { name: 16, displayName: "64 KB"  },
                { name: 17, displayName: "128 KB" },
                { name: 18, displayName: "256 KB" },
                { name: 19, displayName: "512 KB" },
                { name: 20, displayName: "1 MB"   },
                { name: 21, displayName: "2 MB"   },
                { name: 22, displayName: "4 MB"   },
                { name: 23, displayName: "8 MB"   },
                { name: 24, displayName: "16 MB"  },
                { name: 25, displayName: "32 MB"  },
                { name: 26, displayName: "64 MB"  },
                { name: 27, displayName: "128 MB" },
                { name: 28, displayName: "256 MB" },
                { name: 29, displayName: "512 MB" },
                { name: 30, displayName: "1 GB"   },
                { name: 31, displayName: "2 GB"   },
                { name: 32, displayName: "4 GB"   },
            ],
        },
    ],

    validate: function (instance, report) {
        /* JS numbers are doubles, so addresses >= 0x80000000 stay positive
         * for % and comparisons. Do not use ">>> 0" here: it wraps values
         * above 32 bits and would defeat the maxLocal check below. */
        let owned      = isOwnedByMemRegion(instance);
        let addrField  = owned ? "localAddr_mr" : "localAddr";
        let localAddr  = getLocalAddr(instance);
        let systemAddr = instance.systemAddr;
        let size       = Math.pow(2, instance.size);
        let maxLocal   = 0x100000000;
        let maxSystem  = 0x1000000000000;

        if ((localAddr % size) !== 0)
            report.logError(
                `Local address must be aligned to 0x${size.toString(16).toUpperCase()}` +
                (owned ? " (memory region size rounded up to a power of 2). " +
                         "Align the memory region start address or reduce its size." : ""),
                instance, addrField);

        if ((systemAddr % size) !== 0)
            report.logError(
                `System address must be aligned to 0x${size.toString(16).toUpperCase()}`,
                instance, "systemAddr");

        if (localAddr >= maxLocal)
            report.logError(
                `Local address must be <= 0x${(maxLocal - 1).toString(16).toUpperCase()}`,
                instance, addrField);

        if (systemAddr >= maxSystem)
            report.logError(
                `System address must be <= 0x${(maxSystem - 1).toString(16).toUpperCase()}`,
                instance, "systemAddr");

        if (systemAddr === 0)
            report.logWarning(
                "Translated system address is 0x0. Enter the system address this region maps to.",
                instance, "systemAddr");

        /* The rounded-up size is clamped to 4 GB, so a larger memory region
         * would otherwise be only partly covered without any message. */
        if (owned && size < instance.$ownedBy.size)
            report.logError(
                `RAT region size 0x${size.toString(16).toUpperCase()} does not cover the ` +
                `memory region size 0x${instance.$ownedBy.size.toString(16).toUpperCase()}`,
                instance, "size");

        let module = instance.$module;
        for (let other of module.$instances) {
            if (other === instance) continue;
            let otherLocal = getLocalAddr(other);
            let otherSize  = Math.pow(2, other.size);
            let thisSize   = Math.pow(2, instance.size);
            if (
                (localAddr >= otherLocal &&
                 localAddr <  otherLocal + otherSize) ||
                (otherLocal >= localAddr &&
                 otherLocal <  localAddr + thisSize)
            ) {
                report.logError("Overlapping local address regions are not allowed",
                    instance, addrField);
            }
        }
    },

    getRatBaseAddr,
    getLocalAddr,
};

exports = rat_cfg_module;
