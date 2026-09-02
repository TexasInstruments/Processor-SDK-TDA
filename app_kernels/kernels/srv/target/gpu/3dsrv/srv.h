/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter owns
 * or controls to make, have made, use, import, offer to sell and sell
 * ("Utilize") this software subject to the terms herein.  With respect to the
 * foregoing patent license, such license is granted solely to the extent that
 * any such patent is necessary to Utilize the software alone. The patent
 * license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI
 * Devices"). No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this
 * license (including the above copyright notice and the disclaimer and (if
 * applicable) source code license limitations below) in the documentation
 * and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted
 * provided that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this
 * software is permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI
 * Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the
 * software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are
 * met:
 *
 * *       any redistribution and use of the source code, including any
 * resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the
 * source code and any resulting derivative works, are licensed by TI for use
 * only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its
 * suppliers
 *
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _SRV_H_
#define _SRV_H_

#ifdef __cplusplus
extern "C"
{
#endif

// srv types and defines
#define POINTS_WIDTH (1080)
#define POINTS_HEIGHT (1080)
#define POINTS_SUBX 4
#define POINTS_SUBY 4

#define QUADRANTS 4
#define QUADRANT_WIDTH (POINTS_WIDTH / (POINTS_SUBX * 2) + 1)
#define QUADRANT_HEIGHT (POINTS_HEIGHT / (POINTS_SUBY * 2) + 1)
#define QUADRANT_SIZE (QUADRANT_WIDTH * QUADRANT_HEIGHT)

    typedef unsigned char BLENDLUT_DATATYPE;
#define GL_BLENDLUT_DATATYPE GL_UNSIGNED_BYTE

#define GL_VERTEX_ENUM GL_SHORT
#define GL_VERTEX_DATATYPE GLshort

#define GL_BLEND_ENUM GL_UNSIGNED_BYTE
#define GL_BLEND_DATATYPE unsigned char
    // GLubyte

#define USE_SHORT_TEXCOORDS
#ifdef USE_SHORT_TEXCOORDS
#define GL_TEXCOORD_ENUM GL_SHORT
#define GL_TEXCOORD_DATATYPE GLshort
#else
#define GL_TEXCOORD_ENUM GL_FLOAT
#define GL_TEXCOORD_DATATYPE GLfloat
#endif

    typedef struct _srv_lut_t
    {
        GL_VERTEX_DATATYPE   x, y, z;
        GL_TEXCOORD_DATATYPE u1, v1, u2, v2;
#ifdef SRV_INTERLEAVED_BLEND
        GL_BLEND_DATATYPE blend1, blend2;
#endif
    } srv_lut_t;

    typedef struct _srv_blend_lut_t
    {
        GL_BLEND_DATATYPE blend1, blend2;
    } srv_blend_lut_t;

    enum srv_bowl_reshape_t
    {
        left,
        right,
        front,
        back
    };

    // srv functions
    int  srv_setup(render_state_t* pObj);
    void srv_draw(render_state_t* pObj, GLuint* texYuv, int viewport_id);
    int  srv_generate_lut(uint32_t width, uint32_t height, uint32_t camw, uint32_t camh,
                          uint32_t subx, uint32_t suby, srv_lut_t* lut);
    int  srv_generate_blend_lut(uint32_t width, uint32_t height, uint32_t subx, uint32_t suby,
                                srv_blend_lut_t* lut);

    void srv_bowl_inc();
    void srv_bowl_dec();
#ifdef STANDALONE
    void srv_bowl_update();
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*   _SRV_H_    */
