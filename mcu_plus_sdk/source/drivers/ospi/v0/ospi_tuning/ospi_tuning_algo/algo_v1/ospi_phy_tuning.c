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

/**
 *  \file ospi_phy_new_tuning.c
 *
 *  \brief File containing the implementation of the new tuning algorithm for
 *         the OSPi device.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ospi_phy_tuning.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None*/

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t OSPI_phyCheckDiagonal(void* handle, uint32_t offset, OSPI_phyOps *phyOps,
                              OSPI_PhyConfig point)
{
    int32_t status = OSPI_PHY_TUNING_SUCCESS;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    int32_t rxMin = point.rxDLL - (phyParams->radius);
    int32_t rxMax = point.rxDLL + (phyParams->radius);
    int32_t txMin = point.txDLL - (phyParams->radius);
    int32_t txMax = point.txDLL + (phyParams->radius);
    int32_t readDelay = point.rdDelay;

    /* First check if the circle stays within valid parameter boundaries (0-127) */
    if((rxMax > phyParams->rxTxDllMax) || (rxMin < phyParams->rxTxDllMin) ||
       (txMax > phyParams->rxTxDllMax) || (txMin < phyParams->rxTxDllMin))
    {
        status = OSPI_PHY_TUNING_FAILURE;
    }

    while((rxMax > point.rxDLL) && (txMax > point.txDLL) &&
          (status == OSPI_PHY_TUNING_SUCCESS))
    {
        /* Configure the test point with current coordinates and same
         * read delay as center
         */
        OSPI_PhyConfig temp = {txMax, rxMax, readDelay};

        /* Test if this configuration passes */
        if(phyOps->ops(handle, offset, &temp) ==
           OSPI_PHY_TUNING_FAILURE)
        {
            /* If any point within the circle fails, the entire test
             * fails. This ensures the center point has sufficient
             * margin in all directions
             */
            status = OSPI_PHY_TUNING_FAILURE;
            break;
        }

        rxMax--;
        txMax--;
    }

    if(status == OSPI_PHY_TUNING_SUCCESS)
    {
        while((rxMin < point.rxDLL) && (txMin < point.txDLL) &&
              (status == OSPI_PHY_TUNING_SUCCESS))
        {
            /* Configure the test point with current coordinates and same
             * read delay as center
             */
            OSPI_PhyConfig temp = {txMin, rxMin, readDelay};

            /* Test if this configuration passes */
            if(phyOps->ops(handle, offset, &temp) ==
               OSPI_PHY_TUNING_FAILURE)
            {
                /* If any point within the circle fails, the entire test
                 * fails. This ensures the center point has sufficient
                 * margin in all directions
                 */
                status = OSPI_PHY_TUNING_FAILURE;
                break;
            }

            rxMin++;
            txMin++;
        }
    }

    return status;
}

int32_t OSPI_phyCheckCircleArea(void* handle, uint32_t offset,
                                OSPI_phyOps *phyOps, int32_t radius,
                                OSPI_PhyConfig *point)
{
    int32_t status = OSPI_PHY_TUNING_SUCCESS;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    /* Pre-calculate square of radius and boundaries once */
    int32_t radiusSquared = radius * radius;
    int32_t rxMin = point->rxDLL - radius;
    int32_t rxMax = point->rxDLL + radius;
    int32_t txMin = point->txDLL - radius;
    int32_t txMax = point->txDLL + radius;
    int32_t i = 0;
    int32_t j = 0;

    /* First check if the circle stays within valid parameter boundaries (0-127) */
    if((rxMax > phyParams->rxTxDllMax) || (rxMin < phyParams->rxTxDllMin) ||
       (txMax > phyParams->rxTxDllMax) || (txMin < phyParams->rxTxDllMin))
    {
        status = OSPI_PHY_TUNING_FAILURE;
    }

    if(status == OSPI_PHY_TUNING_SUCCESS)
    {
        /* Iterate through all points within a square that contains bottom left
         * quadrant of circle
         */
        for(i = rxMin; (i <= point->rxDLL) &&
                       (status == OSPI_PHY_TUNING_SUCCESS); i++)
        {
            int32_t dx = i - point->rxDLL;
            int32_t dxSquared = dx * dx;

            for(j = txMin; (j <= point->txDLL) &&
                           (status == OSPI_PHY_TUNING_SUCCESS); j++)
            {
                if((point->rxDLL == i) && (point->txDLL == j))
                {
                    /* Skip search point */
                    continue;
                }

                /* Calculate Euclidean distance from the center point */
                int32_t dy = j - point->txDLL;
                int32_t distance = dxSquared + (dy * dy);

                /* Only test points that fall within the circular boundary */
                if (distance <= radiusSquared)
                {
                    /* Configure the test point with current coordinates and same
                     * read delay as center
                     */
                    OSPI_PhyConfig temp = {j, i, point->rdDelay};

                    /* Test if this configuration passes */
                    if(phyOps->ops(handle, offset, &temp) ==
                       OSPI_PHY_TUNING_FAILURE)
                    {
                        /* If any point within the circle fails, the entire test
                         * fails. This ensures the center point has sufficient
                         * margin in all directions
                         */
                        status = OSPI_PHY_TUNING_FAILURE;
                        break;
                    }
                }
            }
        }

        /* Iterate through all points within a square that contains top right
         * quadrant of the circle
         */
        for(i = point->rxDLL; (i <= rxMax) &&
                              (status == OSPI_PHY_TUNING_SUCCESS); i++)
        {
            int32_t dx = i - point->rxDLL;
            int32_t dxSquared = dx * dx;

            for(j = point->txDLL; (j <= txMax) &&
                                  (status == OSPI_PHY_TUNING_SUCCESS); j++)
            {
                if((point->rxDLL == i) && (point->txDLL == j))
                {
                    /* Skip search point */
                    continue;
                }

                /* Calculate Euclidean distance from the center point */
                int32_t dy = j - point->txDLL;
                int32_t distance = dxSquared + (dy * dy);

                /* Only test points that fall within the circular boundary */
                if (distance <= radiusSquared)
                {
                    /* Configure the test point with current coordinates and
                     * same read delay as center
                     */
                    OSPI_PhyConfig temp = {j, i, point->rdDelay};

                    /* Test if this configuration passes */
                    if(phyOps->ops(handle, offset, &temp) ==
                       OSPI_PHY_TUNING_FAILURE)
                    {
                        /* If any point within the circle fails, the entire test
                         * fails. This ensures the center point has sufficient
                         * margin in all directions
                         */
                        status = OSPI_PHY_TUNING_FAILURE;
                        break;
                    }
                }
            }
        }
    }

    return status;
}

/**
 * \brief Find consecutive passing points by incrementing TX/RX DLL values
 *
 * This function tests for consecutive passing points in the upward direction
 * (increasing DLL values). Starting from the given search point, it increments
 * both TX and RX DLL values simultaneously and checks if they pass the read
 * attack test. The function continues until either:
 * - It finds the requested number of consecutive passing points (max)
 * - It encounters a failing point
 *
 * \param     handle         Device handle for PHY operations
 * \param     offset         Memory offset for read attack operation
 * \param     phyOps         Structure containing PHY operation functions
 * \param     searchPoint    Starting configuration point for search
 * \param     max            Required number of consecutive passing points
 * \param     count          Counter for consecutive passing points found
 * \param     rdAttackStatus Expected status for passing points
 *
 * \return OSPI_PHY_TUNING_SUCCESS if required consecutive points found
 *         OSPI_PHY_TUNING_FAILURE otherwise
 */
static int32_t OSPI_phyConsecutivePassingPointsUp(void* handle, uint32_t offset,
                                                  OSPI_phyOps *phyOps,
                                                  OSPI_PhyConfig searchPoint,
                                                  int32_t max, int8_t *count,
                                                  int32_t rdAttackStatus)
{
    int32_t status = OSPI_PHY_TUNING_FAILURE;
    /* Copy the starting search point for manipulation */
    OSPI_PhyConfig passPoint = searchPoint;

    /* Continue searching until we find enough consecutive passing points */
    while(*count < max)
    {
        /* Increment both TX and RX DLL values for the next test point */
        passPoint.txDLL++;
        passPoint.rxDLL++;

        /* Test if the new point passes the read attack test */
        if(phyOps->ops(handle, offset, &passPoint) == rdAttackStatus)
        {
            /* Increment counter for consecutive passing points */
            (*count)++;
        }
        else
        {
            /* Break out of loop when a failing point is encountered */
            break;
        }
    }

    /* Set return status based on whether we found enough passing points */
    if(*count >= max)
    {
        /* Success: Found the required number of consecutive passing points */
        status = OSPI_PHY_TUNING_SUCCESS;
    }
    else
    {
        /* Failure: Did not find enough consecutive passing points */
        status = OSPI_PHY_TUNING_FAILURE;
    }

    return status;
}

/**
 * Checks for consecutive passing points in the downward direction.
 *
 * This function decrements TX and RX DLL values from the given search point and
 * verifies if the required number of consecutive points pass the read attack test.
 *
 * \param handle          Pointer to the OSPI controller handle
 * \param offset          Memory offset for read operation
 * \param phyOps          Pointer to PHY operations structure
 * \param searchPoint     Starting point configuration for the search
 * \param max             Maximum number of consecutive passing points required
 * \param count           Pointer to store the count of consecutive passing points
 * \param rdAttackStatus  Expected status value indicating a passing read attack
 *
 * \return OSPI_PHY_TUNING_SUCCESS if required consecutive points are found,
 *         OSPI_PHY_TUNING_FAILURE otherwise
 */
static int32_t OSPI_phyConsecutivePassingPointsDown(void* handle,
                                                    uint32_t offset,
                                                    OSPI_phyOps *phyOps,
                                                    OSPI_PhyConfig searchPoint,
                                                    int32_t max, int8_t* count,
                                                    int32_t rdAttackStatus)
{
    /* Initialize status to failure as default */
    int32_t status = OSPI_PHY_TUNING_FAILURE;

    /* Copy the search point to work with */
    OSPI_PhyConfig passPoint = searchPoint;

    /* Keep decrementing TX/RX DLL values until max count is reached or test fails */
    while(*count < max)
    {
        /* Decrement both TX and RX DLL values to move down in the search space */
        passPoint.txDLL--;
        passPoint.rxDLL--;

        /* Perform PHY operation and check if it passes with expected status */
        if(phyOps->ops(handle, offset, &passPoint) == rdAttackStatus)
        {
            /* Increment counter for consecutive passing points */
            (*count)++;
        }
        else
        {
            /* Break loop at first failure */
            break;
        }
    }

    /* Set success if required number of consecutive passing points were found */
    if(*count >= max)
    {
        status = OSPI_PHY_TUNING_SUCCESS;
    }
    else
    {
        /* Keep failure status if we didn't find enough passing points */
        status = OSPI_PHY_TUNING_FAILURE;
    }

    return status;
}

/**
 * OSPI_phyFindTuningPoint - Finds an optimal configuration point in PHY calibration
 *
 * This function determines the tuning point of the valid PHY configuration area
 * by exploring boundaries and testing if a radius around the tuning point
 * contains valid configurations.
 *
 * \param handle      : Pointer to OSPI device handle
 * \param offset      : Memory offset for operations
 * \param phyOps      : PHY operation function pointers
 * \param radius      : Required valid radius around tuningPoint
 * \param searchPoint : Input/output parameter for PHY configuration
 *
 * \return OSPI_PHY_TUNING_SUCCESS if a valid midpoint is found, otherwise failure
 */
static int32_t OSPI_phyFindTuningPoint(void* handle, uint32_t offset,
                                       OSPI_phyOps *phyOps, int32_t radius,
                                       OSPI_PhyConfig *searchPoint)
{
    int32_t status = OSPI_PHY_TUNING_FAILURE;
    OSPI_PhyConfig topLeft = *searchPoint;
    OSPI_PhyConfig bottomRight = *searchPoint;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    /* Find upper-left boundary by decreasing TX and increasing RX */
    while((topLeft.txDLL >= phyParams->rxTxDllMin) &&
          (topLeft.rxDLL <= phyParams->rxTxDllMax))
    {
        int32_t oldTx = topLeft.txDLL;
        int32_t oldRx = topLeft.rxDLL;

        topLeft.txDLL -= 1;
        topLeft.rxDLL += 1;

        if(phyOps->ops(handle, offset, &topLeft) != OSPI_PHY_TUNING_SUCCESS) {
            /* Restore last valid values */
            topLeft.txDLL = oldTx;
            topLeft.rxDLL = oldRx;
            break;
        }
    }

    /* Find bottom-right boundary by increasing TX and decreasing RX */
    while((bottomRight.txDLL <= phyParams->rxTxDllMax) &&
          (bottomRight.rxDLL >= phyParams->rxTxDllMin))
    {
        int32_t oldTx = bottomRight.txDLL;
        int32_t oldRx = bottomRight.rxDLL;

        bottomRight.txDLL += 1;
        bottomRight.rxDLL -= 1;

        if(phyOps->ops(handle, offset, &bottomRight) != OSPI_PHY_TUNING_SUCCESS)
        {
            /* Restore last valid values */
            bottomRight.txDLL = oldTx;
            bottomRight.rxDLL = oldRx;
            break;
        }
    }

    /* Calculate squared diagonal length between the two boundary points */
    int32_t txDiff = topLeft.txDLL - bottomRight.txDLL;
    int32_t rxDiff = topLeft.rxDLL - bottomRight.rxDLL;
    int32_t lenSquared = (txDiff * txDiff) + (rxDiff * rxDiff);

    /* Only proceed if area is larger than minimum pass length */
    if(lenSquared > phyParams->minPassSize)
    {
        /* Calculate initial midpoint between the two boundaries */
        txDiff = abs(topLeft.txDLL - bottomRight.txDLL);
        rxDiff = abs(topLeft.rxDLL - bottomRight.rxDLL);

        searchPoint->txDLL = topLeft.txDLL + (txDiff / 2);
        searchPoint->rxDLL = topLeft.rxDLL - (rxDiff / 2);

        /* Check if area around midpoint has valid configurations */
        status = OSPI_phyCheckCircleArea(handle, offset, phyOps, radius,
                                         searchPoint);

        /* If initial midpoint check fails, try alternative positions */
        if(status != OSPI_PHY_TUNING_SUCCESS)
        {
            /* Calculate squared distances from boundaries to current midpoint */
            int32_t txDiff1 = topLeft.txDLL - searchPoint->txDLL;
            int32_t rxDiff1 = topLeft.rxDLL - searchPoint->rxDLL;
            int32_t len3Squared = (txDiff1 * txDiff1) + (rxDiff1 * rxDiff1);

            int32_t txDiff2 = bottomRight.txDLL - searchPoint->txDLL;
            int32_t rxDiff2 = bottomRight.rxDLL - searchPoint->rxDLL;
            int32_t len4Squared = (txDiff2 * txDiff2) + (rxDiff2 * rxDiff2);

            /* If either distance exceeds min pass size, adjust position */
            if((len3Squared >= phyParams->minPassSize) ||
               (len4Squared >= phyParams->minPassSize))
            {
                if(len3Squared == len4Squared)
                {
                    /* Equal distances - try toward top-left first */
                    searchPoint->txDLL = searchPoint->txDLL - (abs(txDiff1) / 2);
                    searchPoint->rxDLL = searchPoint->rxDLL + (abs(rxDiff1) / 2);

                    status = OSPI_phyCheckCircleArea(handle, offset, phyOps,
                                                     radius, searchPoint);

                    if(status != OSPI_PHY_TUNING_SUCCESS)
                    {
                        /* If first attempt fails, try toward bottom-right */
                        searchPoint->txDLL = searchPoint->txDLL +
                                             (abs(txDiff2) / 2);
                        searchPoint->rxDLL = searchPoint->rxDLL -
                                             (abs(rxDiff2) / 2);

                        status = OSPI_phyCheckCircleArea(handle, offset, phyOps,
                                                         radius, searchPoint);
                    }
                }
                else
                {
                    /* Unequal distances - move toward the farther boundary */
                    if(len3Squared > len4Squared)
                    {
                        /* Move toward top-left boundary */
                        searchPoint->txDLL = searchPoint->txDLL -
                                             (abs(txDiff1) / 2);
                        searchPoint->rxDLL = searchPoint->rxDLL +
                                             (abs(rxDiff1) / 2);
                    }
                    else
                    {
                        /* Move toward bottom-right boundary */
                        searchPoint->txDLL = searchPoint->txDLL +
                                             (abs(txDiff2) / 2);
                        searchPoint->rxDLL = searchPoint->rxDLL -
                                             (abs(rxDiff2) / 2);
                    }

                    status = OSPI_phyCheckCircleArea(handle, offset, phyOps,
                                                     radius, searchPoint);
                }
            }
        }
    }

    return status;
}

/**
 * \brief Find next passing point by incrementing DLL values upward
 *
 * This function searches for the next passing configuration point by
 * incrementing both RX and TX DLL values simultaneously until either:
 * - The operation returns the expected status, indicating a passing point
 * - The search reaches the maximum allowed boundary (endPoint)
 *
 * \param handle          Handle to OSPI device
 * \param offset          Memory offset for test operations
 * \param phyOps          Structure containing PHY operation function pointers
 * \param searchPoint     Current PHY configuration, updated during search
 * \param endPoint        Maximum boundary values for the search
 * \param rdAttackStatus  Expected status value indicating a passing point
 */
static void OSPI_phyFindNextPassingPointUp(void *handle, int32_t offset,
                                           OSPI_phyOps *phyOps,
                                           OSPI_PhyConfig *searchPoint,
                                           OSPI_PhyConfig *endPoint,
                                           int32_t rdAttackStatus)
{
    while ((searchPoint->txDLL < endPoint->txDLL) &&
           (searchPoint->rxDLL < endPoint->rxDLL))
    {
        /* Increment both RX and TX DLL values simultaneously */
        searchPoint->rxDLL++;
        searchPoint->txDLL++;

        /* Check if we've found a passing point */
        if (phyOps->ops(handle, offset, searchPoint) == rdAttackStatus)
        {
            /* Exit loop when we find a passing point */
            break;
        }
    }
}

/**
 * \brief Find the largest continuous passing region moving upward in DLL values
 *
 * This function searches upward (increasing both TX and RX DLL values) to find
 * the longest continuous sequence of passing configurations. It updates the
 * startPoint and endPoint to define this optimal region.
 *
 * \param handle      Device handle for the OSPI interface
 * \param offset      Memory offset for read/write operations
 * \param phyOps      Structure containing PHY operation function pointers
 * \param startPoint  Input: initial configuration; Output: optimal region start
 * \param endPoint    Input: search boundary; Output: optimal region end
 */
static void OSPI_phyFindTheBiggestPassingLineUp(void* handle, uint32_t offset,
                                                OSPI_phyOps *phyOps,
                                                OSPI_PhyConfig *startPoint,
                                                OSPI_PhyConfig *endPoint)
{
    OSPI_PhyConfig searchPoint = *startPoint;
    OSPI_PhyConfig bestSequenceStart = *startPoint;

    /* Length of current passing region, start point is already passing */
    int8_t currentLength = 1;
    int8_t maxLength = 1;        /* Maximum length found so far */

    /* Search upward for the longest passing line */
    while (true)
    {
        /* Move diagonally upward */
        searchPoint.txDLL += 1;
        searchPoint.rxDLL += 1;

        /* Check if we're still within bounds */
        if ((searchPoint.txDLL > endPoint->txDLL) ||
            (searchPoint.rxDLL > endPoint->rxDLL))
        {
            break;
        }

        /* Check if the current point passes validation */
        if (phyOps->ops(handle, offset, &searchPoint) == OSPI_PHY_TUNING_SUCCESS)
        {
            /* Current point passes, increment our passing sequence length */
            currentLength++;
        }
        else
        {
            /* Current point failed, evaluate the sequence we just found */
            if (currentLength > maxLength)
            {
                /* We found a longer sequence, update our tracking */
                maxLength = currentLength;
                bestSequenceStart.txDLL = searchPoint.txDLL - currentLength;
                bestSequenceStart.rxDLL = searchPoint.rxDLL - currentLength;
            }
            currentLength = 0;  /* Reset length for next potential sequence */

            /* Find the next passing point continuing upward direction */
            OSPI_phyFindNextPassingPointUp(handle, offset, phyOps,
                                           &searchPoint, endPoint,
                                           OSPI_PHY_TUNING_SUCCESS);

            /* If we found a new passing point, initialize length to 1 */
            if ((searchPoint.txDLL <= endPoint->txDLL) &&
                (searchPoint.rxDLL <= endPoint->rxDLL))
            {
                currentLength = 1;
            }
        }
    }

    /* Check if the final sequence is the longest one */
    if (currentLength > maxLength)
    {
        maxLength = currentLength;
        bestSequenceStart.txDLL = searchPoint.txDLL - currentLength + 1;
        bestSequenceStart.rxDLL = searchPoint.rxDLL - currentLength + 1;
    }

    /* Update output parameters with the optimal region found */
    startPoint->txDLL = bestSequenceStart.txDLL;
    startPoint->rxDLL = bestSequenceStart.rxDLL;
    endPoint->txDLL = startPoint->txDLL + maxLength - 1;
    endPoint->rxDLL = startPoint->rxDLL + maxLength - 1;
}

/**
 * OSPI_phyFindNextPassingPointDown - Search for next passing point downward
 *
 * This function decrements both RxDLL and TxDLL values until either:
 *   1. One of the DLL values reaches its minimum threshold (endPoint)
 *   2. A read operation produces the expected attack status
 *
 * \param handle        : Pointer to OSPI driver instance
 * \param offset        : Memory offset for read operation
 * \param phyOps        : Pointer to PHY operations structure
 * \param searchPoint   : Current DLL configuration to test and adjust
 * \param endPoint      : Minimum allowable DLL values
 * \param rdAttackStatus: Expected read status when operation succeeds
 *
 * \return None
 */
static void OSPI_phyFindNextPassingPointDown(void *handle, int32_t offset,
                                             OSPI_phyOps *phyOps,
                                             OSPI_PhyConfig *searchPoint,
                                             OSPI_PhyConfig *endPoint,
                                             int32_t rdAttackStatus)
{
    /* Single loop with early termination conditions */
    while((searchPoint->txDLL > endPoint->txDLL) &&
          (searchPoint->rxDLL > endPoint->rxDLL))
    {
        /* Decrement both delay values */
        searchPoint->rxDLL--;
        searchPoint->txDLL--;

        /* Check if we found a passing configuration and exit early if so */
        if(phyOps->ops(handle, offset, searchPoint) == rdAttackStatus)
        {
            /* Exit loop when we find a passing point */
            break;
        }
    }
}

/**
 * OSPI_phyFindTheBiggestPassingLineDown - Find longest consecutive passing
 * configuration
 *
 * This function searches downward (decreasing DLL values) to find the longest
 * continuous sequence of passing PHY configurations. It updates the start and
 * end points to define this optimal region.
 *
 * \param handle    : Pointer to OSPI driver instance
 * \param offset    : Memory offset for read operation
 * \param phyOps    : Pointer to PHY operations structure
 * \param startPoint: On entry, the starting search configuration
 *                    On exit, the upper bound of the optimal window
 * \param endPoint  : On entry, the minimum allowed DLL values
 *                    On exit, the lower bound of the optimal window
 *
 * \return None
 */
static void OSPI_phyFindTheBiggestPassingLineDown(void* handle, uint32_t offset,
                                                  OSPI_phyOps *phyOps,
                                                  OSPI_PhyConfig *startPoint,
                                                  OSPI_PhyConfig *endPoint)
{
    OSPI_PhyConfig searchPoint = *startPoint;
    OSPI_PhyConfig bestSequenceStart = *startPoint;
    int8_t currentLength = 1;
    int8_t maxLength = 1;

    /* Search downward for the longest passing line */
    while(true)
    {
        /* Decrement both DLL values by 1 */
        searchPoint.txDLL -= 1;
        searchPoint.rxDLL -= 1;

        /* Check if we're still within bounds */
        if ((searchPoint.txDLL < endPoint->txDLL) ||
            (searchPoint.rxDLL < endPoint->rxDLL))
        {
            break;
        }

        /* Check if the current point passes */
        if(phyOps->ops(handle, offset, &searchPoint) == OSPI_PHY_TUNING_SUCCESS)
        {
            /* Count consecutive passing points */
            currentLength++;
        }
        else
        {
            /* If this segment is longer than previous best, save it */
            if(currentLength > maxLength)
            {
                maxLength = currentLength;
                /* Store upper bound of the best window found so far */
                /* Add 1 to adjust for the failing point */
                bestSequenceStart.txDLL = searchPoint.txDLL + currentLength;
                bestSequenceStart.rxDLL = searchPoint.rxDLL + currentLength;
            }
            currentLength = 0;

            /* Skip failing points until we find the next passing region */
            OSPI_phyFindNextPassingPointDown(handle, offset, phyOps,
                                             &searchPoint, endPoint,
                                             OSPI_PHY_TUNING_SUCCESS);

            if((searchPoint.txDLL >= endPoint->txDLL) &&
               (searchPoint.rxDLL >= endPoint->rxDLL))
            {
                 currentLength = 1;
            }
        }
    }

    /* Check if the final sequence is the longest one */
    if(currentLength > maxLength)
    {
        maxLength = currentLength;
        bestSequenceStart.txDLL = searchPoint.txDLL + currentLength - 1;
        bestSequenceStart.rxDLL = searchPoint.rxDLL + currentLength - 1;
    }

    /* Update output parameters with the optimal region found */
    startPoint->rxDLL = bestSequenceStart.rxDLL;
    startPoint->txDLL = bestSequenceStart.txDLL;
    endPoint->txDLL = startPoint->txDLL - maxLength + 1;
    endPoint->rxDLL = startPoint->rxDLL - maxLength + 1;

    return;
}

/**
 * OSPI_phyRdDelayIsValid - Validates if a read delay configuration is working
 *
 * This function tests OSPI read delay configurations by incrementally adjusting
 * TX and RX DLL values within a given range until a valid configuration is found
 * or the search limit is reached.
 *
 * \param handle      - Pointer to OSPI controller handle
 * \param offset      - Memory offset for the read operation
 * \param phyOps      - Pointer to PHY operations structure
 * \param searchPoint - Starting PHY configuration (TX/RX DLL values)
 * \param endPoint    - Maximum PHY configuration limits
 *
 * \return OSPI_PHY_TUNING_SUCCESS if a valid configuration is found,
 *         OSPI_PHY_TUNING_FAILURE otherwise
 */
static int32_t OSPI_phyRdDelayIsValid(void* handle, uint32_t offset,
                                      OSPI_phyOps *phyOps,
                                      OSPI_PhyConfig* searchPoint,
                                      OSPI_PhyConfig endPoint)
{
    /* Default status indicates failure until a working config is found */
    int32_t status = OSPI_PHY_TUNING_FAILURE;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    /*
     * Incrementally test OSPI configurations by stepping through TX/RX DLL values
     * Continue until a successful configuration is found or we reach search limits
     */
    while((searchPoint->txDLL <=
          (endPoint.txDLL - phyParams->rdDelaySearchStep)) &&
          (searchPoint->rxDLL <=
          (endPoint.rxDLL - phyParams->rdDelaySearchStep)) &&
          (status != OSPI_PHY_TUNING_SUCCESS))
    {
        /* Test the current configuration through PHY operations */
        status = phyOps->ops(handle, offset, searchPoint);

        /* Increment TX and RX DLL values by the defined search step */
        searchPoint->txDLL  += phyParams->rdDelaySearchStep;
        searchPoint->rxDLL  += phyParams->rdDelaySearchStep;
    }

    /* Return the final status after search completes */
    return status;
}

/**
 * \brief Finds valid read delays within a specified range
 *
 * This function iterates through possible read delay values to identify the
 * minimum and maximum valid read delays for a given OSPI configuration.
 *
 * \param handle        OSPI device handle
 * \param offset        Memory offset for testing operations
 * \param phyOps        PHY operations interface
 * \param bottomLeft    Bottom-left corner of the search space
 * \param topRight      Top-right corner of the search space
 * \param minReadDelay  Pointer to store the minimum valid read delay
 * \param maxReadDelay  Pointer to store the maximum valid read delay
 *
 * \return OSPI_PHY_TUNING_SUCCESS if valid read delays are found,
 *         OSPI_PHY_TUNING_FAILURE otherwise
 */
static int32_t OSPI_phyFindValidReadDelays(void* handle, uint32_t offset,
                                           OSPI_phyOps *phyOps,
                                           OSPI_PhyConfig bottomLeft,
                                           OSPI_PhyConfig topRight,
                                           int8_t* minReadDelay,
                                           int8_t* maxReadDelay)
{
    int32_t status = OSPI_PHY_TUNING_SUCCESS;
    int8_t readDelay;
    OSPI_PhyConfig searchPoint = bottomLeft;
    OSPI_phyParams* phyParams = phyOps->phyParams;
    *minReadDelay = READ_DELAY_INVALID;
    *maxReadDelay = READ_DELAY_INVALID;

    /* Iterate through possible read delay values */
    for(readDelay = phyParams->minReadDelay;
        readDelay <= phyParams->maxReadDelay; readDelay++)
    {
        searchPoint.txDLL = bottomLeft.txDLL;
        searchPoint.rxDLL = bottomLeft.rxDLL;
        searchPoint.rdDelay = readDelay;

        /* Check if the current read delay is valid */
        if(OSPI_phyRdDelayIsValid(handle, offset, phyOps, &searchPoint, topRight)
            == OSPI_PHY_TUNING_SUCCESS)
        {
            /* Update min if not set yet */
            if(*minReadDelay == READ_DELAY_INVALID) {
                *minReadDelay = readDelay;
            }
            /* Always update max to the latest valid value */
            *maxReadDelay = readDelay;
        }
    }

    /* Check if any valid read delays were found */
    if((*minReadDelay == READ_DELAY_INVALID) || (*maxReadDelay == READ_DELAY_INVALID))
    {
        status = OSPI_PHY_TUNING_FAILURE;
    }

    return status;
}

/**
 * \brief Search for optimal PHY configuration along the diagonal
 *
 * This function searches for the optimal OSPI PHY configuration by exploring
 * the parameter space diagonally between a bottom-left and top-right point.
 * It attempts to find stable regions in the search space where valid read
 * operations can be performed.
 *
 * \param handle      Device handle
 * \param offset      Memory offset to test
 * \param phyOps      PHY operations structure
 * \param radius      Search radius for midpoint validation
 * \param otp         Output optimal configuration (result)
 * \param bottomLeft  Starting point of search space (bottom left)
 * \param topRight    Ending point of search space (top right)
 *
 * \return OSPI_PHY_TUNING_SUCCESS on success, OSPI_PHY_TUNING_FAILURE on failure
 */
static int32_t OSPI_phySearchDiagonal(void* handle, uint32_t offset,
                                      OSPI_phyOps *phyOps, int32_t radius,
                                      OSPI_PhyConfig *otp,
                                      OSPI_PhyConfig bottomLeft,
                                      OSPI_PhyConfig topRight)
{
    int32_t status = OSPI_PHY_TUNING_FAILURE;
    OSPI_PhyConfig gapLow = {0,0, READ_DELAY_INVALID};
    OSPI_PhyConfig gapHigh = {0,0, READ_DELAY_INVALID};
    OSPI_PhyConfig midPoint = {0,0, READ_DELAY_INVALID};
    int8_t minReadDelay;
    int8_t maxReadDelay;
    int32_t dx;
    int32_t dy;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    /* Find valid read delays for the given search space */
    status = OSPI_phyFindValidReadDelays(handle, offset, phyOps, bottomLeft,
                                         topRight, &minReadDelay, &maxReadDelay);

    if(status == OSPI_PHY_TUNING_SUCCESS)
    {
        /* Refine bottom-left corner of search space with minimal read delay */
        OSPI_PhyConfig refineBl = bottomLeft;
        refineBl.rdDelay = minReadDelay;

        while((refineBl.txDLL <= phyParams->rxTxDllMax) &&
              (refineBl.rxDLL <= phyParams->rxTxDllMax))
        {
            if(phyOps->ops(handle, offset, &refineBl) == OSPI_PHY_TUNING_SUCCESS)
            {
                /* Found valid point */
                break;
            }

            refineBl.txDLL++;
            refineBl.rxDLL++;
        }

        /* Find top-right corner of search space with maximum read delay */
        OSPI_PhyConfig refineTr = topRight;
        refineTr.rdDelay = maxReadDelay;

        while((refineTr.txDLL >= phyParams->rxTxDllMin) &&
              (refineTr.rxDLL >= phyParams->rxTxDllMin))
        {
            if(phyOps->ops(handle, offset, &refineTr) == OSPI_PHY_TUNING_SUCCESS)
            {
                /* Found valid point */
                break;
            }

            refineTr.txDLL--;
            refineTr.rxDLL--;
        }

        /* Case 1: Same read delay for bottom-left and top-right points */
        if(refineBl.rdDelay == refineTr.rdDelay)
        {
            /* Find the largest passing region on the diagonal line */
            OSPI_phyFindTheBiggestPassingLineUp(handle, offset, phyOps,
                                                &refineBl, &refineTr);

            dx = refineBl.txDLL - refineTr.txDLL;
            dy = refineBl.rxDLL - refineTr.rxDLL;

            /* Calculate the length of the passing region */
            int32_t len =  (dx * dx) + (dy * dy);

            /* Check if passing region is large enough */
            if(len > phyParams->minPassSize)
            {
                /* Calculate midpoint of the passing region */
                midPoint.txDLL = refineBl.txDLL +
                                 (abs(dx) / 2);
                midPoint.rxDLL = refineBl.rxDLL +
                                 (abs(dy) / 2);
                midPoint.rdDelay = refineBl.rdDelay;

                /* Validate the midpoint and surrounding region */
                status = OSPI_phyFindTuningPoint(handle, offset, phyOps, radius,
                                                 &midPoint);
                if(status == OSPI_PHY_TUNING_SUCCESS)
                {
                    /* Copy optimal configuration to output */
                    otp->rdDelay = midPoint.rdDelay;
                    otp->rxDLL = midPoint.rxDLL;
                    otp->txDLL = midPoint.txDLL;
                }
            }
            else
            {
                /* Region too small, mark as failure */
                status = OSPI_PHY_TUNING_FAILURE;
            }
        }
        /* Case 2: Different read delays - need to find separate passing regions */
        else
        {
            int8_t count;

            /*
             * First passing region search: Starting from bottom-left,
             * moving diagonally upward
             */
            do
            {
                count = 1;
                /* Try to find consecutive passing points */
                status = OSPI_phyConsecutivePassingPointsUp(handle, offset,
                                                            phyOps, refineBl,
                                                            phyParams->numConsecutivePass,
                                                            &count,
                                                            OSPI_PHY_TUNING_SUCCESS);

                /* If no consecutive passing points found, try next position */
                if(status == OSPI_PHY_TUNING_FAILURE)
                {
                    count += 1;
                    refineBl.txDLL += count;
                    refineBl.rxDLL += count;

                    /* Find the next passing point on diagonal */
                    OSPI_phyFindNextPassingPointUp(handle, offset, phyOps,
                                                   &refineBl, &refineTr,
                                                   OSPI_PHY_TUNING_SUCCESS);
                }
            } while((status != OSPI_PHY_TUNING_SUCCESS) &&
                    (refineBl.txDLL < refineTr.txDLL));

            /* Reset search point if no consecutive passing points found */
            if(status != OSPI_PHY_TUNING_SUCCESS)
            {
                refineBl.rxDLL = 0;
                refineBl.txDLL = 0;
                refineBl.rdDelay = READ_DELAY_INVALID;
            }

            /* If we found a valid bottom region */
            if(refineBl.rdDelay != READ_DELAY_INVALID)
            {
                /* Define upper boundary of the first passing region */
                gapLow.rxDLL = refineBl.rxDLL +
                               phyParams->numConsecutivePass + 1;
                gapLow.txDLL = refineBl.txDLL +
                               phyParams->numConsecutivePass + 1;
                gapLow.rdDelay = refineBl.rdDelay;

                /* Search for the first failing point after passing region */
                while(phyOps->ops(handle, offset, &gapLow) == OSPI_PHY_TUNING_SUCCESS)
                {
                    gapLow.txDLL += 1;
                    gapLow.rxDLL += 1;
                }

                /* Look for consecutive failing points (the gap between regions) */
                do
                {
                    count = 1;
                    status = OSPI_phyConsecutivePassingPointsUp(handle, offset,
                                                                phyOps, gapLow,
                                                                phyParams->numConsecutiveFail,
                                                                &count,
                                                                OSPI_PHY_TUNING_FAILURE);

                    /* If no consecutive failing points found, move up */
                    if(status == OSPI_PHY_TUNING_FAILURE)
                    {
                        count += 1;
                        gapLow.txDLL += count;
                        gapLow.rxDLL += count;

                        /* Find next failing point */
                        OSPI_phyFindNextPassingPointUp(handle, offset, phyOps,
                                                       &gapLow, &refineTr,
                                                       OSPI_PHY_TUNING_FAILURE);
                    }
                } while((status != OSPI_PHY_TUNING_SUCCESS) &&
                        (gapLow.txDLL < refineTr.txDLL));


                /* Find the largest passing region from refineBl to gapLow */
                OSPI_phyFindTheBiggestPassingLineUp(handle, offset, phyOps,
                                                    &refineBl, &gapLow);
            }

            /*
             * Second passing region search: Starting from top-right,
             * moving diagonally downward
             */

            do
            {
                count = 1;
                status = OSPI_phyConsecutivePassingPointsDown(handle, offset,
                                                              phyOps, refineTr,
                                                              phyParams->numConsecutivePass,
                                                              &count,
                                                              OSPI_PHY_TUNING_SUCCESS);

                /* If no consecutive passing points found, move down */
                if(status == OSPI_PHY_TUNING_FAILURE)
                {
                    count += 1;
                    refineTr.txDLL -= count;
                    refineTr.rxDLL -= count;

                    /* Find next passing point downward */
                    OSPI_phyFindNextPassingPointDown(handle, offset, phyOps,
                                                     &refineTr, &refineBl,
                                                     OSPI_PHY_TUNING_SUCCESS);
                }
            } while ((status != OSPI_PHY_TUNING_SUCCESS) &&
                     (refineTr.txDLL >=
                     (refineBl.txDLL + phyParams->numConsecutivePass)));


            /* If no consecutive passing points found, reset */
            if (status != OSPI_PHY_TUNING_SUCCESS)
            {
                refineTr.rdDelay = READ_DELAY_INVALID;
            }

            /* If we found a valid top region */
            if(refineTr.rdDelay != READ_DELAY_INVALID)
            {
                /* Define lower boundary of the second passing region */
                gapHigh.rxDLL = refineTr.rxDLL -
                                phyParams->numConsecutivePass - 1;
                gapHigh.txDLL = refineTr.txDLL -
                                phyParams->numConsecutivePass - 1;
                gapHigh.rdDelay = refineTr.rdDelay;

                /* Search for first failing point below passing region */
                while(phyOps->ops(handle, offset, &gapHigh) ==
                      OSPI_PHY_TUNING_SUCCESS)
                {
                    gapHigh.txDLL -= 1;
                    gapHigh.rxDLL -= 1;
                }

                /* Look for consecutive failing points (the gap between regions) */
                do
                {
                    count = 1;
                    status = OSPI_phyConsecutivePassingPointsDown(handle, offset,
                                                                  phyOps,
                                                                  gapHigh,
                                                                  phyParams->numConsecutiveFail,
                                                                  &count,
                                                                  OSPI_PHY_TUNING_FAILURE);

                    /* If no consecutive failing points, move down */
                    if(status == OSPI_PHY_TUNING_FAILURE)
                    {
                        count += 1;
                        gapHigh.txDLL -= count;
                        gapHigh.rxDLL -= count;
                        OSPI_phyFindNextPassingPointDown(handle, offset, phyOps,
                                                         &gapHigh, &refineBl,
                                                         OSPI_PHY_TUNING_FAILURE);
                    }
                } while((status != OSPI_PHY_TUNING_SUCCESS) &&
                        (gapHigh.txDLL >= refineBl.txDLL));


                /* Find the largest passing region from refineTr to gapHigh */
                OSPI_phyFindTheBiggestPassingLineDown(handle, offset, phyOps,
                                                      &refineTr, &gapHigh);
            }

            if((refineBl.rdDelay != READ_DELAY_INVALID) ||
               (refineTr.rdDelay != READ_DELAY_INVALID))
            {
                /* Calculate the lengths of both passing regions */

                /* Length of bottom passing region */
                int32_t len1 = 0;

                /* Length of top passing region */
                int32_t len2 = 0;

                if(refineBl.rdDelay != READ_DELAY_INVALID)
                {
                    dx = gapLow.txDLL - refineBl.txDLL;
                    dy = gapLow.rxDLL - refineBl.rxDLL;
                    len1 = (dx * dx) + (dy * dy);
                }

                if(refineTr.rdDelay != READ_DELAY_INVALID)
                {
                    dx = refineTr.txDLL - gapHigh.txDLL;
                    dy = refineTr.rxDLL - gapHigh.rxDLL;
                    len2 = (dx * dx) + (dy * dy);
                }

                /* Check if any region is large enough */
                if((len1 > phyParams->minPassSize) ||
                   (len2 > phyParams->minPassSize))
                {
                    /* Calculate midpoints for both regions */
                    OSPI_PhyConfig bottomTuningPoint = {0, 0, READ_DELAY_INVALID};
                    OSPI_PhyConfig topTuningPoint = {0, 0, READ_DELAY_INVALID};

                    /* Calculate top region midpoint if large enough */
                    if(len2 >= phyParams->minPassSize)
                    {
                        topTuningPoint.rxDLL = refineTr.rxDLL -
                                               ((refineTr.rxDLL - gapHigh.rxDLL)
                                               / 2);
                        topTuningPoint.txDLL = refineTr.txDLL -
                                               ((refineTr.txDLL - gapHigh.txDLL)
                                               / 2);
                        topTuningPoint.rdDelay = refineTr.rdDelay;
                    }

                    /* Calculate bottom region midpoint if large enough */
                    if(len1 >= phyParams->minPassSize)
                    {
                        bottomTuningPoint.rxDLL = refineBl.rxDLL +
                                                  ((gapLow.rxDLL - refineBl.rxDLL)
                                                  / 2);
                        bottomTuningPoint.txDLL = refineBl.txDLL +
                                                  ((gapLow.txDLL - refineBl.txDLL)
                                                  / 2);
                        bottomTuningPoint.rdDelay = refineBl.rdDelay;
                    }

                    /* Decide which region to try first based on size */
                    OSPI_PhyConfig *primaryPoint = NULL;
                    OSPI_PhyConfig *secondaryPoint = NULL;
                    bool trySecondary = false;

                    if(len2 >= len1)
                    {
                        if(topTuningPoint.rdDelay != READ_DELAY_INVALID)
                        {
                            primaryPoint = &topTuningPoint;
                        }

                        if(bottomTuningPoint.rdDelay != READ_DELAY_INVALID)
                        {
                            secondaryPoint = &bottomTuningPoint;
                        }
                    }
                    else
                    {
                        if(bottomTuningPoint.rdDelay != READ_DELAY_INVALID)
                        {
                            primaryPoint = &bottomTuningPoint;
                        }

                        if(topTuningPoint.rdDelay != READ_DELAY_INVALID)
                        {
                            secondaryPoint = &topTuningPoint;
                        }
                    }

                    /* Try primary region first if available */
                    if(primaryPoint != NULL)
                    {
                        status = OSPI_phyFindTuningPoint(handle, offset, phyOps,
                                                         radius, primaryPoint);
                        trySecondary = (status != OSPI_PHY_TUNING_SUCCESS);
                    }
                    else
                    {
                        /* No primary region available */
                        trySecondary = true;
                    }

                    /* Try secondary region if needed and available */
                    if(trySecondary && (secondaryPoint != NULL))
                    {
                        status = OSPI_phyFindTuningPoint(handle, offset, phyOps,
                                                         radius,
                                                         secondaryPoint);
                    }

                    /* Set output values if successful */
                    if(status == OSPI_PHY_TUNING_SUCCESS)
                    {
                        OSPI_PhyConfig *selectedPoint = (trySecondary &&
                                                        (secondaryPoint != NULL))
                                                        ? secondaryPoint :
                                                        primaryPoint;

                        if(selectedPoint != NULL)
                        {
                            otp->rxDLL = selectedPoint->rxDLL;
                            otp->txDLL = selectedPoint->txDLL;
                            otp->rdDelay = selectedPoint->rdDelay;
                        }
                    }
                }
                else
                {
                    status = OSPI_PHY_TUNING_FAILURE;
                }
            }
            else
            {
                status = OSPI_PHY_TUNING_FAILURE;
            }

        }
    }

    return status;
}

int32_t OSPI_phyFindOTP4(void* handle, uint32_t offset, OSPI_phyOps *phyOps,
                         int32_t radius, OSPI_PhyConfig *otp)
{
    int32_t status = OSPI_PHY_TUNING_FAILURE;
    OSPI_phyParams* phyParams = phyOps->phyParams;

    /* Initialize search boundaries for primary diagonal */
    OSPI_PhyConfig bottomLeft = {phyParams->rxTxDllMin,
                                 phyParams->rxTxDllMin, READ_DELAY_INVALID};
    OSPI_PhyConfig topRight = {phyParams->rxTxDllMax,
                               phyParams->rxTxDllMax, READ_DELAY_INVALID};

    /* First attempt: search along the main diagonal from (0,0) to (127,127) */
    status = OSPI_phySearchDiagonal(handle, offset, phyOps, radius, otp,
                                   bottomLeft, topRight);

    if(status != OSPI_PHY_TUNING_SUCCESS)
    {
        /* Define max shift value based on search space constraints */
        int32_t shift = phyParams->diagonalShift;

        while((status != OSPI_PHY_TUNING_SUCCESS) &&
              (shift <= phyParams->maxDiagonalShift))
        {
            /* Try upward-shifted diagonal (higher RxDLL values) */
            OSPI_PhyConfig upShiftBottom = {phyParams->rxTxDllMin, shift,
                                            READ_DELAY_INVALID};
            OSPI_PhyConfig upShiftTop = {phyParams->rxTxDllMax - shift,
                                         phyParams->rxTxDllMax, READ_DELAY_INVALID};

            status = OSPI_phySearchDiagonal(handle, offset, phyOps, radius, otp,
                                            upShiftBottom, upShiftTop);

            /* If upward shift fails, try rightward-shifted diagonal */
            if(status != OSPI_PHY_TUNING_SUCCESS)
            {
                /* Create rightward-shifted diagonal (higher TxDLL values) */
                OSPI_PhyConfig rightShiftBottom = {shift, phyParams->rxTxDllMin,
                                                   READ_DELAY_INVALID};
                OSPI_PhyConfig rightShiftTop = {phyParams->rxTxDllMax,
                                                phyParams->rxTxDllMax - shift,
                                                READ_DELAY_INVALID};

                status = OSPI_phySearchDiagonal(handle, offset, phyOps, radius,
                                                otp, rightShiftBottom,
                                                rightShiftTop);
            }

            /* Increment shift for next iteration if both attempts failed */
            if(status != OSPI_PHY_TUNING_SUCCESS)
            {
                shift += phyParams->diagonalShift;
            }
        }
    }

    return status;
}
