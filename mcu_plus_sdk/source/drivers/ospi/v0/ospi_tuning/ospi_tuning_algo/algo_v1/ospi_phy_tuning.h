/*
 *  Copyright (C) 2021-24 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef OSPI_NEW_TUNING_H_
#define OSPI_NEW_TUNING_H_

/**
 *  \file ospi_phy_new_tuning.h
 *
 *  \brief OSPI New tuning algo API/Interface file.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef ENABLE_PHY_TUNING_SOC_BUILD
// Include header files needed for the SOC
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \brief Success status code for OSPI PHY tuning operations
 *
 * Returned when tuning operations complete successfully.
 */
#define OSPI_PHY_TUNING_SUCCESS (0)

/**
 * \brief Failure status code for OSPI PHY tuning operations
 *
 * Returned when tuning operations fail to find valid parameters.
 */
#define OSPI_PHY_TUNING_FAILURE (-1)

/**
 * \brief Fail value for Read delay
 *
 * Fail value for read delay assigned during initialisation and ospi tuning
 * failure
 */
#define READ_DELAY_INVALID -1

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    int txDLL;
    int rxDLL;
    int rdDelay;
} OSPI_PhyConfig;

/**
 * \struct  OSPI_phyParams
 * \brief OSPI parameteters for OSPI PHY tuning
 *
 * This structure provide parameters that are required for OSPI Phy
 * tuning.
 *
 */
typedef struct
{
    /* Search radius used for midpoint verification */
    int32_t radius;

    /**
     * \brief TX & RX DLL min value
     *
     *  Minimum value for OSPI PHY RX/TX DLL configuration setting
     */
    int32_t rxTxDllMin;

    /**
     * \brief TX & RX DLL max value
     *
     *  Maximum value for OSPI PHY RX/TX DLL configuration setting
     */
    int32_t rxTxDllMax;

    /**
     * \brief Minimum read delay setting for OSPI PHY tuning
     *
     * Starting value for read delay during the tuning process.
     */
    int8_t minReadDelay;

    /**
     * \brief Maximum read delay value for OSPI PHY tuning
     *
     * Upper limit for read delay parameter during the tuning process.
     * Values beyond this are not considered valid.
     */
    int8_t maxReadDelay;

    /**
     * \brief Minimum size requirement for a valid passing region
     *
     * The squared length of a passing region must exceed this value
     * to be considered large enough for stable operation.
     * Size is calculated as (dx*dx + dy*dy) where dx and dy are
     * the x and y distances in the parameter space.
     */
    int32_t minPassSize;

    /**
     * \brief Shift value for diagonal search pattern
     *
     * When primary diagonal search fails, this defines how much to shift
     * the diagonal search line for subsequent attempts. Higher values
     * will search more sparsely but cover the parameter space faster.
     */
    int32_t diagonalShift;

    /**
     * /brief Max diagonal shift value
     *
     * Maximum allowed diagonal shift value during PHY tuning process.
     * This value defines the upper limit for diagonal timing adjustments
     * when calibrating the OSPI interface.
     */
    int32_t maxDiagonalShift;


    /**
     * \brief Number of consecutive failing points required
     *
     * When identifying gaps between passing regions, this many consecutive
     * failing test points are required to confirm a genuine gap.
     * Helps distinguish between isolated failures and actual region boundaries.
     */
    int32_t numConsecutiveFail;

    /**
     * \brief Number of consecutive passing points required
     *
     * When searching for stable operating regions, this many consecutive
     * passing test points are required to confirm a valid region.
     * Higher values ensure more robust operation but may be harder to satisfy.
     */
    int32_t numConsecutivePass;

    /**
     * \brief Step size for read delay parameter search
     *
     * Controls the granularity of the search for valid read delay values.
     * Larger values will search more sparsely but complete faster.
     */
    int32_t rdDelaySearchStep;
}OSPI_phyParams;

/**
 * \struct OSPI_phyOps
 * \brief Operation function pointers for OSPI PHY tuning
 *
 * This structure provides function pointers that implement the specific
 * tuning operations for the OSPI PHY interface.
 */
typedef struct
{
    /**
     * \brief Function pointer for PHY tuning operations
     *
     * \param handle      Pointer to the OSPI controller instance or context
     * \param offset      Memory offset to use for tuning operations
     * \param searchPoint Pointer to configuration parameters being evaluated
     *
     * \return Status code indicating success (0) or specific error
     */
    int32_t (*ops)(void* handle, uint32_t offset, OSPI_PhyConfig *searchPoint);

    /* Struct pointer to ospi tuning parameters */
    OSPI_phyParams* phyParams;
}OSPI_phyOps;

/* ========================================================================== */
/*                       Function Declarations                                */
/* ========================================================================== */

/**
 * \brief Validates if all points within a circular area around a configuration
 *        point pass
 *
 * This function tests every point within a specified circular radius around a
 * given OSPI PHY configuration point to verify stability. For a configuration
 * to be considered stable and robust, all points within this circular region
 * must pass the PHY operations test.
 *
 * The function performs these steps:
 * 1. First checks if the specified radius would extend beyond valid parameter
 *    boundaries (0-127)
 * 2. If boundaries are valid, tests each point within the circular area
 * 3. Returns success only if ALL points within the circular region pass
 *
 * This approach ensures that the selected configuration has sufficient margin
 * in all directions and is not at the edge of a passing region, which would
 * make it susceptible to environmental variations or noise.
 *
 * \param handle    Pointer to the OSPI device handle
 * \param offset    Memory offset used for testing
 * \param phyOps    Pointer to PHY operations structure with test functions
 * \param radius    Radius of the circular area to verify (in DLL units)
 * \param point     Pointer to the center configuration point to validate
 *
 * \return OSPI_PHY_TUNING_SUCCESS if all points within the circle pass,
 *         OSPI_PHY_TUNING_FAILURE if any point fails or the circle extends beyond
 *                             valid boundaries
 */
int32_t OSPI_phyCheckCircleArea(void* handle, uint32_t offset,
                                OSPI_phyOps *phyOps, int32_t radius,
                                OSPI_PhyConfig *point);

/**
 * \brief Find optimal PHY configuration using diagonal search patterns
 *
 * This function attempts to find optimal tuning parameters (OTP) for OSPI PHY
 * configuration by performing multiple diagonal search patterns across the
 * TxDLL-RxDLL space. It uses a systematic approach to cover the parameter space:
 *
 * 1. First tries a diagonal search from origin (0,0) to (127,127)
 * 2. If unsuccessful, progressively moves the diagonal upward
 *    (higher RxDLL, lower TxDLL)
 * 3. After each upward shift, also attempts a rightward shift
 *    (higher TxDLL, lower RxDLL)
 * 4. Continues until a valid configuration is found or search space is
 *    exhausted
 *
 * This comprehensive search strategy ensures thorough coverage of the parameter
 * space to identify a stable operating region for the OSPI interface.
 *
 * \param handle    Pointer to the OSPI device handle
 * \param offset    Memory offset used for testing
 * \param phyOps    Pointer to PHY operations structure
 * \param radius    Search radius used for midpoint verification
 * \param otp       Pointer to store the optimal PHY configuration when found
 *
 * \return OSPI_PHY_TUNING_SUCCESS if optimal configuration found,
 *         OSPI_PHY_TUNING_FAILURE if no valid configuration exists in the search
 *                             space
 */
int32_t OSPI_phyFindOTP4(void* handle, uint32_t offset, OSPI_phyOps *phyOps,
                         int32_t radius, OSPI_PhyConfig *otp);

                         /**
 * \brief Performs a diagonal check to ensure the tuning point is valid and away
 *        from noise
 *
 * This function validates the selected tuning point by checking diagonally adjacent
 * points to ensure the tuning point is stable and not affected by signal noise.
 * It helps guarantee the reliability of the selected OSPI PHY configuration.
 *
 * \param handle Pointer to the OSPI driver handle
 * \param offset Memory offset for tuning operations
 * \param phyOps Pointer to PHY operation functions
 * \param point Pointer to the PHY configuration to be validated
 *
 * \return Status code indicating success or failure of the diagonal check
 */
int32_t OSPI_phyCheckDiagonal(void* handle, uint32_t offset, OSPI_phyOps *phyOps,
                              OSPI_PhyConfig point);

#endif
