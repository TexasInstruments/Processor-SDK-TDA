let path = require('path');

let device = "am62px";

const files = {
    common: [
        "sbl_ospi_linux_stage2.c",
        "main_server.c",
        "main.c",
        "app_cpswconfighandler.c",
    ],
};

/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",       /* core_os_combo base */
        "../../..", /* mcu_2_0 base */
        "../../../..", /* apps base */
        "../../../../..", /* ethfw base */
        "$(MCU_PLUS_SDK_PATH)/examples/drivers/boot/common/soc/am62px", /* 2nd stage bootloader */
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
        "${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/dm_stub/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/eval_lib",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/license_lib",
    ],
};

const includes_freertos_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/am62px/r5f",
        "${MCU_PLUS_SDK_PATH}/source/drivers",
        "${MCU_PLUS_SDK_PATH}/source/drivers/udma",
        "${MCU_PLUS_SDK_PATH}/source/drivers/hw_include",
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/port",
        "${MCU_PLUS_SDK_PATH}/source/board/ethphy/enet/rtos_drivers/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/utils/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/phy",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/core",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/soc/k3/am62px",
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
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-config/am62px",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipific_tap/inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/intercore/include",
    ],
};

const libs_freertos_r5f = {
    common: [
        "freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "enet-cpsw.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "board.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "libc.a",
        "libsysbm.a",
        "tsn_combase-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_unibase-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_gptp-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_uniconf-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "lwipif-cpsw-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-contrib-freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "ethfw.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};

const libs_freertos_wkup_r5f = {
    common: [
        "freertos.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "sciserver.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "sciclient_direct.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "rm_pm_hal.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "self_reset.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "dm_stub.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.am62px.r5f.ti-arm-clang.${ConfigName}.lib",
        "enet-cpsw.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "libc.a",
        "libsysbm.a",
        "tsn_combase-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_unibase-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_gptp-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_uniconf-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwipif-cpsw-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-contrib-freertos.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "ethfw.am62px.wkup-r5f.ti-arm-clang.${ConfigName}.lib",
        "yangemb-freertos.am62px.wkup-r5f.ti-arm-clang.lib",
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
        'ETHAPP_ENABLE_IPERF_SERVER',
    ],
};

const defines_wkup_r5f = {
    common: [
        "ENABLE_SCICLIENT_DIRECT",
        "R5F_CORE",
        "ENET_ENABLE_PER_CPSW=1",
        'PRINT_FORMAT_NO_WARNING',
        'SITARA',
        'GPTP_ENABLED=1',
        "MCU_PLUS_SDK",
        "ETHFW_PROXY_ARP_SUPPORT",
        "ETHFW_INTERCORE_ETH_SUPPORT",
        "ETHAPP_ENABLE_INTERCORE_ETH",
        "ENABLE_ETHFW_PROXYARP",
        "CPU_mcu2_0",
        "ETHAPP_ENABLE_IPERF_SERVER",
        "ETHFW_GPTP_SUPPORT",
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
        input: ".project/templates/am62px/freertos/main_freertos.c.xdt",
        output: "../main.c",
        options: {
            entryFunction: "EthApp_initTaskFxn",
            taskPri : "2",
            stackSize : "16384",
        },
    },
];

const templates_freertos_wkup_r5f =
[
    {
        input: ".project/templates/am62px/freertos/main_freertos_dm.c.xdt",
        output: "../main_server.c",
        options: {
            entryFunction: "EthApp_initTaskFxn",
            taskPri : "2",
            stackSize : "16384",
            dmWithBootloader: "true",
            skipDriversClose: "true",
            dmWithEthfw: "true",
        }
    },
];

const buildOptionCombos = [
    { device: device, cpu: "wkup-r5fss0-0", cgt: "ti-arm-clang", board: "am62px-sk", os: "freertos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "ethfw_server";
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

    if(buildOption.cpu.match(/wkup-r5fss0-0/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            const _ = require('lodash');
            let libdirs_freertos_cpy = _.cloneDeep(libdirs_freertos_wkup_r5f);
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
            build_property.libs = libs_freertos_wkup_r5f;
            build_property.templates = templates_freertos_wkup_r5f;
            build_property.defines = defines_wkup_r5f;
            build_property.cflags = cflags_wkup_r5f;
            build_property.lflags = lflags_wkup_r5f;
            build_property.projectspecLnkPath = linker_includePath_freertos;
            build_property.loptflags = loptflags_wkup_r5f;
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
