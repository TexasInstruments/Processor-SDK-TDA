/*
 *******************************************************************************
 *
 * Module Name: TIDL Network Analyzer and Performance Simulator
 *
 * "Performance simulator for CNN netowrks" is software module developed
 *  for TI's C7x + MMA processor architecture. it models the performance
 *  of different layers of CNN netowrk and provides a summarized output
 * of performance

 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *******************************************************************************
*/

/**
  ******************************************************************************
  *  @file     perfsim.h
  *
  *  @brief    Interface Header file for performance simulator for C7x + MMA
  *
  *  @author   PKS
  *
  *  @version  0.1 - Sep 2017 : Initial Version [PKS]
  *
 *****************************************************************************
*/

#ifndef _PERFSIM_H_
#define _PERFSIM_H_

#include "itidl_ti.h"
#include "common.h"
#include "gc.h"

/**
* @defgroup networkAnalyzer NetworkAnalyzer: CNN Network Aanlyzer,
   Optimizer and Performance Simulator
*/

/**
* @addtogroup networkAnalyzer
* @{
*/

#define MAXLAYERS                 (TIDL_NUM_MAX_LAYERS)    /**< Maximum number of supported layers*/
#define TI_NW_COMPILER_VERSION    "TI_NETWORK_COMPILER_11.01"

#define L3MEM_SCRATCH_BUF_SIZEKB (56U) /**< Size of L3 MEM used for TIDL scratch buffers */
#define L2MEM_SCRATCH_BUF_SIZEKB (2U)  /**< Size of L2 MEM used for TIDL scratch buffers */

#define  ERR_FLAG_SPATIAL_JOIN_NOT_SUPPORTED  (1U)
#define  ERR_FLAG_OTF_NOT_SUPPORTED           (2U)
#define FILE_NAME_SIZE  (512)

/**
* @defgroup dataFlowAnalysis DataFlowAnalyzer: Performs the analysis for dataflow
* @ingroup  networkAnalyzer
* @{
*/
  // int32_t firstTransferRemainder    ; /*Effective Prefetch for OTF*/

typedef enum
{
  ERR_UNSUPPORTED_DATA_TYPE = 0 ,
  ERR_MMEORY_OVERLAP ,
  ERR_MMEORY_INSERTION,
  ERR_MMEORY_ALLOCATION,
  ERR_HIGHER_INSTANCEMEM,
  ERR_LESSER_SD_PARTS,
  ERR_LESSER_MAX_CONSTRAINED_DATA,
  ERR_LESSER_MAX_BA_SUBCONSTRAINT,
  ERR_LESSER_MAX_MEM_CHUNKS,
  ERR_NO_OVERLAP_ALLOCATION,
  ERR_WRONG_NETWORK_STRUCTURE,
  ERR_UNSUPPORTED_ST_LAYER,
  ERR_DATAFLOW_DECISION,
  ERR_HIGHER_FLOWCNT, /*if flowCnt[0] > maxOutProc size */
  ERR_WL_COUNT_MISMATCH, /*wLCount not matching with DB count*/
  ERR_ST_GROUP_EXCEED, /*number of ST groups is greater than TIDL_MAX_NUM_GROUPS*/
  ERR_UNSUPPORTED_PROC_SIZE,
  ERR_MEMORY_ALLOCATION_sWLExecutionOrder,
  ERR_MEMCOPY_sWLExecutionOrder,
  ERR_UNSUPPORTED_BROADCAST_DIMENSION,
  ERR_MAX = 32
} eErroCodeBits ;

extern int32_t gErrCode ;


/**
 *  @enum   eDataFlowBuffer
 *  @brief  Enumeration for buffer layout
 *
 */
typedef enum
{
  ROW_CIRCULAR =  0,
         /**< This value indicates that the enrire width of the input feature
          *  map is brought and at least height  = Fc and use
          *  cicrular feature of SE to reuse the data.
          */
  ROW_LINEAR    = 1,
          /**< This value indicates that the enrire width of the input feature
          *  map is brought and at least height  = Fc and grow it vertically
          *  linearly (Frame interleaved) buffer */
  COL           = 2,
          /**< This value indicates that the enrire col of the input feature
          *  map is brought and linearly grown donwards */
  FRAME         = 3  /**< Frame level buffer */
} eDataFlowBuffer ;

#define GETDATAFLOW_BUFTYPE(dataflow)              (((uint32_t)dataflow) & 0xFU)

/**< Assumption of maximum forks and joints in CNN network */
#define MAX_BRANCH (128)

/**<
********************************************************************************
* @struct sBufferInfo_t                                                        *
* @brief  This structure holds the buffer information in memory                *
*                                                                              *
* Legends in figure below                                                      *
* (o)padded region, (-)actual pixels,(+)boundary of feature width,(x)junk data *
*                                                                              *
* 1 feature plane 2D is as shown below is considered in form of linear 1D      *
* +o+oooooooo+                                                                 *
* +o+--------+                                                                 *
* +o+        +                                                                 *
* +o+        + => +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx       *
* +o+        +                                                                 *
* +o+--------+                                                                 *
* +o+oooooooo+oxxxx                                                            *
* 2D representation       ==> 1D representation                                *
*                                                                              *
* So tensor (multiple feature planes) is represeted in 2D form as below        *
*                                                                              *
* <------------- Linearized 1D Feature Plane------------>                      *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  ^                    *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  |                    *
* .                                                       |                    *
* .                                                       Ni (# Feature Planes)*
* .                                                       |                    *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  ^                    *
* Physical buffer Size = bufWidth*bufHeight (not always, but mostly)           *
*                                                                              *
* @param accessor     : Indicates who is doing access - CPU/DMA                *
* @param space        : Indicates which mmeory space: L2/DDR/MSMC              *
* @param bufWidth     : Physical buffer width (including junk pixels)          *
* @param bufHeight    : Physical buffer height (Mostly Ni)                     *
* @param bufSize      : Physical buffer size, not necessary width*height,      *
*                       explain later why? - TBD                               *
* @param activeBufWidth: Pixels in width excluding junk data                   *
* @param baseMem      : Mmeory address assuming start as 0                     *
* @param accessoffset : Indicates first byte offset for accessing the data     *
*                       for read/write operation. Mostly during write padded   *
*                       region is not written by CPU processing and during read*
*                       padded data is consumed by CPU processing              *
*                       Sometimes padding can be larger to have common buffer  *
*                       layout across many layers so it also indicates the     *
*                       relevant position to read/write                        *
*******************************************************************************/

typedef struct
{
  int32_t accessor;
  int32_t space;
  int32_t bufWidth   ;
  int32_t bufHeight  ;
  int32_t bufSize    ;
  int32_t activeBufWidth;
  int32_t baseMem ;
  int32_t accessoffset;
  int32_t padC ;
  int32_t padR ;
  int32_t padCZeros ;
  int32_t padRZeros ;
  int32_t padCFillZeros ;
  int32_t padRFillZeros ;
  int32_t padCReq ;
} sBufferInfo_t;

/**<
*******************************************************************************
@struct sPerfSimConfig_t
@brief  This structure allows user to configure the key parameters for DL-NAPS
@param freqMHz           : Frequency of the C7x+MMA in MHz
@param sizeL2MemKB       : L2 SRAM Memory size which is part of C7x+MMA sub system. This is only SRAM
and L2 cache is not part of this.
@param matPanelSize      : Size of MAtrix Panel (64 in Generation 1)
@param drainLatency      : Latency cycles to drain C matrix out of MMA
@param setupCycles       : Setup cycles for a MMA Loop
@param layerSetupCycles  : Setup cycles for a Layer
@param mmaCoreLoopOH     : Overheads on top of Core Loop in MMA, (>100 means OH per output block
and the per Block OH is mmaCoreLoopOH - 100
< 100 means overall overhead cycles for one function call
@param numCores          : The number of C7x + MMA core is varying (from 1 to 2) in different
SOC of J7 and TDA4 product line. Set for nunber of cores in SOC
@param sizeL3MemKB       : MSMC SRAM size
@param busWidth_L3_L2    : Bus width b/w MSMC and L2
@param busWidthDDR       : Bus width b/w DDR and SOC
@param freqDDR           : Frequency of DDR
@param numEMIFPorts      : Number of EMIF ports to DDR
@param ddrEfficiency     : Efficiency of DDR
@param L2Efficiency      : Efficiency of L2
@param msmcEfficiency    : Efficiency of MSMC
@param dataType          : The data type of network model and features. 0 for  8-bit  1 for  16-bit
@param mmaDoesPooling    : 1 if MMA can do pooling, else 0. Gen 1 it is 0
@param numBiasInstance   : Number of bias instances in unit of matPanelSize
@param alignmentOption   : OPtions for alignement of feature planes, refer eAlignmentOptions
@param fileFormatNet     : Network File Format
@param batchSize         : Number of batches
@param deviceName        : Target Device
@param enablePersistWtAlloc : Value of 1 will try to allocate weight buffer in any remaining
memory available in L2/MSMC so that their DMA can only be done during activate/
de-activate function
*******************************************************************************
*/
typedef struct
{
  int32_t   freqMHz;
  int32_t   sizeL2MemKB;
  int32_t   matPanelSize;
  int32_t   drainLatency;
  int32_t   setupCycles;
  int32_t   layerSetupCycles;
  int32_t   mmaCoreLoopOH;
  int32_t   numCores;
  int32_t   sizeL3MemKB;
  int32_t   busWidth_L3_L2;
  int32_t   busWidthDDR;
  int32_t   freqDDR;
  int32_t   numEMIFPorts;
  float32_tidl     ddrEfficiency;
  float32_tidl     L2Efficiency;
  float32_tidl     msmcEfficiency;
  float32_tidl     bankConflictFact;
  float32_tidl     msmcReadFactor;
  int32_t   dataType;
  int32_t   mmaDoesPooling;
  int32_t   numBiasInstance;
  int32_t   alignmentOption;
  int32_t   fileFormatNet;
  int32_t   batchSize;
  int32_t   deviceName;
  int32_t   enablePersistWtAlloc;

  int32_t   numPanelSplitsVertical;
  int32_t   numWritePortsL3;
  int32_t   numSEReadPortsL3;
  int32_t   numSEreadPortsL2;
  int32_t   numDRUPorts;
  int32_t   BPanelWritePorts;
  int32_t   numLUTWriteMMA;
  int32_t   packetizeMode;

  int32_t   ddrLayers[MAXLAYERS];

  int8_t   traceLevel;

  char fileNameNet[FILE_NAME_SIZE]  ;
  char fileNameParam[FILE_NAME_SIZE]  ;
  char outputDirName[FILE_NAME_SIZE];
  char fileNameGrpInfo[FILE_NAME_SIZE]  ;

} sPerfSimConfig_t;

/**<
 *******************************************************************************
  @struct sPerfSim_t
  @brief  This structure defines the key parameters exported out by simulator
  @param  simConfig : Config Parameters paased to Analysis tool
  @param  contextMemLyrNums : List of layers to be backed up at layer boundary for preemption
  @param  procTime : Processing time from NC required to identify preemption layers
  @param  bufDBIndex : For Layer i, this holds the buffer information to be backed up at the start of layer i
  @param  sdataFlowInfo : dataflow information for each layer of the network
 *******************************************************************************
*/
typedef struct
{
  sPerfSimConfig_t   simConfig;
  int32_t            bufDBIndex[MAXLAYERS][MAX_BRANCH]; // index to buffer data base of (A) all coex, prev, and context
  float64_tidl       procTime[MAXLAYERS];
  int32_t            layerExecutionOrder[MAXLAYERS];
  int32_t            checkErrorFlag[MAXLAYERS];
  float64_tidl       preEmptWorstCaseDelay;
  sGraphCompilerOutArgs_t graphCompOut;
} sPerfSim_t ;

/** @} */ // end of group networkAnalyzer

#endif
