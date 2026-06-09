let path = require('path');

let device = "j722s";

const files = {
    common: [
        "main_client.c",
        "virtnetif_lwipif.c",
        "main.c",
    ],
};

/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",       /* core_os_combo base */
        "../../..", /* mcu_2_1 base */
        "../../../..", /* apps base */
        "../../../../..", /* ethfw base */
    ],
};

const libdirs_freertos = {
    common: [
        "generated",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/lib",
    ],
};

const libdirs_freertos_wkup_r5f = {
    common: [
        "generated",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciserver/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/rm_pm_hal/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/sciclient_direct/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/self_reset/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/lib",
    ],
};

const includes_freertos_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/j722s/r5f",
        "${MCU_PLUS_SDK_PATH}/source/drivers",
        "${MCU_PLUS_SDK_PATH}/source/drivers/udma",
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/port",
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/enet/rtos_drivers/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/utils/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/phy",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/core",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/soc/k3/j722s",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/mdio/V4",
        "${MCU_PLUS_SDK_PATH}/examples/networking/tsn",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp/tilld",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_combase/tilld/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp/gptpconf",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_uniconf",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_uniconf/yangs",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-stack/src/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-port/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-port/freertos/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipif/inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-stack/contrib",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-config/j722s",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract/j722s",
    ],
};

const libs_freertos_r5f = {
    common: [
             "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
             "drivers.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
             "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
             "enet-cpsw.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
             "lwipif-cpsw-freertos.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
             "lwip-freertos.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
             "lwip-contrib-freertos.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
             "ethfw.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_wkup_r5f = {
    common: [
        "freertos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "sciserver.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "sciclient_direct.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "rm_pm_hal.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "self_reset.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "enet-cpsw.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "libc.a",
        "libsysbm.a",
        "tsn_combase-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_unibase-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_gptp-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_uniconf-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwipif-cpsw-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-contrib-freertos.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "ethfw.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const linker_includePath_freertos = {
    common: [
        "${PROJECT_BUILD_DIR}/syscfg",
    ],
};

const defines_r5f = {
    common: [
        "ENET_ENABLE_PER_CPSW=1",
        'PRINT_FORMAT_NO_WARNING',
        'SITARA',
        'GPTP_ENABLED=1',
        "ETHAPP_ENABLE_IPERF_SERVER",
        "MCU_PLUS_SDK",
        "SOC_J722S",
        "CPU_mcu2_1",
    ],
};

const defines_wkup_r5f = {
    common: [
        "ENET_ENABLE_PER_CPSW=1",
        'PRINT_FORMAT_NO_WARNING',
        'SITARA',
        'GPTP_ENABLED=1',
        "ETHFW_PROXY_ARP_SUPPORT",
        "ETHFW_INTERCORE_ETH_SUPPORT",
        "ETHFW_IPERF_SERVER_SUPPORT",
        "CPU_mcu2_0",
        "ETHAPP_ENABLE_IPERF_SERVER",
        "SOC_J722S",
    ],
};

const cflags_r5f = {
    common: [
        "--include tsn_buildconf/sitara_buildconf.h",
    ],
    release: [
        "-Oz",
        "-flto",
    ],
};

const cflags_wkup_r5f = {
    common: [
        "--include tsn_buildconf/sitara_buildconf.h",
    ],
    release: [
        "-Oz",
        "-flto",
    ],
};

const lflags_r5f = {
    common: [
        "--zero_init=on",
        "--use_memset=fast",
        "--use_memcpy=fast"
    ],
};

const lflags_wkup_r5f = {
    common: [
        "--zero_init=on",
        "--use_memset=fast",
        "--use_memcpy=fast"
    ],
};

const loptflags_r5f = {
    release: [
        "-mcpu=cortex-r5",
        "-mfloat-abi=hard",
        "-mfpu=vfpv3-d16",
        "-mthumb",
        "-Oz",
        "-flto"
    ],
};

const loptflags_wkup_r5f = {
    release: [
        "-mcpu=cortex-r5",
        "-mfloat-abi=hard",
        "-mfpu=vfpv3-d16",
        "-mthumb",
        "-Oz",
        "-flto"
    ],
};

const lnkfiles = {
    common: [
        "linker.cmd",
    ]
};

const syscfgfile = "../example.syscfg";

const readmeDoxygenPageTag = "EXAMPLES_ENET_CPSW_TSN_LWIP_GPTP";

const templates_freertos_r5f =
[
    {
        input: ".project/templates/j722s/freertos/main_freertos.c.xdt",
        output: "../main_client.c",
        options: {
            entryFunction: "CpswRemoteApp_initTask",
            taskPri : "2",
            stackSize : "16384",
            skipDriversClose: "true",
        },
    },
];

const templates_freertos_wkup_r5f =
[
    {
        input: ".project/templates/j722s/freertos/main_freertos_wkup.c.xdt",
        output: "../main_server.c",
        options: {
            entryFunction: "EthApp_initTaskFxn",
            taskPri : "2",
            stackSize : "16384",
        },
    }
];

const buildOptionCombos = [
    { device: device, cpu: "mcu-r5fss0-0", cgt: "ti-arm-clang", board: "j722s-evm", os: "freertos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "ethfw_client";
    property.isInternal = false;
    property.isLinuxInSystem = true;
    property.isLinuxFwGen = true;
    property.ipcVringRTOS = true;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.files = files;
    build_property.filedirs = filedirs;
    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.projecspecFileAction = "link";
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;
    if(buildOption.cpu.match(/r5f*/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            const _ = require('lodash');
            let libdirs_freertos_cpy = _.cloneDeep(libdirs_freertos);
            /* Logic to remove generated/ from libdirs_freertos, it generates warning for ccs build */
            if (buildOption.isProjectSpecBuild === true)
            {
                var delIndex = libdirs_freertos_cpy.common.indexOf('generated');
                if (delIndex !== -1) {
                    libdirs_freertos_cpy.common.splice(delIndex, 1);
                }
            }
            build_property.includes = includes_freertos_r5f;
            build_property.libdirs = libdirs_freertos_cpy;
            build_property.libs = libs_freertos_r5f;
            build_property.templates = templates_freertos_r5f;
            build_property.defines = defines_r5f;
            build_property.cflags = cflags_r5f;
            build_property.lflags = lflags_r5f;
            build_property.projectspecLnkPath = linker_includePath_freertos;
            build_property.loptflags = loptflags_r5f;
        }
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
