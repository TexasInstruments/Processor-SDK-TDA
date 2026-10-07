let path = require('path');

let device = "j722s";

const files = {
    common: [
        "main_baremetal.c",
    ],
};

const filedirs = {
    common: [
        "..",
        "../../..",
    ],
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "IP_FMA_DRU_TR_OVERFLOW_PAGE";

const libdirs_nortos_main_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
    ],
};

const includes_nortos_main_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/ip_fma/inc",
    ],
};

const libs_nortos_main_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.main-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const templates_nortos_main_r5f =
[
    {
        input: ".project/templates/j722s/common/linker_main-r5f_nortos.cmd.xdt",
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
    { device: device, cpu: "main-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "nortos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "dru_tr_of";
    property.isInternal = false;
    property.tirexResourceSubClass = [ "example.gettingstarted" ];
    property.description = "A simple \"IP FMA DRU\" Example. "
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

    if(buildOption.cpu.match(/main-r5f*/)) {
        if(buildOption.os.match(/nortos*/)) {
            build_property.includes = includes_nortos_main_r5f;
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
