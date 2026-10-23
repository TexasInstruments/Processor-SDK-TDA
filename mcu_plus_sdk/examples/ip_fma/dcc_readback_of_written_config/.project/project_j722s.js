let path = require('path');

let device = "j722s";

/*
 * This example shares the SAME main_baremetal.c as examples/ip_fma/clk_ip_fma_app.
 * The only difference is the linked library:
 *   - clk_ip_fma_app links the plain 'sdl' library  -> periodic readback only.
 *   - this example links 'sdl_ipfma'                -> the SDL DCC APIs additionally
 *     read back every written configuration register (REG_READBACK), i.e. the
 *     "Software Read-back of Written Configuration" safety mechanism.
 * The read-back error callback SDL_DCC_readbackErrCbk(), which sdl_ipfma requires
 * the application to provide, is defined in the shared main_baremetal.c (it is
 * harmless dead code in the plain-'sdl' clk_ip_fma_app build).
 */
const files = {
    common: [
        "main_baremetal.c",
        "ip_fma_clk.c",
        "ip_fma_common.c",
        "dpl_interface.c",
    ],
};

/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",                                    /* core_os_combo base */
        "../../../../clk_ip_fma_app",            /* shared main_baremetal.c */
        "../../../../../../source/ip_fma/src",   /* ip_fma_clk.c, ip_fma_common.c */
        "../../../../../../examples/sdl/dpl",    /* dpl_interface.c */
    ],
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "SDL_DCC_RB_WRITTEN_CONFIG_PAGE";

const includes_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/ip_fma/inc",
        "${MCU_PLUS_SDK_PATH}/examples/sdl/dpl",
    ],
};

const defines_mcu_r5f = {
    common: [
        "R5F_CORE",
        "BUILD_MCU_R5",
    ],
};

const libdirs_nortos_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/sdl/sdl_ipfma/lib",
    ],
};

const libs_nortos_mcu_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "sdl_ipfma.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const templates_nortos_mcu_r5f =
[
    {
        input: ".project/templates/j722s/common/linker_mcu-r5f_nortos.cmd.xdt",
        output: "linker.cmd",
        options: {
            heapSize: 0x8000,
            stackSize: 0x4000,
            irqStackSize: 0x0100,
            svcStackSize: 0x1000,
            fiqStackSize: 0x0100,
            abortStackSize: 0x0100,
            undefinedStackSize: 0x0100,
            dmStubstacksize: 0x0400,
        },
    },
];

const buildOptionCombos = [
    { device: device, cpu: "mcu-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "nortos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "dcc_readback_of_written_config";
    property.isInternal = false;
    property.description = "DCC Software read-back of written configuration Safety Mechanism example (shares clk_ip_fma_app main, links sdl_ipfma)";
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.files = files;
    build_property.filedirs = filedirs;
    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;

    if(buildOption.cpu.match(/mcu-r5f*/)) {
        if(buildOption.os.match(/nortos*/)) {
            build_property.includes = includes_mcu_r5f;
            build_property.defines = defines_mcu_r5f;
            build_property.libdirs = libdirs_nortos_mcu_r5f;
            build_property.libs = libs_nortos_mcu_r5f;
            build_property.templates = templates_nortos_mcu_r5f;
        }
    }
    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
