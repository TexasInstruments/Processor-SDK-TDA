/*
 * Populates MMU region instances matching the static appMmuMap() in
 * vision_apps/platform/j721e/rtos/c7x/main.c.
 * Called from memory_region_mmu.syscfg.js after the mmu_region instances
 * are created:
 *   populate_mmu_regions(inst, ind, device, core)
 *
 * Region structure mirrors the Mmu_map() calls in appMmuMap():
 *   - MAIR0 group: device/peripheral address ranges (identity mapped)
 *   - MAIR4 group: non-cached shared buffers (IPC, log, fileio, obj desc)
 *   - MAIR7 group: cached memory (MSMC, L2 SRAM, DDR local heap/scratch/DTS/shared)
 *
 * NOTE: vAddr == pAddr for all regions except LOCAL_HEAP and SCRATCH which
 * use a non-identity mapping because they live above 4 GB physically
 * (DDR_64BIT_BASE_PADDR offset) but are accessed via a lower virtual alias.
 * The virtual aliases equal the app_mem_map.h symbols directly (no offset).
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
 * C7x_1 regions — extracted from j721e/rtos/c7x_1/main.c appMmuMap()
 * J721E has one C7x core (c71ss0-0) only.
 *
 * MAIR0 block: 5 identity device/peripheral regions + 1 L1D SRAM region
 * MAIR4 block: 6 non-cached shared DDR regions
 * MAIR7 block: 6 cached regions (MSMC, L2 SRAM, DTS, LOCAL_HEAP, SCRATCH, SHARED_MEM)
 * -------------------------------------------------------------------------- */
const C7X_1_REGIONS = [
    /* MAIR0 — device/strongly-ordered (peripherals) */
    { name: "PERIPH_0x00000000", vAddr: 0x00000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x20000000", vAddr: 0x20000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x40000000", vAddr: 0x40000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x60000000", vAddr: 0x60000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },
    { name: "PERIPH_0x70000000", vAddr: 0x70000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR0" },

    /* MAIR4 — non-cached normal (shared DDR buffers) */
    { name: "APP_LOG_MEM",          vAddr: 0xAC000000, size: 0x00040000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_LOG_MEM_ADDR / APP_LOG_MEM_SIZE */
    { name: "APP_FILEIO_MEM",       vAddr: 0xB0000000, size: 0x00400000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* APP_FILEIO_MEM_ADDR / APP_FILEIO_MEM_SIZE */
    { name: "TIOVX_OBJ_DESC_MEM",   vAddr: 0xAC040000, size: 0x03FC0000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_OBJ_DESC_MEM_ADDR / TIOVX_OBJ_DESC_MEM_SIZE */
    { name: "IPC_VRING_MEM",        vAddr: 0xAA000000, size: 0x02000000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* IPC_VRING_MEM_ADDR / IPC_VRING_MEM_SIZE */
    { name: "DDR_C7x_1_IPC",        vAddr: 0xB2000000, size: 0x00100000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* DDR_C7x_1_IPC_ADDR / DDR_C7x_1_IPC_SIZE */
    { name: "TIOVX_LOG_RT_MEM",     vAddr: 0xB0400000, size: 0x01C00000, attrIndx: "Mmu_AttrIndx_MAIR4" },   /* TIOVX_LOG_RT_MEM_ADDR / TIOVX_LOG_RT_MEM_SIZE */

    /* MAIR7 — cached normal (MSMC, L2 SRAM, DDR local regions) */
    { name: "MSMC",                 vAddr: 0x70000000, size: 0x00800000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* 8 MB MSMC (overrides device mapping for this range) */
    { name: "L2_SRAM",              vAddr: 0x64800000, size: 0x00200000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* L2 SRAM 448 KB */
    { name: "DDR_C7x_1_DTS",        vAddr: 0xB2100000, size: 0x05F00000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7x_1_DTS_ADDR / DDR_C7x_1_DTS_SIZE */
    /* LOCAL_HEAP and SCRATCH use non-identity vAddr/pAddr (64-bit DDR alias) */
    { name: "DDR_C7x_1_LOCAL_HEAP", vAddr: 0x117000000, pAddr: 0x897000000, size: 0x10000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_1_LOCAL_HEAP_VADDR / PADDR */
    { name: "DDR_C7x_1_SCRATCH",    vAddr: 0x100000000, pAddr: 0x880000000, size: 0x17000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_C7X_1_SCRATCH_VADDR / PADDR */
    { name: "DDR_SHARED_MEM",       vAddr: 0xB8000000, size: 0x20000000, attrIndx: "Mmu_AttrIndx_MAIR7" },   /* DDR_SHARED_MEM_ADDR / DDR_SHARED_MEM_SIZE */
];

const REGIONS_BY_CORE = {
    "c71ss0-0": C7X_1_REGIONS,
};

/* --------------------------------------------------------------------------
 * Main export
 * -------------------------------------------------------------------------- */
function populate_mmu_regions(containerInst, ind, device, core) {
    if (device !== "J721E_DRA829_TDA4VM_AM752x" && device !== "J721E") return;

    let coreKey;
    if (core.includes("c71ss0-0")) coreKey = "c71ss0-0";
    else return;

    let regions = REGIONS_BY_CORE[coreKey];
    if (!regions) return;

    let cfg = regions[ind];
    if (cfg === undefined || cfg === null) return;

    applyRegion(containerInst.mmu_region[ind], cfg);
}

function getMmuRegionCount(core) {
    if (core.includes("c71ss0-0")) return C7X_1_REGIONS.length;
    return 0;
}

exports.populate_mmu_regions = populate_mmu_regions;
exports.getMmuRegionCount    = getMmuRegionCount;
