/*
*
* Copyright (c) 2022 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/**
@file      tidl_scalability_support.h
@brief     This file defines half and quarter vector dataypes and SE templates for easy implementation of c7x scalable programming model
@version 0.1 Nov 2021 : Initial Code
*/

#ifndef TIDL_SCALABLE_SUPPORT_H_
#define TIDL_SCALABLE_SUPPORT_H_

#include <c7x_scalable.h>

/*****************************************************************************/
/* PORTING SCALABLE APIS FROM 2.0.1 TO 2.1.0 COMPILER                                                   */
/*****************************************************************************/
/* This segment is put here in order for compatibility of the scalable APIs with the new compiler. It maps old typedefs and macros to new ones with similar functioning. To be removed later. */ 
#define VEC_NELEM(dataType) (c7x::max_simd<dataType>::value)

/*****************************************************************************/
/* MIXED WIDTH VECTOR TYPES                                                   */
/*****************************************************************************/
typedef c7x::make_vector<uchar,VEC_NELEM(ushort)>::type uchar_shortlen_vec;
typedef c7x::make_vector<uchar,VEC_NELEM(uint)>::type uchar_intlen_vec;
typedef c7x::make_vector<uchar,VEC_NELEM(ulong)>::type uchar_longlen_vec;

typedef c7x::make_vector<int8_t,VEC_NELEM(ushort)>::type char_shortlen_vec;
typedef c7x::make_vector<int8_t,VEC_NELEM(uint)>::type char_intlen_vec;
typedef c7x::make_vector<int8_t,VEC_NELEM(ulong)>::type char_longlen_vec;

typedef c7x::make_vector<uint16_t,VEC_NELEM(uint)>::type ushort_intlen_vec;
typedef c7x::make_vector<uint16_t,VEC_NELEM(ulong)>::type ushort_longlen_vec;

typedef c7x::make_vector<int16_t,VEC_NELEM(uint)>::type short_intlen_vec;
typedef c7x::make_vector<int16_t,VEC_NELEM(ulong)>::type short_longlen_vec;

typedef c7x::make_vector<uint32_t,VEC_NELEM(ulong)>::type uint_longlen_vec;

typedef c7x::make_vector<int32_t,VEC_NELEM(ulong)>::type int_longlen_vec;


/* TIDL_SIMD_WIDTH_SHIFT VARIABLE*/
/**
 *  \brief TIDL_SIMD_WIDTH_SHIFT value is normally derived from TIDL_SIMD_WIDTH as log2(TIDL_SIMD_WIDTH), but since log2() from cmath is computationally expensive, this method of defining TIDL_SIMD_WIDTH_SHIFT has been introduced
 *  Example: SIMD_SHIFT_FROM_WIDTH<64>::value [yields 6]
 */
template<uint32_t S> struct SIMD_SHIFT_FROM_WIDTH {};

template<> struct SIMD_SHIFT_FROM_WIDTH<128>
{ static const uint32_t _value = 7; };
template<> struct SIMD_SHIFT_FROM_WIDTH<64>
{ static const uint32_t _value = 6; };
template<> struct SIMD_SHIFT_FROM_WIDTH<32>
{ static const uint32_t _value = 5; };
template<> struct SIMD_SHIFT_FROM_WIDTH<16>
{ static const uint32_t _value = 4; };
template<> struct SIMD_SHIFT_FROM_WIDTH<8>
{ static const uint32_t _value = 3; };
template<> struct SIMD_SHIFT_FROM_WIDTH<4>
{ static const uint32_t _value = 2; };
template<> struct SIMD_SHIFT_FROM_WIDTH<2>
{ static const uint32_t _value = 1; };
template<> struct SIMD_SHIFT_FROM_WIDTH<1>
{ static const uint32_t _value = 0; };

/* Typedefs for C++ style SE templates*/
/**
 *  \brief Shortened versions of C++ SE and SEADV templates for better readability and for more similarity with existing C style code
 *  Example: SEADV(0, int16) [yields (c7x::strm_eng<0, int16>::get_adv())]
 *           SE(0, int16) [yields (c7x::strm_eng<0, int16>::get())]
 *           SE_VECLEN_NELEM(64) yields c7x::se_veclen_from_traits<64>::value
 *           SA_VECLEN_NELEM(64) yields c7x::sa_veclen_from_traits<64>::value
 *           SE_ELEDUP(64) yields __SE_ELEDUP_64X
 */
#define SEADV(id, dataType) (c7x::strm_eng<id, dataType>::get_adv())
#define SE(id, dataType) (c7x::strm_eng<id, dataType>::get())
#define SE_VECLEN_NELEM(num) c7x::se_veclen_from_traits<num>::value
#define SA_VECLEN_NELEM(num) c7x::sa_veclen_from_traits<num>::value
#define SE_ELEDUP(num) se_eledup_from_traits<num>::value;

/*****************************************************************************/
/* C-STYLE TEMPLATED REINTERPRETS                                                 */
/*****************************************************************************/
#define reinterpret_vec(var, vecType) c7x::reinterpret<vecType>(var)
#define convert_vec(var, vecType) c7x::convert<vecType>(var)

/* Typedefs for MMA parameters*/
/**
 *  \brief This contains all typedefs corresponding to different scalable parameters that are used to configure the MMA in C7x DSPs
 *  Examples: MMA_C_BANK_HEIGHT corresponds to the height/number of rows in the C bank of MMA [yields 64 on C7100, 32 on C7504 etc]
 *           MMA_B_BANK_HEIGHT corresponds to the height/number of rows in the B bank of MMA [yields 64 on C7100, 32 on C7504 etc]
 * */
#define MMA_C_BANK_HEIGHT TIDL_SIMD_WIDTH
#define MMA_B_BANK_HEIGHT TIDL_SIMD_WIDTH

#endif /*TIDL_SCALABLE_SUPPORT_ */
/*==========================================================================*/
/*     END OF THE FILE                                                      */
/*==========================================================================*/
