/*
 * Populates MPU region instances matching the static mpu_cfg.c files for
 * J721S2 R5F cores. The data here must stay in sync with vision_apps/platform/j721s2/rtos/.
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
    return core.endsWith("-0") ? 65536 : 32768;   /* 64 KB or 32 KB */
}

/* --------------------------------------------------------------------------
 * FreeRTOS common regions — index 8 is a null placeholder replaced per-core.
 * mcu2_0 (r5fss0-0) appends a VISS region (10 total).
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
            name: "IPC_VRING",
            addr: 0xA8000000,   /* IPC_VRING_MEM_ADDR */
            size: 134217728,    /* 128 MB */
            xn: true,
            attr: "NonCached",
        },
        null,   /* index 8: per-core IPC — filled in below */
    ];
}

/* --------------------------------------------------------------------------
 * SafeRTOS common regions — index 8 is a null placeholder replaced per-core.
 * Structure verified against j721s2_safertos_mpu_cfg.c for all 6 cores.
 * BTCM uses ADVANCED (cacheable=1, policy=NON_CACHEABLE) per static files.
 * mcu2_0 appends a VISS region (10 total).
 * -------------------------------------------------------------------------- */
function makeSafeRTOSRegions(core) {
    let tcm = tcmSize(core);
    return [
        {
            name: "BACKGROUND_REGION",
            addr: 0x0,
            size: 2147483648,   /* 2 GB — portmpuLARGEST_REGION_SIZE_ACTUAL */
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
            size: tcm,
            xn: false,
            attr: "ADVANCED",
            cacheable:   true,
            cachePolicy: "CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE",
            shareable:   false,
            memAttr:     "0U",
        },
        {
            name: "FSS_DAT",
            addr: 0x50000000,
            size: 134217728,    /* 128 MB */
            xn: false,
            attr: "Cached",
        },
        {
            name: "IPC_VRING",
            addr: 0xA8000000,   /* IPC_VRING_MEM_ADDR */
            size: 134217728,    /* 128 MB */
            xn: true,
            attr: "NonCached",
        },
        null,   /* index 8: per-core IPC — filled in below */
    ];
}

/* --------------------------------------------------------------------------
 * Per-core IPC region (region index 8)
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

/* mcu2_0 (r5fss0-0) only: VISS shadow config heap — same size/attrs in both RTOS */
const VISS_REGION = {
    name: "VISS_CONFIG_HEAP",
    addr: 0xB9E00000,       /* DDR_MCU2_0_VISS_CONFIG_HEAP_ADDR */
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
    else return;

    let isSafeRTOS = (process.env.RTOS || "").toUpperCase() === "SAFERTOS";
    let hasViss    = (coreKey === "r5fss0-0");

    let regions = isSafeRTOS ? makeSafeRTOSRegions(coreKey) : makeFreeRTOSRegions(coreKey);
    regions[8]  = makeIpcRegion(coreKey);
    if (hasViss) regions.push(VISS_REGION);

    let cfg = regions[ind];
    if (cfg === undefined || cfg === null) return;

    applyRegion(containerInst.mpu_region[ind], cfg);
}

function getMpuRegionCount(core, rtos) {
    /* mcu2_0 (r5fss0-0) has an extra VISS region in both RTOS.
     * "mcu-r5fss0-0" (mcu1_0) also contains "r5fss0-0", so exclude it explicitly. */
    if (core.includes("r5fss0-0") && !core.includes("mcu-r5fss0-0")) return 10;
    return 9;
}

exports.populate_mpu_regions = populate_mpu_regions;
exports.getMpuRegionCount    = getMpuRegionCount;
