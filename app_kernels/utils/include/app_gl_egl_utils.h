/*
Copyright (c) [2012 - 2026] Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 license under copyrights and patents it now or hereafter owns or controls to
 make,  have made, use, import, offer to sell and sell ("Utilize") this software
 subject to the terms herein.  With respect to the foregoing patent license,
 such license is granted  solely to the extent that any such patent is necessary
 to Utilize the software alone.  The patent license shall not apply to any
 combinations which include this software, other than combinations with devices
 manufactured by or for TI ("TI Devices").  No hardware patent is licensed
 hereunder.

 Redistributions must preserve existing copyright notices and reproduce this
 license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

 * No reverse engineering, decompilation, or disassembly of this software
   is permitted with respect to any software provided in binary form.

 * Any redistribution and use are licensed by TI for use only with TI Devices.

 * Nothing shall obligate TI to provide you with source code for the software
   licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution of
 the source code are permitted provided that the following conditions are met:

 * Any redistribution and use of the source code, including any resulting
   derivative works, are licensed by TI for use only with TI Devices.

 * Any redistribution and use of any object code compiled from the source code
   and any resulting derivative works, are licensed by TI for use only with TI
   Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this software
 without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI’S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI’S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _APP_GL_EGL_UTILS_H_
#define _APP_GL_EGL_UTILS_H_

/**
 * \defgroup group_vision_apps_utils_opengl OpenGL APIs
 *
 * \brief This section contains APIs to use OpenGL within Apps
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */


#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <stdint.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define APP_EGL_TEX_MAX_PLANES   (2)

#define APP_EGL_DF_RGBX      (0)
#define APP_EGL_DF_RGB       (1)
#define APP_EGL_DF_NV12      (2)
#define APP_EGL_DF_YUYV      (3)
#define APP_EGL_DF_UYVY      (4)

typedef struct
{
    uint32_t dataFormat;
    /**< Data format of the texture APP_GL_EGL_DF_* */
    uint32_t width;
    /**< in pixels */
    uint32_t height;
    /**< in lines */
    uint32_t pitch[APP_EGL_TEX_MAX_PLANES];
    /**< in bytes, only pitch[0] used right now */
#ifdef x86_64
    int64_t     dmaBufFd[APP_EGL_TEX_MAX_PLANES];
#else
    int32_t     dmaBufFd[APP_EGL_TEX_MAX_PLANES];
#endif
    /**< dma buf fd per plane */
    uint32_t dmaBufFdOffset[APP_EGL_TEX_MAX_PLANES];
    /**< dma buf fd offset per plane */
    void*     bufAddr[APP_EGL_TEX_MAX_PLANES];
    /**< buffer address (QNX) */
} app_egl_tex_prop_t;

#ifdef __cplusplus
extern "C" {
#endif

void appEglCheckGlError(const char* op);
void appEglCheckEglError(const char* op, EGLBoolean returnVal);
void appEglPrintGLString(const char *name, GLenum s);

void    *appEglWindowOpen();
uint32_t appEglWindowGetTexYuv(void *eglWindow, app_egl_tex_prop_t *pProp);
void     appEglSwap(void *eglWindow);
void     appEglBindFrameBuffer(void *eglWindow, app_egl_tex_prop_t *prop);
int32_t  appEglWindowClose(void *eglWindow);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* @} */
#endif
