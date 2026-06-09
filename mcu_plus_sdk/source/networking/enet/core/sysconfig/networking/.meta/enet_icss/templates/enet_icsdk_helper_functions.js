function importHsrHeaders(instances)
{
    for(let i in instances)
    {
        if (instances[i].derivedMode === "HSR") {
            return `
#include <hsr/RX_PRU_SLICE0_bin.h>
#include <hsr/RX_PRU_SLICE1_bin.h>
#include <hsr/RTU0_SLICE0_bin.h>
#include <hsr/RTU0_SLICE1_bin.h>
#include <hsr/TX_PRU_SLICE0_bin.h>
#include <hsr/TX_PRU_SLICE1_bin.h>

        `;
        }
    }

    return "";
}

function importPrpHeaders(instances)
{
    for(let i in instances)
    {
        if (instances[i].derivedMode === "PRP") {
            return `
#include <prp/RX_PRU_SLICE0_bin.h>
#include <prp/RX_PRU_SLICE1_bin.h>
#include <prp/RTU0_SLICE0_bin.h>
#include <prp/RTU0_SLICE1_bin.h>
#include <prp/TX_PRU_SLICE0_bin.h>
#include <prp/TX_PRU_SLICE1_bin.h>
        `;
        }
    }

    return "";
}

function assignHsrPrpFirmware(instance)
{
    if (instance.derivedMode === "HSR") {
        return `
    /* HSR firmware for both slices */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_bin_HSR,
            .pruSize   = sizeof(RX_PRU_SLICE0_bin_HSR),
            .rtu       = RTU0_SLICE0_bin_HSR,
            .rtuSize   = sizeof(RTU0_SLICE0_bin_HSR),
            .txpru     = TX_PRU_SLICE0_bin_HSR,
            .txpruSize = sizeof(TX_PRU_SLICE0_bin_HSR)
        },
        {
            .pru       = RX_PRU_SLICE1_bin_HSR,
            .pruSize   = sizeof(RX_PRU_SLICE1_bin_HSR),
            .rtu       = RTU0_SLICE1_bin_HSR,
            .rtuSize   = sizeof(RTU0_SLICE1_bin_HSR),
            .txpru     = TX_PRU_SLICE1_bin_HSR,
            .txpruSize = sizeof(TX_PRU_SLICE1_bin_HSR)
        },
    },
        `
    } 
    if (instance.derivedMode === "PRP") {
        return `
    /* PRP firmware for both slices */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_bin_PRP,
            .pruSize   = sizeof(RX_PRU_SLICE0_bin_PRP),
            .rtu       = RTU0_SLICE0_bin_PRP,
            .rtuSize   = sizeof(RTU0_SLICE0_bin_PRP),
            .txpru     = TX_PRU_SLICE0_bin_PRP,
            .txpruSize = sizeof(TX_PRU_SLICE0_bin_PRP)
        },
        {
            .pru       = RX_PRU_SLICE1_bin_PRP,
            .pruSize   = sizeof(RX_PRU_SLICE1_bin_PRP),
            .rtu       = RTU0_SLICE1_bin_PRP,
            .rtuSize   = sizeof(RTU0_SLICE1_bin_PRP),
            .txpru     = TX_PRU_SLICE1_bin_PRP,
            .txpruSize = sizeof(TX_PRU_SLICE1_bin_PRP)
        },
    },
        `
    }
    return "";
}

module.exports = {importHsrHeaders, importPrpHeaders, assignHsrPrpFirmware};

