/*
*******************************************************************************
*
* Module Name: Graph compiler - Primarilly designed for execution plan for DNN
* based graphs, but should scale for any graph based execution planner
*
* "Performance simulator - Predicts time for overall graph execution for TI's
* C7x + MMA processor architecture. It models the performance of different
* processing elements of graph and provides a summarized output of performance
*
* Copyright (C) 2015-21 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*******************************************************************************
*/

/**
******************************************************************************
*  @file     gc_helper.h
*
*  @brief    Interface of helper function module for GC,
*            can be used by client of GC module
*
*  @author   PKS
*
*  @version  0.1 - Sep 2021: Initial Version [PKS]
*
*****************************************************************************
*/

#ifndef _GC_HELPER_H_
#define _GC_HELPER_H_

#include "gc.h"
#include "stdint.h"
#include "itidl_ti.h"


#define ICNT_MAX_16_BIT ((1U << 16U) - 1U)
/**
*  @enum       eLinkSubtype
*  @brief      Enumeration for different types of Links, This is critical
*              for processing chain to get implemented. prefix of LINK_X
*              indicates transfer link and LINK_P is for processing link
*              *** NOT EXPECTED TO BE USED, KEEPING IT FOR SOME TIME ***
*              *** AND SHALL MOVE INTERNALLY LATER FOR INFORMATIONAL PURPOSE ***
*/
typedef enum {
  LINK_X_FM_IN_FULL_TO_PART = 0,    /**< Feature map full to part                 */
  LINK_X_COEFF_IN_FULL_TO_ADV,      /**< Coeffs full to advance                   */
  LINK_X_COEFF_IN_FULL_TO_STAGE,    /**< Coeffs full to intermediate              */
  LINK_X_COEFF_IN_ANY_TO_FINAL,     /**< Coeffs from any place to final           */
  LINK_X_BIAS_IN_FULL_TO_FINAL,     /**< Bias from full to final                  */
  LINK_X_FM_OUT_PART_TO_FULL,       /**< During write out staging(part) to final  */
  LINK_X_FM_CTXT_ST,                /**< context store link                       */
  LINK_X_FM_CTXT_RST,               /**< context restore link                     */
  LINK_P,                           /**< Processing link                          */
  LINK_X_FM_CTXT_ACROSS_CORE,       /**< context tensor genrated by different core*/
  MAX_LINK_SUBTYPE
} eLinkSubtype;

/**<
***************************************************************************************************
* @struct sWLMetaDataID_t                                                                         *
* @brief  This structure holds the information to create a  32-bit unique ID                      *
*                                                                                                 *
* @param numSplit : number of partitions in which a channel is split along the  height            *
*                   (into sets of columns). This is done in case one channel size is large enough *
*                   to not be transferred at once.                                                *
*        tileType : the Type of Tile represented by the WL Unit                                   *
*                   Single WL is created for each tiletype of a layer                             *
*                   WL for First Tile and Last Tile will be executed only once, whereas           *
*                   WL for Normal tiles may be executed greater than once                         *
*                   Refer eTileType                                                               *
*    inTileHeight : height of the input (tiled)tensor to this WL                                  *
*    outTileHeight: height of the output (tiled) tensor from this WL                              *
* passiveLinkIndex : Although all normal tiles of a layer are                                     *
*                     represented by single workload unit                                         *
*                     For some layers the very first normal tile has some different property.     *
*                     i)Any positive value means that the link at index mentioned by              *
*                     this variable is not active during first iteration of the workload unit.    *
*                     ii)a value of (-1) NOT VALID, will indicate that this field can be ignored. *
*           Now the question arise that why one would even add the link to workload unit          *
*           if it is not active. So to understand that one should be aware that same              *
*           workload unit can represent multiple processing unit (sWorkloadSubGroup_t:count > 1)  *
*           So whenever count is > 1, same workload unit is having multiple iterations            *
*           with just change of few variables to address at different place in buffer.            *
*           Now for this situation if one has to be informed about some of the iteration          *
*           having different property then this variable comes in existence.                      *
*           So this variable is valid only for first iteration of such workload unit.             *
*           Internal details: Not relevant for use of workload but documenting for more rationale.*
*           There are situations when first tile from normal tile group doesn't have any context  *
*           to restore but rest of them have so this link should not be executed for only first   *
*           iteration of normal tile group workload unit.                                         *
* isFirstTile : if a Workload (having tiletype = Normal Tile) behaves like a first Tile ,*
*                        in the first iteration of the workload, then this variable is set to 1   *
*                        otherwise 0.                                                             *
*                        This situation occurs when a tiled layer has no first Tile.              *
* isLastTile : if a Workload (having tiletype = Normal Tile) behaves like a last Tile ,  *
*                        in the last iteration of the workload, then this variable is set to 1    *
*                        otherwise 0.                                                             *
*                        This situation occurs when a tiled layer has no last Tile.               *
**************************************************************************************************/
typedef struct
{
  uint32_t numSplit;
  int32_t tileType;
  int32_t inTileHeight;
  int32_t outTileHeight;
  int32_t passiveLinkIndex;
  int32_t isFirstTile;
  int32_t isLastTile;
  int32_t dataFlowType;
  int32_t packetizeMode;
} sWLMetaDataID_t;

/**<
***************************************************************************************
* @struct sWLMetaDataIDExt_t                                                                 *
* @brief  This structure is an extension to meta Data to extra information                   *
*         related to batch defualt mode as well as channel split in low-latency mode         *
*                                                                                            *
* @param numInCh    : number of Input channels to be executed by a WL                        *
* @param numOutCh   : number of Output channels to be executed by a WL                       *
* @param numGrp     : number of groups to be executed by a WL                                *
*                                                                                            * 
*                                                                                            *
**************************************************************************************/
typedef struct
{
  uint32_t numSplitB;
  int32_t inNumBatch;
  int32_t outNumBatch;
  int32_t numInCh;
  int32_t numOutCh;
  int32_t numGrp;
} sWLMetaDataIDExt_t;

//private
typedef struct
{
  int64_t networkId : 4;
  int64_t layerId : 16;
  int64_t subLayerId : 4;
  int64_t type : 5;
  int64_t dependentLayerId : 16;
  int64_t tileType : 4;
  int64_t multiBufIndex : 4;
  int64_t rsvd : 11;
} sIntMetaDataID_t;

typedef struct
{
  uint32_t numSplit : 8;
  int32_t tileType : 4;
  int32_t inTileHeight : 20;
  int32_t outTileHeight : 20;
  int32_t passiveLinkIndex : 4;
  int32_t isFirstTile : 2;
  int32_t isLastTile  : 2;
  uint32_t dataFlowType : 2;
  int32_t packetizeMode  : 2;
} sIntWLMetaDataID_t;

typedef struct
{
  uint64_t numSplitB : 8;
  int64_t inNumBatch : 8;
  int64_t outNumBatch : 8;
  int64_t numInCh : 16;
  int64_t numOutCh : 16;
  int64_t numGrp : 8;
} sIntWLMetaDataIDExt_t;

/**<
***************************************************************************************
* @struct sMetaDataID_t                                                               *
* @brief  This structure holds the metaData to create a single 64-bit unique ID       *
*                                                                                     *
* @param networkId    : Unique ID for a network                                       *
* @param layerId      : Layer number in the network, this is the layer number in      *
*                       original netowrk, not execution order of layer                *
* @param subLayerId   : Part of the layer being processed by a processing core (It    *
*                        not sub part within a core)                                  *
* @param type         : This has different interpertation for link and buffer, but    *
*                       to generalize, this is type of the object                     *
* @param dependentLayerId : the layer ID whoes produced data is used by the link.     *
*                           This variable is only relevant for links and not buffers  *
*                           It is set to NOT_VALID for buffers
* @param multiBufIndex :    This is applicable only for buffers and indicates multiple*
*                           buffers with of same type. For example matMul layer can   *
*                           have multiple inputs in L2, this feild is to distinguish  *
*                           them. Default value is 0                                  *
* @param tileType :   For Buffers=>                                                   *
*                     In ST for a layer for every type Buffer except BUF_COEFF_FULL   *
*                     we have a single buffer used for all tiletypes                  *
*                     and tileType is set to NO_TILE in buffer ID Metadata.           *
*                     For BUF_COEFF_FULL buffer we have separate copies for each tile *
*                     type and each buffer has its ID's metadata tiletype component   *
*                     respectively(First, Normal,Last)                                *
*                   For Links=>                                                       *
*                   tiletype of the WL (which tile of the tensor the WL is processing)*
*                     Refer etileType
**************************************************************************************/
typedef struct
{
  int32_t networkId;
  int32_t layerId;
  int32_t subLayerId;
  int32_t type;
  int32_t dependentLayerId;
  int32_t multiBufIndex;
  int32_t tileType;
} sMetaDataID_t;


/**<
***************************************************************************************
* @struct sGCHelperObj                                                               *
* @brief  This structure holds the metaData to create a single 64-bit unique ID       *
* Note: This is an internal/private driver structure and should not beused or modified by caller. *
*                                                                                     *
* @param pNet    : TIDL network structure                                       *
* @param pBufDB      : Pointer to buffer database                *
* @param pWLDB   : Pointer to workload database)                                  *
* @param pAuxWLDB         : Pointer to Auxilary workload database                    *
**************************************************************************************/

typedef struct
{
  const sTIDL_Network_t *pNet;
  sBufDataBase_t *pBufDB ;
  sWLDataBase_t *pWLDB ;
  sWLAuxilaryDataBase_t *pAuxWLDB ;
}sGCHelperHandle ;


/**
*******************************************************************************
*  @func         getMetaDataID
*  @brief        This function provides structure sWLMetaDataID_t for a 32-bit id
*                id should have been formed using geWLtId function
*  @param [IN]   id          : id formed by getWLId routine and want to retrive back
*  @param [OUT]  pWLMetaDataID : Pointer to WL meta Data structure
*  @return       64-bit metadata info
*******************************************************************************
*/
void getWLMetaDataID(int64_t WLId, sWLMetaDataID_t *pWLMetaDataID);
void getWLMetaDataIDExt(int64_t WLId, sWLMetaDataIDExt_t *pWLMetaDataIDExt);

/**
*******************************************************************************
*  @func         getMetaDataID
*  @brief        This function provides structure sMetaDataID_t for a 32-bit id
*                id should have been formed using getId function
*  @param [IN]   id          : id formed by getId routine and want to retrive back
*  @param [OUT]  pMetaDataID : Pointer to meta Data structure
*  @return       64-bit metadata info
*******************************************************************************
*/
void getMetaDataID(int64_t id, sMetaDataID_t *pMetaDataID);


/**
*******************************************************************************
*  @func         initGCHelper
*  @brief        This function shall be called before calling query functions
*                such as getWLUnit, getLayerInfo, getProcLink, getLink etc.
*                It initializes the GC helper module with relevant data bases
*
*  @param [IN]   pNet   : Pointer to Network structure
*  @param [IN]   pGCDataBaseInfo   : Pointer to GC Data base info
*  @param [IN]   dataBaseType : Type of Database to initialise (BUFF/ WL / AUXWL)
*  @return       NOT_VALID(-1) if failure, 0 otherwise
*  @remarks
*******************************************************************************
*/
int32_t initGCHelperHandle(sGCHelperHandle * gcHelperHandle,
                               const sTIDL_Network_t *pNet,
                               sGCDataBaseInfo_t *pGCDataBaseInfo,
                               sGCCommonDataBaseInfo_t *pGCCommonDataBaseInfo);

/**
*******************************************************************************
*  @func         getBufParamsFromBufIndex
*  @brief        This function returns the buffParams for a given bufIndex
*
*  @param [IN]   bufIndex   : Index of the buffer in buffer data base
*  @return       Pointer to buffParams at given index
*  @remarks
*******************************************************************************
*/

sBufParams_t * getBufParamsFromBufIndex(const sGCHelperHandle * gcHelperHandle, int32_t bufIndex);

/**
*******************************************************************************
*  @func         getBufParamsFromBufIndexStrict
*  @brief        This function returns the buffParams for a given bufIndex with invalid checks
*
*  @param [IN]   bufIndex   : Index of the buffer in buffer data base
*  @return       Pointer to buffParams at given index
*  @remarks
*******************************************************************************
*/

sBufParams_t * getBufParamsFromBufIndexStrict(const sGCHelperHandle * gcHelperHandle, int32_t bufIndex);

/**
*******************************************************************************
*  @func         getLinkPtr
*  @brief        This function returns all the links of the workload unit
*                indicated by the link subType (or link Idx) . User can provide the linkSubType/linkIdx
*                along with workload unit
*
*  @param [IN]   pWL         : Pointer to workload unit
*  @param [IN]   linkSubType : subType of the link (can be supplied as NOT_VALID if linkIdx is provided)
*  @param [IN]   linkIdx     : Index of link in given workload (used only if linkSubType == NOT_VALID )
*  @param [OUT]  linkPtrList : Pointer to all Link objects of specified subType or link object at index 'linkIdx'
*  @return       Number of link objects returned
*  @remarks
*******************************************************************************
*/
void getLinkPtrs(const sWorkloadUnit_t *pWL, int32_t linkSubType, int32_t linkIdx, sLink_t* linkPtrList[]);

sWorkloadUnit_t * getWLUnitPtr(sGCHelperHandle * gcHelperHandle, int32_t unitIdx);

int32_t getLinkIdx(const sWorkloadUnit_t *pWL, int32_t linkSubType, int32_t *linkIdx);

//:TODO: Temporary location need to work on right location for this structure
typedef struct
{
  int32_t groupIdx;
  int32_t subGroupIdx;
  int32_t countIdx;
  int32_t workLoadIdx;
  int32_t coreId;
  int32_t algLayerIdx;
  int32_t layerGroupId;
  sTIDL_Network_t       *net;
  sGCHelperHandle * gcHelperHandle;
  sWorkloadSuperGroup_t *wlSuperGroup;
}sGetLayerIdContext_t;


/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         getLayerIdInit
@brief      Function to initialize the context for getLayerId functions
@param      getIdContext[in] : Memory for the context. User can directly use sGetLayerIdContext_t to allocate memory for
                                          this
@param      coreId[in] : Core Id for the core to be executed
@param      net[in] : TIDL net structure
@param      gcHelperHandle[in] : Pointer to the gcHelperHandle
@param      wlSuperGroup[in] : Pointer to the wlSuperGroup
@remarks   None
@return     None
----------------------------------------------------------------------------
*/
void getLayerIdInit(void * getIdContext,
                            int32_t coreId,
                            int32_t layerGroupId,
                            sTIDL_Network_t       *net_m,
                            sGCHelperHandle         *gcHelperHandle,
                            sWorkloadSuperGroup_t *wlSuperGroup);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         getLayerIdToExecute
@brief      Function to get the layer Id to be execute ( i.e. alloc, init or process). This function
              also returns the workLoadUnit
@param      getIdContext[in] :Context for getting the layerId
@param      isInit[in] : Set to 1 if layerId is needed for init/alloc, else 0
@param      workLoadUnit[out]    : Pointer to the current workLoadUnit (NULL if not available)

@remarks    None
@return     Layer Id to be executed ( LayerId will be NOT_VALID if we are done with execution )
----------------------------------------------------------------------------
*/
int32_t getLayerIdToExecute(void * getIdContext,
                                         int32_t                isInit,
                                         int32_t                *currAlgLayerIdx,
                                         int32_t                *workloadIdx,
                                         sWorkloadUnit_t        **workLoadUnit);

int32_t getNumSplits(const void *workloadHandle);
int8_t * get_int8_t_pointer(int8_t arr[], int32_t offset);
void TIDL_factorizeBy64K(uint32_t icnt0, uint32_t icnt1, uint32_t *factorLarge, uint32_t *factorSmall);
int8_t * getGCDataBase(sGraphCompilerOutArgs_t *gcInfo, int32_t dataBaseType);

/*this function is used to split a dimension across multi-core*/
int32_t getDimSplitForCore(int32_t coreIdx, int32_t numCore, int32_t totalDimValue);

/*this function is used calculate input channel to be processed by a core across multi-core channel split*/
int32_t getNumInChforCore(int32_t coreIdx, int32_t numCore, int32_t totalNi, int32_t totalGrpNum, int32_t layerType);

/*this function is used calculate output channel to be processed by a core across multi-core channel split*/
int32_t getNumOutChforCore(int32_t coreIdx, int32_t numCore, int32_t totalNo, int32_t totalGrpNum, int32_t layerType);
#endif

#if 0

/*--------------------------------------------------------------------------- */
/* For a given network, given layer - there can be one or multiple workload   */
/* unit, we shall provide query functions to retrieve multiple                */
/* information. Some of the key information are                               */
/* 1. Number of links                                                         */
/* 2. Number of iterations                                                    */
/* 3. For a given link by subtype                                             */
/*  a.Number of source                                                        */
/*  b.what kind of data is at given source index                              */
/*  c.buffer data base index for a given source index                         */
/*  d.Meta information of the workload unit to which the link belongs to      */
/*   d.1 {inwidth, inheight, inChannels, outChannels, Fr, Fc, grp, pad}       */
/*   d.2 {subWidth, subHeight, subNi, subNo, startX, startY, startNi, startNo}*/
/*   d.3 {procWidth, procHeight, procNi, procNo}                              */
/*--------------------------------------------------------------------------- */

typedef struct
{
  int32_t Fr;
  int32_t Fc;
  int32_t grp;
  int32_t dilationR;
  int32_t dilationC;
} sMetaDataPramas_t;

typedef struct
{
  int32_t width;
  int32_t height;
  int32_t channels;
  int32_t batch;
  int32_t layout;
} sMetaDataTensorDim_t;

typedef struct
{
  int32_t startX;
  int32_t startY;
  int32_t startChannel;
  int32_t startBatch;
} sMetaDataTensorPos_t;

typedef enum
{
TENSOR_FULL = 0,
TENSOR_SUB,
TENSOR_PROC
} eTensorSizeType;

typedef enum
{
IN = 0,
OUT
} eDir;

int32_t getTensorPosInfo(
  sMetaDataTensorPos_t *pTensor,
  int32_t networkId,
  int32_t layerId,
  int32_t subLayerId,
  int32_t tenosrSizeType,
  int32_t dir);

int32_t getTensorDimInfo(
  sMetaDataTensorDim_t  *pTensor,
  int32_t networkId,
  int32_t layerId,
  int32_t subLayerId,
  int32_t tenosrSizeType,
  int32_t dir);
#endif


