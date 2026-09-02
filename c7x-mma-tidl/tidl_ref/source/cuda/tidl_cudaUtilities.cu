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
#include "itidl_ti.h"


#define THREADS_PER_BLOCK (256)
#define THREADS_PER_BLOCK_X (64)
#define THREADS_PER_BLOCK_Y (8)
#define GRID_SIZE(a,b) ((a+b-1)/b)
#define MEM_BUFF_ARRAY_LEN (512)
#define lutTensorZP (0.0)
#define SCALE_PRECISION_BITS (8)
#define FLOAT_MANTISSA_PRECISION (23U)
#define MAX_NUM_HISTOGRAM_BINS (1024U)

typedef union
{
  float f;
  struct
  {
    uint32_t mantisa : 23;
    uint32_t exponent : 8;
    uint32_t sign : 1;
  } parts;
}float_bits;

#define checkCudaErr(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            printf("CUDA Error at %s:%d - %s\n", __FILE__, __LINE__, \
                       cudaGetErrorString(err)); \
            return IALG_EFAIL; \
        } \
    } while(0)

template <class Tout>
__global__ void TIDL_convFillZeroPoint(Tout *Y, int32_t bufferSize, int32_t padFillValue)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < (bufferSize))
  {
    Y[i] = (Tout)padFillValue;
  }
}

template <class Tout>
__global__ void TIDL_fillZeroPoint(
    Tout* __restrict__ accumulator,
    Tout padFillValue,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int b = (idx / (numDIM1 * numDIM2 * numChannels * inHeight * inWidth)) % numBatches;
    int d1 = (idx / (numDIM2 * numChannels * inHeight * inWidth)) % numDIM1;
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate offsets - exact same as reference
    uint32_t outOffset = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    // Sum operation - always accumulate (no branching needed)
    accumulator[outOffset] = (Tout)padFillValue;
}

template <class Tout>
__device__ inline Tout cuda_sat(float val)
{
    float max_val = (float)cuda::std::numeric_limits<Tout>::max();
    float min_val = (float)cuda::std::numeric_limits<Tout>::lowest();
    float temp_val = __float2int_rn(val);
    temp_val = (temp_val < min_val) ? min_val : temp_val;
    float out = (temp_val > max_val) ? max_val : temp_val;
    return (Tout)out;
}

__device__ inline bfloat16_tidl cuda_sat_bf16(bfloat16_tidl val)
{
    bfloat16_tidl max_val = (float)cuda::std::numeric_limits<bfloat16_tidl>::max();
    bfloat16_tidl min_val = (float)cuda::std::numeric_limits<bfloat16_tidl>::lowest();
    bfloat16_tidl temp_val = val;
    temp_val = (temp_val < min_val) ? min_val : temp_val;
    bfloat16_tidl out = (temp_val > max_val) ? max_val : temp_val;
    return out;
}

__device__ inline int64_t cuda_roundSatMMA(int64_t val, int32_t bits, int32_t min, int32_t max)
{
  int64_t temp;
  int64_t temp_val = val;
  if (bits > 0)
  {
    temp = (val >> (bits - 1)) + (int64_t)1;
    temp_val = temp >> 1;
  }

  temp_val = (temp_val <= min) ? min : temp_val;
  temp_val = (temp_val >= max) ? max : temp_val;
  return temp_val;
}

// CUDA device function for fixed-point rounding and saturation
__device__ inline int64_t cuda_roundSat(int64_t val, int32_t bits, float min_val, float max_val)
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
    temp_val = (temp_val < min) ? min : temp_val;
    return temp_val;
}

__device__ inline bfloat16_tidl cuda_bf16Sat(float val, float actMin, float actMax)
{
    // Clamp activation bounds to BF16 representable range.
    // Use cuda::std::numeric_limits — the std:: specialization is only visible
    // to the host compiler; device code requires the cuda::std:: counterpart.
    float bf16Max = (float)cuda::std::numeric_limits<bfloat16_tidl>::max();
    float bf16Min = (float)cuda::std::numeric_limits<bfloat16_tidl>::lowest();
    float clampMax = (actMax > bf16Max) ? bf16Max : actMax;
    float clampMin = (actMin < bf16Min) ? bf16Min : actMin;
    // Clamp value
    float clamped = (val > clampMax) ? clampMax : val;
    clamped = (clamped < clampMin) ? clampMin : clamped;
    // Downcast using RNE
    return bfloat16_tidl(clamped);
}


__device__ inline void convertFloatToScaleAndShift(float val, int32_t *scale, int32_t *shift, int32_t precisionBits)
{
  float_bits FloatStruct;
  uint32_t mant;

  FloatStruct.f = val;
  mant = FloatStruct.parts.mantisa;

  mant += (1U << FLOAT_MANTISSA_PRECISION);
  mant = (mant) >> (FLOAT_MANTISSA_PRECISION + 1U - (uint32_t)precisionBits);
  *shift = FloatStruct.parts.exponent - 127U - (uint32_t)precisionBits + 1U;
  *scale = (int32_t)mant;
}

__device__ inline int32_t cuda_float_to_int(float x)
{
    int32_t x_int = (int32_t)x;
    int32_t outMax = cuda::std::numeric_limits<int32_t>::max();
    int32_t outMin = cuda::std::numeric_limits<int32_t>::lowest();
    x_int = (x > outMax) ? outMax : x_int;
    x_int = (x < outMin) ? outMin : x_int;


    if((abs(x - float(x_int)) > 0.5f) || ((abs(x - float(x_int)) == 0.5f) && (((uint32_t)x_int & (uint32_t)0x1) == (uint32_t)0x1)))
    {
        if(x > 0.0f)
        {
            x_int += 1;
        }
        else
        {
            x_int -= 1;
        }
    }

    return x_int;
}

__device__ inline float cuda_exp_taylor(float x)
{
    float ln2      = 0.693147180559945f;
    float oneByLn2 = 1.44269504090f;
    float oneBy6   = 0.1666667f;
    float oneBy24  = 0.0416667f;
    float y        = oneByLn2 * x;
    int yI         = (int)y;
    float yf       = y - (float)yI;
    float oneBy65356 = 0.0000152587890625f;

    float floatRes = yf * ln2;

    float floatRes2 = floatRes * floatRes;
    float floatRes3 = floatRes2 * floatRes;
    float floatRes4 = floatRes2 * floatRes2;
    float twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
    twoPwF = twoPwF + (floatRes3 * oneBy6);
    twoPwF = twoPwF + (floatRes4 * oneBy24);

    // __vpred vp = __cmp_gt_pred(yI, 0);
    bool vp = (yI > 0);
    // int32_t tempShiftL = __shift_left((1 << 16), yI);
    int tempShiftL = (1 << 16) << yI;
    // int32_t tempShiftR = __shift_right((1 << 16), -yI);
    int tempShiftR = (1 << 16) >> (-yI);
    // tempShiftL = __select(vp, tempShiftL, tempShiftR);
    tempShiftL = vp ? tempShiftL : tempShiftR;
    float ePwX = twoPwF * (float)(tempShiftL);
    ePwX = ePwX * oneBy65356;
    // vp = __cmp_gt_pred((int32_t)-16, yI);
    vp = (-16 > yI);
    // ePwX = __select(vp, (float32_tidl)0.0f, ePwX);
    ePwX = vp ? 0.0f : ePwX;
    // vp = __cmp_gt_pred(yI, (int32_t)14);
    vp = (yI > 14);
    // ePwX = __select(vp, (float32_tidl)FLT_MAX, ePwX);
    ePwX = vp ? FLT_MAX : ePwX;
    return ePwX;
}

__device__ inline bfloat16_tidl cuda_exp_taylor_bf16(bfloat16_tidl x)
{
    using namespace floating_point::bf16_c7x;

    bfloat16_tidl twoPwF, ePwX;
    bfloat16_tidl ln2      = (bfloat16_tidl)0.693147180559945f;
    bfloat16_tidl oneByLn2 = (bfloat16_tidl)1.44269504090f;
    bfloat16_tidl oneBy6   = (bfloat16_tidl)0.1666667f;
    bfloat16_tidl oneBy24  = (bfloat16_tidl)0.0416667f;
    bfloat16_tidl y        = oneByLn2 * x;
    int32_t yI            = (int32_t)y;
    bfloat16_tidl yf       = y - (bfloat16_tidl)yI;
    bfloat16_tidl oneBy65356 = (bfloat16_tidl)0.0000152587890625f;

    bfloat16_tidl floatRes = yf * ln2;

    /* Precomputing constant products to avoid cumulative quantization error */
    bfloat16_tidl temp1 = (bfloat16_tidl)0.240226507f; /* ln2 * ln2 * 0.5 */
    bfloat16_tidl temp2 = (bfloat16_tidl)0.0555041209f; /* ln2 * ln2 * ln2 * oneBy6 */
    bfloat16_tidl temp3 = (bfloat16_tidl)0.00961813703f; /* ln2 * ln2 * ln2 * ln2 * oneBy24 */

    twoPwF = (bfloat16_tidl)1.0f + floatRes + ((yf * yf) * temp1);
    twoPwF = twoPwF + ((yf * yf * yf) * temp2);
    twoPwF = twoPwF + ((yf * yf * yf * yf) * temp3);

    int32_t tempShiftL = (1 << 16) << yI;
    int32_t tempShiftR = (1 << 16) >> (-yI);

    if(yI > 0)
    {
        tempShiftL = tempShiftL;
    }
    else
    {
        tempShiftL = tempShiftR;
    }

    ePwX = twoPwF * (bfloat16_tidl)(tempShiftL);

    ePwX = ePwX * oneBy65356;

    if((int32_t)-16 > yI)
    {
        ePwX=(bfloat16_tidl)0.0f;
    }

    if(yI > (int32_t)14)
    {
        ePwX = cuda::std::numeric_limits<bfloat16_tidl>::max();
    }

    return ePwX;
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

/*C7x Intrinsic Implementations:*/

enum : uint32_t {
    FC_ROUND_MASK  = 0x3u,
    FC_ROUND_EVEN  = 0x0u,
    FC_ROUND_DOWN  = 0x1u,
    FC_ROUND_UP    = 0x2u,
    FC_ROUND_OFF   = 0x3u,
    FC_FLUSH_ZERO  = 0x4u
};

static const uint32_t pZERO_SP = 0x00000000u;
static const uint32_t nZERO_SP = 0x80000000u;
static const uint32_t pINF_SP  = 0x7F800000u;
static const uint32_t nINF_SP  = 0xFF800000u;
static const uint32_t pQNAN_SP = 0x7FC00000u;
static const uint32_t LFPN_SP  = 0x7F7FFFFFu;
static const uint32_t SFPN_SP  = 0x00800000u;
static const int16_t  ZERO_EXP = 0;

enum fp_type_t { FT_ZERO, FT_NUM, FT_INF, FT_NAN };

struct unpack_sp_t {
    fp_type_t type;
    bool signals;
    bool sign;
    uint32_t frac;
    int16_t exp;
};

__device__ __forceinline__ uint32_t renorm_sp(uint32_t sp, int16_t& exp) {
    if ( sp < (1u << (25 - 16 + 1)) ) { sp <<= 16; exp -= 16; }
    if ( sp < (1u << (25 -  8 + 1)) ) { sp <<=  8; exp -=  8; }
    if ( sp < (1u << (25 -  4 + 1)) ) { sp <<=  4; exp -=  4; }
    if ( sp < (1u << (25 -  2 + 1)) ) { sp <<=  2; exp -=  2; }
    if ( sp < (1u << (25 -  1 + 1)) ) { sp <<=  1; exp -=  1; }
    return sp;
}

__device__ __forceinline__ unpack_sp_t unpack_sp(uint32_t sp, uint32_t fc) {
    unpack_sp_t usp;

    usp.signals = false;
    usp.sign    = (sp >> 31) & 1;
    usp.frac    = (sp << 2) & ((1u << 25) - 1);
    usp.exp     = (sp >> 23) & 0xFF;

    if ( usp.exp ) {
        usp.frac |= 1u << 25;
        usp.exp--;

        usp.type = usp.exp  < 0xFE        ? FT_NUM
                 : usp.frac == (1u << 25) ? FT_INF
                 :                          FT_NAN;

        if ( usp.type == FT_NAN )
            usp.signals = ( ( usp.frac >> 24 ) & 1 ) == 0;

        return usp;
    }

    if ( !usp.frac ) {
        usp.exp  = ZERO_EXP;
        usp.type = FT_ZERO;
        return usp;
    }

    if ( fc & FC_FLUSH_ZERO ) {
        usp.exp  = ZERO_EXP;
        usp.type = FT_ZERO;
        usp.frac = 0;
        return usp;
    }

    usp.type = FT_NUM;
    usp.frac = renorm_sp( usp.frac, usp.exp );
    return usp;
}

__device__ __forceinline__ uint32_t pack_sp(unpack_sp_t usp, uint32_t fc) {
    const uint32_t round_mode    = fc & FC_ROUND_MASK;
    const bool     flush_to_zero = (fc & FC_FLUSH_ZERO) != 0;
    uint32_t result = 0;

    if ( usp.exp > 0xFD ) {
        usp.exp  = 0xFF;
        result   = 0;
    } else {
        if ( usp.exp < 0 ) {
            if ( usp.exp < -25 || flush_to_zero ) {
                result = 1;
            } else {
                result = usp.frac >> -usp.exp;
                if ( ( result << -usp.exp ) != usp.frac )
                    result |= 1;
            }
            usp.exp = 0;
        } else {
            result = usp.frac;
        }
    }

    if      ( round_mode == FC_ROUND_DOWN ) { if (  usp.sign ) result += 3; }
    else if ( round_mode == FC_ROUND_UP   ) { if ( !usp.sign ) result += 3; }
    else if ( round_mode == FC_ROUND_EVEN ) { result += (result & 4) ? 2 : 1; }

    result >>= 2;
    result  += (uint32_t)usp.exp << 23;

    if ( result >= 0x7F800000u ) {
        if      ( round_mode == FC_ROUND_DOWN && !usp.sign ) result = LFPN_SP;
        else if ( round_mode == FC_ROUND_UP   &&  usp.sign ) result = LFPN_SP;
        else if ( round_mode == FC_ROUND_OFF               ) result = LFPN_SP;
    }

    if ( result == 1 && flush_to_zero )
        result = SFPN_SP;

    if ( usp.sign )
        result |= 1u << 31;

    return result;
}

__device__ __forceinline__ uint32_t rcpsq_tbl(uint32_t index) {
    __device__ static const unsigned char sqrtReciprocalTable[] = {
        0x6a,0x69,0x69,0x68,0x67,0x67,0x66,0x65,0x65,0x64,0x63,0x63,0x62,0x61,0x61,0x60,
        0x5f,0x5f,0x5e,0x5d,0x5d,0x5c,0x5b,0x5b,0x5a,0x5a,0x59,0x58,0x58,0x57,0x57,0x56,
        0x55,0x55,0x54,0x54,0x53,0x52,0x52,0x51,0x51,0x50,0x50,0x4f,0x4e,0x4e,0x4d,0x4d,
        0x4c,0x4c,0x4b,0x4b,0x4a,0x4a,0x49,0x48,0x48,0x47,0x47,0x46,0x46,0x45,0x45,0x44,
        0x44,0x43,0x43,0x42,0x42,0x41,0x41,0x40,0x40,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3c,
        0x3c,0x3c,0x3b,0x3b,0x3a,0x3a,0x39,0x39,0x38,0x38,0x37,0x37,0x37,0x36,0x36,0x35,
        0x35,0x34,0x34,0x33,0x33,0x33,0x32,0x32,0x31,0x31,0x30,0x30,0x30,0x2f,0x2f,0x2e,
        0x2e,0x2e,0x2d,0x2d,0x2c,0x2c,0x2c,0x2b,0x2b,0x2a,0x2a,0x2a,0x29,0x29,0x28,0x28,
        0x28,0x27,0x27,0x26,0x26,0x26,0x25,0x25,0x25,0x24,0x24,0x23,0x23,0x23,0x22,0x22,
        0x22,0x21,0x21,0x21,0x20,0x20,0x1f,0x1f,0x1f,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1c,
        0x1c,0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x1a,0x19,0x19,0x19,0x18,0x18,0x18,0x17,0x17,
        0x17,0x16,0x16,0x16,0x15,0x15,0x15,0x14,0x14,0x14,0x14,0x13,0x13,0x13,0x12,0x12,
        0x12,0x11,0x11,0x11,0x10,0x10,0x10,0x10,0x0f,0x0f,0x0f,0x0e,0x0e,0x0e,0x0d,0x0d,
        0x0d,0x0d,0x0c,0x0c,0x0c,0x0b,0x0b,0x0b,0x0b,0x0a,0x0a,0x0a,0x0a,0x09,0x09,0x09,
        0x08,0x08,0x08,0x08,0x07,0x07,0x07,0x06,0x06,0x06,0x06,0x05,0x05,0x05,0x05,0x04,
        0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x00,
        0x00,0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4,0xf3,0xf3,0xf2,
        0xf1,0xf0,0xef,0xee,0xed,0xec,0xeb,0xea,0xea,0xe9,0xe8,0xe7,0xe6,0xe5,0xe4,0xe4,
        0xe3,0xe2,0xe1,0xe0,0xdf,0xdf,0xde,0xdd,0xdc,0xdb,0xdb,0xda,0xd9,0xd8,0xd7,0xd7,
        0xd6,0xd5,0xd4,0xd4,0xd3,0xd2,0xd1,0xd1,0xd0,0xcf,0xce,0xce,0xcd,0xcc,0xcb,0xcb,
        0xca,0xc9,0xc9,0xc8,0xc7,0xc6,0xc6,0xc5,0xc4,0xc4,0xc3,0xc2,0xc2,0xc1,0xc0,0xc0,
        0xbf,0xbe,0xbe,0xbd,0xbc,0xbc,0xbb,0xba,0xba,0xb9,0xb8,0xb8,0xb7,0xb7,0xb6,0xb5,
        0xb5,0xb4,0xb3,0xb3,0xb2,0xb2,0xb1,0xb0,0xb0,0xaf,0xaf,0xae,0xad,0xad,0xac,0xac,
        0xab,0xaa,0xaa,0xa9,0xa9,0xa8,0xa8,0xa7,0xa6,0xa6,0xa5,0xa5,0xa4,0xa4,0xa3,0xa3,
        0xa2,0xa2,0xa1,0xa0,0xa0,0x9f,0x9f,0x9e,0x9e,0x9d,0x9d,0x9c,0x9c,0x9b,0x9b,0x9a,
        0x9a,0x99,0x99,0x98,0x98,0x97,0x97,0x96,0x96,0x95,0x95,0x94,0x94,0x93,0x93,0x92,
        0x92,0x91,0x91,0x90,0x90,0x8f,0x8f,0x8e,0x8e,0x8d,0x8d,0x8c,0x8c,0x8c,0x8b,0x8b,
        0x8a,0x8a,0x89,0x89,0x88,0x88,0x87,0x87,0x87,0x86,0x86,0x85,0x85,0x84,0x84,0x83,
        0x83,0x83,0x82,0x82,0x81,0x81,0x80,0x80,0x80,0x7f,0x7f,0x7e,0x7e,0x7e,0x7d,0x7d,
        0x7c,0x7c,0x7b,0x7b,0x7b,0x7a,0x7a,0x79,0x79,0x79,0x78,0x78,0x77,0x77,0x77,0x76,
        0x76,0x76,0x75,0x75,0x74,0x74,0x74,0x73,0x73,0x72,0x72,0x72,0x71,0x71,0x71,0x70,
        0x70,0x6f,0x6f,0x6f,0x6e,0x6e,0x6e,0x6d,0x6d,0x6d,0x6c,0x6c,0x6b,0x6b,0x6b,0x6a
    };
    return sqrtReciprocalTable[index];
}

__device__ __forceinline__ float __recip_sqrt_cuda(float x) {
    const uint32_t fc = FC_ROUND_EVEN;  // match your desired rounding/FTZ
    uint32_t op1 = __float_as_uint(x);

    unpack_sp_t u1 = unpack_sp( op1, fc );

    if ( u1.type == FT_NAN ) return __uint_as_float(pQNAN_SP);
    if ( u1.type == FT_ZERO ) return __uint_as_float(u1.sign ? nINF_SP : pINF_SP);
    if ( u1.exp < 0 ) return __uint_as_float(u1.sign ? nINF_SP : pINF_SP);
    if ( u1.type == FT_INF && !u1.sign ) return __uint_as_float(pZERO_SP);
    if ( u1.sign ) return __uint_as_float(pQNAN_SP);

    unpack_sp_t rc;
    rc.type = FT_NUM;
    rc.sign = false;

    int index = (op1 >> 15) & 0x1FF;

    rc.exp  = (u1.exp + 2) >> 1;
    rc.exp  = 0xFC - (rc.exp + 62);
    rc.exp += ( index == 256 );

    rc.frac = rcpsq_tbl( index ) << 17;

    uint32_t out_bits = pack_sp( rc, fc );
    return __uint_as_float(out_bits);
}

__device__ __forceinline__ uint32_t rcp_tbl(uint32_t index) {
    __device__ static const unsigned char reciprocalTable[] = {
        0x00,0xfe,0xfc,0xfa,0xf8,0xf6,0xf4,0xf2,0xf0,0xef,0xed,0xeb,0xe9,0xe7,0xe5,0xe4,
        0xe2,0xe0,0xde,0xdd,0xdb,0xd9,0xd7,0xd6,0xd4,0xd2,0xd1,0xcf,0xce,0xcc,0xca,0xc9,
        0xc7,0xc6,0xc4,0xc2,0xc1,0xbf,0xbe,0xbc,0xbb,0xb9,0xb8,0xb6,0xb5,0xb3,0xb2,0xb1,
        0xaf,0xae,0xac,0xab,0xaa,0xa8,0xa7,0xa5,0xa4,0xa3,0xa1,0xa0,0x9f,0x9d,0x9c,0x9b,
        0x9a,0x98,0x97,0x96,0x95,0x93,0x92,0x91,0x90,0x8e,0x8d,0x8c,0x8b,0x8a,0x88,0x87,
        0x86,0x85,0x84,0x83,0x82,0x80,0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x76,0x75,
        0x74,0x73,0x72,0x71,0x70,0x6f,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0x67,0x66,0x65,
        0x64,0x63,0x62,0x61,0x60,0x5f,0x5e,0x5e,0x5d,0x5c,0x5b,0x5a,0x59,0x58,0x57,0x56,
        0x55,0x54,0x54,0x53,0x52,0x51,0x50,0x4f,0x4e,0x4e,0x4d,0x4c,0x4b,0x4a,0x49,0x49,
        0x48,0x47,0x46,0x45,0x44,0x44,0x43,0x42,0x41,0x40,0x40,0x3f,0x3e,0x3d,0x3d,0x3c,
        0x3b,0x3a,0x3a,0x39,0x38,0x37,0x37,0x36,0x35,0x34,0x34,0x33,0x32,0x32,0x31,0x30,
        0x2f,0x2f,0x2e,0x2d,0x2d,0x2c,0x2b,0x2b,0x2a,0x29,0x29,0x28,0x27,0x27,0x26,0x25,
        0x25,0x24,0x23,0x23,0x22,0x21,0x21,0x20,0x1f,0x1f,0x1e,0x1e,0x1d,0x1c,0x1c,0x1b,
        0x1a,0x1a,0x19,0x19,0x18,0x17,0x17,0x16,0x16,0x15,0x15,0x14,0x13,0x13,0x12,0x12,
        0x11,0x10,0x10,0x0f,0x0f,0x0e,0x0e,0x0d,0x0d,0x0c,0x0b,0x0b,0x0a,0x0a,0x09,0x09,
        0x08,0x08,0x07,0x07,0x06,0x06,0x05,0x05,0x04,0x04,0x03,0x03,0x02,0x02,0x01,0x01
    };
    return reciprocalTable[index];
}


// ---------------------------------------------------------------------------
// bf16_recip_bitmatch: bit-accurate SW model of the C7604 VRCPBF instruction.
//
// Mirrors the algorithm in bf16_standalone.h (namespace vrcpbf):
//   • 128-entry LUT indexed by the 7 mantissa bits of the BF16 input
//   • Each entry is the BF16 bit-pattern of recip(1 + m/128), m = 0..127
//   • Exponent inversion:
//       m == 0 (exact power-of-2) → new_biased_exp = 254 - old_biased_exp
//       m  > 0                    → new_biased_exp = 253 - old_biased_exp
//   • Special cases:
//       NaN          → canonical +qNaN (0x7FC0), sign cleared
//       ±inf         → ±zero
//       zero/subnorm → ±inf
// ---------------------------------------------------------------------------
__device__ __forceinline__ bfloat16_tidl bf16_recip_bitmatch(bfloat16_tidl x) {
    __device__ static const uint16_t vrcpbf_lut[128] = {
        /* m=  0 */ 0x3F80, 0x3F7E, 0x3F7C, 0x3F7A,
        /* m=  4 */ 0x3F78, 0x3F76, 0x3F74, 0x3F72,
        /* m=  8 */ 0x3F71, 0x3F6F, 0x3F6D, 0x3F6B,
        /* m= 12 */ 0x3F6A, 0x3F68, 0x3F67, 0x3F65,
        /* m= 16 */ 0x3F63, 0x3F62, 0x3F60, 0x3F5F,
        /* m= 20 */ 0x3F5D, 0x3F5C, 0x3F5A, 0x3F59,
        /* m= 24 */ 0x3F57, 0x3F56, 0x3F55, 0x3F53,
        /* m= 28 */ 0x3F52, 0x3F50, 0x3F4F, 0x3F4E,
        /* m= 32 */ 0x3F4D, 0x3F4B, 0x3F4A, 0x3F49,
        /* m= 36 */ 0x3F48, 0x3F46, 0x3F45, 0x3F44,
        /* m= 40 */ 0x3F43, 0x3F42, 0x3F41, 0x3F3F,
        /* m= 44 */ 0x3F3E, 0x3F3D, 0x3F3C, 0x3F3B,
        /* m= 48 */ 0x3F3A, 0x3F39, 0x3F38, 0x3F37,
        /* m= 52 */ 0x3F36, 0x3F35, 0x3F34, 0x3F33,
        /* m= 56 */ 0x3F32, 0x3F31, 0x3F30, 0x3F2F,
        /* m= 60 */ 0x3F2E, 0x3F2D, 0x3F2C, 0x3F2B,
        /* m= 64 */ 0x3F2A, 0x3F2A, 0x3F29, 0x3F28,
        /* m= 68 */ 0x3F27, 0x3F26, 0x3F25, 0x3F24,
        /* m= 72 */ 0x3F24, 0x3F23, 0x3F22, 0x3F21,
        /* m= 76 */ 0x3F20, 0x3F20, 0x3F1F, 0x3F1E,
        /* m= 80 */ 0x3F1D, 0x3F1D, 0x3F1C, 0x3F1B,
        /* m= 84 */ 0x3F1A, 0x3F1A, 0x3F19, 0x3F18,
        /* m= 88 */ 0x3F17, 0x3F17, 0x3F16, 0x3F15,
        /* m= 92 */ 0x3F15, 0x3F14, 0x3F13, 0x3F13,
        /* m= 96 */ 0x3F12, 0x3F11, 0x3F11, 0x3F10,
        /* m=100 */ 0x3F0F, 0x3F0F, 0x3F0E, 0x3F0E,
        /* m=104 */ 0x3F0D, 0x3F0C, 0x3F0C, 0x3F0B,
        /* m=108 */ 0x3F0B, 0x3F0A, 0x3F09, 0x3F09,
        /* m=112 */ 0x3F08, 0x3F08, 0x3F07, 0x3F07,
        /* m=116 */ 0x3F06, 0x3F05, 0x3F05, 0x3F04,
        /* m=120 */ 0x3F04, 0x3F03, 0x3F03, 0x3F02,
        /* m=124 */ 0x3F02, 0x3F01, 0x3F01, 0x3F00
    };

    // bfloat16_tidl stores the raw 16-bit pattern in the public member x.
    uint16_t bits16 = x.x;

    bool     sign = (bits16 & 0x8000u) != 0u;
    uint16_t exp  = (bits16 & 0x7F80u) >> 7;
    uint16_t man  =  bits16 & 0x007Fu;

    auto make = [](uint16_t b) { return bfloat16_tidl(b, bfloat16_tidl::from_bits()); };

    // NaN (exp=0xFF, man!=0) → canonical +qNaN (0x7FC0), sign cleared
    if (exp == 0xFFu && man != 0u)
        return make(0x7FC0u);

    // ±inf → ±zero
    if (exp == 0xFFu)
        return make(sign ? 0x8000u : 0x0000u);

    // zero / subnormal (exp=0) → ±inf
    if (exp == 0u)
        return make(sign ? 0xFF80u : 0x7F80u);

    // Normal: look up reciprocal significand for input significand in [1, 2)
    uint16_t lut_entry = vrcpbf_lut[man];
    uint16_t lut_man   = lut_entry & 0x007Fu;
    int      new_exp   = (man == 0u) ? (254 - (int)exp) : (253 - (int)exp);

    if (new_exp >= 0xFF) return make(sign ? 0xFF80u : 0x7F80u);  // overflow → ±inf
    if (new_exp <= 0   ) return make(sign ? 0x8000u : 0x0000u);  // underflow → ±zero

    uint16_t result = (uint16_t)(sign ? 0x8000u : 0x0000u)
                    | (uint16_t)((new_exp & 0xFF) << 7)
                    | lut_man;
    return make(result);
}

__device__ __forceinline__ float __recip_bitmatch(float x) {
    const uint32_t fc = FC_ROUND_EVEN; // use your desired rounding/FTZ mode
    uint32_t op1 = __float_as_uint(x);


    unpack_sp_t u1 = unpack_sp(op1, fc);


    if ( u1.type == FT_NAN ) return __uint_as_float(pQNAN_SP);
    if ( u1.type == FT_ZERO ) return __uint_as_float(u1.sign ? nINF_SP : pINF_SP);
    if ( u1.exp < 0 ) return __uint_as_float(u1.sign ? nINF_SP : pINF_SP);
    if ( u1.type == FT_INF ) return __uint_as_float(u1.sign ? nZERO_SP : pZERO_SP);


    const bool is_one_half = (u1.frac & (u1.frac - 1)) == 0;


    if ( u1.exp > 0xFC || ( u1.exp == 0xFC && !is_one_half ) )
        return __uint_as_float(u1.sign ? nZERO_SP : pZERO_SP);


    unpack_sp_t rc;
    rc.type = FT_NUM;
    rc.sign = u1.sign;
    rc.exp  = 0xFC - u1.exp;


    uint32_t idx = (u1.frac >> 17) & 0xFF;
    if ( !idx ) rc.exp++; // exact 1/2 case


    rc.frac = rcp_tbl(idx) << 17;


    uint32_t out_bits = pack_sp(rc, fc);
    return __uint_as_float(out_bits);
}

/*End of C7x intrinsic implementation*/

//Instantiations:
template void minmaxGPU<int>(const int, const int*, int*, int*);
template void minmaxGPU<float>(const int, const float*, int*, int*);
template void minmax_Thrust<int>(int, int const*, int*, int*);
template void minmax_Thrust<float>(int, float const*, float*, float*);
template void minmax_Thrust<long>(int, long const*, long*, long*);
