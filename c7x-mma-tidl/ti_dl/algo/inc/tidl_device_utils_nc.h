/*
 * Module Name        :TIDL
 *
 * Module Description :TI Deep learning Library module is TI's CNN/DNN
 *                     acceleration on EVE and DSP
 *
 * Copyright (C) 2016-2019 Texas Instruments Incorporated - http://www.ti.com/
 *
 * ALL RIGHTS RESERVED
 *
 *
 */

/**
 *  \file tidl_device_utils_nc.h
 *
 *  \brief This file contains headers for functions used by inference and graph compiler
 */

#include "perfsim.h"
#include <math.h>
#include <float.h>
#include "gc_helper.h"
#include "tidl_deviceInfo.h"
#include "../../inc/tidl_dataflow.h"
#include "../common/MMALIB_bufParams.h"
#include "mmalib_cnn.h"
#include "tidl_types.h"

#ifndef ITIDL_DEVICE_UTILS_NC_H
#define ITIDL_DEVICE_UTILS_NC_H

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Total number of inputs in GS */
#define TIDL_GS_NUM_INP_BUFS (4)
/* Input index of feat input */
#define TIDL_GS_INPUT_IDX_FEAT (0)
/* Input index of offsets/grid input */
#define TIDL_GS_INPUT_IDX_GRID (1)
/* Input index of scratch buffer for transpose */
#define TIDL_GS_INPUT_IDX_SCRATCH_TRANSPOSE (2)
/* Input index of mask */
#define TIDL_GS_INPUT_IDX_MASK (3)

#define TIDL_DEFORM_CONV_BUF_IDX_SRC 0
#define TIDL_DEFORM_CONV_BUF_IDX_WEIGHTS 1
#define TIDL_DEFORM_CONV_BUF_IDX_OFFSET 2
#define TIDL_DEFORM_CONV_BUF_IDX_BIAS 3
#define TIDL_DEFORM_CONV_BUF_IDX_MASK 4
#define TIDL_DEFORM_CONV_BUF_IDX_SCRATCH 5
#define TIDL_DEFORM_CONV_BUF_IDX_MAX 6



#define TIDL_DEFORM_CONV_JOINT_IDX_SRC 0
#define  TIDL_DEFORM_CONV_JOINT_IDX_OFFSET 1
#define  TIDL_DEFORM_CONV_JOINT_IDX_MASK 2
#define  TIDL_DEFORM_CONV_JOINT_IDX_WEIGHTS 3
#define  TIDL_DEFORM_CONV_JOINT_IDX_BIAS 4
#define  TIDL_DEFORM_CONV_JOINT_IDX_MAX 5


#define TIDL_MATMUL_DEVICE_INPUTA    (0)
#define TIDL_MATMUL_DEVICE_INPUTB    (1)
#define TIDL_MATMUL_DEVICE_MAXINBUFS (2U)
#define TIDL_MATMUL_DEVICE_OUTPUT    (TIDL_MATMUL_DEVICE_MAXINBUFS)
#define TIDL_MATMUL_DEVICE_MAXBUFS   (TIDL_MATMUL_DEVICE_OUTPUT + (1U))

#define TIDL_MATMUL_KERNEL_CENTER       (0b00U)  /*0*/
#define TIDL_MATMUL_KERNEL_RIGHT        (0b01U)  /*1*/
#define TIDL_MATMUL_KERNEL_BOTTOM       (0b10U)  /*2*/
#define TIDL_MATMUL_KERNEL_BOTTOM_RIGHT (TIDL_MATMUL_KERNEL_BOTTOM | TIDL_MATMUL_KERNEL_RIGHT) /*3*/
#define TIDL_MATMUL_KERNEL_MAX_MEMBERS  (TIDL_MATMUL_KERNEL_BOTTOM_RIGHT + 1U)

#define TIDL_CEIL_DIV(NUM, DENOM) (((NUM) + (DENOM) - 1U)/(DENOM))

#define TIDL_1913_NOT_FIXED (1)
#define TIDL_2386_NOT_FIXED (1)
#define TIDL_2383_NOT_FIXED (1)
#define TIDL_3350_NOT_FIXED (1)

#define  TIDLCONV_TYPE_CONV (0)
#define  TIDLCONV_TYPE_DECONV (1)
/* For DepthToSpace/PixelShuffle Layer */
#define  TIDLCONV_TYPE_CONVPS (2)


#define TIDL_SAT_HI_UINT16             (65535)

#define TIDL_TOPK_CONTEXT (0)
#define TIDL_TOPK_KERNEL_1 (1)
#define TIDL_TOPK_KERNEL_2 (2)
#define TIDL_MAX_TOPK_HANDLE_MEMBERS (3)



#define TIDL_TOPK_KERNEL_1_IDX           (0)
#define TIDL_TOPK_KERNEL_2_IDX           (1)
#define TIDL_TOPK_KERNEL_MAX_IDX         (2)


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void TIDL_GetTensorDimensions(const sTIDL_Network_t *myNet,
                              const void *workloadUnit,
                              int32_t  layerIdx,
                              int32_t *inWidth,
                              int32_t *inHeight,
                              int32_t *inChannels,
                              int32_t *outWidth,
                              int32_t *outHeight,
                              int32_t *outChannels);

void TIDL_GetTensorHigherDimensions(const sTIDL_Network_t *myNet,
                              const void *workloadUnit,
                              int32_t  layerIdx,
                              int32_t *inDim1,
                              int32_t *inDim2,
                              int32_t *inBatch,
                              int32_t *outDim1,
                              int32_t *outDim2,
                              int32_t *outBatch);
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getDataParams
@brief      Function to Check whether current data buffer is Input to
            current algorithm instance

@param      pTIDLNetStructure : Base pointer for myNet
@param      dataId            : Current data buffer ID
@remarks    None
@return     Address of the Data Params Structure Base - If Successful
            NULL - If Faile - Requested dataID Not found
----------------------------------------------------------------------------
*/
sTIDL_DataParams_t * TIDL_getDataParams(sTIDL_Network_t * pTIDLNetStructure,
int32_t dataId);

#if defined TIDL_DYNAMIC_SHAPE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getMultipleDataParams
@brief      Optimized batch lookup function to retrieve multiple
            sTIDL_DataParams_t* at once. Performs a single O(n) pass through
            the layer list to find all requested dataIds, which is more
            efficient than calling TIDL_getDataParams() multiple times.

@param      pTIDLNetStructure : Network structure containing all layers
@param      dataIds           : Array of dataIds to look up
@param      numDataIds        : Number of dataIds in the array
@param      dataParams        : Output array to store the found pointers
                                (must be pre-allocated with size >= numDataIds)
@remarks    Entries in outDataParams will be set to NULL if corresponding
            dataId is not found. This function is optimized for multi-input
            layers where multiple dataIds need to be resolved simultaneously.
@return     Number of dataIds successfully found (0 to numDataIds)
            Returns -1 if input validation fails
----------------------------------------------------------------------------
*/
int32_t TIDL_getMultipleDataParams(sTIDL_Network_t *pTIDLNetStructure,
                                    const int32_t *dataIds,
                                    int32_t numDataIds,
                                    sTIDL_DataParams_t **dataParams);
#endif

void tidl_printf(int8_t traceLevel, const char *format, ...);

int32_t TIDL_deviceUtilsGetPrefetchLFMJoint(const sWorkloadUnit_t *workloadUnit, const sJoint_t *joint);

/* Returns pointer to buffer containing info about the tensor the joint is trying to access */
const sBufParams_t * TIDL_getBufferParams(const sJoint_t *joint, const sGCHelperHandle * gcHelperHandle);

static inline int32_t getAlgLyrIdxFromWL(const sWLDataBase_t *pWLDB, const sWorkloadUnit_t  *workloadUnit);

int32_t TIDL_deviceUtilsSetKernelIoBuffer(const sLink_t *myLink,
                                        const sTIDL_Network_t *myNet,
                                        const sGCHelperHandle   *gcHelperHandle,
                                        const void *workloadUnit,
                                        int32_t layerIdx,
                                        int32_t flowStage,
                                        TIDL_bufParams3D_t     *srcAddr1,
                                        TIDL_bufParams3D_t     *dstAddr);

int32_t TIDL_matmulSetInputIdx(const sLink_t              *myLink,
                                          const sGCHelperHandle * gcHelperHandle,
                                          int32_t inLayerDataIdA,
                                          int32_t inLayerDataIdB,
                                          int32_t *srcIdxA,
                                          int32_t *srcIdxB);

int32_t TIDL_matmulDeviceGetWlLoopMaxIter(const sWorkloadUnit_t *workloadUnit,
                                        const sTIDL_Layer_t *tidlLayer,
                                        const sGCHelperHandle * gcHelperHandle);


int32_t TIDL_deviceUtilsSetKernelIoBufferArgOp(const sLink_t *myLink,
                                             const sTIDL_Network_t *myNet,
                                             const sGCHelperHandle   *gcHelperHandle,
                                             const void *workloadUnit,
                                             int32_t layerIdx,
                                             int32_t flowStage,
                                             TIDL_bufParams3D_t     *srcAddr2,
                                             TIDL_bufParams3D_t     *dstAddr);

int32_t TIDL_deviceUtilsSetKernelIoBufferConcat(const sLink_t *myLink,
                                             const sTIDL_Network_t *myNet,
                                             const sGCHelperHandle   *gcHelperHandle,
                                             const void *workloadUnit,
                                             int32_t layerIdx,
                                             int32_t flowStage,
                                             TIDL_bufParams3D_t     *srcAddr1,
                                             TIDL_bufParams3D_t     *dstAddr);

int32_t TIDL_deviceUtilsSetKernelIoBufferEltwise(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              TIDL_bufParams3D_t     *srcAddr2,
                                              TIDL_bufParams3D_t     *dstAddr);


int32_t TIDL_deviceUtilsSetKernelIoBufferTopK(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              TIDL_bufParams6D_t     *srcAddr2,
                                              TIDL_bufParams6D_t     *dstAddr);


int32_t TIDL_deviceUtilsSetKernelIoBufferTranspose(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              TIDL_bufParams6D_t *srcAddr,
                                              TIDL_bufParams6D_t *dstAddr);


int32_t TIDL_getDeformConvNumSplitHorAndVer(const sTIDL_Network_t * myNet,
                                                int32_t layerIdx,
                                                int32_t *numFeatureSplitVer,
                                                int32_t *numFeatureSplitHor, 
                                                int32_t scratchSizeAvailable);


int32_t TIDL_deviceUtilsSetKernelIoBufferDeFormConv(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              TIDL_bufParams6D_t *srcAddr,
                                              TIDL_bufParams6D_t *dstAddr);


int32_t TIDL_deviceUtilsSetKernelIoBufferGridSample(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              TIDL_bufParams6D_t *srcAddr,
                                              TIDL_bufParams6D_t *dstAddr);

int32_t TIDL_deviceUtilsSetKernelIoBufferGatherLayer(const sLink_t *slink,
                                                    const sTIDL_Network_t *snet,
                                                    const sGCHelperHandle *gcHelperHandle,
                                                    const void *workloadUnit,
                                                    int32_t layerIdx,
                                                    int32_t flowStage,
                                                    TIDL_bufParams6D_t *srcAddr,
                                                    TIDL_bufParams6D_t *dstAddr);

int32_t TIDL_isSecondTensorSplit(const void *workloadUnit);

int32_t TIDL_getProcWidthFromWorkloadForMultiInput(const void *workloadUnit,
                                                   const sTIDL_Layer_t *tidlLayer,
                                                   const sLink_t         *myLink,
                                                   const sGCHelperHandle * gcHelperHandle,
                                                   int32_t srcIdxA, int32_t srcIdxB, int32_t isSecondInputTranspose);

template <class typeBufParams>
int32_t TIDL_deviceUtilsSetKernelIoBufferMatMul(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              typeBufParams *srcAddrList,
                                              typeBufParams *dstAddrList);

template <class typeInt>
void TIDL_matmulGetChannelAxisDimvalues(const sTIDL_Layer_t *tidlLayer,
                                        const void *workloadUnit,
                                        const sLink_t *myLink,
                                        int32_t flowStage,
                                        const sGCHelperHandle * gcHelperHandle,
                                        int32_t srcIdxA, int32_t srcIdxB,
                                        uint32_t *srcAdimZ, typeInt *srcApitchZ,
                                        uint32_t *srcBdimZ, typeInt *srcBpitchZ,
                                        uint32_t *dstdimZ,  typeInt *dstpitchZ);

/*this functions returns the Physical height or width */
int32_t TIDL_conv2dGetPhysicalDimension(int32_t inWidth, int32_t eF, int32_t targetDevice);

void TIDL_conv2dGetSrcCoeffJointIdx(const sLink_t              *myLink,
                                                  const sGCHelperHandle * gcHelperHandle,
                                                  int32_t                    *srcIdx,
                                                  int32_t                    *coeffIdx,
                                                  int32_t                    *biasIdx);

template <class typeBufParams>
void TIDL_UpdateSrcAddrConvColFlow(typeBufParams *srcAddr4,
                              int32_t featureWidth1,
                              int32_t blockFeatureHeight1,
                              int32_t numSplit);

void  TIDL_conv2dColFlowGetMmaWidthHeight(const sTIDL_ConvParams_t  *convParams, 
                                          const sJoint_t *srcJoint, 
                                          int32_t inFeatWidth, 
                                          int32_t inFeatHeight, 
                                          int32_t colPad,
                                          int32_t effKerW, 
                                          int32_t *mmaFeatureWidth, 
                                          int32_t *mmaFeatureHeight, 
                                          int32_t flowStage, 
                                          int32_t targetDevice);

int32_t TIDL_conv2dColFlow_getNumGroupsPerKernelCall(const sJoint_t* srcJoint);

template <class typeBufParamsSrc, class typeBufParamsDst>
int32_t TIDL_deviceUtilsSetKernelIoBufferConvColFlow(const sLink_t *myLink,
                                                 const sTIDL_Network_t *myNet,
                                                 const sGCHelperHandle   *gcHelperHandle,
                                                 const void *workloadUnit,
                                                 int32_t layerIdx,
                                                 int32_t flowStage,
                                                 typeBufParamsSrc *srcAddr7,
                                                 typeBufParamsDst *dstAddr);

int32_t TIDL_isColumnFlowlayer(const sTIDL_Layer_t *tidlLayer,
                                          const sWorkloadUnit_t* workloadUnit,
                                          const sGCHelperHandle * gcHelperHandle);

template <class typeBufParamsSrc, class typeBufParamsDst>
int32_t TIDL_deviceUtilsSetKernelIoBufferConvRowFlow(const sLink_t *myLink,
                                             const sTIDL_Network_t *myNet,
                                             const sGCHelperHandle   *gcHelperHandle,
                                             const void *workloadUnit,
                                             int32_t layerIdx,
                                             int32_t flowStage,
                                             typeBufParamsSrc *srcAddr7,
                                             typeBufParamsDst *dstAddr);

void TIDL_dataConvertGetDimAndPitchNew(const sJoint_t *jointParams,
                                               const sBufParams_t * bufferParams,
                                               const sTIDL_Network_t * myNet,
                                               const void * workloadUnit,
                                               int32_t layerIdx,
                                               int32_t &dimX,
                                               int32_t &dimY,
                                               int32_t &dimZ,
                                               int32_t &pitchDimY,
                                               int32_t &pitchDimZ,
                                               int32_t bufType);

template <class typeBufParams>
int32_t TIDL_deviceUtilsSetKernelIoBufferDataConvert(const sLink_t *myLink,
                                              const sTIDL_Network_t *myNet,
                                              const sGCHelperHandle   *gcHelperHandle,
                                              const void *workloadUnit,
                                              int32_t layerIdx,
                                              int32_t flowStage,
                                              typeBufParams* srcAddr,
                                              typeBufParams* dstAddr);

uint32_t TIDL_getMmalibDatatypeFromGcDatatype(int32_t gcDataType, int32_t procElemSize);

void TIDL_dataConvertSetKernelLayout(int32_t myType,
                                     int32_t layout,
                                     int32_t outLayout,
                                     int32_t *srcLayoutType,
                                     int32_t *dstLayoutType);

int32_t TIDL_conv2dRowFlowIsGrpOptAvailable(const sJoint_t *coeffJoint, 
                                            const sTIDL_ConvParams_t * params,
                                            int32_t convType);

int32_t TIDL_conv2dGetConvType(int32_t layerType);


#endif
