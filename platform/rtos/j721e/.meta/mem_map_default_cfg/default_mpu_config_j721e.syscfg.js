/*
 * Populates MPU region instances matching the static mpu_cfg.c files for
 * J721E R5F cores. The data here must stay in sync with vision_apps/platform/j721e/rtos/.
 *
 * Called from mpu_cfg.syscfg.js after the mpu_cfg module instances
 * are created:
 *   populate_mpu_regions(inst, ind, device, core)
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

    /* Advanced fields — only needed when attr === "ADVANCED" */
    if (cfg.attr === "ADVANCED") {
        inst.cacheable   = cfg.cacheable   !== undefined ? cfg.cacheable : false;
        inst.cachePolicy = cfg.cachePolicy || "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE";
        inst.shareable   = cfg.shareable   !== undefined ? cfg.shareable : false;
        inst.memAttr     = cfg.memAttr     || "0U";
    }
}

/* --------------------------------------------------------------------------
 * FreeRTOS regions — index 10 is per-core IPC (null placeholder).
 * mcu2_0 (r5fss0-0) appends INTERCORE_ETH (x2) + VISS regions.
 * -------------------------------------------------------------------------- */
const FREERTOS_COMMON_REGIONS = [
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
        size: 32768,        /* 32 KB */
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
        size: 32768,        /* 32 KB */
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
        addr: 0xAA000000,   /* IPC_VRING_MEM_ADDR */
        size: 33554432,     /* 32 MB */
        xn: true,
        attr: "NonCached",
    },
    {
        name: "APP_LOG_TIOVX_OBJ_DESC",
        addr: 0xAC000000,   /* APP_LOG_MEM_ADDR */
        size: 67108864,     /* 64 MB */
        xn: true,
        attr: "NonCached",
    },
    {
        name: "APP_FILEIO_TIOVX_LOG_RT",
        addr: 0xB0000000,   /* APP_FILEIO_MEM_ADDR */
        size: 33554432,     /* 32 MB */
        xn: true,
        attr: "NonCached",
    },
    null,   /* index 10: per-core IPC — filled in below */
    {
        name: "INTERCORE_ETH_DESC_CACHED",
        addr: 0xE4000000,   /* INTERCORE_ETH_DESC_MEM_ADDR */
        size: 33554432,     /* 32 MB */
        xn: true,
        attr: "CachedShareable",
    },
    {
        name: "INTERCORE_ETH_DESC_NONCACHED",
        addr: 0xE4000000,   /* INTERCORE_ETH_DESC_MEM_ADDR */
        size: 8388608,      /* 8 MB */
        xn: true,
        attr: "ADVANCED",
        cacheable:   false,
        cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
        shareable:   true,
        memAttr:     "0U",
    },
];

/* mcu2_0 FreeRTOS only: VISS shadow config heap */
const FREERTOS_VISS_REGION = {
    name: "VISS_CONFIG_HEAP",
    addr: 0xD9F00000,       /* DDR_MCU2_0_VISS_CONFIG_HEAP_ADDR */
    size: 1048576,          /* 1 MB */
    xn: true,
    attr: "ADVANCED",
    cacheable:   true,
    cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",
    shareable:   false,
    memAttr:     "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",
};

/* --------------------------------------------------------------------------
 * SafeRTOS regions — different structure from FreeRTOS:
 *   - No ATCM_GLOBAL_ALIAS
 *   - No MSMC
 *   - OCSRAM = 512 KB (not 1 MB)
 *   - ATCM/BTCM always 32 KB (no lock-step distinction)
 *   - BTCM is ADVANCED (cacheable=1, policy=NON_CACHEABLE)
 *   - Has APP_LOG (0xAC000000, 64 MB) and APP_FILEIO (0xB0000000, 32 MB)
 *   - mcu1_0 and other non-mcu2 cores: 9 regions WITH BACKGROUND
 *   - mcu2_0: 10 regions WITHOUT BACKGROUND, + INTERCORE_ETH 2 MB + VISS 1 MB
 * -------------------------------------------------------------------------- */
function makeSafeRTOSBaseRegions(includeBackground) {
    let regions = [];
    if (includeBackground) {
        regions.push({
            name: "BACKGROUND_REGION",
            addr: 0x0,
            size: 2147483648,   /* 2 GB — portmpuLARGEST_REGION_SIZE_ACTUAL */
            xn: true,
            attr: "StronglyOrdered",
        });
    }
    regions.push(
        {
            name: "R5F_ATCM",
            addr: 0x0,
            size: 32768,        /* 32 KB — always 32 KB in SafeRTOS */
            xn: false,
            attr: "Cached",
        },
        {
            name: "MCU_OCSRAM",
            addr: 0x41C00000,
            size: 524288,       /* 512 KB */
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
            size: 32768,        /* 32 KB */
            xn: false,
            attr: "ADVANCED",
            cacheable:   true,
            cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
            shareable:   false,
            memAttr:     "0U",
        },
        {
            name: "IPC_VRING",
            addr: 0xAA000000,   /* IPC_VRING_MEM_ADDR */
            size: 33554432,     /* 32 MB */
            xn: true,
            attr: "NonCached",
        },
        {
            name: "APP_LOG_TIOVX_OBJ_DESC",
            addr: 0xAC000000,   /* APP_LOG_MEM_ADDR */
            size: 67108864,     /* 64 MB */
            xn: true,
            attr: "NonCached",
        },
        {
            name: "APP_FILEIO_TIOVX_LOG_RT",
            addr: 0xB0000000,   /* APP_FILEIO_MEM_ADDR */
            size: 33554432,     /* 32 MB */
            xn: true,
            attr: "NonCached",
        },
        null    /* per-core IPC placeholder — filled in below */
    );
    return regions;
}

/* mcu2_0 SafeRTOS only: INTERCORE_ETH (2 MB) + VISS (1 MB) */
const SAFERTOS_MCU2_0_EXTRA_REGIONS = [
    {
        name: "INTERCORE_ETH_DESC",
        addr: 0xE4000000,   /* INTERCORE_ETH_DESC_MEM_ADDR */
        size: 2097152,      /* 2 MB */
        xn: true,
        attr: "ADVANCED",
        cacheable:   false,
        cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
        shareable:   true,
        memAttr:     "CSL_ARM_R5_MEM_ATTR_STRONGLY_NON_CACHED",
    },
    {
        name: "VISS_CONFIG_HEAP",
        addr: 0xD9F00000,   /* DDR_MCU2_0_VISS_CONFIG_HEAP_ADDR */
        size: 1048576,      /* 1 MB */
        xn: true,
        attr: "ADVANCED",
        cacheable:   true,
        cachePolicy: "CSL_ARM_R5_CACHE_POLICY_WT_NO_WA",
        shareable:   false,
        memAttr:     "CSL_ARM_R5_MEM_ATTR_CACHED_WT_NO_WA",
    },
];

/* --------------------------------------------------------------------------
 * Per-core IPC addresses
 * -------------------------------------------------------------------------- */
const IPC_ADDR = {
    "mcu-r5fss0-0": 0xA0000000,   /* DDR_MCU1_0_IPC_ADDR */
    "r5fss0-0":     0xA2000000,   /* DDR_MCU2_0_IPC_ADDR */
    "r5fss0-1":     0xA4000000,   /* DDR_MCU2_1_IPC_ADDR */
    "r5fss1-0":     0xA6000000,   /* DDR_MCU3_0_IPC_ADDR */
    "r5fss1-1":     0xA7000000,   /* DDR_MCU3_1_IPC_ADDR */
};

const IPC_NAME = {
    "mcu-r5fss0-0": "DDR_MCU1_0_IPC_MPU",
    "r5fss0-0":     "DDR_MCU2_0_IPC_MPU",
    "r5fss0-1":     "DDR_MCU2_1_IPC_MPU",
    "r5fss1-0":     "DDR_MCU3_0_IPC_MPU",
    "r5fss1-1":     "DDR_MCU3_1_IPC_MPU",
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
 * Main export
 * -------------------------------------------------------------------------- */
function populate_mpu_regions(containerInst, ind, device, core) {
    if (device !== "J721E_DRA829_TDA4VM_AM752x" && device !== "J721E") return;

    let coreKey;
    if      (core.includes("mcu-r5fss0-0")) coreKey = "mcu-r5fss0-0";
    else if (core.includes("r5fss0-0"))     coreKey = "r5fss0-0";
    else if (core.includes("r5fss0-1"))     coreKey = "r5fss0-1";
    else if (core.includes("r5fss1-0"))     coreKey = "r5fss1-0";
    else if (core.includes("r5fss1-1"))     coreKey = "r5fss1-1";
    else return;

    let isSafeRTOS = (process.env.RTOS || "").toUpperCase() === "SAFERTOS";
    let regions;

    if (isSafeRTOS) {
        let isMcu2_0 = (coreKey === "r5fss0-0");
        /* mcu2_0 has no BACKGROUND region (10 total); all others have it (9 total) */
        regions = makeSafeRTOSBaseRegions(!isMcu2_0);
        /* IPC placeholder is always the last element from makeSafeRTOSBaseRegions */
        regions[regions.length - 1] = makeIpcRegion(coreKey);
        if (isMcu2_0) {
            regions = regions.concat(SAFERTOS_MCU2_0_EXTRA_REGIONS);
        }
    } else {
        /* FreeRTOS */
        regions = FREERTOS_COMMON_REGIONS.slice();
        regions[10] = makeIpcRegion(coreKey);
        if (coreKey === "r5fss0-0") {
            /* mcu2_0: append VISS (INTERCORE_ETH already in FREERTOS_COMMON_REGIONS) */
            regions = regions.concat([FREERTOS_VISS_REGION]);
        }
    }

    let cfg = regions[ind];
    if (cfg === undefined || cfg === null) return;

    applyRegion(containerInst.mpu_region[ind], cfg);
}

function getMpuRegionCount(core, rtos) {
    /* "mcu-r5fss0-0" (mcu1_0) also contains "r5fss0-0", so exclude it explicitly */
    const isMcu2_0 = core.includes("r5fss0-0") && !core.includes("mcu-r5fss0-0");
    if (rtos === "safertos") {
        /* mcu2_0 has 10 regions (no BACKGROUND + INTERCORE_ETH + VISS); others have 9 */
        return isMcu2_0 ? 10 : 9;
    }
    /* FreeRTOS: mcu2_0 has extra VISS region (14 total); others have 13 */
    return isMcu2_0 ? 14 : 13;
}

exports.populate_mpu_regions = populate_mpu_regions;
exports.getMpuRegionCount    = getMpuRegionCount;
