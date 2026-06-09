let path = require('path');

let device = "am275x";

const files = {
    common: [
        "empty.c",
        "main.c",
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
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
    ],
};

const libs_nortos_r5f = {
    common: [
        "nortos.am275x.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.am275x.r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "EXAMPLES_EMPTY";

const templates_nortos_r5f =
[
    {
        input: ".project/templates/am275x/common/linker_main-r5f_nortos.cmd.xdt",
        output: "linker.cmd",
    },
    {
        input: ".project/templates/am275x/nortos/main_nortos.c.xdt",
        output: "../main.c",
        options: {
            entryFunction: "empty_main",
        },
    }
];

const buildOptionCombos = [
    { device: device, cpu: "r5fss0-0", cgt: "ti-arm-clang", board: "am275x-evm", os: "nortos"},
];

const systemProjects = [
    {
        name: "empty",
        tag: "nortos",
        skipProjectSpec: false,
        readmeDoxygenPageTag: readmeDoxygenPageTag,
        board: "am275x-evm",
        projects: [
            { device: device, cpu: "r5fss0-0", cgt: "ti-arm-clang", board: "am275x-evm", os: "nortos"},
        ],
    },
]

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "empty";
    property.isInternal = false;
    property.tirexResourceSubClass = [ "example.gettingstarted" ];
    property.description = "An Empty Example. This example is intended to be a starting point for new development"
    property.skipProjectSpec = true;
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
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;
    if(buildOption.cpu.match(/r5f*/)) {
        build_property.libs = libs_nortos_r5f;
        build_property.templates = templates_nortos_r5f;
    }
    return build_property;
}

function getSystemProjects(device)
{
    return null;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
    getSystemProjects,
};
