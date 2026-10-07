let path = require('path');

let device = "j722s";

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
        "..",       /* core_os_combo base */
        "../../..", /* Example base */
        "../../../../../../source/ip_fma/src",
        "../../../../../../examples/sdl/dpl",
    ],
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "IP_FMA_CLK_PAGE";

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
        "${MCU_PLUS_SDK_PATH}/source/sdl/lib",
    ],
};

const libs_nortos_mcu_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "sdl.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
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
    property.name = "clk_ip_fma_app";
    property.isInternal = false;
    property.description = "Clock IP FMA application example";
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