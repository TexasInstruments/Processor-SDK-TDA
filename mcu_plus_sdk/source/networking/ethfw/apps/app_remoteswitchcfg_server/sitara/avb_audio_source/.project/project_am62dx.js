let path = require('path');

let device = "am62dx";

const files = {
    common: [
        "sbl_ospi_stage2.c",
        "sbl_emmc_stage2.c",
        "sbl_stage2_common.c",
        "main_server.c",
        "main.c",
        "app_cpswconfighandler.c",
        "ethfw_avtp.c",
        "shm_cirbuf.c",
    ],
};

const remote_files = {
    common: [
        "main.c",
        "shm_cirbuf.c",
        "remote_main.c"
    ],
};
/* Relative to where the makefile will be generated
 * Typically at <example_folder>/<BOARD>/<core_os_combo>/<compiler>
 */
const filedirs = {
    common: [
        "..",             /* core_os_combo base */
        "$(MCU_PLUS_SDK_PATH)/source/networking/ethfw/apps/app_remoteswitchcfg_server/sitara",
        "$(MCU_PLUS_SDK_PATH)/source/networking/ethfw/apps/app_remoteswitchcfg_server",
        "$(MCU_PLUS_SDK_PATH)/examples/drivers/boot/common/soc/am62dx", /* 2nd stage bootloader */
    ],
};

const remote_filedirs = {
    common: [
        "..",          /* core_os_combo base */
        "$(MCU_PLUS_SDK_PATH)/source/networking/ethfw/apps/app_remoteswitchcfg_server/sitara/avb_audio_source",
        "$(MCU_PLUS_SDK_PATH)/source/networking/ethfw/apps/app_remoteswitchcfg_server/sitara",
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

const libdirs_freertos_dm_r5f = {
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
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/am62dx/r5f",
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
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/soc/k3/am62dx",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/mdio/V4",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/eval_inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/eval_inc/tsn_l2",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_unibase",
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
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-config/am62dx",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipific_tap/inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/intercore/include",
    ],
};

const includes_freertos_c75 = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/FreeRTOS-Kernel/include",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/portable/TI_CGT/DSP_C75X",
        "${MCU_PLUS_SDK_PATH}/source/kernel/freertos/config/am62dx/c75x",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/apps/app_remoteswitchcfg_server/sitara",
    ],
};

const libs_freertos_dm_r5f = {
    common: [
		"rm_pm_hal.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
		"sciclient_direct.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
		"self_reset.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "sciserver.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
		"freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
		"drivers.am62dx.dm-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "dm_stub.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "enet-cpsw.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "libc.a",
        "libsysbm.a",
        "tsn_combase-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_unibase-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_gptp-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_uniconf-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "tsn_l2-freertos.am62dx.r5f.ti-arm-clang.lib",
        "lwipif-cpsw-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "lwip-contrib-freertos.am62dx.r5f.ti-arm-clang.${ConfigName}.lib",
        "ethfw.am62dx.dm-r5f.ti-arm-clang.${ConfigName}.lib",
        "yangemb-freertos.am62dx.r5f.ti-arm-clang.lib",
    ],
};

const libs_freertos_c75 = {
    common: [
        "freertos.am62dx.c75x.ti-c7000.${ConfigName}.lib",
        "drivers.am62dx.c75x.ti-c7000.${ConfigName}.lib",
    ],
};

const linker_includePath_freertos = {
    common: [
        "${PROJECT_BUILD_DIR}/syscfg",
    ],
};

const defines_dm_r5f = {
    common: [
        "UB_LOGCAT=5",
        "SOC_AM62DX",
        "ENABLE_SCICLIENT_DIRECT",
        "R5F_CORE",
        "ENET_ENABLE_PER_CPSW=1",
        "PRINT_FORMAT_NO_WARNING",
        "SITARA",
        "GPTP_ENABLED=1",
        "MCU_PLUS_SDK",
        "ETHFW_PROXY_ARP_SUPPORT",
        "ETHFW_INTERCORE_ETH_SUPPORT",
        "ETHAPP_ENABLE_INTERCORE_ETH",
        "ENABLE_ETHFW_PROXYARP",
        "CPU_mcu2_0",
        "ETHAPP_ENABLE_IPERF_SERVER",
        "ETHFW_GPTP_SUPPORT",
        "ENABLE_MAC_ONLY_PORTS",
        "AVTP_ENABLED=1",
        "AVTP_TALKER_MODE",
        "NO_GETOPT_LONG=1",
        "AVTP_HAVE_NO_SIGNAL=1",
        "AVTP_DIRECT_MODE=1",
        "AUTOAMP_APP_ENABLED=1",
    ],
};

const defines_c75 = {
    common: [
        "SOC_AM62DX",
    ],
};

const cflags_dm_r5f = {
    common: [
        "--include tsn_buildconf/sitara_buildconf.h",
    ],
    release: [
        "-Oz",
        "-flto",
    ],
};

const lflags_dm_r5f = {
    common: [
        "--zero_init=on",
        "--use_memset=fast",
        "--use_memcpy=fast"
    ],
};

const loptflags_dm_r5f = {
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

const templates_freertos_dm_r5f =
[

];

const templates_freertos_c75 =
[
    {
        input: "source/networking/enet/core/sysconfig/.project/templates/freertos/main_freertos.c.xdt",
        output: "../main.c",
        options: {
            entryFunction: "RemoteApp_mainTask",
            stackSize: 64*1024,
        },
    }
];

const buildOptionCombos = [
    { device: device, cpu: "r5fss0-0", cgt: "ti-arm-clang", board: "am62dx-evm", os: "freertos"},
    { device: device, cpu: "c75ss0-0", cgt: "ti-c7000",     board: "am62dx-evm", os: "freertos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "executable";
    property.name = "ethfw_server_audio_source";
    property.isInternal = false;
    property.isLinuxInSystem = false;
    property.isLinuxFwGen = false;
    property.ipcVringRTOS = true;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    if(buildOption.cpu.match(/r5f*/))
    {
        build_property.files = files;
        build_property.filedirs = filedirs;
    }
    else if(buildOption.cpu.match(/c75*/))
    {
        build_property.files = remote_files;
        build_property.filedirs = remote_filedirs;
    }

    build_property.lnkfiles = lnkfiles;
    build_property.syscfgfile = syscfgfile;
    build_property.projecspecFileAction = "link";
    build_property.readmeDoxygenPageTag = readmeDoxygenPageTag;

    if(buildOption.cpu.match(/r5fss0-0/)) {
        if(buildOption.os.match(/freertos*/) )
        {
            const _ = require('lodash');
            let libdirs_freertos_cpy = _.cloneDeep(libdirs_freertos_dm_r5f);
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
            build_property.libs = libs_freertos_dm_r5f;
            build_property.templates = templates_freertos_dm_r5f;
            build_property.defines = defines_dm_r5f;
            build_property.cflags = cflags_dm_r5f;
            build_property.lflags = lflags_dm_r5f;
            build_property.projectspecLnkPath = linker_includePath_freertos;
            build_property.loptflags = loptflags_dm_r5f;
        }
    }
    if (buildOption.cpu.match(/c75*/))
    {
        build_property.defines = defines_c75;
        build_property.includes = includes_freertos_c75;
        build_property.libdirs = libdirs_freertos;
        build_property.libs = libs_freertos_c75;
        build_property.templates = templates_freertos_c75;
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