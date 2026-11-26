/*
 * Populates MMU region instances matching the static appMmuMap() in
 * vision_apps/platform/j742s2/rtos/c7x_{1,2,3}/main.c.
 * Called from memory_region_mmu.syscfg.js after the mmu_region instances
 * are created:
 *   populate_mmu_regions(inst, ind, device, core)
 *
 * Region structure mirrors the Mmu_map() calls in appMmuMap():
 *   - MAIR0 group: device/peripheral ranges + non-cacheable DDR aliases
 *   - MAIR7 group: cached (OCMC, MSMC shared, L2 SRAM, DDR local regions)
 *   - MAIR4 group: non-cached shared buffers (IPC, log, fileio, obj desc)
 *
 * NOTE: J742S2 has split cacheable/non-cacheable DDR regions per C7x core.
 *       Each core also maps the peer core's heap/scratch at a different vAddr
 *       for cross-core shared access.
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Helper: set one memory_region_mmu instance from a plain config object.
 * -------------------------------------------------------------------------- */
function applyRegion(inst, cfg) {
    if (cfg.name)  inst.$name   = cfg.name;
    inst.vAddr    = cfg.vAddr;
    inst.pAddr    = cfg.pAddr !== undefined ? cfg.pAddr : cfg.vAddr;
    inst.size     = cfg.size;
    inst.attrIndx = cfg.attrIndx;
}

/* --------------------------------------------------------------------------
 * C7x_1 regions — extracted from j742s2/rtos/c7x_1/main.c appMmuMap()
 *
 * MAIR0 block: 5 peripheral + 4 non-cacheable DDR aliases (own + peer C7x_2)
 * MAIR7 block: 10 cached (OCMC×3, MSMC shared, L2 SRAM, DTS,
 *              LOCAL_HEAP own + peer, SCRATCH own + peer, SHARED_MEM,
 *              MSMC local)
 * MAIR4 block: 6 non-cached shared DDR regions
 * -------------------------------------------------------------------------- */
const C7X_1_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "CLEC_0x78000000",   vAddr: 0x78000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    /* Non-cacheable DDR aliases (own heap/scratch + peer C7x_2 view) */
    { name: "DDR_C7X_1_LOCAL_HEAP_NC",   vAddr: 0x100000000, pAddr: 0x880000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_2_LOCAL_HEAP_NC",   vAddr: 0x120000000, pAddr: 0x8A0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_1_SCRATCH_NC",      vAddr: 0x110000000, pAddr: 0x890000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_2_SCRATCH_NC",      vAddr: 0x130000000, pAddr: 0x8B0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR7 — cached normal */
    { name: "OCMC_0x80000000",   vAddr: 0x80000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0xA0000000",   vAddr: 0xA0000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_SHARED",       vAddr: 0x70000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 4 MB shared MSMC */
    { name: "OCMC_0x41C00000",   vAddr: 0x41C00000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "L2_SRAM",           vAddr: 0x64800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* L2 SRAM (mapped as 2MB for perf) */
    { name: "DDR_C7x_1_DTS",     vAddr: 0xB2100000, size: 0x01F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7x_1_DTS_ADDR / DDR_C7x_1_DTS_SIZE */
    /* Own heap/scratch (cacheable) */
    { name: "DDR_C7X_1_LOCAL_HEAP",  vAddr: 0x108000000, pAddr: 0x888000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    /* Peer C7x_2 heap/scratch viewed from C7x_1 */
    { name: "DDR_C7X_2_LOCAL_HEAP",  vAddr: 0x128000000, pAddr: 0x8A8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_1_SCRATCH",     vAddr: 0x118000000, pAddr: 0x898000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_2_SCRATCH",     vAddr: 0x138000000, pAddr: 0x8B8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_SHARED_MEM",        vAddr: 0xC0000000,  pAddr: 0x900000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_C7x_1",            vAddr: 0x68000000,  size: 0x00300000,                    attrIndx: "Mmu_AttrIndx_MAIR7" },   /* Local MSMC */

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",        vAddr: 0xAF000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "APP_FILEIO_MEM",     vAddr: 0xB1000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_OBJ_DESC_MEM", vAddr: 0xAF040000, size: 0x01FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "IPC_VRING_MEM",      vAddr: 0xAC000000, size: 0x03000000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "DDR_C7x_1_IPC",      vAddr: 0xB2000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_LOG_RT_MEM",   vAddr: 0xB1400000, size: 0x00C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },
];

/* --------------------------------------------------------------------------
 * C7x_2 regions — extracted from j742s2/rtos/c7x_2/main.c appMmuMap()
 * -------------------------------------------------------------------------- */
const C7X_2_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "CLEC_0x78000000",   vAddr: 0x78000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    /* Non-cacheable DDR aliases (own + peer C7x_1 view) */
    { name: "DDR_C7X_2_LOCAL_HEAP_NC",   vAddr: 0x100000000, pAddr: 0x8A0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_1_LOCAL_HEAP_NC",   vAddr: 0x120000000, pAddr: 0x880000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_2_SCRATCH_NC",      vAddr: 0x110000000, pAddr: 0x8B0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_1_SCRATCH_NC",      vAddr: 0x130000000, pAddr: 0x890000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR7 — cached normal */
    { name: "OCMC_0x80000000",   vAddr: 0x80000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0xA0000000",   vAddr: 0xA0000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_SHARED",       vAddr: 0x70000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0x41C00000",   vAddr: 0x41C00000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "L2_SRAM",           vAddr: 0x65800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7x_2_DTS",     vAddr: 0xB4100000, size: 0x01F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_2_LOCAL_HEAP",  vAddr: 0x108000000, pAddr: 0x8A8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_1_LOCAL_HEAP",  vAddr: 0x128000000, pAddr: 0x888000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_2_SCRATCH",     vAddr: 0x118000000, pAddr: 0x8B8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_1_SCRATCH",     vAddr: 0x138000000, pAddr: 0x898000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_SHARED_MEM",        vAddr: 0xC0000000,  pAddr: 0x900000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_C7x_2",            vAddr: 0x69000000,  size: 0x00300000,                    attrIndx: "Mmu_AttrIndx_MAIR7" },

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",        vAddr: 0xAF000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "APP_FILEIO_MEM",     vAddr: 0xB1000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_OBJ_DESC_MEM", vAddr: 0xAF040000, size: 0x01FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "IPC_VRING_MEM",      vAddr: 0xAC000000, size: 0x03000000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "DDR_C7x_2_IPC",      vAddr: 0xB4000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_LOG_RT_MEM",   vAddr: 0xB1400000, size: 0x00C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },
];

/* --------------------------------------------------------------------------
 * C7x_3 regions — extracted from j742s2/rtos/c7x_3/main.c appMmuMap()
 * C7x_3 has no peer cross-mapping; only its own heap/scratch.
 * MSMC is 8MB shared (vs 4MB for C7x_1/2).
 * -------------------------------------------------------------------------- */
const C7X_3_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "CLEC_0x78000000",   vAddr: 0x78000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    /* Non-cacheable DDR aliases */
    { name: "DDR_C7X_3_LOCAL_HEAP_NC", vAddr: 0x100000000, pAddr: 0x8C0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "DDR_C7X_3_SCRATCH_NC",    vAddr: 0x110000000, pAddr: 0x8D0000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR7 — cached normal */
    { name: "OCMC_0x80000000",   vAddr: 0x80000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0xA0000000",   vAddr: 0xA0000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_SHARED",       vAddr: 0x70000000, size: 0x00800000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 8 MB shared MSMC */
    { name: "OCMC_0x41C00000",   vAddr: 0x41C00000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "L2_SRAM",           vAddr: 0x66800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7x_3_DTS",     vAddr: 0xB6100000, size: 0x01F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_3_LOCAL_HEAP",  vAddr: 0x108000000, pAddr: 0x8C8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_C7X_3_SCRATCH",     vAddr: 0x118000000, pAddr: 0x8D8000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "DDR_SHARED_MEM",        vAddr: 0xC0000000,  pAddr: 0x900000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC_C7x_3",            vAddr: 0x6A000000,  size: 0x00300000,                    attrIndx: "Mmu_AttrIndx_MAIR7" },

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",        vAddr: 0xAF000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "APP_FILEIO_MEM",     vAddr: 0xB1000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_OBJ_DESC_MEM", vAddr: 0xAF040000, size: 0x01FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "IPC_VRING_MEM",      vAddr: 0xAC000000, size: 0x03000000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "DDR_C7x_3_IPC",      vAddr: 0xB6000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },
    { name: "TIOVX_LOG_RT_MEM",   vAddr: 0xB1400000, size: 0x00C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },
];

/* --------------------------------------------------------------------------
 * Per-core region tables
 * -------------------------------------------------------------------------- */
const REGIONS_BY_CORE = {
    "c71ss0-0": C7X_1_REGIONS,
    "c71ss1-0": C7X_2_REGIONS,
    "c71ss2-0": C7X_3_REGIONS,
};

/* --------------------------------------------------------------------------
 * Main export
 * -------------------------------------------------------------------------- */
function populate_mmu_regions(containerInst, ind, device, core) {
    if (device !== "J742S2" && device !== "J742S2_TDA4VH") return;

    let coreKey;
    if      (core.includes("c71ss0-0")) coreKey = "c71ss0-0";
    else if (core.includes("c71ss1-0")) coreKey = "c71ss1-0";
    else if (core.includes("c71ss2-0")) coreKey = "c71ss2-0";
    else return;

    let regions = REGIONS_BY_CORE[coreKey];
    if (!regions) return;

    let cfg = regions[ind];
    if (cfg === undefined || cfg === null) return;

    applyRegion(containerInst.mmu_region[ind], cfg);
}

function getMmuRegionCount(core) {
    if (core.includes("c71ss0-0")) return C7X_1_REGIONS.length;
    if (core.includes("c71ss1-0")) return C7X_2_REGIONS.length;
    if (core.includes("c71ss2-0")) return C7X_3_REGIONS.length;
    return 0;
}

exports.populate_mmu_regions = populate_mmu_regions;
exports.getMmuRegionCount    = getMmuRegionCount;
