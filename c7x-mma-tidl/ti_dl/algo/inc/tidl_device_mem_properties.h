/******************************************************************************
*                                                                             *
* module name       :  TIDL                                                     *
*                                                                             *
* module description :  TIDL Library module for J7                               *
*                                                                             *
* Copyright (C) 2017-2026 Texas Instruments Incorporated - http://www.ti.com/ *
* ALL RIGHTS RESERVED                                                         *
*                                                                             *
******************************************************************************/
/**
*******************************************************************************
*  @file     tidl_device_mem_properties.h                                     *
*                                                                             *
*  @brief    Public header file for TIDL Performance modelling utility        *
*            function                                                         *
*                                                                             *
*  @version  0.1 - Feb 2026 : Initial Version - placeholder/Template version  *
*            with structure and functions [AAL]                               *
*                                                                             *
*******************************************************************************
*/
#ifndef TIDL_DEVICE_MEM_PROPERTIES_H_
#define TIDL_DEVICE_MEM_PROPERTIES_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#if defined (HOST_EMULATION)
  /*
    Disabling PERF_MODELLING in Host emulation because of mismatch version in MMA
    TODO: Need to revert the changes when correct MMA verison is there.
    #define PERF_MODELLING
  */
#endif

#ifdef PERF_MODELLING
typedef enum
{
  MEMSPACE_L2 = 0,
  MEMSPACE_MSMC,
  MEMSPACE_DDR,
  MEMSPACE_L1,
  MEMSPACE_MAX
} eMemorySpaces_t;

typedef enum
{
  DMA_DDR_READ_IDX = 0,
  DMA_DDR_WRITE_IDX,
  DMA_MSMC_READ_IDX,
  DMA_MSMC_WRITE_IDX,
  DMA_L2_READ_IDX,
  DMA_L2_WRITE_IDX
} eMemoryReadWriteIdx_t;

typedef struct
{
    void* baseAddr;
    int32_t size;
    double bytesPerCPUCycle;
    double trCyclesPerByte; /* for DMA transfers */
} sDeviceMemProperties_t;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void TIDL_setMemSpaceConfig(sDeviceMemProperties_t memSpaceProperties[MEMSPACE_MAX]);

double TIDL_getReadAccessCycles(const void* addr, const uint32_t numBytes);

double TIDL_getWriteAccessCycles(const void* addr, const uint32_t numBytes);

/**
 * Below are the APIs used for DMA Performance modelling
 * executation order
 * 1. DmaUtilsAutoInc3d_setPerfExecMode(void); Set the perf mode else it will be regular transfer mode. This API will be called once at Init time
 * 2. DmaUtilsAutoInc3d_resetPerfData(void) ; It will reset the internel cycle counter of DMA perf model. This shall be called at Workload boundary
 * 3. DmaUtilsAutoInc3d_wait_wrapper(..); This is wrapper across wait and will be called for every dma wait. In this call actual wait/perf modelling calculation will happen
 * 4. DmaUtilsAutoInc3d_getDmaPerformance(..); This will get DMA perf modelling cycles. This shall be called at Workload boundary.
 **/

void DmaUtilsAutoInc3d_setPerfExecMode(void);
void DmaUtilsAutoInc3d_resetPerfData(void) ;
void DmaUtilsAutoInc3d_wait_wrapper(void *autoIncrementContext, int32_t channelId);
void DmaUtilsAutoInc3d_getDmaPerformance(double *pPerfDataInfo);

/**
 * Below are the APIs used for Memory BW estimation
 * Call reset API at the start of layer
 * call getDmaMemBwData API at end of all workloads
 **/

void DmaUtilsAutoInc3d_resetDmaMemBwData(void);
void DmaUtilsAutoInc3d_getDmaMemBwData(uint64_t *pDataBWInfo);
void DmaUtilsAutoInc3d_wait_copyWrapper(void *autoIncrementContext, int32_t channelId, bool enableSingleChCopy);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#endif /* PERF_MODELLING */
#endif /* #ifndef TIDL_DEVICE_MEM_PROPERTIES_H_ */
