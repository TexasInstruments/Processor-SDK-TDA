let path = require('path');

let device = "j722s";

const files = {
    common: [
        "csirx_periodic_readback.c",
        "main_baremetal.c",
        "ip_fma_common.c",
        "ip_fma_csirx.c",
    ],
};

const filedirs = {
    common: [
        "..",
        "../../..",
        "$(MCU_PLUS_SDK_PATH)/source/ip_fma/src",
    ],
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "IP_FMA_CSIRX_PAGE";

const libdirs_mcu_nortos = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
    ],
};

const includes_nortos_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/ip_fma/inc",
    ],
};

const libdirs_freertos_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
    ],
};

const libs_nortos_mcu_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_mcu_r5f = {
    common: [
        "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.main-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
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
            irqStackSize: 0x1000,
            svcStackSize: 0x0100,
            fiqStackSize: 0x0100,
            abortStackSize: 0x0100,
            undefinedStackSize: 0x0100,
            dmStubstacksize: 0x0400,
        },
    }
];

const buildOptionCombos = [
    { device: device, cpu: "mcu-r5fss0-0",  cgt: "ti-arm-clang", board: "j722s-evm", os: "nortos"}
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "ip_fma_csirx";
    property.isInternal = false;
    property.tirexResourceSubClass = [ "example.gettingstarted" ];
    property.description = "A simple \"IP FMA CSIRX\" Example. "
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
        build_property.libs = libs_nortos_mcu_r5f;
        build_property.templates = templates_nortos_mcu_r5f;
        build_property.libdirs = libdirs_mcu_nortos;
        build_property.includes = includes_nortos_r5f;
    }
   
    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
