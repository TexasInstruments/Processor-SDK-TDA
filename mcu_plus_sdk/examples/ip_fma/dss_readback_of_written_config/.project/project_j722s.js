let path = require('path');

let device = "j722s";

/*
 * DSS "Software Read-back of Written Configuration" safety-mechanism example.
 *
 * A separate build target from dss_display_test, so automation has a distinct
 * app that exercises the DSS read-back SM. It reuses the dss_display_test
 * application sources UNCHANGED (dss_display_test_v1.c, dss_display_pattern.c)
 * via filedirs, and adds only its own main_baremetal.c (entry point + the
 * read-back error callback).
 *
 * It links the drivers_ipfma library (source/drivers/drivers_ipfma) which is the
 * same drivers build with readback enabled for the DSS module on main-r5f. On a
 * mismatch csl_dssCommon.c calls CSL_DSSReadbackErr(), which main_baremetal.c
 * defines - the library supplies no default, so the build requires it.
 */
const files_nortos_main_r5 = {
    common: [
        "dss_display_test_v1.c",    /* reused from dss_display_test (unchanged) */
        "dss_display_pattern.c",    /* reused from dss_test_pattern (unchanged) */
        "main_baremetal.c",         /* this example's own source: entry + callback */
    ],
};

/* Relative to where the makefile will be generated:
 * examples/ip_fma/dss_readback_of_written_config/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang
 */
const filedirs_nortos_main_r5 = {
    common: [
        "..",                                           /* core_os_combo base */
        "../../..",                                      /* example base (main_baremetal.c) */
        "../../../../../drivers/dss/dss_display_test",   /* dss_display_test_v1.c */
        "../../../../../drivers/dss/dss_test_pattern",   /* dss_display_pattern.c */
    ],
};

const libdirs_nortos_main_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/drivers_ipfma/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
    ],
};

/* drivers_ipfma replaces the drivers library (never link both). */
const libs_nortos_main_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers_ipfma.j722s.main-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const syscfgfile = "../example.syscfg"

const readmeDoxygenPageTag = "IP_FMA_DSS_RB_WRITTEN_CONFIG_PAGE";

const templates_nortos_main_r5f =
[
    {
        input: ".project/templates/j722s/common/linker_main-r5f_nortos.cmd.xdt",
        output: "linker.cmd",
        options: {
            heapSize: 0x10000,
            stackSize: 0x8000,
            irqStackSize: 0x1000,
            svcStackSize: 0x1000,
            fiqStackSize: 0x0100,
            abortStackSize: 0x0100,
            undefinedStackSize: 0x0100,
            dmStubstacksize: 0x0400,
            dssFrameBuf: "true"
        },
    },
];

const buildOptionCombos = [
    { device: device, cpu: "main-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "nortos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "dss_readback_of_written_config";
    property.isInternal = false;
    property.tirexResourceSubClass = [ "example.gettingstarted" ];
    property.description = "DSS Software read-back of written configuration Safety Mechanism example (reuses dss_display_test sources, links drivers_ipfma)";
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;

    if(buildOption.cpu.match(/main-r5f*/)) {
        if(buildOption.os.match(/nortos*/)) {
            build_property.files = files_nortos_main_r5;
            build_property.filedirs = filedirs_nortos_main_r5;
            build_property.libdirs = libdirs_nortos_main_r5f;
            build_property.libs = libs_nortos_main_r5f;
            build_property.templates = templates_nortos_main_r5f;
        }
    }
    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
