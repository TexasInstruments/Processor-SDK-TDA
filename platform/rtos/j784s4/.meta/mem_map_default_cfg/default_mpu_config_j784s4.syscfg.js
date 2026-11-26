/*
 * Populates MPU region instances matching the static mpu_cfg.c files for
 * J784S4 R5F cores. The data here must stay in sync with vision_apps/platform/j784s4/rtos/.
 *
 * Called from mpu_cfg_container.syscfg.js after the mpu_cfg module instances
 * are created:
 *   populate_mpu_regions(containerInst, ind, device, core)
 */

/* --------------------------------------------------------------------------
 * Helper: set one mpu_cfg instance from a plain config object.
 * -------------------------------------------------------------------------- */
function applyRegion(inst, cfg) {
    if (cfg.name) inst.$name = cfg.name;

    inst.baseAddr         = cfg.addr || 0x0;
    inst.sizeBytes        = cfg.size;
    inst.exeNeverControl  = cfg.xn   !== undefined ? cfg.xn   : false;
    inst.subRegionEnable  = cfg.sub  !== undefined ? cfg.sub  : 0xFF;
    inst.accessPermission = cfg.ap   || "CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR";
    inst.attributes       = cfg.attr || "NonCached";

    if (cfg.attr === "ADVANCED") {
        inst.cacheable   = cfg.cacheable   !== undefined ? cfg.cacheable : false;
        inst.cachePolicy = cfg.cachePolicy || "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE";
        inst.shareable   = cfg.shareable   !== undefined ? cfg.shareable : false;
        inst.memAttr     = cfg.memAttr     || "0U";
    }
}

/* --------------------------------------------------------------------------
 * Lock-step cores (_0 suffix) use 64 KB TCMs.
 * Non-lock-step cores (_1 suffix) use 32 KB.
 * -------------------------------------------------------------------------- */
function tcmSize(core) {
    return core.endsWith("-0") ? 65536 : 32768;
}

/* --------------------------------------------------------------------------
 * FreeRTOS common regions — index 9 is per-core IPC (null placeholder).
 * mcu2_0 (r5fss0-0) adds 3 extra regions: INTERCORE_ETH cached+noncached, VISS.
 * mcu4_0 (r5fss2-0) adds 1 extra region: VISS.
 * -------------------------------------------------------------------------- */
function makeFreeRTOSRegions(core) {
    let tcm = tcmSize(core);
    return [
        {
            name: "BACKGROUND_REGION",
            addr: 0x0,
            size: 4294967296,   /* 4 GB */
            xn: true,
            attr: "StronglyOrdered",
        },
        {
            name: "R5F_ATCM",
            addr: 0x0,
            size: tcm,
            xn: false,
            attr: "Cached",
        },
        {
            name: "R5F_ATCM_GLOBAL_ALIAS",
            addr: 0x41000000,
            size: 32768,        /* 32 KB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "R5F_BTCM",
            addr: 0x41010000,
            size: tcm,
            xn: false,
            attr: "Cached",
        },
        {
            name: "MCU_OCSRAM",
            addr: 0x41C00000,
            size: 1048576,      /* 1 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "MSMC",
            addr: 0x70000000,
            size: 8388608,      /* 8 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "DDR_2GB",
            addr: 0x80000000,
            size: 2147483648,   /* 2 GB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "IPC_VRING",
            addr: 0xAC000000,   /* IPC_VRING_MEM_ADDR */
            size: 67108864,     /* 64 MB */
            xn: true,
            attr: "NonCached",
        },
        {
            name: "APP_FILEIO_TIOVX_LOG_RT",
            addr: 0xB1000000,   /* APP_FILEIO_MEM_ADDR */
            size: 16777216,     /* 16 MB */
            xn: true,
            attr: "NonCached",
        },
        null,   /* index 9: per-core IPC — filled in below */
    ];
}

/* --------------------------------------------------------------------------
 * SafeRTOS — distinct per-core patterns:
 *   mcu1_0 (mcu-r5fss0-0): has MSMC and no APP_FILEIO (9 regions)
 *   mcu2_0/4_0: no MSMC, has APP_FILEIO, + VISS (9 regions)
 *   mcu2_1/3_0/3_1/4_1: no MSMC, has APP_FILEIO, no VISS (8 regions)
 * -------------------------------------------------------------------------- */
function makeSafeRTOSRegions_mcu1_0() {
    /* mcu1_0 SafeRTOS has MSMC + FSS_DAT, no APP_FILEIO */
    return [
        {
            name: "BACKGROUND_REGION",
            addr: 0x0,
            size: 2147483648,   /* 2 GB */
            xn: true,
            attr: "StronglyOrdered",
        },
        {
            name: "R5F_ATCM",
            addr: 0x0,
            size: 65536,        /* 64 KB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "MCU_OCSRAM",
            addr: 0x41C00000,
            size: 1048576,      /* 1 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "MSMC",
            addr: 0x70000000,
            size: 4194304,      /* 4 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "DDR_2GB",
            addr: 0x80000000,
            size: 2147483648,   /* 2 GB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "R5F_BTCM",
            addr: 0x41010000,
            size: 65536,        /* 64 KB */
            xn: false,
            attr: "ADVANCED",
            cacheable:   true,
            cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
            shareable:   false,
            memAttr:     "0U",
        },
        {
            name: "IPC_VRING",
            addr: 0xAC000000,   /* IPC_VRING_MEM_ADDR */
            size: 67108864,     /* 64 MB */
            xn: true,
            attr: "NonCached",
        },
        {
            name: "APP_FILEIO_TIOVX_LOG_RT",
            addr: 0xB1000000,   /* APP_FILEIO_MEM_ADDR */
            size: 16777216,     /* 16 MB */
            xn: true,
            attr: "NonCached",
        },
        null,   /* index 8: per-core IPC — filled in below */
    ];
}

function makeSafeRTOSRegions_others(core) {
    /* mcu2_x, mcu3_x, mcu4_x: no MSMC, has IPC_VRING+APP_FILEIO */
    let tcm = tcmSize(core);
    return [
        {
            name: "BACKGROUND_REGION",
            addr: 0x0,
            size: 2147483648,   /* 2 GB */
            xn: true,
            attr: "StronglyOrdered",
        },
        {
            name: "R5F_ATCM",
            addr: 0x0,
            size: tcm,
            xn: false,
            attr: "Cached",
        },
        {
            name: "MCU_OCSRAM",
            addr: 0x41C00000,
            size: 1048576,      /* 1 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "DDR_2GB",
            addr: 0x80000000,
            size: 2147483648,   /* 2 GB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "R5F_BTCM",
            addr: 0x41010000,
            size: tcm,
            xn: false,
            attr: "ADVANCED",
            cacheable:   true,
            cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
            shareable:   false,
            memAttr:     "0U",
        },
        {
            name: "IPC_VRING",
            addr: 0xAC000000,   /* IPC_VRING_MEM_ADDR */
            size: 67108864,     /* 64 MB */
            xn: true,
            attr: "NonCached",
        },
        {
            name: "APP_FILEIO_TIOVX_LOG_RT",
            addr: 0xB1000000,   /* APP_FILEIO_MEM_ADDR */
            size: 16777216,     /* 16 MB */
            xn: true,
            attr: "NonCached",
        },
        null,   /* index 7: per-core IPC — filled in below */
    ];
}

/* --------------------------------------------------------------------------
 * Per-core IPC addresses
 * -------------------------------------------------------------------------- */
const IPC_ADDR = {
    "mcu-r5fss0-0": 0xA0000000,   /* DDR_MCU1_0_IPC_ADDR */
    "r5fss0-0":     0xA2000000,   /* DDR_MCU2_0_IPC_ADDR */
    "r5fss0-1":     0xA4000000,   /* DDR_MCU2_1_IPC_ADDR */
    "r5fss1-0":     0xA5000000,   /* DDR_MCU3_0_IPC_ADDR */
    "r5fss1-1":     0xA6000000,   /* DDR_MCU3_1_IPC_ADDR */
    "r5fss2-0":     0xA7000000,   /* DDR_MCU4_0_IPC_ADDR */
    "r5fss2-1":     0xA8000000,   /* DDR_MCU4_1_IPC_ADDR */
};

const IPC_NAME = {
    "mcu-r5fss0-0": "DDR_MCU1_0_IPC_MPU",
    "r5fss0-0":     "DDR_MCU2_0_IPC_MPU",
    "r5fss0-1":     "DDR_MCU2_1_IPC_MPU",
    "r5fss1-0":     "DDR_MCU3_0_IPC_MPU",
    "r5fss1-1":     "DDR_MCU3_1_IPC_MPU",
    "r5fss2-0":     "DDR_MCU4_0_IPC_MPU",
    "r5fss2-1":     "DDR_MCU4_1_IPC_MPU",
};

function makeIpcRegion(coreKey) {
    return {
        name: IPC_NAME[coreKey] || "DDR_IPC",
        addr: IPC_ADDR[coreKey],
        size: 1048576,      /* 1 MB */
        xn: true,
        attr: "NonCached",
    };
}

/* --------------------------------------------------------------------------
 * mcu2_0 (r5fss0-0) FreeRTOS only: INTERCORE_ETH and VISS regions
 * -------------------------------------------------------------------------- */
const MCU2_0_FREERTOS_EXTRA_REGIONS = [
    {
        name: "INTERCORE_ETH_DESC_CACHED",
        addr: 0xBE000000,   /* INTERCORE_ETH_DESC_MEM_ADDR */
        size: 33554432,     /* 32 MB */
        xn: true,
        attr: "CachedShareable",
    },
    {
        name: "INTERCORE_ETH_DESC_NONCACHED",
        addr: 0xBE000000,   /* INTERCORE_ETH_DESC_MEM_ADDR */
        size: 8388608,      /* 8 MB */
        xn: true,
        attr: "ADVANCED",
        cacheable:   false,
        cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
        shareable:   true,
        memAttr:     "0U",
    },
    {
        name: "VISS_CONFIG_HEAP",
        addr: 0xBB600000,   /* DDR_MCU2_0_VISS_CONFIG_HEAP_ADDR */
        size: 2097152,      /* 2 MB */
        xn: true,
        attr: "ADVANCED",
        cacheable:   true,
        cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",
        shareable:   false,
        memAttr:     "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",
    },
];

/* mcu2_0 and mcu4_0 SafeRTOS: VISS region */
const MCU2_0_SAFERTOS_VISS_REGION = {
    name: "VISS_CONFIG_HEAP",
    addr: 0xBB600000,       /* DDR_MCU2_0_VISS_CONFIG_HEAP_ADDR */
    size: 2097152,          /* 2 MB */
    xn: true,
    attr: "ADVANCED",
    cacheable:   true,
    cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",
    shareable:   false,
    memAttr:     "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",
};

const MCU4_0_VISS_REGION = {
    name: "VISS_CONFIG_HEAP",
    addr: 0xBD600000,       /* DDR_MCU4_0_VISS_CONFIG_HEAP_ADDR */
    size: 2097152,          /* 2 MB */
    xn: true,
    attr: "ADVANCED",
    cacheable:   true,
    cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",
    shareable:   false,
    memAttr:     "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",
};

/* --------------------------------------------------------------------------
 * Main export
 * -------------------------------------------------------------------------- */
function populate_mpu_regions(containerInst, ind, device, core) {
    let coreKey;
    if      (core.includes("mcu-r5fss0-0")) coreKey = "mcu-r5fss0-0";
    else if (core.includes("r5fss0-0"))     coreKey = "r5fss0-0";
    else if (core.includes("r5fss0-1"))     coreKey = "r5fss0-1";
    else if (core.includes("r5fss1-0"))     coreKey = "r5fss1-0";
    else if (core.includes("r5fss1-1"))     coreKey = "r5fss1-1";
    else if (core.includes("r5fss2-0"))     coreKey = "r5fss2-0";
    else if (core.includes("r5fss2-1"))     coreKey = "r5fss2-1";
    else return;

    let isSafeRTOS = (process.env.RTOS || "").toUpperCase() === "SAFERTOS";
    let regions;

    if (isSafeRTOS) {
        if (coreKey === "mcu-r5fss0-0") {
            /* mcu1_0 SafeRTOS: unique structure with MSMC */
            regions = makeSafeRTOSRegions_mcu1_0();
            regions[8] = makeIpcRegion(coreKey);
        } else {
            /* All other cores: no MSMC, has APP_FILEIO */
            regions = makeSafeRTOSRegions_others(coreKey);
            regions[7] = makeIpcRegion(coreKey);
            if (coreKey === "r5fss0-0") {
                /* mcu2_0 SafeRTOS: + VISS (9 regions total) */
                regions.push(MCU2_0_SAFERTOS_VISS_REGION);
            } else if (coreKey === "r5fss2-0") {
                /* mcu4_0 SafeRTOS: + VISS (9 regions total) */
                regions.push(MCU4_0_VISS_REGION);
            }
        }
    } else {
        /* FreeRTOS */
        regions = makeFreeRTOSRegions(coreKey);
        regions[9] = makeIpcRegion(coreKey);

        if (coreKey === "r5fss0-0") {
            /* mcu2_0 FreeRTOS: add INTERCORE_ETH + VISS (13 regions total) */
            regions = regions.concat(MCU2_0_FREERTOS_EXTRA_REGIONS);
        } else if (coreKey === "r5fss2-0") {
            /* mcu4_0 FreeRTOS: add VISS only (11 regions total) */
            regions.push(MCU4_0_VISS_REGION);
        }
        /* Other cores: 10 regions (no extras) */
    }

    let cfg = regions[ind];
    if (cfg === undefined || cfg === null) return;

    applyRegion(containerInst.mpu_region[ind], cfg);
}

function getMpuRegionCount(core, rtos) {
    if (rtos === "safertos") {
        /* mcu1_0: 9 regions */
        if (core.includes("mcu-r5fss0-0")) return 9;
        /* mcu2_0, mcu4_0: 9 regions (+VISS); others: 8 regions */
        if (core.includes("r5fss0-0") || core.includes("r5fss2-0")) return 9;
        return 8;
    }
    /* FreeRTOS: mcu1_0 ("mcu-r5fss0-0") also contains "r5fss0-0", so exclude it explicitly */
    if (core.includes("r5fss0-0") && !core.includes("mcu-r5fss0-0")) return 13;   /* mcu2_0: INTERCORE_ETH + VISS */
    if (core.includes("r5fss2-0")) return 11;   /* mcu4_0: VISS only */
    return 10;
}

exports.populate_mpu_regions = populate_mpu_regions;
exports.getMpuRegionCount    = getMpuRegionCount;
