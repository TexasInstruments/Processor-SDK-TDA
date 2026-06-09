//GPU_UTILITIES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <cuda_runtime.h>
#include <assert.h>
#include "tidl_cuda.h"
#include <cuda.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include<thrust/extrema.h>
#include <iostream>
#include <cuda/std/limits>


#define THREADS_PER_BLOCK (256)
#define MEM_BUFF_ARRAY_LEN (512)

static inline int checkCudaError(cudaError_t code, const char* expr, const char* file, int line)
{
  if(code)
  {
    printf("CUDA error at %s:%d, code=%d (%s) in '%s'", file, line, (int) code, cudaGetErrorString(code), expr);
    return 1;
  }
  return 0;
}

#define checkCudaErr(...)       do { int err = checkCudaError(__VA_ARGS__, #__VA_ARGS__, __FILE__, __LINE__); if (err) return err; } while (0)

template <class Tout>
__device__ inline Tout cuda_sat(float val)
{
    float max_val = (float)cuda::std::numeric_limits<Tout>::max();
    float min_val = (float)cuda::std::numeric_limits<Tout>::lowest();
    float temp_val = (val < min_val) ? min_val : val;
    float out = (temp_val > max_val) ? max_val : temp_val;
    return (Tout)out;
}

// CUDA device function for fixed-point rounding and saturation
__device__ inline int64_t cuda_roundSat(int64_t val, int32_t bits, int32_t min_val, int32_t max_val)
{
    int64_t temp_val = val;
    if (bits > 0) {
        int64_t round_val = 1LL << (bits - 1);
        temp_val += round_val;
        temp_val >>= bits;
    }
    temp_val = (temp_val <= min_val) ? min_val : temp_val;
    temp_val = (temp_val >= max_val) ? max_val : temp_val;
    return temp_val;
}

// min max values passed based on activation
__device__ inline float cuda_floatSat(float val, float max, float min)
{
    float temp_val = (val > max) ? max : val;
    temp_val = (temp_val < min) ? min : val;
    return temp_val;
}

/**
----------------------------------------------------------------------------
@fn         minmaxGPU
@brief      Function finds the minimum and maximum value in the given device
stream

@param      N : Size of the device stream
@param      x : Device pointer to the stream
@param      min : Pointer to the variable where the minimum value will be stored
@param      max : Pointer to the variable where the maximum value will be stored

@remarks    None
----------------------------------------------------------------------------
*/
template <class Tx>
void minmaxGPU(const int N,const Tx* x,int *min,int *max)
{
thrust::device_ptr<const Tx>pWrapper(x);
thrust::pair<const thrust::device_ptr<const Tx>, const thrust::device_ptr<const Tx> > result = thrust::minmax_element(pWrapper, pWrapper + N);
*min=(int)(*result.first);
*max=(int)(*result.second);
}

//Fully templated version of the above code:
template <class Tx>
void minmax_Thrust(const int N,const Tx* x,Tx *min,Tx *max)
{
thrust::device_ptr<const Tx>pWrapper(x);
thrust::pair<const thrust::device_ptr<const Tx>, const thrust::device_ptr<const Tx> > result = thrust::minmax_element(pWrapper, pWrapper + N);
*min=(Tx)(*result.first);
*max=(Tx)(*result.second);
}

//Instantiations:
template void minmaxGPU<int>(const int, const int*, int*, int*);
template void minmaxGPU<float>(const int, const float*, int*, int*);
template void minmax_Thrust<int>(int, int const*, int*, int*);
template void minmax_Thrust<float>(int, float const*, float*, float*);
template void minmax_Thrust<long>(int, long const*, long*, long*);
