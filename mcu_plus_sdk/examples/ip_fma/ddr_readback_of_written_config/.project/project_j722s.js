let path = require('path');

let device = "j722s";

/*
 * DDR "Software Read-back of Written Configuration" safety-mechanism example.
 *
 * This example is built as a minimal bootloader (like sbl_null): it is the
 * first image the ROM runs, so DDR is NOT yet up when it executes. That is the
 * only context in which DDR_init() actually applies the controller/PHY/PI
 * register configuration (DDR_isEnabled() is false), and therefore the only
 * context in which the write-config read-back fires - in any ordinary
 * application, run after the SBL, DDR is already up and the config writes (and
 * the read-back) are skipped.
 *
 * It links the 'drivers_ipfma' library (source/drivers/drivers_ipfma) INSTEAD
 * of the stock 'drivers' library. drivers_ipfma is the read-back-enabled build
 * of 'drivers' (same sources), with REG_READBACK_ENABLED +
 * REG_READBACK_MODULE_MASK=(REG_RB_MOD_LPDDR), so DDR_init() -> LPDDR4_WriteReg()
 * reads back every written controller/PHY/PI register. main_baremetal.c overrides
 * LPDDR4_ddrReadbackErr() to record mismatches and reports the result once the
 * console UART is open.
 */
const files = {
    common: [
        "main_baremetal.c",
    ],
};

/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",       /* core_os_combo base */
        "../../..", /* Example base */
    ],
};

const libdirs_nortos = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/drivers_ipfma/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciclient_direct/sbl/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/sbl/lib",
    ],
};

const libs_nortos_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        /* read-back-enabled build of 'drivers' - used INSTEAD of drivers.lib */
        "drivers_ipfma.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "sciclient_direct_sbl.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "rm_pm_hal_sbl.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const templates_bootloader =
[
    {
		input: ".project/templates/j722s/common/bootloader_linker.cmd.xdt",
		output: "linker.cmd",
		options: {
			heapSize: 0x8000,
			stackSize: 0x4000,
			irqStackSize: 0x1000,
			svcStackSize: 0x0100,
			fiqStackSize: 0x0100,
			abortStackSize: 0x0100,
			undefinedStackSize: 0x0100,
		},
	}
]

const defines = {
    common:[
        "ENABLE_SCICLIENT_DIRECT",
    ]
}

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "IP_FMA_DDR_RB_WRITTEN_CONFIG_PAGE";

const buildOptionCombos = [
    { device: device, cpu: "wkup-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "nortos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "ddr_readback_of_written_config";
    property.isInternal = false;
    property.isBootLoader = true;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.files = files;
    build_property.filedirs = filedirs;
    build_property.libdirs = libdirs_nortos;
    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.defines = defines;
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;
    build_property.templates = templates_bootloader;
    build_property.libs = libs_nortos_r5f;

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
