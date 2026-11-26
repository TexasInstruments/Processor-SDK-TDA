let path = require('path');

let device = "j722s";

/*
 * drivers_ipfma: readback-enabled build of the 'drivers' library.
 *
 * Built in parallel to 'drivers' lib:
 * it compiles exactly the same source files, defines and cflags as 'drivers' lib
 * for the given core, and additionally defines REG_READBACK_ENABLED together
 * with a REG_READBACK_MODULE_MASK selecting the IP FMA modules for readback.
 * An application links EITHER 'drivers' OR 'drivers_ipfma' lib, never both.
 *
 * Readback module selection (see drivers/hw_include/reg_readback.h):
 *   - Each module is assigned a unique bit (e.g. REG_RB_MOD_DSS = 0x4).
 *   - A source file identifies its module by defining REG_READBACK_MODULE
 *     before including reg_readback.h:
 *         #define REG_READBACK_MODULE  REG_RB_MOD_DSS
 *         #include <drivers/hw_include/reg_readback.h>
 *   - This library defines, per core, the set of enabled modules:
 *         REG_READBACK_MODULE_MASK = (REG_RB_MOD_DSS | ...)
 *   - The readback check is compiled in only if the module bit is set in
 *     REG_READBACK_MODULE_MASK. All other source files are compiled
 *     identically to the 'drivers' library.
 *
 * Modules enabled per core:
 *   - main-r5f : REG_RB_MOD_DSS   (dss/v1/hw_include/V3/csl_dssCommon.c)
 *   - wkup-r5f : REG_RB_MOD_LPDDR (ddr/cdn_drv/common/lpddr4.c)
 *
 * To enable read-back for another module/core, add the core to
 * buildOptionCombos and its module bit to readback_module_mask below.
 */
const driversProject = require('../../.project/project.js');

const buildOptionCombos = [
    { device: device, cpu: "main-r5f", cgt: "ti-arm-clang"},
    { device: device, cpu: "wkup-r5f", cgt: "ti-arm-clang"},
];

/* REG_READBACK_MODULE_MASK per core (key = buildOption.cpu) */
const readback_module_mask = {
    "main-r5f": "(REG_RB_MOD_DSS)",
    "wkup-r5f": "(REG_RB_MOD_LPDDR)",
};

/* 'drivers' filedirs are relative to source/drivers; this library is one level
 * deeper (source/drivers/drivers_ipfma), so prefix relative paths with "../". */
function prefixFiledirs(filedirs) {
    let out = {};
    for (const key in filedirs) {
        out[key] = filedirs[key].map(
            (dir) => (dir.startsWith("$") || dir.startsWith("/")) ? dir : `../${dir}`);
    }
    return out;
}

function getComponentProperty() {
    let property = driversProject.getComponentProperty(device);

    property.dirPath = path.resolve(__dirname, "..");
    property.name = "drivers_ipfma";
    property.isInternal = false;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = driversProject.getComponentBuildProperty(buildOption);

    if (build_property.filedirs) {
        build_property.filedirs = prefixFiledirs(build_property.filedirs);
    }

    const mask = readback_module_mask[buildOption.cpu];
    if (mask) {
        build_property.defines = {
            ...(build_property.defines || {}),
            common: [
                ...((build_property.defines && build_property.defines.common) || []),
                "REG_READBACK_ENABLED",
                `REG_READBACK_MODULE_MASK='${mask}'`,
            ],
        };
    }

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};