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
 @file    tidl_cuda_mem_manager.h
 @brief   CUDA Memory Manager - Parallel GPU memory management for TIDL memrecords
 @version 1.0 (Nov 2025) : Initial version
 ----------------------------------------------------------------------------
*/

#ifndef TIDL_CUDA_MEM_MANAGER_H
#define TIDL_CUDA_MEM_MANAGER_H

#ifdef BUILD_WITH_CUDA

#include "itidl_ti.h"
#include <stdint.h>

/* Maximum number of dependencies per layer */
#define TIDL_MAX_LAYER_DEPENDENCIES 32

/**
 * @struct TIDL_CudaMemRecord
 * @brief GPU mirror of a single IALG_MemRec
 */
typedef struct {
    void*    d_base;           /**< Device (GPU) base pointer */
    void*    h_base;           /**< Host (CPU) base pointer (reference) */
    uint32_t size;             /**< Size in bytes */
    uint32_t space;            /**< Memory space (L1/L2/L3/DDR) */
    uint32_t attrs;            /**< Attributes (PERSIST/SCRATCH) */
    uint32_t alignment;        /**< Alignment requirement */
    int32_t  isAllocated;      /**< Flag indicating if GPU memory is allocated */
    int32_t  needsSync;        /**< Flag indicating if sync is needed */
    int32_t  lastSyncDirection;/**< 0=none, 1=H2D, 2=D2H */
} TIDL_CudaMemRecord;

typedef struct {
    void* d_base;              /**< Device (GPU) base pointer */
    void* h_base;              /**< Host (CPU) base pointer (reference) */
    uint32_t size;             /**< Size in bytes */
}TIDL_CudaMemBuffer;


/*Temporary duplicate - alg_int.h has this defined & it must be in sync with it:*/
#define NUM_MEMRECS_TIDL 16U
#define NUM_BUFFERS_TIDL 256U

/**
 * @struct TIDL_LayerDependency
 * @brief Structure to track layer dependencies
 */
typedef struct {
    int32_t numInputLayers;                                /**< Number of input layers */
    int32_t inputLayerIds[TIDL_MAX_LAYER_DEPENDENCIES];    /**< IDs of input layers */
    int32_t inputBufferToLayerMap[TIDL_NUM_IN_BUFS];       /**< Buffer to source layer mapping */
    int32_t numOutputLayers;                               /**< Number of output layers */
    int32_t outputLayerIds[TIDL_MAX_LAYER_DEPENDENCIES];   /**< IDs of output layers */
} TIDL_LayerDependency;

/**
 * @struct TIDL_CudaMemManager
 * @brief Complete CUDA memory manager parallel to TIDL memrecords
 */
typedef struct {
    TIDL_CudaMemRecord memRecs[NUM_MEMRECS_TIDL];  /**< GPU mirrors of all memrecs */
    TIDL_CudaMemBuffer *memBufs;                   /**< GPU buffers for data not in memrecs */
    int32_t            numMemBufs;                 /**< Number of memory buffers */
    int32_t            numMemRecs;                 /**< Number of memrecs */
    int32_t            isInitialized;              /**< Initialization flag */
    
    /* Performance tracking */
    uint64_t           totalH2DTransfers;          /**< Total Host->Device transfers */
    uint64_t           totalD2HTransfers;          /**< Total Device->Host transfers */
    uint64_t           totalBytesH2D;              /**< Total bytes H2D */
    uint64_t           totalBytesD2H;              /**< Total bytes D2H */
    
    /* Layer execution tracking */
    int32_t            currentLayer;               /**< Current executing layer */
    int32_t            *layerGpuSupport;           /**< Array indicating GPU support per layer */
    int32_t            numLayers;                  /**< Total number of layers */
    
    /* Layer dependency tracking */
    TIDL_LayerDependency *layerDependencies;       /**< Array of layer dependencies */

    int32_t              streamsInitialized;       /**<Flag to check if cuda stream is initialized or not */

    /* Per-process CUDA stream */
    void*                processStream;          /**< Single non-blocking stream for all layers */

    /* Per-layer CUDA streams */
    void**               layerStreams;            /**< Array of per-layer CUDA streams */

} TIDL_CudaMemManager;

/* Thread-local CUDA context — replaces the old process-global g_cudaMemManager
 * and CUDNNLC for thread safety. Each inference thread sets its own context
 * before executing layers. Access via getter/setter functions below.
 */

/**
 * @brief Set the per-thread CUDA memory manager (call before layer execution)
 * @param manager Pointer to this thread's TIDL_CudaMemManager instance
 */
void TIDL_cudaSetThreadManager(TIDL_CudaMemManager *manager);

/**
 * @brief Get the per-thread CUDA memory manager
 * @return Pointer to this thread's TIDL_CudaMemManager, or NULL if not set
 */
TIDL_CudaMemManager* TIDL_cudaGetThreadManager(void);

/**
 * @brief Set the per-thread current layer index (replaces global CUDNNLC)
 * @param layerIdx Current layer index for this thread
 */
void TIDL_cudaSetThreadLayerIdx(int32_t layerIdx);

/**
 * @brief Get the per-thread current layer index
 * @return Current layer index for this thread
 */
int32_t TIDL_cudaGetThreadLayerIdx(void);

/**
 * @enum TIDL_MemSyncDirection
 * @brief Memory synchronization direction
 */
typedef enum {
    TIDL_SYNC_NONE = 0,
    TIDL_SYNC_H2D  = 1,  /**< Host to Device */
    TIDL_SYNC_D2H  = 2   /**< Device to Host */
} TIDL_MemSyncDirection;

/* ============================================================================
 * Logging Functions
 * ============================================================================
 */

/**
 * @brief Set the CUDA logging level
 * @param level Log level (0=ERROR, 1=INFO, 2=DEBUG, 3=TRACE)
 */
void TIDL_cudaSetLogLevel(int32_t level);

/**
 * @brief Get the current CUDA logging level
 * @return Current log level
 */
int32_t TIDL_cudaGetLogLevel(void);

/* ============================================================================
 * Function Declarations
 * ============================================================================
 */

/**
 * @brief Initialize CUDA memory manager
 * @param manager Pointer to CUDA memory manager
 * @param memRec Array of CPU memrecords
 * @param numMemRecs Number of memrecords
 * @param numLayers Total number of layers in network
 * @param TIDLLayers Array of TIDL layer structures
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerInit(
    TIDL_CudaMemManager *manager,
    const IALG_MemRec memRec[],
    int32_t numMemRecs,
    int32_t numLayers,
    sTIDL_Layer_t* TIDLLayers
);

/**
 * @brief Allocate GPU memory for all memrecords
 * @param manager Pointer to CUDA memory manager
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerAllocate(TIDL_CudaMemManager *manager);


/**
 * @brief Allocate memory on GPU
 * @param manager Pointer to CUDA memory manager
 * @param cpuPtr CPU pointer
 * @param gpuPtr Output GPU pointer
 * @param size memory size in bytes
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaAllocBuffer(
    TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    void** gpuPtr,
    uint32_t size
);

/**
 * @brief Free all GPU memory
 * @param manager Pointer to CUDA memory manager
 */
void TIDL_cudaMemManagerFree(TIDL_CudaMemManager *manager);

/**
 * @brief Copy persistent data from CPU to GPU (initial setup)
 * @param manager Pointer to CUDA memory manager
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerCopyPersistentH2D(TIDL_CudaMemManager *manager);

/**
 * @brief Translate CPU pointer to GPU pointer
 * @param manager Pointer to CUDA memory manager
 * @param cpuPtr CPU pointer
 * @param gpuPtr Output GPU pointer
 * @param size size in bytes of applied cudaMemcpy/cudaMemset
 * @return IALG_EOK if found, IALG_EFAIL if not found
 */
int32_t TIDL_cudaTranslatePtrCPUtoGPU(
    const TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    void **gpuPtr,
    int32_t size
);

/**
 * @brief Translate GPU pointer to CPU pointer
 * @param manager Pointer to CUDA memory manager
 * @param gpuPtr GPU pointer
 * @param cpuPtr Output CPU pointer
 * @param memRecIdx Output memrec index (optional, can be NULL)
 * @return IALG_EOK if found, IALG_EFAIL if not found
 */
int32_t TIDL_cudaTranslatePtrGPUtoCPU(
    const TIDL_CudaMemManager *manager,
    const void *gpuPtr,
    void **cpuPtr,
    int32_t *memRecIdx
);

/**
 * @brief Copy data from device to device corresponding to host source and destination pointers
 * @param dstPtr CPU destination pointer
 * @param srcPtr CPU source pointer
 * @param size size of data transfer in bytes
 * @return IALG_EOK on success, IALG_EFAIL otherwise
 */
int32_t TIDL_cudaForceSync(
    TIDL_CudaMemManager *manager,
    void* dstPtr,
    void* srcPtr,
    size_t size
);

/**
 * @brief Synchronize specific memrec between CPU and GPU
 * @param manager Pointer to CUDA memory manager
 * @param memRecIdx Index of memrec to sync
 * @param direction Sync direction (H2D or D2H)
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerSync(
    TIDL_CudaMemManager *manager,
    int32_t memRecIdx,
    TIDL_MemSyncDirection direction
);

/**
 * @brief Synchronize a specific buffer within a memrec
 * @param manager Pointer to CUDA memory manager
 * @param cpuPtr CPU pointer to buffer
 * @param size Size of buffer to sync
 * @param direction Sync direction
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerSyncBuffer(
    TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    uint32_t size,
    TIDL_MemSyncDirection direction
);

/**
 * @brief Mark layer GPU support
 * @param manager Pointer to CUDA memory manager
 * @param layerIdx Layer index
 * @param isGpuSupported 1 if GPU supported, 0 otherwise
 */
void TIDL_cudaMemManagerSetLayerGpuSupport(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    int32_t isGpuSupported
);

/**
 * @brief Build layer dependency graph
 * @param manager Pointer to CUDA memory manager
 * @param TIDLLayers Array of TIDL layer structures
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaBuildLayerDependencyGraph(
    TIDL_CudaMemManager *manager,
    sTIDL_Layer_t* TIDLLayers
);

/**
 * @brief Pre-layer synchronization (handles input transfers)
 * @param manager Pointer to CUDA memory manager
 * @param layerIdx Current layer index
 * @param inPtrs Array of input pointers
 * @param numInBufs Number of input buffers
 * @param inDataSizes Array of input data sizes
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerPreLayerSync(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    void *inPtrs[],
    int32_t bufferIdsToSynchronize[],
    int32_t numInBufs,
    const uint32_t inDataSizes[]
);

/**
 * @brief Post-layer synchronization (handles output transfers)
 * @param manager Pointer to CUDA memory manager
 * @param layerIdx Current layer index
 * @param outPtrs Array of output pointers
 * @param numOutBufs Number of output buffers
 * @param outDataSizes Array of output data sizes
 * @return IALG_EOK on success, IALG_EFAIL on failure
 */
int32_t TIDL_cudaMemManagerPostLayerSync(
    TIDL_CudaMemManager *manager,
    int32_t layerIdx,
    void *outPtrs[],
    int32_t numOutBufs,
    const uint32_t outDataSizes[]
);

/**
 * @brief Get GPU pointer for a specific memrec
 * @param manager Pointer to CUDA memory manager
 * @param memRecIdx Memrec index
 * @return GPU pointer or NULL if invalid
 */
void* TIDL_cudaMemManagerGetDevicePtr(
    const TIDL_CudaMemManager *manager,
    int32_t memRecIdx
);

/**
 * @brief Print memory manager statistics
 * @param manager Pointer to CUDA memory manager
 */
void TIDL_cudaMemManagerPrintStats(const TIDL_CudaMemManager *manager);

/**
 * @brief Reset transfer statistics
 * @param manager Pointer to CUDA memory manager
 */
void TIDL_cudaMemManagerResetStats(TIDL_CudaMemManager *manager);

/**
 * @brief Check if a CPU pointer belongs to a specific memrec
 * @param manager Pointer to CUDA memory manager
 * @param cpuPtr CPU pointer to check
 * @param memRecIdx Memrec index to check against
 * @return 1 if pointer belongs to memrec, 0 otherwise
 */
int32_t TIDL_cudaMemManagerIsPointerInMemRec(
    const TIDL_CudaMemManager *manager,
    const void *cpuPtr,
    int32_t memRecIdx
);

/**
 * @brief Get the CUDA stream for a specific layer
 * @param manager Pointer to CUDA memory manager
 * @param layerIdx Layer index
 * @return CUDA stream for the layer, or NULL if not available
 */
void* TIDL_cudaGetLayerStream(TIDL_CudaMemManager* manager, int32_t layerIdx);

void* TIDL_cudaGetProcessStream(TIDL_CudaMemManager* manager);


#endif /* BUILD_WITH_CUDA */

#endif /* TIDL_CUDA_MEM_MANAGER_H */
