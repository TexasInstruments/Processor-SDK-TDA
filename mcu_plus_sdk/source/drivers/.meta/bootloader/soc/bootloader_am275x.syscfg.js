let common = system.getScript("/common");

const bootloader_config_r5fss = [
    {
        name            : "BOOTLOADER0",
    },
];

const bootloader_bootmedia = [

    { name: "FLASH", displayName: "Flash" },
    { name: "MEM", displayName: "SOC Memory" },
    { name: "EMMC", displayName: "EMMC"},
    { name: "SD", displayName: "SD"},
    { name: "UART", displayName: "UART"},
];

const dma_restrict_regions = [
    { start : "CSL_WKUP_R5FSS0_CORE0_ATCM_BASE", size : "CSL_WKUP_R5FSS0_CORE0_ATCM_SIZE" },
    { start : "CSL_WKUP_R5FSS0_CORE0_BTCM_BASE", size : "CSL_WKUP_R5FSS0_CORE0_BTCM_SIZE" },
    { start : "CSL_R5FSS0_CORE0_ATCM_BASE", size : "CSL_R5FSS0_CORE0_ATCM_SIZE" },
    { start : "CSL_R5FSS0_CORE0_BTCM_BASE", size : "CSL_R5FSS0_CORE0_BTCM_SIZE" },
    { start : "CSL_R5FSS0_CORE1_ATCM_BASE", size : "CSL_R5FSS0_CORE1_ATCM_SIZE" },
    { start : "CSL_R5FSS0_CORE1_BTCM_BASE", size : "CSL_R5FSS0_CORE1_BTCM_SIZE" },
    { start : "CSL_R5FSS1_CORE0_ATCM_BASE", size : "CSL_R5FSS1_CORE0_ATCM_SIZE" },
    { start : "CSL_R5FSS1_CORE0_BTCM_BASE", size : "CSL_R5FSS1_CORE0_BTCM_SIZE" },
    { start : "CSL_R5FSS1_CORE1_ATCM_BASE", size : "CSL_R5FSS1_CORE1_ATCM_SIZE" },
    { start : "CSL_R5FSS1_CORE1_BTCM_BASE", size : "CSL_R5FSS1_CORE1_BTCM_SIZE" },

];

function getDmaRestrictedRegions() {

    return dma_restrict_regions;
}

function getDefaultConfig()
{
    return bootloader_config_r5fss[0];
}

function getConfigArr() {

    return bootloader_config_r5fss;
}

function getBootMediaArr() {

	return bootloader_bootmedia;
}

exports = {
    getDmaRestrictedRegions,
    getDefaultConfig,
    getConfigArr,
    getBootMediaArr,
};

