/******************************************************************************
*                                                                             *
* module name       :TIDL                                                     *
*                                                                             *
* module descripton :TI Deep learning Library module is TI's CNN/DNN          *
*                     acceleration on EVE and DSP                             *
*                                                                             *
* Copyright (C) 2026 Texas Instruments Incorporated - http://www.ti.com/      *
* ALL RIGHTS RESERVED                                                         *
*                                                                             *
******************************************************************************/
/**
*******************************************************************************
*  @file    tidl_device_mem_properties.c                                      *
*                                                                             *
*  @brief   Implementation to calculate the read and write access cycles      *
            for performance estimate calculation                              *
*                                                                             *
*  @version 0.1 - Jan 2026 : Initial Version - placeholder/Template version   *
*           with function implementation [AAL, PKS]                           *
*                                                                             *
*******************************************************************************
*/

#include <stdint.h>
#include "tidl_device_mem_properties.h"

#ifdef PERF_MODELLING
#include "tidl_device_utils.h"
#include <ti/csl/arch/c7x/cslr_C7X_CPU.h>
#if defined (SOC_J784S4) ||  defined (SOC_J721S2)
#include "ti/drv/udma/dmautils/src/dmautils_autoincrement_3d_priv.h"
#else
#include "dmautils/include/dmautils_autoincrement_3d.h"
#endif

#if defined (__C7100__) || defined (__C7120__)
    #define MSMC_READFACTOR (3); 
#else
    #define MSMC_READFACTOR (1);
#endif

#define DRU_LOCAL_EVENT_START_DEFAULT (192U) // Default for J721E and J721S2
#define DRU_LOCAL_EVENT_START_J784S4 (664U)
#define MAX_MEMORY_ACCESS_TYPES (6) // refer eMemoryReadWriteIdx_t

#define DMA_SETUP_CYCLES (250) /* DMA setup cycles for each transfer*/
typedef struct 
{
    /* Source element size in bytes */
    float32_tidl srcElementSize;   
    /* Destination element size in bytes */
    float32_tidl dstElementSize;    
}DmaUtilsAutoInc3d_ElementSizes;
typedef struct {
    uint32_t totalSrcElements;
    uint32_t totalDstElements;
    uint32_t srcSizeBytes;
    uint32_t dstSizeBytes;
}DmaUtilsAutoInc3d_xFerSize;

float64_tidl DmaUtilsAutoInc3dPerfData[1];
uint64_t DmaUtilsAutoInc3dMemBwData[MAX_MEMORY_ACCESS_TYPES];

static sDeviceMemProperties_t memSpaceProp[MEMSPACE_MAX];
static int32_t TIDL_getMemoryAccessCycles(void *dstAddr, void *srcAddr, int32_t dstSize, int32_t srcSize, float32_tidl *cycles, uint64_t *pDmaBwData);
static void DmaPerfModelling_getUtcInfo(uint32_t * pUtcId, uint32_t * pDru_local_event_start, uint32_t coreId);

void TIDL_setMemSpaceConfig(sDeviceMemProperties_t memSpaceProperties[MEMSPACE_MAX])
{
    memSpaceProp[MEMSPACE_L2].baseAddr   = memSpaceProperties[MEMSPACE_L2].baseAddr;
    memSpaceProp[MEMSPACE_MSMC].baseAddr = memSpaceProperties[MEMSPACE_MSMC].baseAddr;
    memSpaceProp[MEMSPACE_DDR].baseAddr  = memSpaceProperties[MEMSPACE_DDR].baseAddr;
    memSpaceProp[MEMSPACE_L1].baseAddr  = memSpaceProperties[MEMSPACE_L1].baseAddr;

    memSpaceProp[MEMSPACE_L2].size   = memSpaceProperties[MEMSPACE_L2].size;
    memSpaceProp[MEMSPACE_MSMC].size = memSpaceProperties[MEMSPACE_MSMC].size;
    memSpaceProp[MEMSPACE_DDR].size  = memSpaceProperties[MEMSPACE_DDR].size;
    memSpaceProp[MEMSPACE_L1].size  = memSpaceProperties[MEMSPACE_L1].size;

    memSpaceProp[MEMSPACE_L2].bytesPerCPUCycle   = memSpaceProperties[MEMSPACE_L2].bytesPerCPUCycle;
    memSpaceProp[MEMSPACE_MSMC].bytesPerCPUCycle = memSpaceProperties[MEMSPACE_MSMC].bytesPerCPUCycle;
    memSpaceProp[MEMSPACE_DDR].bytesPerCPUCycle  = memSpaceProperties[MEMSPACE_DDR].bytesPerCPUCycle;
    memSpaceProp[MEMSPACE_L1].bytesPerCPUCycle  = memSpaceProperties[MEMSPACE_L1].bytesPerCPUCycle;

    memSpaceProp[MEMSPACE_L2].trCyclesPerByte   = memSpaceProperties[MEMSPACE_L2].trCyclesPerByte ;
    memSpaceProp[MEMSPACE_MSMC].trCyclesPerByte = memSpaceProperties[MEMSPACE_MSMC].trCyclesPerByte;
    memSpaceProp[MEMSPACE_DDR].trCyclesPerByte  = memSpaceProperties[MEMSPACE_DDR].trCyclesPerByte;
    memSpaceProp[MEMSPACE_L1].trCyclesPerByte  = memSpaceProperties[MEMSPACE_L1].trCyclesPerByte;
}

static int32_t TIDL_getMemSpace(void* addr)
{
    if ((addr >= memSpaceProp[MEMSPACE_L2].baseAddr) &&
        (addr < ((uint8_t *)memSpaceProp[MEMSPACE_L2].baseAddr + memSpaceProp[MEMSPACE_L2].size)))
    {
      return MEMSPACE_L2;
    }
    else if ((addr >= memSpaceProp[MEMSPACE_MSMC].baseAddr) &&
             (addr < ((uint8_t *)memSpaceProp[MEMSPACE_MSMC].baseAddr + memSpaceProp[MEMSPACE_MSMC].size)))
    {
      return MEMSPACE_MSMC;
    }
    else if ((addr >= memSpaceProp[MEMSPACE_L1].baseAddr) &&
             (addr < ((uint8_t *)memSpaceProp[MEMSPACE_L1].baseAddr + memSpaceProp[MEMSPACE_L1].size)))
    {
      return MEMSPACE_L1;
    }
    else 
    {
      return MEMSPACE_DDR;
    }
}

double TIDL_getReadAccessCycles(const void* srcAddr, const uint32_t numBytes)
{
    double readCycles;
    int32_t readSpace;
    
    readSpace = TIDL_getMemSpace((void*)srcAddr);
    readCycles = (double)numBytes/memSpaceProp[readSpace].bytesPerCPUCycle;
    
    if(readSpace == MEMSPACE_MSMC)
    {
        readCycles *= MSMC_READFACTOR;
    }
    if(readSpace == MEMSPACE_DDR)
    {
      DmaUtilsAutoInc3dMemBwData[DMA_DDR_READ_IDX] += (uint64_t)numBytes;
    }
    return readCycles;
}

double TIDL_getWriteAccessCycles(const void* dstAddr, const uint32_t numBytes)
{
    double writeCycles;
    int32_t writeSpace;
    
    writeSpace = TIDL_getMemSpace((void*)dstAddr);
    writeCycles = (double)numBytes/memSpaceProp[writeSpace].bytesPerCPUCycle;
    
    if(writeSpace == MEMSPACE_DDR)
    {
      DmaUtilsAutoInc3dMemBwData[DMA_DDR_WRITE_IDX] += (uint64_t)numBytes;
    }
    return writeCycles;
}
/* ======================================================================== */
/*   DMA related Perf Estimate functions                                    */
/* ======================================================================== */

typedef enum eDmaTrModes_t
{
   DMA_REGULAR_MODE,
   DMA_PERFMODEL_MODE
}eDmaTrModes_t;

int32_t TIDL_getMemoryAccessCycles(void *dstAddr, void *srcAddr, int32_t dstSize, int32_t srcSize,  float32_tidl *pDmaCycles, uint64_t *pdmaMemBWData)
{
  // Calculate and return based up synch boundary, element type and all that --- DONE
  // Might need to align CNT0 to some boundary like 64 bytes if CNT0 and DIM1 are not same
  // Need to have memory space knowledge and apply factor for cycles -- AVAILABLE

  // First get transferBytes: Product of all iCNT till sync boundary and consider eleType
  // Get memory space for source and destination
  // Get factor for both memory and use the one which is higher
  // Multiply transferBytes with the factor based upon deviceCap

  /*
  I/p transfers
  DDR --> MSMC
  DDR --> L2
  DDR --> DDR

  O/p transfers
  L2   --> DDR
  MSMC --> DDR
  DDR  --> DDR
  
  1. farthest and fastest memory has Same size ---> size * DDR_efficiency
  2. farthest memory has higher size           ---> DDR_size * DDR_efficiency
  3. fastest memory has higher size.           ---> MAX(DDR_size * DDR_efficiency, L2/L3_size * L2/L3_efficiency) 
  */
 int32_t status = 0;
 int32_t dstSpace = TIDL_getMemSpace(dstAddr);
 int32_t srcSpace = TIDL_getMemSpace(srcAddr);
 double trCycles;
 float readFactor = 1;
 float writeFactor = 1;
 double rmwReadCycles = 0;
 bool dstIsRmw = FALSE;

 if((uintptr_t)dstAddr & 0x3F)/* check alignment for 64*/
 {
   writeFactor = 2 ;
   rmwReadCycles = (double)dstSize * memSpaceProp[dstSpace].trCyclesPerByte;
   dstIsRmw = TRUE;
 }
 if((uintptr_t)srcAddr & 0x3F)/* check alignment for 64*/
 {
   readFactor = 2 ;
 }
#if 1
 //double dstCycles = (double)dstSize * memSpaceProp[dstSpace].trCyclesPerByte * writeFactor;
 double dstCycles = (double)dstSize * memSpaceProp[dstSpace].trCyclesPerByte * writeFactor ;
 double srcCycles = (double)srcSize * memSpaceProp[srcSpace].trCyclesPerByte * readFactor;

//#else
// srcCycles = TIDL_getReadAccessCycles(srcAddr, srcSize);
// dstCycles = TIDL_getWriteAccessCycles(dstAddr, dstSize);
#endif
 /* same memory space(src and dst in same memory) overhead is applicable to DDR only, MSMC and L2 have enough ports and there wont be any overhead */
 if((dstSpace == MEMSPACE_DDR) && (srcSpace == MEMSPACE_DDR))
 {
   trCycles = dstCycles + srcCycles + rmwReadCycles;
 }
 else
 {
   trCycles = MAX(dstCycles, srcCycles) + rmwReadCycles;
 }

  *pDmaCycles = DMA_SETUP_CYCLES + trCycles;
#if 0
  if(dstIsRmw)
  {
    if(dstSpace == MEMSPACE_DDR)
      pdmaMemBWData[DMA_DDR_READ_IDX] += (uint64_t)dstSize;
    else if(dstSpace == MEMSPACE_MSMC)
      pdmaMemBWData[DMA_MSMC_READ_IDX] += (uint64_t)dstSize;
    else if(dstSpace == MEMSPACE_L2)
      pdmaMemBWData[DMA_L2_READ_IDX] += (uint64_t)dstSize;
  }
#endif
  if(srcSpace == MEMSPACE_DDR)
  {
    pdmaMemBWData[DMA_DDR_READ_IDX] = (uint64_t)srcSize;
  }
  else if(srcSpace == MEMSPACE_MSMC)
  {
    pdmaMemBWData[DMA_MSMC_READ_IDX] = (uint64_t)srcSize;
  }
  else if(srcSpace == MEMSPACE_L2)
  {
    pdmaMemBWData[DMA_L2_READ_IDX] = (uint64_t)srcSize;
  }
  else
  {
    /* error case*/
  }
  if(dstSpace == MEMSPACE_DDR)
  {
    pdmaMemBWData[DMA_DDR_WRITE_IDX] = (uint64_t)dstSize;
  }
  else if(dstSpace == MEMSPACE_MSMC)
  {
    pdmaMemBWData[DMA_MSMC_WRITE_IDX] = (uint64_t)dstSize;
  }
  else if(dstSpace == MEMSPACE_L2)
  {
    pdmaMemBWData[DMA_L2_WRITE_IDX] = (uint64_t)dstSize;
  }
  else
  {
    /* error case*/
  }

// tidl_printf(2," Tr is from %d %d %x %x\t", srcSpace, dstSpace, srcAddr, dstAddr);  
// tidl_printf(2," Tr sizes and cycles %d %d %d\n", dstSize, srcSize, (int)*pCycles);

 return status;
}
/* 2D array lookup table for element sizes: [eltype][0]=srcElementSize, [eltype][1]=dstElementSize */
#define DMAUTILS_ELEMENT_SIZE_TABLE_ROWS  (12U)
static const float gElementSizeTable[DMAUTILS_ELEMENT_SIZE_TABLE_ROWS][2] =
{
    /* eltype 0  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_1)    */ { 1.0f,  1.0f  },
    /* eltype 1  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_1p5)  */ { 1.5f,  1.5f  },
    /* eltype 2  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_2)    */ { 2.0f,  2.0f  },
    /* eltype 3  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_3)    */ { 3.0f,  3.0f  },
    /* eltype 4  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_4)    */ { 4.0f,  4.0f  },
    /* eltype 5  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_5)    */ { 5.0f,  5.0f  },
    /* eltype 6  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_16)   */ { 16.0f, 16.0f },
    /* eltype 7  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_32)   */ { 32.0f, 32.0f },
    /* eltype 8  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_1_2)  */ { 1.0f,  2.0f  },
    /* eltype 9  (CSL_UDMAP_TR_FMTFLAGS_ELYPE_1p5_2)*/ { 1.5f,  2.0f  },
    /* eltype 10 (CSL_UDMAP_TR_FMTFLAGS_ELYPE_2_1)  */ { 2.0f,  1.0f  },
    /* eltype 11 (CSL_UDMAP_TR_FMTFLAGS_ELYPE_2_1p5)*/ { 2.0f,  1.5f  }
};

static DmaUtilsAutoInc3d_ElementSizes DmaUtilsAutoInc3d_getElementSizes(uint32_t eltype)
{
    DmaUtilsAutoInc3d_ElementSizes sizes;

    if (eltype < DMAUTILS_ELEMENT_SIZE_TABLE_ROWS)
    {
        sizes.srcElementSize = gElementSizeTable[eltype][0];
        sizes.dstElementSize = gElementSizeTable[eltype][1];
    }
    else
    {
        /* Default fallback for out-of-range eltype */
        sizes.srcElementSize = 1.0f;
        sizes.dstElementSize = 1.0f;
    }

    return sizes;
}

static int32_t DmaUtilsAutoInc3d_calculateTransferSize(const CSL_UdmapTR* tr, DmaUtilsAutoInc3d_xFerSize *xferSizes)
{

    // uint32_t totalElements=0;
     uint32_t eltype;
     int32_t syncType;
     DmaUtilsAutoInc3d_ElementSizes sizes;
     int32_t retVal=0;

     /* Extract ELYPE from fmtflags */
     eltype= (tr->fmtflags & CSL_UDMAP_TR_FMTFLAGS_ELYPE_MASK) >> CSL_UDMAP_TR_FMTFLAGS_ELYPE_SHIFT;
     syncType = (tr->flags & CSL_UDMAP_TR_FLAGS_EVENT_SIZE_MASK) >> CSL_UDMAP_TR_FLAGS_EVENT_SIZE_SHIFT ;

     /* Get element sizes */
     sizes = DmaUtilsAutoInc3d_getElementSizes(eltype);

     if(CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT1_DEC == syncType)
     {
         xferSizes->totalSrcElements = tr->icnt0;
         xferSizes->totalDstElements = tr->dicnt0;
     }
     else if (CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT2_DEC == syncType)
     {
         xferSizes->totalSrcElements = tr->icnt0 * tr->icnt1;
         xferSizes->totalDstElements = tr->dicnt0 * tr->dicnt1;
     }
     else if (CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT3_DEC == syncType)
     {
         xferSizes->totalSrcElements = tr->icnt0 * tr->icnt1 * tr->icnt2;
         xferSizes->totalDstElements = tr->dicnt0 * tr->dicnt1 * tr->dicnt2;
     }
     else if(CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION == syncType)
     {
         xferSizes->totalSrcElements = tr->icnt0 * tr->icnt1 * tr->icnt2 * tr->icnt3;
         xferSizes->totalDstElements = tr->dicnt0 * tr->dicnt1 * tr->dicnt2 * tr->dicnt3;
     }

     /* Calculate size based on source or destination */
     xferSizes->srcSizeBytes = xferSizes->totalSrcElements * sizes.srcElementSize;
     xferSizes->dstSizeBytes = xferSizes->totalDstElements * sizes.dstElementSize;

     return retVal;
}

static int32_t DmaUtilsAutoInc3d_getTransferCycles(CSL_UdmapTR *pTRRecord, DmaUtilsAutoInc3d_Context *dmautilsContext, float32_tidl *pDmaCycles, uint64_t *pdmaMemBWData)
{
  // Might need to align CNT0 to some boundary like 64 bytes if CNT0 and DIM1 are not same
  int32_t retVal = 0;
  DmaUtilsAutoInc3d_xFerSize xferSizes;
  retVal = DmaUtilsAutoInc3d_calculateTransferSize(pTRRecord, &xferSizes);
  retVal += TIDL_getMemoryAccessCycles((void *)pTRRecord->daddr, 
                                       (void *)pTRRecord->addr, 
                                       xferSizes.dstSizeBytes,
                                       xferSizes.srcSizeBytes,                                        
                                       pDmaCycles, pdmaMemBWData);
  return retVal;
}

void DmaUtilsAutoInc3d_wait_perfModelling(void *autoIncrementContext, int32_t channelId, double *pDmaUtilsAutoInc3dPerfData, uint64_t *pDmaUtilsAutoInc3dMemBwData, bool enableSingleChCopy) 
{
  //double totalTransferCycles = 0;
  uint32_t utcId;
  uint32_t chId;
  CSL_DRU_t *druRegs;
  DmaUtilsAutoInc3d_Context *dmautilsContext;
  CSL_UdmapTR *pTRRecord;
  float32_tidl dmaCycles=0;
  uint32_t chCount = 32;
  
  dmautilsContext = (DmaUtilsAutoInc3d_Context *)autoIncrementContext;

  DmaPerfModelling_getUtcInfo(&utcId, NULL, dmautilsContext->initParams.coreId);
 // utcId = dmautilsContext->initParams.coreId; 
  Udma_DrvHandle udmaDrvHandle = (Udma_DrvHandle)dmautilsContext->initParams.udmaDrvHandle;
  druRegs = udmaDrvHandle->utcInfo[utcId].druRegs;

  // IN HE, Wait can happen for more than one channel on which transfer is active
  for (chId = 0; chId < chCount; chId++) // :TODO: Remove hard coded value of 32
  {
    if ((druRegs->CHRT[chId].CHRT_SWTRIG & CSL_DRU_CHRT_SWTRIG_GLOBAL_TRIGGER0_MASK) == 1U) {
      pTRRecord = (CSL_UdmapTR *)(void *)&druRegs->CHATOMIC[chId];
      uint64_t dmaMemBWData[MAX_MEMORY_ACCESS_TYPES]= {0};
      DmaUtilsAutoInc3d_getTransferCycles(pTRRecord, dmautilsContext, &dmaCycles, &dmaMemBWData[0]);
      
      *pDmaUtilsAutoInc3dPerfData += dmaCycles;
      pDmaUtilsAutoInc3dMemBwData[DMA_DDR_READ_IDX] += dmaMemBWData[DMA_DDR_READ_IDX];
      pDmaUtilsAutoInc3dMemBwData[DMA_DDR_WRITE_IDX] += dmaMemBWData[DMA_DDR_WRITE_IDX];
      pDmaUtilsAutoInc3dMemBwData[DMA_MSMC_READ_IDX] += dmaMemBWData[DMA_MSMC_READ_IDX];
      pDmaUtilsAutoInc3dMemBwData[DMA_MSMC_WRITE_IDX] += dmaMemBWData[DMA_MSMC_WRITE_IDX];
      pDmaUtilsAutoInc3dMemBwData[DMA_L2_READ_IDX] += dmaMemBWData[DMA_L2_READ_IDX];
      pDmaUtilsAutoInc3dMemBwData[DMA_L2_WRITE_IDX] += dmaMemBWData[DMA_L2_WRITE_IDX];

      if(enableSingleChCopy == FALSE)
      {
        /* Use reserved space for tracking number of triggers submitted for a given channel */
        /* This is increamented during trigger here for every wait its decreamented*/
        druRegs->CHRT[chId].Resv_256[0] -= 1; 
        if (druRegs->CHRT[chId].Resv_256[0] == 0) 
        {
          /* Clear the sw trigger so that next wait wont count perf cycles */
          druRegs->CHRT[chId].CHRT_SWTRIG  = druRegs->CHRT[chId].CHRT_SWTRIG & (uint64_t)(~CSL_DRU_CHRT_SWTRIG_GLOBAL_TRIGGER0_MASK);
        }
      }
    }
  }
}

static enum eDmaTrModes_t gDMAMode = DMA_REGULAR_MODE;//PERF_MODEL; // or REAL_HW
//static DmaUtilsAutoInc3d_PerfData perfData; //Global to store the cycles

void DmaUtilsAutoInc3d_wait_wrapper(void *autoIncrementContext, int32_t channelId) 
{
  bool enableSingleChCopy = FALSE;
  //Keeping gloabl variable for now, can identify right mechansim to supply later
  if(gDMAMode != DMA_PERFMODEL_MODE)
  {
    DmaUtilsAutoInc3d_wait(autoIncrementContext, channelId);
  }
  else
  {
    DmaUtilsAutoInc3d_wait_perfModelling(autoIncrementContext, channelId, &DmaUtilsAutoInc3dPerfData[0], &DmaUtilsAutoInc3dMemBwData[0], enableSingleChCopy);
  }
  return ;
}
void DmaUtilsAutoInc3d_wait_copyWrapper(void *autoIncrementContext, int32_t channelId, bool enableSingleChCopy) 
{
  
  DmaUtilsAutoInc3d_wait_perfModelling(autoIncrementContext, channelId, &DmaUtilsAutoInc3dPerfData[0], &DmaUtilsAutoInc3dMemBwData[0], enableSingleChCopy);
  
  return ;
}
void DmaUtilsAutoInc3d_setPerfExecMode(void)
{
  gDMAMode = DMA_PERFMODEL_MODE;
}
void DmaUtilsAutoInc3d_getDmaPerformance(double *pPerfDataInfo) 
{
  pPerfDataInfo[0] = DmaUtilsAutoInc3dPerfData[0];
}
void DmaUtilsAutoInc3d_resetPerfData(void) {
    DmaUtilsAutoInc3dPerfData[0] = 0;
}
void DmaUtilsAutoInc3d_getDmaMemBwData(uint64_t *pDataMemBWInfo) 
{
  pDataMemBWInfo[DMA_DDR_READ_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_DDR_READ_IDX]; /* DDR Read*/
  pDataMemBWInfo[DMA_DDR_WRITE_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_DDR_WRITE_IDX]; /* DDR Write*/
  pDataMemBWInfo[DMA_MSMC_READ_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_MSMC_READ_IDX]; /* MSMC Read*/
  pDataMemBWInfo[DMA_MSMC_WRITE_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_MSMC_WRITE_IDX]; /* MSMC Write*/
  pDataMemBWInfo[DMA_L2_READ_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_L2_READ_IDX]; /* MSMC Read*/
  pDataMemBWInfo[DMA_L2_WRITE_IDX] = DmaUtilsAutoInc3dMemBwData[DMA_L2_WRITE_IDX]; /* MSMC Write*/
}

/*DmaUtilsAutoInc3dMemBwData is global array */
void DmaUtilsAutoInc3d_resetDmaMemBwData(void) 
{
  DmaUtilsAutoInc3dMemBwData[DMA_DDR_READ_IDX]   = 0;
  DmaUtilsAutoInc3dMemBwData[DMA_DDR_WRITE_IDX]  = 0;
  DmaUtilsAutoInc3dMemBwData[DMA_MSMC_READ_IDX]  = 0;
  DmaUtilsAutoInc3dMemBwData[DMA_MSMC_WRITE_IDX] = 0;
  DmaUtilsAutoInc3dMemBwData[DMA_L2_READ_IDX]    = 0;
  DmaUtilsAutoInc3dMemBwData[DMA_L2_WRITE_IDX]   = 0;
}

static void DmaPerfModelling_getUtcInfo(uint32_t * pUtcId, uint32_t * pDru_local_event_start, uint32_t coreId) {
  uint32_t utcId = 0;
  uint32_t dru_local_event_start = DRU_LOCAL_EVENT_START_DEFAULT;

  #if defined(SOC_J784S4) || defined(SOC_J742S2)
  uint32_t corePacNum = coreId + CSL_C7X_CPU_COREPACK_NUM_C7X1;
  
  if(CSL_C7X_CPU_COREPACK_NUM_C7X1 == corePacNum){
    utcId = UDMA_UTC_ID_C7X_MSMC_DRU4;
    dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96U * 0U); // TODO: Pick from CSL if possible
  }
  else if(CSL_C7X_CPU_COREPACK_NUM_C7X2 == corePacNum){
    utcId = UDMA_UTC_ID_C7X_MSMC_DRU5;
    dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96U * 1U); // TODO: Pick from CSL if possible
  }
  else if(CSL_C7X_CPU_COREPACK_NUM_C7X3 == corePacNum){
    utcId = UDMA_UTC_ID_C7X_MSMC_DRU6;
    dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96U * 2U); // TODO: Pick from CSL if possible
  }
  #if !defined(SOC_J742S2)
  else if(CSL_C7X_CPU_COREPACK_NUM_C7X4 == corePacNum){
    utcId = UDMA_UTC_ID_C7X_MSMC_DRU7;
    dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96U * 3U); // TODO: Pick from CSL if possible
  }
  #endif
  else
  {
    /* Do Nothing */
  }
  #else
  //J7ES and J721S2 will fall in this condition
  {
    utcId = UDMA_UTC_ID_MSMC_DRU0;
    dru_local_event_start = DRU_LOCAL_EVENT_START_DEFAULT; // TODO: Pick from CSL if possible
  }
  #endif

  if (pUtcId != NULL) {
    * pUtcId = utcId;
  }
  if (pDru_local_event_start != NULL) {
    * pDru_local_event_start = dru_local_event_start;
  }
  return;
}
#endif
/* ======================================================================== */
/*  End of file:  tidl_device_mem_properties.c                              */
/* ======================================================================== */

