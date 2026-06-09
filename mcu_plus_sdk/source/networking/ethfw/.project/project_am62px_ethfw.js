let path = require('path');

let device = "am62px";

const files = {
    common: [
        /* apps */
        "ipc_trace.c",

        /* ethremotecfg */
        "cpsw_proxy_server.c",
        "cpsw_proxy.c",
        "ethfw_api.c",
        "ethfw_arp.c",
        "ethfw_mcast.c",
        "ethfw_monitor.c",
        "ethfw_vlan.c",
        "ethfw_tsn.c",
        "ethfw_portmirror.c",

        /* Utils */
        "ethfw_osal.c",
        "ethfw_ipc.c",
        /* board */
        "board_am62px_evm.c",

        /* ethfw_callbacks */
        "ethfw_callbacks_lwipif.c",

        /* ethfw_common */
        "ethfw_trace.c",

        /* MCM */
        "enet_mcm.c",
        
        /* LwIP IC */
        "bufpool.c",
        "lwip2enet_ic.c",
        "lwip2lwipif_ic.c",
        
        /* Intercore */
        "ic_queue_data.c",
        "ic_queue.c",
        "pbufQ_ic.c",
        "custom_pbuf_ic.c",
    ],
};

const filedirs = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/utils/V2",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipific_tap/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/intercore/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/apps/ipc_cfg",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/ethremotecfg/server/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/ethremotecfg/client/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/board/src/am62px",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/console_io/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_callbacks/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_common/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_estdemo/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_stats/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/intervlan/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/mem/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/perf_stats/src",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/remote_services/src",
    ],
};

const includes = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/ethremotecfg/client/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/ethremotecfg/server/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/board/src/am62px",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/console_io/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_abstract/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_callbacks/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_common/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_estdemo/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/ethfw_stats/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/intervlan/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/mem/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/perf_stats/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/ethfw/utils/remote_services/include",


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
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/core",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/include/phy",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/src/phy",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include",

        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/cpsw/V5",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/cpsw/V5/V5_0",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/cpts",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/hw_include/mdio",

        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_combase",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_combase/tilld",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_combase/tilld/sitara",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp/tilld",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_gptp/gptpconf",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_uniconf",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_uniconf/yangs",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_uniconf/yangs/generated",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_unibase",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_l2",
        "${MCU_PLUS_SDK_PATH}/source/networking/tsn/tsn-stack/tsn_l2/l2conf",

        "${MCU_PLUS_SDK_PATH}/source/networking/enet/soc/k3/am62px",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-stack/src/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-port/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-port/freertos/include",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipif/inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-stack/contrib",
        "${MCU_PLUS_SDK_PATH}/source/networking/lwip/lwip-config/am62px",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/lwipific_tap/inc",
        "${MCU_PLUS_SDK_PATH}/source/networking/enet/core/intercore/include",
    ],
};


const cflags = {
    common: [
        "-Wno-extra",
        "-Wno-error=unused-but-set-variable",
        "-Wno-unused-but-set-variable",
        "-Wno-error=address-of-packed-member",
        "-Wno-address-of-packed-member",
        "-Wno-visibility",
    ],
    release: [
        "-Oz",
        "-flto",
    ],
};

const cflags_a53 = {
    common: [
        "-Wno-extra",
        "-Wno-error=unused-but-set-variable",
        "-Wno-unused-but-set-variable",
        "-Wno-unused-function",
    ],
    release: [
        "-flto",
    ],
};

const defines_r5f = {
    common: [
        "MAKEFILE_BUILD",
        "ENET_CFG_ASSERT=1",
        "ENET_CFG_PRINT_ENABLE",
        "ENET_CFG_TRACE_LEVEL=3",
        "ENET_ENABLE_PER_CPSW=1",
        "ENABLE_ENET_LOG",
        "FREERTOS",
        "ETHFW_PROXY_ARP_SUPPORT",
        "ETHFW_INTERCORE_ETH_SUPPORT",
        "ETHFW_IPERF_SERVER_SUPPORT",
        "MCU_PLUS_SDK",
        "SOC_AM62PX"
    ],
    debug: [
        "ENET_CFG_DEV_ERROR=1",
        "LWIPIF_INSTRUMENTATION_ENABLED=1",
        "ENETDMA_INSTRUMENTATION_ENABLED=1",
    ],
};

const defines_wkup_r5f = {
    common: [
        "MAKEFILE_BUILD",
        "ENET_CFG_ASSERT=1",
        "ENET_CFG_PRINT_ENABLE",
        "ENET_CFG_TRACE_LEVEL=3",
        "ENET_ENABLE_PER_CPSW=1",
        "ENABLE_ENET_LOG",
        "FREERTOS",
        "ENABLE_SCICLIENT_DIRECT",
        "ETHFW_PROXY_ARP_SUPPORT",
        "ETHFW_INTERCORE_ETH_SUPPORT",
        "ETHFW_IPERF_SERVER_SUPPORT",
        "ETHFW_PROXY_ARP_HANDLING",
        "ETHFW_GPTP_SUPPORT",
        "CPU_mcu2_0",
        "MCU_PLUS_SDK",
        "SOC_AM62PX"
    ],
    debug: [
        "ENET_CFG_DEV_ERROR=1",
        "LWIPIF_INSTRUMENTATION_ENABLED=1",
        "ENETDMA_INSTRUMENTATION_ENABLED=1",
    ],
};


const defines_a53 = {
    common: [
        "MAKEFILE_BUILD",
        "ENET_CFG_ASSERT=1",
        "ENET_CFG_PRINT_ENABLE",
        "ENET_CFG_TRACE_LEVEL=3",
        "ENET_ENABLE_PER_CPSW=1",
        "ENABLE_ENET_LOG",
    ],
    debug: [
        "ENET_CFG_DEV_ERROR=1",
        "LWIPIF_INSTRUMENTATION_ENABLED=1",
        "ENETDMA_INSTRUMENTATION_ENABLED=1",
    ],
};

const buildOptionCombos = [
    { device: device, cpu: "wkup-r5f", cgt: "ti-arm-clang"},
    { device: device, cpu: "mcu-r5f", cgt: "ti-arm-clang"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "library";
    property.name = "ethfw";
    property.tag = "cpsw";
    property.isInternal = false;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    build_property.filedirs = filedirs;
    build_property.files = files;
    build_property.includes = includes;
    if(buildOption.cpu.match(/r5f*/)) {
        build_property.cflags = cflags;
        build_property.defines = defines_r5f;
    }

    if(buildOption.cpu.match(/wkup-r5f*/)) {
        build_property.cflags = cflags;
        build_property.defines = defines_wkup_r5f;
    }

    if(buildOption.cpu.match(/a53*/)) {
        build_property.cflags = cflags_a53;
        build_property.defines = defines_a53;
    }

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
