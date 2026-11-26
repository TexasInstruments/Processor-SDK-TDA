/*
 * Populates MMU region instances matching the static appMmuMap() in
 * vision_apps/platform/j721s2/rtos/c7x_{1,2}/main.c.
 * Called from memory_region_mmu.syscfg.js after the mmu_region instances
 * are created:
 *   populate_mmu_regions(inst, ind, device, core)
 *
 * Region structure mirrors the Mmu_map() calls in appMmuMap():
 *   - MAIR0 group: device/peripheral address ranges (identity mapped)
 *   - MAIR7 group: cached memory (MSMC, OCMC, L2 SRAM, DDR local regions)
 *   - MAIR4 group: non-cached shared buffers (IPC, log, fileio, obj desc)
 *
 * NOTE: vAddr == pAddr for all regions except LOCAL_HEAP, SCRATCH, and
 * DDR_SHARED_MEM which use non-identity mappings for 64-bit DDR aliasing.
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
 * C7x_1 regions — extracted from j721s2/rtos/c7x_1/main.c appMmuMap()
 *
 * MAIR0 block: 5 identity device/peripheral regions
 * MAIR7 block: 8 cached regions (OCMC, MSMC, L2 SRAM, DTS, LOCAL_HEAP, SCRATCH, SHARED_MEM)
 * MAIR4 block: 6 non-cached shared DDR regions
 * -------------------------------------------------------------------------- */
const C7X_1_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "CLEC_0x78000000",   vAddr: 0x78000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR7 — cached normal (OCMC, MSMC, L2 SRAM, DDR local regions) */
    { name: "OCMC_0x80000000",   vAddr: 0x80000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0xA0000000",   vAddr: 0xA0000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC",              vAddr: 0x70000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 4 MB MSMC */
    { name: "OCMC_0x41C00000",   vAddr: 0x41C00000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 1 MB OCMC */
    { name: "L2_SRAM",           vAddr: 0x64800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* L2 SRAM 448 KB (mapped as 2MB for perf) */
    { name: "DDR_C7x_1_DTS",     vAddr: 0xB0100000, size: 0x05F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7x_1_DTS_ADDR / DDR_C7x_1_DTS_SIZE */
    /* LOCAL_HEAP and SCRATCH use non-identity vAddr/pAddr (64-bit DDR alias) */
    { name: "DDR_C7x_1_LOCAL_HEAP", vAddr: 0x117000000, pAddr: 0x897000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_1_LOCAL_HEAP */
    { name: "DDR_C7x_1_SCRATCH",    vAddr: 0x100000000, pAddr: 0x880000000, size: 0x17000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_1_SCRATCH */
    { name: "DDR_SHARED_MEM",       vAddr: 0xC0000000,  pAddr: 0x900000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_SHARED_MEM (virt→phys alias) */

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",        vAddr: 0xAA000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_LOG_MEM_ADDR / APP_LOG_MEM_SIZE */
    { name: "APP_FILEIO_MEM",     vAddr: 0xAE000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_FILEIO_MEM_ADDR / APP_FILEIO_MEM_SIZE */
    { name: "TIOVX_OBJ_DESC_MEM", vAddr: 0xAA040000, size: 0x03FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_OBJ_DESC_MEM_ADDR / TIOVX_OBJ_DESC_MEM_SIZE */
    { name: "IPC_VRING_MEM",      vAddr: 0xA8000000, size: 0x02000000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* IPC_VRING_MEM_ADDR / IPC_VRING_MEM_SIZE */
    { name: "DDR_C7x_1_IPC",      vAddr: 0xB0000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* DDR_C7x_1_IPC_ADDR / DDR_C7x_1_IPC_SIZE */
    { name: "TIOVX_LOG_RT_MEM",   vAddr: 0xAE400000, size: 0x01C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_LOG_RT_MEM_ADDR / TIOVX_LOG_RT_MEM_SIZE */
];

/* --------------------------------------------------------------------------
 * C7x_2 regions — extracted from j721s2/rtos/c7x_2/main.c appMmuMap()
 *
 * Same peripheral/OCMC/MSMC layout as C7x_1; only DDR IPC/DTS/heap/scratch differ.
 * -------------------------------------------------------------------------- */
const C7X_2_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "CLEC_0x78000000",   vAddr: 0x78000000, size: 0x08000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR7 — cached normal (OCMC, MSMC, L2 SRAM, DDR local regions) */
    { name: "OCMC_0x80000000",   vAddr: 0x80000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "OCMC_0xA0000000",   vAddr: 0xA0000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },
    { name: "MSMC",              vAddr: 0x70000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 4 MB MSMC */
    { name: "OCMC_0x41C00000",   vAddr: 0x41C00000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 1 MB OCMC */
    { name: "L2_SRAM",           vAddr: 0x65800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* L2 SRAM for C7x_2 */
    { name: "DDR_C7x_2_DTS",     vAddr: 0xB6100000, size: 0x01F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7x_2_DTS_ADDR / DDR_C7x_2_DTS_SIZE */
    /* LOCAL_HEAP and SCRATCH use non-identity vAddr/pAddr (64-bit DDR alias) */
    { name: "DDR_C7x_2_LOCAL_HEAP", vAddr: 0x127000000, pAddr: 0x8A7000000, size: 0x01000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_2_LOCAL_HEAP */
    { name: "DDR_C7x_2_SCRATCH",    vAddr: 0x128000000, pAddr: 0x8A8000000, size: 0x04000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_2_SCRATCH */
    { name: "DDR_SHARED_MEM",       vAddr: 0xC0000000,  pAddr: 0x900000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_SHARED_MEM (virt→phys alias) */

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",        vAddr: 0xAA000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_LOG_MEM_ADDR / APP_LOG_MEM_SIZE */
    { name: "APP_FILEIO_MEM",     vAddr: 0xAE000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_FILEIO_MEM_ADDR / APP_FILEIO_MEM_SIZE */
    { name: "TIOVX_OBJ_DESC_MEM", vAddr: 0xAA040000, size: 0x03FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_OBJ_DESC_MEM_ADDR / TIOVX_OBJ_DESC_MEM_SIZE */
    { name: "IPC_VRING_MEM",      vAddr: 0xA8000000, size: 0x02000000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* IPC_VRING_MEM_ADDR / IPC_VRING_MEM_SIZE */
    { name: "DDR_C7x_2_IPC",      vAddr: 0xB6000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* DDR_C7x_2_IPC_ADDR / DDR_C7x_2_IPC_SIZE */
    { name: "TIOVX_LOG_RT_MEM",   vAddr: 0xAE400000, size: 0x01C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_LOG_RT_MEM_ADDR / TIOVX_LOG_RT_MEM_SIZE */
];

/* --------------------------------------------------------------------------
 * Per-core region tables
 * -------------------------------------------------------------------------- */
const REGIONS_BY_CORE = {
    "c71ss0-0": C7X_1_REGIONS,
    "c71ss1-0": C7X_2_REGIONS,
};

/* --------------------------------------------------------------------------
 * Main export
 * -------------------------------------------------------------------------- */
function populate_mmu_regions(containerInst, ind, device, core) {
    if (device !== "J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x" && device !== "J721S2") return;

    let coreKey;
    if      (core.includes("c71ss0-0")) coreKey = "c71ss0-0";
    else if (core.includes("c71ss1-0")) coreKey = "c71ss1-0";
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
    return 0;
}

exports.populate_mmu_regions = populate_mmu_regions;
exports.getMmuRegionCount    = getMmuRegionCount;
