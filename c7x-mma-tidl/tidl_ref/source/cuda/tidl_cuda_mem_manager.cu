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
#ifndef CUDA_API_PER_THREAD_DEFAULT_STREAM
#  define CUDA_API_PER_THREAD_DEFAULT_STREAM 1
#endif

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include <stdio.h>
#include <stdarg.h>
#include "tidl_cuda_mem_manager.h"
#include "tidl_cuda.h"


/* NOTE: g_cudaMemManager global has been removed for thread safety.
 * TIDL_CudaMemManager* is now passed through function parameters.\n * Each algHandle stores its own manager in algHandle->cudaMemManager.
 */

/* Thread-local context: each thread gets its own manager pointer and layer index.
 * Set by TIDL_cudaSetThreadManager/TIDL_cudaSetThreadLayerIdx before layer execution.
 */
static __thread TIDL_CudaMemManager* tls_cudaMemManager = NULL;
static __thread int32_t tls_cudaLayerIdx = -1;

void TIDL_cudaSetThreadManager(TIDL_CudaMemManager *manager)
{
    tls_cudaMemManager = manager;
}

TIDL_CudaMemManager* TIDL_cudaGetThreadManager(void)
{
    return tls_cudaMemManager;
}

void TIDL_cudaSetThreadLayerIdx(int32_t layerIdx)
{
    tls_cudaLayerIdx = layerIdx;
}

int32_t TIDL_cudaGetThreadLayerIdx(void)
{
    return tls_cudaLayerIdx;
}

/* Layer IDs currently span 0..TIDL_UnsupportedLayer, so this must cover TIDL_UnsupportedLayer entries. */
#define NUM_TIDL_LAYER_TYPES (TIDL_UnsupportedLayer + 1)

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
    0,  /* TIDL_LogicalOpLayer(47)*/
    0,  /* TIDL_RMSNormalizationLayer (48) */
    0,  /* TIDL_LSTMLayer (49) */
    0,  /* TIDL_GRULayer (50) */
    0,  /* TIDL_RNNLayer (51) */
    0,  /* TIDL_GatherNDLayer (52) */
    0,  /* TIDL_CastLayer (53) */
    0,  /* TIDL_GatherElementsLayer(54)*/
    0,  /* TIDL_ShapeLayer (55) */
    0,  /* TIDL_SizeLayer (56) */
    0,  /* TIDL_AttentionLayer(57)*/
    0,  /* TIDL_NonZeroLayer (58)*/
    0,  /* TIDL_UnsupportedLayer (58) */
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

#define MAX_MEMORY_DUMP_SIZE 64

/* Global log level - can be set externally or via environment variable */
static int32_t g_cuda_log_level = TIDL_LOG_LEVEL_ERROR;

void memory_dump(const void* ptr, int bytes)
{
    for(size_t i = 0; i < bytes; i++)
    {
        if(i % 16 == 0) printf("%04zx: ", i);
        printf("%02x ", *((const unsigned char*)ptr + i));
        if(((i+1) % 16 == 0) || (i + 1 == bytes)) printf("\n");
    }
}

void compare_memory(int logLevel, const void* cpuPtr, const void* gpuPtr, int size)
{
    if(logLevel <= g_cuda_log_level)
    {
        int bytes = min(size, MAX_MEMORY_DUMP_SIZE);
        unsigned char* hostBuf = (unsigned char*)malloc(bytes);
        cudaMemcpy(hostBuf, gpuPtr, bytes, cudaMemcpyDeviceToHost);
        printf("CPU (%p): \n", cpuPtr);
        memory_dump(cpuPtr, bytes);
        printf("GPU (%p): \n", gpuPtr);
        memory_dump(hostBuf, bytes);
        free(hostBuf);
    }
}

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

#if 0
static int32_t isPointerOverlapping(const void *ptr, uint32_t size, const void *base, uint32_t bufferSize)
{
    uintptr_t ptrAddr = (uintptr_t)ptr;
    uintptr_t baseAddr = (uintptr_t)base;
    return ((ptrAddr + size) <= baseAddr || (baseAddr + bufferSize) <= ptrAddr) ? 0 : 1;
}
#endif

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

    /* Find max dataId to size the map correctly (dataId values may exceed numLayers) */
    int32_t maxDataId = 0;
    for(int32_t layerId = 0; layerId < manager->numLayers; layerId++)
    {
        int32_t outId = TIDLLayers[layerId].outData.dataId;
        if(outId > maxDataId) maxDataId = outId;
        for(int32_t b = 0; b < TIDLLayers[layerId].numInBufs; b++)
        {
            int32_t inId = TIDLLayers[layerId].inData[b];
            if(inId > maxDataId) maxDataId = inId;
        }
    }
    int32_t mapSize = maxDataId + 1;

    int32_t *dataIdToLayerMap = (int32_t*)malloc(mapSize * sizeof(int32_t));
    if(dataIdToLayerMap == NULL)
    {
        return IALG_EFAIL;
    }
    memset(dataIdToLayerMap, -1, sizeof(int32_t) * mapSize);

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
            tidl_printf(3, "Layer %d: Input buffer %d comes from layer %d\n", layerId, inBufIdx, prevLayerId);
            int duplicate = 0;
            for(int j = 0; j < layerDep->numInputLayers; j++){
                if(layerDep->inputLayerIds[j] == prevLayerId) {
                    duplicate = 1;
                    break;
                }
            }
            if(duplicate) continue;

            layerDep->inputLayerIds[layerDep->numInputLayers++] = prevLayerId;
            tidl_printf(3, "Layer %d depends on input from layer %d\n", layerId, prevLayerId);
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
            tidl_printf(3, "Layer %d outputs to layer %d\n", prevLayerId, layerId);
        }
    }
    free(dataIdToLayerMap);
    return IALG_EOK;
}

int32_t TIDL_cudaMemManagerInit(
    TIDL_CudaMemManager *manager,
    const IALG_MemRec memRec[],
    int32_t numMemRecs,
    int32_t numLayers,
    sTIDL_Layer_t* TIDLLayers)
{
    cudaFree(0);
    if (manager == NULL || memRec == NULL || numMemRecs <= 0 || numMemRecs > NUM_MEMRECS_TIDL)
    {
        tidl_printf(0, "TIDL_cudaMemManagerInit: Invalid parameters\n");
        return IALG_EFAIL;
    }

    /* Clear manager structure */
    memset(manager, 0, sizeof(TIDL_CudaMemManager));

    manager->numMemRecs = numMemRecs;
    manager->numLayers = numLayers;
    manager->numMemBufs = 0;
    
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

        /* Allocate per-layer CUDA streams */
        manager->layerStreams = (void**)malloc(numLayers * sizeof(void*));
        if (manager->layerStreams == NULL)
        {
            tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to allocate layerStreams array\n");
            free(manager->layerDependencies);
            free(manager->layerGpuSupport);
            manager->layerDependencies = NULL;
            manager->layerGpuSupport = NULL;
            return IALG_EFAIL;
        }

        /* Initialize all layer stream pointers to NULL */
        memset(manager->layerStreams, 0, numLayers * sizeof(void*));

        /* Create one CUDA stream per layer */
        for (int32_t i = 0; i < numLayers; i++)
        {
            cudaStream_t stream = NULL;
            if (cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking) != cudaSuccess)
            {
                tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to create stream for layer %d\n", i);
                /* Clean up successfully-created streams */
                for (int32_t j = 0; j < i; j++)
                {
                    cudaStream_t cleanupStream = (cudaStream_t)manager->layerStreams[j];
                    if (cleanupStream != NULL)
                    {
                        cudaStreamDestroy(cleanupStream);
                        manager->layerStreams[j] = NULL;
                    }
                }
                free(manager->layerStreams);
                free(manager->layerDependencies);
                free(manager->layerGpuSupport);
                manager->layerStreams = NULL;
                manager->layerDependencies = NULL;
                manager->layerGpuSupport = NULL;
                return IALG_EFAIL;
            }
            manager->layerStreams[i] = (void*)stream;
        }
        tidl_printf(1, "Created %d per-layer CUDA streams\n", numLayers);

        manager->memBufs = (TIDL_CudaMemBuffer*)malloc(NUM_BUFFERS_TIDL * sizeof(TIDL_CudaMemBuffer));
        if(manager->memBufs == NULL)
        {
            tidl_printf(0, "TIDL_cudaMemManagerInit: Failed to allocate gpu memory buffers\n");
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
    cudaStream_t streamMain = (cudaStream_t)TIDL_cudaGetProcessStream(manager);
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {
        TIDL_CudaMemRecord *rec = &manager->memRecs[i];

        if (rec->size == 0 || rec->h_base == NULL)
        {
            tidl_printf(2, "Skipping memrec %d (size=%u, base=%p)\n", i, rec->size, rec->h_base);
            continue;
        }

        /* Allocate GPU memory */
        CUDA_CHECK(cudaMallocAsync(&rec->d_base, rec->size, streamMain));
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
    
    manager->isInitialized = 0;
    cudaStream_t streamMain = (cudaStream_t)TIDL_cudaGetProcessStream(manager);
    
    for (int32_t i = 0; i < manager->numMemRecs; i++)
    {

        if (manager->memRecs[i].isAllocated && manager->memRecs[i].d_base != NULL)
        {
            CUDA_CHECK_VOID(cudaFreeAsync(manager->memRecs[i].d_base,streamMain));
            manager->memRecs[i].d_base = NULL;
            manager->memRecs[i].isAllocated = 0;
        }
    }

    for (int32_t i = 0; i < manager->numMemBufs; i++)
    {
        if (manager->memBufs[i].d_base != NULL)
        {
            CUDA_CHECK_VOID(cudaFreeAsync(manager->memBufs[i].d_base, streamMain));
            manager->memBufs[i].d_base = NULL;
        }
    }

    /* Free memBufs container */
    if (manager->memBufs != NULL)
    {
        free(manager->memBufs);
        manager->memBufs = NULL;
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

    /* Synchronize and destroy per-layer streams */
    if (manager->layerStreams != NULL)
    {
        for (int32_t i = 0; i < manager->numLayers; i++)
        {
            if (manager->layerStreams[i] != NULL)
            {
                cudaStream_t stream = (cudaStream_t)manager->layerStreams[i];
                cudaError_t err = cudaStreamDestroy(stream);
                if (err != cudaSuccess)
                {
                    tidl_printf(1, "Warning: Failed to destroy layer stream %d: %s\n", i, cudaGetErrorString(err));
                }
                manager->layerStreams[i] = NULL;
            }
        }
        free(manager->layerStreams);
        manager->layerStreams = NULL;
        tidl_printf(1, "Destroyed all per-layer CUDA streams\n");
    }

     /* Synchronize and destroy the per-process stream */
    if (manager->processStream != NULL)
    {
        cudaStream_t stream = (cudaStream_t)manager->processStream;
        /* Ensure all async operations complete before destroying stream */
        cudaError_t err = cudaStreamSynchronize(stream);
        if (err != cudaSuccess)
        {
            tidl_printf(0, "Stream sync error: %s\n", cudaGetErrorString(err));
        }
        else
        {
            tidl_printf(1, "Stream synchronized successfully\n");
        }

        err = cudaStreamDestroy(stream);
        if (err != cudaSuccess)
        {
            tidl_printf(0, "Stream destroy error: %s\n", cudaGetErrorString(err));
        }
        manager->processStream = NULL;
        tidl_printf(1, "Destroyed per-process CUDA stream\n");
    }

    /* Ensure device is synchronized before freeing GPU memory */
    tidl_printf(1, "Device synchronizing before GPU memory free...\n");
    cudaError_t err = cudaDeviceSynchronize();
    if (err != cudaSuccess)
    {
        tidl_printf(0, "Device sync error: %s\n", cudaGetErrorString(err));
    }

    tidl_printf(1, "CUDA Memory Manager freed\n");
}

int32_t TIDL_cudaMemManagerCopyPersistentH2D(TIDL_CudaMemManager *manager)
{
    if (manager == NULL || !manager->isInitialized)
    {
        tidl_printf(0, "TIDL_cudaMemManagerCopyPersistentH2D: Manager not initialized\n");
        return IALG_EFAIL;
    }
    
    uint64_t totalCopied = 0;

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetProcessStream(manager);
    
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
            CUDA_CHECK(cudaMemcpyAsync(rec->d_base, rec->h_base, rec->size, cudaMemcpyHostToDevice, stream));
            rec->lastSyncDirection = TIDL_SYNC_H2D;
            totalCopied += rec->size;
            
            tidl_printf(2, "Copied persistent memrec[%d]: %u bytes H2D\n", i, rec->size);
        }
    }
    
    CUDA_CHECK(cudaDeviceSynchronize());
    
    tidl_printf(1, "Copied %.2f MB of persistent data to GPU\n", totalCopied / (1024.0 * 1024.0));
    
    return IALG_EOK;
}

int32_t TIDL_cudaAllocBuffer(
    TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    void** gpuPtr,
    uint32_t size
)
{
    if (manager == NULL || !manager->isInitialized || cpuPtr == NULL)
    {
        return IALG_EFAIL;
    }

    if (manager->numMemBufs >= NUM_BUFFERS_TIDL)
    {
        tidl_printf(0, "TIDL_cudaAllocBuffer: memBufs array full (%d/%d)\n", manager->numMemBufs, NUM_BUFFERS_TIDL);
        return IALG_EFAIL;
    }

    /* Check if an existing buffer already covers this CPU pointer */
    for(int32_t i = 0; i < manager->numMemBufs; i++)
    {
        TIDL_CudaMemBuffer *buf = &manager->memBufs[i];
        if(buf->h_base == cpuPtr && buf->size >= size)
        {
            *gpuPtr = buf->d_base;
            tidl_printf(2, "Reusing existing GPU buffer[%d]: CPU=%p, GPU=%p, size=%u\n", i, cpuPtr, buf->d_base, buf->size);
            return IALG_EOK;
        }
    }

    TIDL_CudaMemBuffer *buf = &manager->memBufs[manager->numMemBufs];
    buf->h_base = (void*)cpuPtr;
    buf->size = size;
    CUDA_CHECK(cudaMalloc(&buf->d_base, buf->size));
    *gpuPtr = buf->d_base;
    manager->numMemBufs++;
    return IALG_EOK;
}

int32_t TIDL_cudaTranslatePtrCPUtoGPU(
    const TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    void **gpuPtr,
    int32_t size)
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

            if(offset + size > rec->size)
            {
                int32_t overflow = offset + size - rec->size;
                tidl_printf(1, "Warning: Pointer translation in layer %d attempts transfer exceeding memrec[%d] bounds by %d bytes\n", TIDL_cudaGetThreadLayerIdx(), i, overflow);
                break;
            }

            *gpuPtr = (char*)rec->d_base + offset;

            tidl_printf(2, "CPU pointer (%p) found in memrec[%d], returning gpu mirror (%p)\n", cpuPtr, i, *gpuPtr);
            
            return IALG_EOK;
        }
    }

    tidl_printf(2, "CPU pointer (%p) not found in any memrec, checking gpu memory buffers\n", cpuPtr);

    /* Search through all membufs */
    for(int32_t i = 0; i < manager->numMemBufs; i++)
    {
        const TIDL_CudaMemBuffer *buf = &manager->memBufs[i];
        if (buf->h_base == NULL || buf->d_base == NULL)
        {
            continue;
        }

        /* Check if pointer is within this membuf */
        if (isPointerInRange(cpuPtr, buf->h_base, buf->size))
        {
            /* Calculate offset and return GPU pointer */
            ptrdiff_t offset = getPointerOffset(cpuPtr, buf->h_base);

            if(offset + size > buf->size)
            {
                int32_t overflow = offset + size - buf->size;
                tidl_printf(1, "Warning: Pointer translation in layer %d attempts transfer exceeding membuf[%d] bounds by %d bytes\n", TIDL_cudaGetThreadLayerIdx(), i, overflow);
                break;
            }

            *gpuPtr = (char*)buf->d_base + offset;
            
            tidl_printf(2, "CPU pointer (%p) found in dynamic memory, returning gpu mirror (%p)\n", cpuPtr, *gpuPtr);
            
            return IALG_EOK;
        }
    }

    /* Pointer not found in any memrec or membuf*/
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

int32_t TIDL_cudaForceSync(
    TIDL_CudaMemManager *manager,
    void* dstPtr,
    void* srcPtr,
    size_t size
)
{
    void* d_dstPtr = NULL;
    void* d_srcPtr = NULL;
    TIDL_cudaTranslatePtrCPUtoGPU(manager, dstPtr, (void**)&d_dstPtr, size);
    TIDL_cudaTranslatePtrCPUtoGPU(manager, srcPtr, (void**)&d_srcPtr, size);

    if(d_srcPtr == NULL) TIDL_cudaAllocBuffer(manager, srcPtr, &d_srcPtr, size);
    CUDA_CHECK(cudaMemcpy(d_srcPtr, srcPtr, size, cudaMemcpyHostToDevice));

    printf("CPU sync: Copied data from (%p) to (%p)\n", srcPtr, dstPtr);
    printf("Matching GPU forced sync: Copied data from (%p) to (%p)\n", d_srcPtr, d_dstPtr);
    CUDA_CHECK(cudaMemcpy(d_dstPtr, d_srcPtr, size, cudaMemcpyDeviceToDevice));
    return IALG_EOK;
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
    
    if (TIDL_cudaTranslatePtrCPUtoGPU(manager, cpuPtr, &gpuPtr, size) != IALG_EOK)
    {
        tidl_printf(2, "TIDL_cudaMemManagerSyncBuffer: CPU pointer (%p) not found in any memrec or membuf\n", cpuPtr);
        return IALG_EFAIL;
    }
    
    /* Perform the sync */
    if (direction == TIDL_SYNC_H2D)
    {
        compare_memory(2, cpuPtr, gpuPtr, size);
        CUDA_CHECK(cudaMemcpy(gpuPtr, cpuPtr, size, cudaMemcpyHostToDevice));
        manager->totalH2DTransfers++;
        manager->totalBytesH2D += size;

        tidl_printf(2, "Copied %d bytes of data from CPU pointer (%p) to GPU pointer (%p)\n", size, cpuPtr, gpuPtr);
        if(memRecIdx != -1)
        {
            tidl_printf(2, "Synced buffer H2D: %u bytes (memrec[%d])\n", size, memRecIdx);
        }
        else
        {
            tidl_printf(2, "Synced buffer H2D: %u bytes (dynamic memory buffer)\n", size);
        }
    }
    else if (direction == TIDL_SYNC_D2H)
    {
        compare_memory(2, cpuPtr, gpuPtr, size);
        CUDA_CHECK(cudaMemcpy((void*)cpuPtr, gpuPtr, size, cudaMemcpyDeviceToHost));
        manager->totalD2HTransfers++;
        manager->totalBytesD2H += size;

        tidl_printf(2, "Copied %d bytes of data from GPU pointer (%p) to CPU pointer (%p)\n", size, gpuPtr, cpuPtr);

        if(memRecIdx != -1)
        {
            tidl_printf(2, "Synced buffer D2H: %u bytes (memrec[%d])\n", size, memRecIdx);
        }
        else
        {
            tidl_printf(2, "Synced buffer D2H: %u bytes (dynamic memory buffer)\n", size);
        }
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
    int32_t bufferIdsToSynchronize[],
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

    tidl_printf(2, "Input CPU pointers to layer %d:\n", layerIdx);
    for(int32_t i = 0; i < numInBufs; i++)
    {
        tidl_printf(2, "%p\n", inPtrs[i]);
    }
    
    /* Check if current layer is GPU-supported */
    int32_t currentLayerIsGpu = manager->layerGpuSupport[layerIdx];
    
    /* If current layer runs on GPU, check if any input needs to be synced from CPU */
    if (currentLayerIsGpu)
    {
        TIDL_LayerDependency *layerDep = &manager->layerDependencies[layerIdx];

        for (int32_t i = 0; i < numInBufs; i++)
        {
            int32_t inBufIdx = (bufferIdsToSynchronize != NULL) ? bufferIdsToSynchronize[i] : i;
            int32_t sourceLayerId = layerDep->inputBufferToLayerMap[inBufIdx];
            if (sourceLayerId < 0 || sourceLayerId >= manager->numLayers) continue;
            if (!manager->layerGpuSupport[sourceLayerId])
            {
                /* Source layer ran on CPU, need to sync this input buffer */
                if (inPtrs[i] != NULL && inDataSizes[i] > 0)
                {
                    tidl_printf(2, "Layer %d: Input buffer %d from CPU layer %d, syncing\n", layerIdx, inBufIdx, sourceLayerId);
                    TIDL_cudaMemManagerSyncBuffer(manager, inPtrs[i], inDataSizes[i], TIDL_SYNC_H2D);
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

    tidl_printf(2, "Output CPU pointers of layer %d:\n", layerIdx);
    for(int32_t i = 0; i < numOutBufs; i++)
    {
        tidl_printf(2, "%p\n", outPtrs[i]);
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

void* TIDL_cudaGetLayerStream(TIDL_CudaMemManager* manager, int32_t layerIdx)
{
    if (manager == NULL || !manager->isInitialized)
    {
        tidl_printf(3, "TIDL_cudaGetLayerStream: mgr null or not init\n");
        return NULL;
    }

    if (layerIdx < 0 || layerIdx >= manager->numLayers)
    {
        tidl_printf(3, "TIDL_cudaGetLayerStream: idx out of range %d vs %d\n", layerIdx, manager->numLayers);
        return NULL;
    }

    if (manager->layerStreams == NULL)
    {
        tidl_printf(3, "TIDL_cudaGetLayerStream: layerStreams is NULL\n");
        return NULL;
    }

    return manager->layerStreams[layerIdx];
}

void* TIDL_cudaGetProcessStream(TIDL_CudaMemManager* manager)
{
    if (manager == NULL || !manager->isInitialized)
        return NULL;

    /* Create stream once per process (lazy initialization) */
    if (manager->processStream == NULL)
    {
        cudaStream_t stream = NULL;
        if (cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking) == cudaSuccess)
        {
            manager->processStream = (void*)stream;
            manager->streamsInitialized = 1;
            tidl_printf(1, "Created per-process CUDA stream: %p\n", stream);
        }
        else
        {
            tidl_printf(0, "Failed to create per-process CUDA stream\n");
            return NULL;
        }
    }

    return manager->processStream;
}


#endif /* BUILD_WITH_CUDA */
