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

#ifndef _RENDER_H_
#define _RENDER_H_

#ifdef __cplusplus
// GLM includes
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
extern "C"
{
#endif

#ifndef STANDALONE
#include <app_gl_egl_utils.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#endif
void               System_eglCheckEglError(const char* op, EGLBoolean returnVal);
void               System_eglCheckGlError(const char* op);
#include <stdio.h>
#include <stdlib.h>

#endif

#include "renderutils.h"

#define ENABLE_VBO_DRAW
#define ENABLE_CPP

#if 0
#define D_PRINTF printf
#define GL_CHECK(x)                                                                                \
    {                                                                                              \
        int err = glGetError();                                                                    \
        printf("GL Error = %x for %s\n", err, (char*)(#x));                                        \
    }
#else
static inline void dummy_printf(const char* in, ...){};
#define D_PRINTF dummy_printf
#define GL_CHECK(x)
#endif

#ifdef STANDALONE
#define SRV_NUM_IMAGE_SETS 4
#endif

    typedef struct _gl_state
    {
        int    program;
        GLuint textureID[4];
        GLuint attribIndices[2];
        GLint  mvMatrixOffsetLoc;
        GLint  samplerLoc;
        GLuint vboID[4];
        float  carMouse[5];
        float  delta[5];
        float  carMouseMax[5];
        float  carMouseMin[5];
    } gl_state;

    typedef struct
    {
        int x;
        int y;
    } point2d;

    typedef struct
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } vertex3df;

    typedef struct
    {
        point2d l;
        point2d r;
        point2d t;
        point2d b;
    } intercepts;

#ifdef TEXTURE_FROM_LOCAL
#define TEXTURE_WIDTH 1
#define TEXTURE_HEIGHT 1
#else
#define TEXTURE_WIDTH 1280
#define TEXTURE_HEIGHT 720
#endif

#define MAX_IMAGES (4)

#define SRVMAXPATHLENGTH (512u)

#ifndef STANDALONE
#ifndef EGL_TI_raw_video
#define EGL_TI_raw_video 1
#define EGL_RAW_VIDEO_TI 0x333A            /* eglCreateImageKHR target */
#define EGL_GL_VIDEO_FOURCC_TI 0x3331      /* eglCreateImageKHR attribute */
#define EGL_GL_VIDEO_WIDTH_TI 0x3332       /* eglCreateImageKHR attribute */
#define EGL_GL_VIDEO_HEIGHT_TI 0x3333      /* eglCreateImageKHR attribute */
#define EGL_GL_VIDEO_BYTE_STRIDE_TI 0x3334 /* eglCreateImageKHR attribute */
#define EGL_GL_VIDEO_BYTE_SIZE_TI 0x3335   /* eglCreateImageKHR attribute */
#define EGL_GL_VIDEO_YUV_FLAGS_TI 0x3336   /* eglCreateImageKHR attribute */
#endif

#ifndef EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE
#define EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE (0 << 0)
#define EGLIMAGE_FLAGS_YUV_FULL_RANGE (1 << 0)
#define EGLIMAGE_FLAGS_YUV_BT601 (0 << 1)
#define EGLIMAGE_FLAGS_YUV_BT709 (1 << 1)
#endif

#define FOURCC(a, b, c, d)                                                                         \
    ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) |     \
     ((uint32_t)(uint8_t)(d) << 24))
#define FOURCC_STR(str) FOURCC(str[0], str[1], str[2], str[3])

    struct EglimgTexMap
    {
        EGLBoolean  mapped;
        GLuint      tex;
        EGLImageKHR eglimg;
        char*       imgbuf;
    };
    struct GLConfig
    {
        EGLDisplay display;
        EGLConfig  config;
        EGLContext context;
        EGLSurface surface;
    };
// struct GLConfig gl;
// struct EglimgTexMap eglMapArray[MAX_IMAGES];
#endif

#define SYSTEM_EGL_MAX_TEXTURES 4
    typedef struct
    {
        GLuint texYuv[SYSTEM_EGL_MAX_TEXTURES];
        //    EGLImageKHR texImg[SYSTEM_EGL_MAX_TEXTURES];
        void* bufAddr[SYSTEM_EGL_MAX_TEXTURES];
        int   numBuf;

        int width;
        int height;

    } System_EglObj;

#define MAX_SRV_VIEWS 10

    typedef struct _srv_coords_t
    {
        float camx;
        float camy;
        float camz;
        float targetx;
        float targety;
        float targetz;
        float anglex;
        float angley;
        float anglez;
    } srv_coords_t;

    typedef struct
    {
        uint32_t screen_width;
        uint32_t screen_height;

        uint32_t cam_width;
        uint32_t cam_height;
        uint32_t cam_bpp;

        uint32_t num_srv_views;

        void* LUT3D;
        void* blendLUT3D;
        void* PALUT3D;
        void* BoxLUT;
        void* BoxPose3D;

        gl_state car_gl1;

        srv_coords_t srv_views[MAX_SRV_VIEWS];

#ifndef STANDALONE
        app_egl_tex_prop_t texProp;
#endif

    } render_state_t;

#include "srv.h"

    int render_setup(render_state_t* pObj);

#ifdef STANDALONE
    void render_renderFrame(render_state_t* pObj, System_EglObj* pEglObj, GLuint* texYuv);
#else
void               render_renderFrame(render_state_t* pObj, void* pEglObj, GLuint* texYuv);
#endif

    /* used by rendering internally.
     * NOT to be used by sgxDisplayLink directly
     */

    void   render_updateView();
    GLuint render_createProgram(const char* pVertexSource, const char* pFragmentSource);
    void   render_renderFrame3DSRV(render_state_t* pObj, System_EglObj* pEglObj, GLuint* texYuv);
    void   render3dFrame(render_state_t* pObj, System_EglObj* pEglObj, GLuint* texYuv);

    void render_ndcToScreen(int* xscr, int* yscr, float xndc, float yndc);
    void render_worldToNdc(float* xndc, float* yndc, GLint x, GLint y, GLint z);
    void render_worldToScreen(int* xscr, int* yscr, GLint x, GLint y, GLint z);
    void render_lineIntercepts(intercepts* intcepts, int x1, int y1, int x2, int y2, int xleft,
                               int xright, int ytop, int ybottom);
    void render_getIntercepts(intercepts* worldx, intercepts* worldy, int xleft, int xright,
                              int ytop, int ybottom);

    void screen1_draw_vbo(int texYuv);
    void screen1_init_vbo();

    char* get_file_path();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*   _RENDER_H_    */
