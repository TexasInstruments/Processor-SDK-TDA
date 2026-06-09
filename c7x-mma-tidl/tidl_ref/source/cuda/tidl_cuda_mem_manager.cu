/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*/

/**
 ----------------------------------------------------------------------------
 @file    tidl_cuda_mem_manager.cu
 @brief   CUDA Memory Manager Implementation
 @version 1.0 (Nov 2025) : Initial version
 ----------------------------------------------------------------------------
*/

#ifdef BUILD_WITH_CUDA

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include <stdio.h>
#include <stdarg.h>
#include "tidl_cuda_mem_manager.h"
#include "tidl_cuda.h"

/**
----------------------------------------------------------------------------
@fn         TIDL_cudaFreeAllCudaPtrs
@brief      Wrapper function that calls all individual CUDA free functions
            to free device pointers and reset initialization flags
----------------------------------------------------------------------------
*/
void TIDL_cudaFreeAllCudaPtrs()
{
  TIDL_cudaFreeConvCudaPtrs();
  TIDL_cudaFreeGridSampleCudaPtrs();
  TIDL_cudaFreeInnerProductCudaPtrs();
  TIDL_cudaFreeBatchNormCudaPtrs();
  TIDL_cudaFreeEltwiseCudaPtrs();
  TIDL_cudaFreeSliceCudaPtrs();
  TIDL_cudaFreeTransposeCudaPtrs();
}

/* Global pointer to the CUDA Memory Manager for access from other CUDA files */
TIDL_CudaMemManager* g_cudaMemManager = NULL;

/* Maximum number of TIDL layer types (should be at least as large as the highest layer type value) */
#define NUM_TIDL_LAYER_TYPES 50

/* Define which layer types are supported on GPU */
static const int32_t isSupportedOnGPU[NUM_TIDL_LAYER_TYPES] = {
    0,  /* TIDL_DataLayer (0) */
    0,  /* TIDL_ConvolutionLayer (1) */
    0,  /* TIDL_PoolingLayer (2) */
    0,  /* TIDL_ReLULayer (3) */
    0,  /* TIDL_PReLULayer (4) */
    0,  /* TIDL_EltWiseLayer (5) */
    0,  /* TIDL_InnerProductLayer (6) */
    0,  /* TIDL_SoftMaxLayer (7) */
    0,  /* TIDL_BatchNormLayer (8) */
    0,  /* TIDL_BiasLayer (9) */
    0,  /* TIDL_ScaleLayer (10) */
    0,  /* TIDL_Deconv2DLayer (11) */
    0,  /* TIDL_ConcatLayer (12) */
    0,  /* TIDL_SplitLayer (13) */
    0,  /* TIDL_SliceLayer (14) */
    0,  /* TIDL_CropLayer (15) */
    0,  /* TIDL_FlattenLayer (16) */
    0,  /* TIDL_DropOutLayer (17) */
    0,  /* TIDL_ArgMaxLayer (18) */
    0,  /* TIDL_DetectionOutputLayer (19) */
    0,  /* TIDL_ShuffleChannelLayer (20) */
    0,  /* TIDL_ResizeLayer (21) */
    0,  /* TIDL_RoiPoolingLayer (22) */
    0,  /* TIDL_OdPostProcessingLayer (23) */
    0,  /* TIDL_DepthToSpaceLayer (24) */
    0,  /* TIDL_SigmoidLayer (25) */
    0,  /* TIDL_PadLayer (26) */
    0,  /* TIDL_ColorConversionLayer (27) */
    0,  /* TIDL_OdOutputReformatLayer (28) */
    0,  /* TIDL_DataConvertLayer (29) */
    0,  /* TIDL_CustomLayer (30) */
    0,  /* TIDL_BatchReshapeLayer (31) */
    0,  /* TIDL_ReduceLayer (32) */
    0,  /* TIDL_ScatterElementsLayer (33) */
    0,  /* TIDL_SqueezeLayer (34) */
    0,  /* TIDL_TanhLayer (35) */
    0,  /* TIDL_HardSigmoidLayer (36) */
    0,  /* TIDL_ELULayer (37) */
    0,  /* TIDL_ReshapeLayer (38) */
    0,  /* TIDL_ConstDataLayer (39) */
    0,  /* TIDL_GatherLayer (40) */
    0,  /* TIDL_TransposeLayer (41) */
    0,  /* TIDL_LayerNormLayer (42) */
    0,  /* TIDL_GridSampleLayer (43) */
    0,  /* TIDL_TopKLayer (44) */
    0,  /* TIDL_DeformableConvLayer (45) */
    0,  /* TIDL_TileLayer (46) */
    0,  /* TIDL_RMSNormalizationLayer (47) */
    0,  /* TIDL_LSTMLayer (48) */
    0,  /* TIDL_GRULayer (49) */
    0,  /* TIDL_RNNLayer (50) */
    0,  /* TIDL_UnsupportedLayer (51) */
};

/* ============================================================================
 * Logging System for x86/CUDA
 * ============================================================================
 */

/* Log levels - matching TIDL trace levels */
#define TIDL_LOG_LEVEL_ERROR    0  /**< Critical errors */
#define TIDL_LOG_LEVEL_INFO     1  /**< Important information */
#define TIDL_LOG_LEVEL_DEBUG    2  /**< Debug information */
#define TIDL_LOG_LEVEL_TRACE    3  /**< Detailed trace */

/* Global log level - can be set externally or via environment variable */
static int32_t g_cuda_log_level = TIDL_LOG_LEVEL_INFO;

/**
 * @brief Set the CUDA logging level
 * @param level Log level (0=ERROR, 1=INFO, 2=DEBUG, 3=TRACE)
 */
void TIDL_cudaSetLogLevel(int32_t level)
{
    g_cuda_log_level = level;
}

/**
 * @brief Get the current CUDA logging level
 * @return Current log level
 */
int32_t TIDL_cudaGetLogLevel(void)
{
    return g_cuda_log_level;
}

/**
 * @brief Printf implementation for CUDA with log level filtering
 * @param logLevel Minimum log level for this message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
static void cuda_printf(int32_t logLevel, const char *format, ...)
{
    if (logLevel <= g_cuda_log_level)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        fflush(stdout);  /* Ensure output is visible immediately */
    }
}

/* Macro to replace tidl_printf for CUDA code */
#define tidl_printf cuda_printf

/* Helper macros */
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            cuda_printf(TIDL_LOG_LEVEL_ERROR, \
                       "CUDA Error at %s:%d - %s\n", __FILE__, __LINE__, \
                       cudaGetErrorString(err)); \
            return IALG_EFAIL; \
        } \
    } while(0)

#define CUDA_CHECK_VOID(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            cuda_printf(TIDL_LOG_LEVEL_ERROR, \
                       "CUDA Error at %s:%d - %s\n", __FILE__, __LINE__, \
                       cudaGetErrorString(err)); \
        } \
    } while(0)

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================
 */

/**
 * @brief Check if pointer is within a memory range
 */
static int32_t isPointerInRange(const void *ptr, const void *base, uint32_t size)
{
    uintptr_t ptrAddr = (uintptr_t)ptr;
    uintptr_t baseAddr = (uintptr_t)base;
    uintptr_t endAddr = baseAddr + size;
    
    return (ptrAddr >= baseAddr && ptrAddr < endAddr) ? 1 : 0;
}

/**
 * @brief Get offset of pointer within base
 */
static ptrdiff_t getPointerOffset(const void *ptr, const void *base)
{
    return (const char*)ptr - (const char*)base;
}

/* ============================================================================
 * Public Function Implementations
 * ============================================================================
 */

/**
 * @brief Build layer dependency graph based on layer input/output connections
 */
int32_t TIDL_cudaBuildLayerDependencyGraph(
    TIDL_CudaMemManager *manager,
    sTIDL_Layer_t* TIDLLayers)
{
    if (manager == NULL || manager->layerDependencies == NULL || TIDLLayers == NULL)
    {
        return IALG_EFAIL;
    }

    tidl_printf(1, "Building layer dependency graph for %d layers\n", manager->numLayers);

    sTIDL_Layer_t *layer;
    TIDL_LayerDependency *layerDep;

    int32_t dataIdToLayerMap[manager->numLayers];
    memset(dataIdToLayerMap, -1, sizeof(int32_t) * manager->numLayers);

    for(int32_t layerId = 0; layerId < manager->numLayers; layerId++)
    {
        layerDep = &manager->layerDependencies[layerId];
        layer = &TIDLLayers[layerId];
        memset(layerDep->inputBufferToLayerMap, -1, sizeof(int32_t) * TIDL_NUM_IN_BUFS);
        layerDep->numInputLayers = 0;
        layerDep->numOutputLayers = 0;
        int32_t outId = layer->outData.dataId;
        if(layer->layerType == TIDL_DataLayer && layerId != 0 && outId ==0) continue; // output layer
        dataIdToLayerMap[outId] = layerId;
    }

    for(int32_t layerId = 0; layerId < manager->numLayers; layerId++)
    {
        layerDep = &manager->layerDependencies[layerId];
        layer = &TIDLLayers[layerId];
        for(int32_t inBufIdx = 0; inBufIdx < layer->numInBufs; inBufIdx++)
        {
            int32_t inDataId = layer->inData[inBufIdx];
            int32_t prevLayerId = dataIdToLayerMap[inDataId];
            if(prevLayerId < 0 || prevLayerId == layerId) continue;
            layerDep->inputBufferToLayerMap[inBufIdx] = prevLayerId;
            tidl_printf(2, "Layer %d: Input buffer %d comes from layer %d\n", layerId, inBufIdx, prevLayerId);
            int duplicate = 0;
            for(int j = 0; j < layerDep->numInputLayers; j++){
                if(layerDep->inputLayerIds[j] == prevLayerId) {
                    duplicate = 1;
                    break;
                }
            }
            if(duplicate) continue;

            layerDep->inputLayerIds[layerDep->numInputLayers++] = prevLayerId;
            tidl_printf(2, "Layer %d depends on input from layer %d\n", layerId, prevLayerId);
            TIDL_LayerDependency *prevLayerDep = &manager->layerDependencies[prevLayerId];

            int already_added = 0;
            for(int j = 0; j < prevLayerDep->numOutputLayers; j++) {
                if(prevLayerDep->outputLayerIds[j] == layerId) {
                    already_added = 1;
                    break;
                }
            }

            if(already_added) continue;

            prevLayerDep->outputLayerIds[prevLayerDep->numOutputLayers++] = layerId;
            tidl_printf(2, "Layer %d outputs to layer %d\n", prevLayerId, layerId);
        }
    }
    return IALG_EOK;
}

int32_t TIDL_cudaMemManagerInit(
    TIDL_CudaMemManager *manager,
    const IALG_MemRec memRec[],
    int32_t numMemRecs,
    int32_t numLayers,
    sTIDL_Layer_t* TIDLLayers)
{
    if (manager == NULL || memRec == NULL || numMemRecs <= 0 || numMemRecs > NUM_MEMRECS_TIDL)
    {
        tidl_printf(0, "TIDL_cudaMemManagerInit: Invalid parameters\n");
        return IALG_EFAIL;
    }
    
    /* Clear manager structure */
    memset(manager, 0, sizeof(TIDL_CudaMemManager));
    
    manager->numMemRecs = numMemRecs;
    manager->numLayers = numLayers;
    
    /* Initialize each memrec */
    for (int32_t i = 0; i < numMemRecs; i++)
    {
        manager->memRecs[i].h_base = memRec[i].base;
        manager->memRecs[i].size = memRec[i].size;
        manager->memRecs[i].space = memRec[i].space;
        manager->memRecs[i].attrs = memRec[i].attrs;
        manager->memRecs[i].alignment = memRec[i].alignment;
        manager->memRecs[i].d_base = NULL;
        manager->memRecs[i].isAllocated = 0;
        manager->memRecs[i].needsSync = 0;
        manager->memRecs[i].lastSyncDirection = TIDL_SYNC_NONE;
    }
    
    /* Allocate layer GPU support array */
    if (numLayers > 0)
    {
        manager->layerGpuSupport = (int32_t*)malloc(numLayers * sizeof(int32_t));
        if (manager->layerGpuSupport == NULL)
        {
            tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to allocate layerGpuSupport\n");
            return IALG_EFAIL;
        }
        
        /* Initialize layer GPU support based on layer type */
        for (int32_t i = 0; i < numLayers; i++)
        {
            manager->layerGpuSupport[i] = isSupportedOnGPU[TIDLLayers[i].layerType];
        }
        
        /* Allocate layer dependencies array */
        manager->layerDependencies = (TIDL_LayerDependency*)malloc(numLayers * sizeof(TIDL_LayerDependency));
        if (manager->layerDependencies == NULL)
        {
            tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to allocate layerDependencies\n");
            free(manager->layerGpuSupport);
            manager->layerGpuSupport = NULL;
            return IALG_EFAIL;
        }
        
        /* Initialize layer dependencies */
        memset(manager->layerDependencies, 0, numLayers * sizeof(TIDL_LayerDependency));
        
        /* Build the dependency graph */
        if (TIDL_cudaBuildLayerDependencyGraph(manager, TIDLLayers) != IALG_EOK)
        {
            tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to build layer dependency graph\n");
            free(manager->layerDependencies);
            free(manager->layerGpuSupport);
            manager->layerDependencies = NULL;
            manager->layerGpuSupport = NULL;
            return IALG_EFAIL;
        }
    }
    
    manager->isInitialized = 1;
    manager->currentLayer = -1;
    
    tidl_printf(1, "CUDA Memory Manager initialized with %d memrecs and %d layers\n",
               numMemRecs, numLayers);
    
    return IALG_EOK;
}

int32_t TIDL_cudaMemManagerAllocate(TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        tidl_printf(0, "TIDL_cudaMemManagerAllocate: Manager not initialized\n");
        return IALG_EFAIL;
    }
    
    uint64_t totalGpuMem = 0;
    
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        TIDL_CudaMemRecord *rec = &manager->memRecs[i];
        
        if (rec->size == 0 || rec->h_base == NULL)
        {
            tidl_printf(2, "Skipping memrec %d (size=%u, base=%p)\n", i, rec->size, rec->h_base);
            continue;
        }
        
        /* Allocate GPU memory */
        CUDA_CHECK(cudaMalloc(&rec->d_base, rec->size));
        rec->isAllocated = 1;
        totalGpuMem += rec->size;
        
        /* Clear GPU memory */
        CUDA_CHECK(cudaMemset(rec->d_base, 0, rec->size));
        
        tidl_printf(2, "Allocated GPU memrec[%d]: %u bytes at %p (CPU: %p)\n",
                   i, rec->size, rec->d_base, rec->h_base);
    }
    
    CUDA_CHECK(cudaDeviceSynchronize());
    
    tidl_printf(1, "Total GPU memory allocated: %.2f MB\n", totalGpuMem / (1024.0 * 1024.0));
    
    return IALG_EOK;
}

void TIDL_cudaMemManagerFree(TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        return;
    }
    
    /* Free all GPU memory */
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        if (manager->memRecs[i].isAllocated && manager->memRecs[i].d_base != NULL)
        {
            CUDA_CHECK_VOID(cudaFree(manager->memRecs[i].d_base));
            manager->memRecs[i].d_base = NULL;
            manager->memRecs[i].isAllocated = 0;
        }
    }
    
    /* Free layer support array */
    if (manager->layerGpuSupport != NULL)
    {
        free(manager->layerGpuSupport);
        manager->layerGpuSupport = NULL;
    }
    
    /* Free layer dependencies array */
    if (manager->layerDependencies != NULL)
    {
        free(manager->layerDependencies);
        manager->layerDependencies = NULL;
    }
    
    tidl_printf(1, "CUDA Memory Manager freed\n");
    
    manager->isInitialized = 0;
}

int32_t TIDL_cudaMemManagerCopyPersistentH2D(TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        tidl_printf(0, "TIDL_cudaMemManagerCopyPersistentH2D: Manager not initialized\n");
        return IALG_EFAIL;
    }
    
    uint64_t totalCopied = 0;
    
    /* Copy all PERSISTENT memory to GPU */
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        TIDL_CudaMemRecord *rec = &manager->memRecs[i];
        
        if (!rec->isAllocated || rec->h_base == NULL || rec->d_base == NULL)
        {
            continue;
        }
        
        /* Only copy persistent data */
        if (rec->attrs == IALG_PERSIST)
        {
            CUDA_CHECK(cudaMemcpy(rec->d_base, rec->h_base, rec->size, cudaMemcpyHostToDevice));
            rec->lastSyncDirection = TIDL_SYNC_H2D;
            totalCopied += rec->size;
            
            tidl_printf(2, "Copied persistent memrec[%d]: %u bytes H2D\n", i, rec->size);
        }
    }
    
    CUDA_CHECK(cudaDeviceSynchronize());
    
    tidl_printf(1, "Copied %.2f MB of persistent data to GPU\n", totalCopied / (1024.0 * 1024.0));
    
    return IALG_EOK;
}

int32_t TIDL_cudaTranslatePtrCPUtoGPU(
    const TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    void **gpuPtr,
    int32_t *memRecIdx)
{
    if (manager == NULL || !manager->isInitialized || cpuPtr == NULL || gpuPtr == NULL)
    {
        return IALG_EFAIL;
    }
    
    /* Search through all memrecs */
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        const TIDL_CudaMemRecord *rec = &manager->memRecs[i];
        
        if (!rec->isAllocated || rec->h_base == NULL || rec->d_base == NULL)
        {
            continue;
        }
        
        /* Check if pointer is within this memrec */
        if (isPointerInRange(cpuPtr, rec->h_base, rec->size))
        {
            /* Calculate offset and return GPU pointer */
            ptrdiff_t offset = getPointerOffset(cpuPtr, rec->h_base);
            *gpuPtr = (char*)rec->d_base + offset;
            
            if (memRecIdx != NULL)
            {
                *memRecIdx = i;
            }
            
            return IALG_EOK;
        }
    }
    
    /* Pointer not found in any memrec */
    return IALG_EFAIL;
}

int32_t TIDL_cudaTranslatePtrGPUtoCPU(
    const TIDL_CudaMemManager *manager,
    const void *gpuPtr,
    void **cpuPtr,
    int32_t *memRecIdx)
{
    if (manager == NULL || !manager->isInitialized || gpuPtr == NULL || cpuPtr == NULL)
    {
        return IALG_EFAIL;
    }
    
    /* Search through all memrecs */
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        const TIDL_CudaMemRecord *rec = &manager->memRecs[i];
        
        if (!rec->isAllocated || rec->h_base == NULL || rec->d_base == NULL)
        {
            continue;
        }
        
        /* Check if pointer is within this GPU memrec */
        if (isPointerInRange(gpuPtr, rec->d_base, rec->size))
        {
            /* Calculate offset and return CPU pointer */
            ptrdiff_t offset = getPointerOffset(gpuPtr, rec->d_base);
            *cpuPtr = (char*)rec->h_base + offset;
            
            if (memRecIdx != NULL)
            {
                *memRecIdx = i;
            }
            
            return IALG_EOK;
        }
    }
    
    /* Pointer not found in any memrec */
    return IALG_EFAIL;
}

int32_t TIDL_cudaMemManagerSync(
    TIDL_CudaMemManager *manager,
    int32_t memRecIdx,
    TIDL_MemSyncDirection direction)
{
    if (manager == NULL || !manager->isInitialized || 
        memRecIdx < 0 || memRecIdx >= manager->numMemRecs)
    {
        return IALG_EFAIL;
    }
    
    TIDL_CudaMemRecord *rec = &manager->memRecs[memRecIdx];
    
    if (!rec->isAllocated || rec->h_base == NULL || rec->d_base == NULL)
    {
        return IALG_EFAIL;
    }
    
    if (direction == TIDL_SYNC_H2D)
    {
        CUDA_CHECK(cudaMemcpy(rec->d_base, rec->h_base, rec->size, cudaMemcpyHostToDevice));
        manager->totalH2DTransfers++;
        manager->totalBytesH2D += rec->size;
        rec->lastSyncDirection = TIDL_SYNC_H2D;
        
        tidl_printf(3, "Synced memrec[%d] H2D: %u bytes\n", memRecIdx, rec->size);
    }
    else if (direction == TIDL_SYNC_D2H)
    {
        CUDA_CHECK(cudaMemcpy(rec->h_base, rec->d_base, rec->size, cudaMemcpyDeviceToHost));
        manager->totalD2HTransfers++;
        manager->totalBytesD2H += rec->size;
        rec->lastSyncDirection = TIDL_SYNC_D2H;
        
        tidl_printf(3, "Synced memrec[%d] D2H: %u bytes\n", memRecIdx, rec->size);
    }
    
    return IALG_EOK;
}

int32_t TIDL_cudaMemManagerSyncBuffer(
    TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    uint32_t size,
    TIDL_MemSyncDirection direction)
{
    if (manager == NULL || !manager->isInitialized || cpuPtr == NULL || size == 0)
    {
        return IALG_EFAIL;
    }
    
    /* Find which memrec contains this pointer */
    int32_t memRecIdx = -1;
    void *gpuPtr = NULL;
    
    if (TIDL_cudaTranslatePtrCPUtoGPU(manager, cpuPtr, &gpuPtr, &memRecIdx) != IALG_EOK)
    {
        tidl_printf(2, "TIDL_cudaMemManagerSyncBuffer: CPU pointer not found in any memrec\n");
        return IALG_EFAIL;
    }
    
    /* Perform the sync */
    if (direction == TIDL_SYNC_H2D)
    {
        CUDA_CHECK(cudaMemcpy(gpuPtr, cpuPtr, size, cudaMemcpyHostToDevice));
        manager->totalH2DTransfers++;
        manager->totalBytesH2D += size;
        
        tidl_printf(3, "Synced buffer H2D: %u bytes (memrec[%d])\n", size, memRecIdx);
    }
    else if (direction == TIDL_SYNC_D2H)
    {
        CUDA_CHECK(cudaMemcpy((void*)cpuPtr, gpuPtr, size, cudaMemcpyDeviceToHost));
        manager->totalD2HTransfers++;
        manager->totalBytesD2H += size;
        
        tidl_printf(3, "Synced buffer D2H: %u bytes (memrec[%d])\n", size, memRecIdx);
    }
    
    return IALG_EOK;
}

void TIDL_cudaMemManagerSetLayerGpuSupport(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    int32_t isGpuSupported)
{
    if (manager == NULL || !manager->isInitialized || 
        layerIdx < 0 || layerIdx >= manager->numLayers ||
        manager->layerGpuSupport == NULL)
    {
        return;
    }
    
    manager->layerGpuSupport[layerIdx] = isGpuSupported;
}

int32_t TIDL_cudaMemManagerPreLayerSync(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    void *inPtrs[],
    int32_t numInBufs,
    const uint32_t inDataSizes[])
{
    if (manager == NULL || !manager->isInitialized || 
        layerIdx < 0 || layerIdx >= manager->numLayers ||
        manager->layerGpuSupport == NULL)
    {
        return IALG_EFAIL;
    }
    
    manager->currentLayer = layerIdx;
    
    /* Check if current layer is GPU-supported */
    int32_t currentLayerIsGpu = manager->layerGpuSupport[layerIdx];
    
    /* If current layer runs on GPU, check if any input needs to be synced from CPU */
    if (currentLayerIsGpu)
    {
        TIDL_LayerDependency *layerDep = &manager->layerDependencies[layerIdx];
        
        for (int32_t inBufIdx = 0; inBufIdx < numInBufs; inBufIdx++)
        {
            int32_t sourceLayerId = layerDep->inputBufferToLayerMap[inBufIdx];            
            if (!manager->layerGpuSupport[sourceLayerId])
            {
                /* Source layer ran on CPU, need to sync this input buffer */
                if (inPtrs[inBufIdx] != NULL && inDataSizes[inBufIdx] > 0)
                {
                    tidl_printf(2, "Layer %d: Input buffer %d from CPU layer %d, syncing\n", layerIdx, inBufIdx, sourceLayerId);
                    TIDL_cudaMemManagerSyncBuffer(manager, inPtrs[inBufIdx], inDataSizes[inBufIdx], TIDL_SYNC_H2D);
                }
            }
        }
    }
    
    return IALG_EOK;
}

int32_t TIDL_cudaMemManagerPostLayerSync(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    void *outPtrs[],
    int32_t numOutBufs,
    const uint32_t outDataSizes[])
{
    if (manager == NULL || !manager->isInitialized || 
        layerIdx < 0 || layerIdx >= manager->numLayers ||
        manager->layerGpuSupport == NULL)
    {
        return IALG_EFAIL;
    }
    
    /* Check if current layer is GPU-supported */
    int32_t currentLayerIsGpu = manager->layerGpuSupport[layerIdx];
    
    /* If current layer ran on GPU, check if any output needs to be synced back to CPU */
    if (currentLayerIsGpu) 
    {
        TIDL_LayerDependency *layerDep = &manager->layerDependencies[layerIdx];
        
        /* Check if any consumer layer runs on CPU */
        int needsSync = 0;
        
        for (int32_t i = 0; i < layerDep->numOutputLayers; i++) 
        {
            int32_t consumerLayerId = layerDep->outputLayerIds[i];
            if (!manager->layerGpuSupport[consumerLayerId]) 
            {
                /* This consumer runs on CPU, need to sync */
                needsSync = 1;
                break;
            }
        }
        
        /* Also sync if this is the last layer */
        if (layerIdx == manager->numLayers - 1) 
        {
            needsSync = 1;
        }
        
        /* If sync is needed, sync all output buffers */
        if (needsSync) 
        {
            tidl_printf(2, "Layer %d: GPU layer with CPU consumers, syncing %d outputs\n", layerIdx, numOutBufs);
            for (int32_t i = 0; i < numOutBufs; i++) 
            {
                if (outPtrs[i] != NULL && outDataSizes[i] > 0) 
                {
                    TIDL_cudaMemManagerSyncBuffer(manager, outPtrs[i], outDataSizes[i], TIDL_SYNC_D2H);
                }
            }
        }
    }
    
    return IALG_EOK;
}

void* TIDL_cudaMemManagerGetDevicePtr(
    const TIDL_CudaMemManager *manager,
    int32_t memRecIdx)
{
    if (manager == NULL || !manager->isInitialized || 
        memRecIdx < 0 || memRecIdx >= manager->numMemRecs)
    {
        return NULL;
    }
    
    return manager->memRecs[memRecIdx].d_base;
}

void TIDL_cudaMemManagerPrintStats(const TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        return;
    }
    
    tidl_printf(0, "\n========== CUDA Memory Manager Statistics ==========\n");
    tidl_printf(0, "Total H2D transfers: %llu (%.2f MB)\n", 
               (unsigned long long)manager->totalH2DTransfers,
               manager->totalBytesH2D / (1024.0 * 1024.0));
    tidl_printf(0, "Total D2H transfers: %llu (%.2f MB)\n", 
               (unsigned long long)manager->totalD2HTransfers,
               manager->totalBytesD2H / (1024.0 * 1024.0));
    tidl_printf(0, "Total data transferred: %.2f MB\n",
               (manager->totalBytesH2D + manager->totalBytesD2H) / (1024.0 * 1024.0));
    
    /* Print memrec status */
    tidl_printf(0, "\nMemory Records:\n");
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        const TIDL_CudaMemRecord *rec = &manager->memRecs[i];
        if (rec->isAllocated)
        {
            tidl_printf(0, "  [%d] CPU: %p, GPU: %p, Size: %u bytes, Attrs: %s\n",
                       i, rec->h_base, rec->d_base, rec->size,
                       (rec->attrs == IALG_PERSIST) ? "PERSIST" : "SCRATCH");
        }
    }
    
    /* Print GPU layer support */
    if (manager->layerGpuSupport != NULL && manager->numLayers > 0)
    {
        int32_t gpuLayerCount = 0;
        for (int32_t i = 0; i < manager->numLayers; i++)
        {
            if (manager->layerGpuSupport[i])
            {
                gpuLayerCount++;
            }
        }
        tidl_printf(0, "\nGPU-supported layers: %d / %d\n", gpuLayerCount, manager->numLayers);
    }
    
    /* Print layer dependencies */
    if (manager->layerDependencies != NULL && manager->numLayers > 0)
    {
        tidl_printf(0, "\nLayer Dependencies:\n");
        for (int32_t i = 0; i < manager->numLayers; i++)
        {
            const TIDL_LayerDependency *dep = &manager->layerDependencies[i];
            
            tidl_printf(0, "  Layer %d:", i);
            
            /* Print input dependencies */
            tidl_printf(0, "    Inputs from: ");
            if (dep->numInputLayers == 0)
            {
                tidl_printf(0, "None (input layer)");
            }
            else
            {
                for (int32_t j = 0; j < dep->numInputLayers; j++)
                {
                    tidl_printf(0, "%d ", dep->inputLayerIds[j]);
                }
            }
            
            /* Print output dependencies */
            tidl_printf(0, "    Outputs to: ");
            if (dep->numOutputLayers == 0)
            {
                tidl_printf(0, "None (output layer)");
            }
            else
            {
                for (int32_t j = 0; j < dep->numOutputLayers; j++)
                {
                    tidl_printf(0, "%d ", dep->outputLayerIds[j]);
                }
            }
            tidl_printf(0, "\n");
        }
    }
    
    tidl_printf(0, "====================================================\n\n");
}

void TIDL_cudaMemManagerResetStats(TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        return;
    }
    
    manager->totalH2DTransfers = 0;
    manager->totalD2HTransfers = 0;
    manager->totalBytesH2D = 0;
    manager->totalBytesD2H = 0;
}

int32_t TIDL_cudaMemManagerIsPointerInMemRec(
    const TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    int32_t memRecIdx)
{
    if (manager == NULL || !manager->isInitialized || 
        memRecIdx < 0 || memRecIdx >= manager->numMemRecs ||
        cpuPtr == NULL)
    {
        return 0;
    }
    
    const TIDL_CudaMemRecord *rec = &manager->memRecs[memRecIdx];
    
    if (!rec->isAllocated || rec->h_base == NULL)
    {
        return 0;
    }
    
    return isPointerInRange(cpuPtr, rec->h_base, rec->size);
}

#endif /* BUILD_WITH_CUDA */
