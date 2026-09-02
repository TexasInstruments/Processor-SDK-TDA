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

 THIS SOFTWARE IS PROVIDED BY TI AND TI�S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI�S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <vector>
#include <memory>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <functional>
#include <stdlib.h>
#include <stdio.h>


#include "TI/tivx.h"
#include "VX/vx.h"

#include <app_gl_egl_utils.h>
#include <utils/mem/include/app_mem.h>
#include <kernels/srv/target/gpu/3dsrv/render.h>

#ifndef EGL_TI_raw_video
#  define EGL_TI_raw_video             1
#  define EGL_RAW_VIDEO_TI             0x333A   /* eglCreateImageKHR target */
#  define EGL_GL_VIDEO_FOURCC_TI       0x3331   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_WIDTH_TI        0x3332   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_HEIGHT_TI       0x3333   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_BYTE_STRIDE_TI  0x3334   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_BYTE_SIZE_TI    0x3335   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_YUV_FLAGS_TI    0x3336   /* eglCreateImageKHR attribute */
#endif

#ifndef EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE
#  define EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE (0 << 0)
#  define EGLIMAGE_FLAGS_YUV_FULL_RANGE       (1 << 0)
#  define EGLIMAGE_FLAGS_YUV_BT601            (0 << 1)
#  define EGLIMAGE_FLAGS_YUV_BT709            (1 << 1)
#endif

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define MAX_ABS_FILENAME          (1024u)

#define APP_EGL_MAX_TEXTURES        (20u)
#define APP_EGL_MAX_RENDER_TEXTURES (20u)

#define APP_MAXNUM_EGL_BUFFERS               (1)
#define APP_VIDEO_MAX_PLANES                 (3)

typedef struct
{
    void *bufAddr[APP_VIDEO_MAX_PLANES];
    /**< Actual Buffer address in memory */
    int32_t dmaFd[APP_VIDEO_MAX_PLANES];
    /**< userspace handle to represent buffer */
    void *eglPixmap;
    /**< An EglPixmap buffer that will be consumed
     *   by an EGLLink to render on */
    void *pixmapNative;
    /**< Pixmap information to be used by EGL */

} App_EglVideoFrameBuffer;

typedef struct
{
    uint32_t width;
    /**< Output width */
    uint32_t height;
    /**< Output height */
    void *desc_array[APP_MAXNUM_EGL_BUFFERS];
    /**< place holder for storing Output descriptor details */
    void *buff_array[APP_MAXNUM_EGL_BUFFERS];
    /**< place holder for storing Output buffer details */

} App_EglOutputBufObj;

typedef struct
{
    EGLDisplay display;
    Display *nativeDisplay;
    Window nativeWindow;
    EGLConfig config;
    EGLContext context;
    App_EglVideoFrameBuffer *eglBuffers[APP_MAXNUM_EGL_BUFFERS];
    EGLNativeWindowType windowNative;
    EGLSurface surface;
    GLuint      texYuv[APP_EGL_MAX_TEXTURES];
    EGLImageKHR texImg[APP_EGL_MAX_TEXTURES];
    int64_t     dmaBufFd[APP_EGL_MAX_TEXTURES];
    uint32_t numBuf;
    App_EglOutputBufObj outBufInfo;
} App_EglWindowObj;

#ifdef __cplusplus
extern "C" {
#endif

int counter = 0;
static int load_texture(GLuint tex, int width, int height, int textureType, int64_t data)
{
    GLenum target = GL_TEXTURE_2D;
    GLint param = GL_NEAREST;
    char file[SRVMAXPATHLENGTH];

    if ((textureType == GL_RGB) || (textureType == GL_RGBA)) {
        target = GL_TEXTURE_2D;
        param = GL_NEAREST;

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            textureType,
            width,
            height,
            0,
            textureType,
            GL_UNSIGNED_BYTE,//textureFormat,
            (void*)data
            );
        appEglCheckGlError("glTexImage2D");
    } else {
        printf("Incorrect texture type %x\n", textureType);
        return -1;
    }

    return 0;
}

static int createNativeDisplay(Display** nativeDisplay)
{
    /* Check for a valid display */
    if (!nativeDisplay)	{    return 0; }

    /* Open the display */
    *nativeDisplay = XOpenDisplay(0);
    if (!*nativeDisplay)
    {
        printf("Error: Unable to open X display\n");
        return 0;
    }
    return 1;
}

static int createNativeWindow(Display* nativeDisplay, Window* nativeWindow)
{
    /* Get the default screen for the display */
    int defaultScreen = XDefaultScreen(nativeDisplay);

    /* Get the default depth of the display */
    int defaultDepth = DefaultDepth(nativeDisplay, defaultScreen);

    /* Select a visual info */
    std::unique_ptr<XVisualInfo> visualInfo(new XVisualInfo);
    XMatchVisualInfo(nativeDisplay, defaultScreen, defaultDepth, TrueColor, visualInfo.get());
    if (!visualInfo.get())
    {
        printf("Error: Unable to acquire visual\n");
        return 0;
    }

    /* Get the root window for the display and default screen */
    Window rootWindow = RootWindow(nativeDisplay, defaultScreen);

    /* Create a color map from the display, root window and visual info */
    Colormap colorMap = XCreateColormap(nativeDisplay, rootWindow, visualInfo->visual, AllocNone);

    /* Now setup the final window by specifying some attributes */
    XSetWindowAttributes windowAttributes;

    /* Set the color map that was just created */
    windowAttributes.colormap = colorMap;

    /* Set events that will be handled by the app, add to these for other events. */
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;

    /* Create the window */
    *nativeWindow = XCreateWindow(nativeDisplay,              /* The display used to create the window */
                                  rootWindow,                 /* The parent (root) window - the desktop */
                                  0,                          /* The horizontal (x) origin of the window */
                                  0,                          /* The vertical (y) origin of the window */
                                  WINDOW_WIDTH,               /*  The width of the window */
                                  WINDOW_HEIGHT,              /*  The height of the window */
                                  0,                          /*  Border size - set it to zero */
                                  visualInfo->depth,          /*  Depth from the visual info */
                                  InputOutput,                /*  Window type - this specifies InputOutput. */
                                  visualInfo->visual,         /*  Visual to use */
                                  CWEventMask | CWColormap,   /*  Mask specifying these have been defined in the window attributes */
                                  &windowAttributes);         /*  Pointer to the window attribute structure */

    /* Make the window viewable by mapping it to the display */
    XMapWindow(nativeDisplay, *nativeWindow);

    /* Set the window title */
    XStoreName(nativeDisplay, *nativeWindow, "Surround View");

    /* Setup the window manager protocols to handle window deletion events */
    Atom windowManagerDelete = XInternAtom(nativeDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(nativeDisplay, *nativeWindow, &windowManagerDelete , 1);

    return 1;
}

void appEglPrintGLString(const char *name, GLenum s)
{
   const char *v = (const char *) glGetString(s);

   printf("APP EGL: GL %s = %s\n", name, v);
}

void appEglCheckGlError(const char* op)
{
   GLint error;

   for (error = glGetError(); error; error = glGetError())
   {
       fprintf(stderr, "GL: after %s() glError (0x%x)\n", op, error);
   }
}

void appEglCheckEglError(const char* op, EGLBoolean returnVal)
{
   EGLint error;

   if (returnVal != EGL_TRUE)
   {
       fprintf(stderr, " EGL: %s() returned %d\n", op, returnVal);
   }

   for (error = eglGetError(); error != EGL_SUCCESS; error = eglGetError())
   {
       fprintf(stderr, " EGL: after %s() eglError (0x%x)\n", op, error);
   }
}

void *appEglWindowOpen()
{
    EGLint num_configs;
    EGLint majorVersion;
    EGLint minorVersion;
    int32_t ret, count, status = 0;

    App_EglWindowObj *pEglWindowObj;

    pEglWindowObj = (App_EglWindowObj *)malloc(sizeof(App_EglWindowObj));
    if(pEglWindowObj==NULL)
    {
        status = -1;
    }

    /* TODO: Different from standalone */
    const EGLint attribs[] = {
       EGL_RED_SIZE, 8,
       EGL_GREEN_SIZE, 8,
       EGL_BLUE_SIZE, 8,
       EGL_ALPHA_SIZE, 8,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
       EGL_DEPTH_SIZE, 8,
       EGL_NONE
    };

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    if (status == 0)
    {
        /* Get access to a native display */
        if (!createNativeDisplay(&pEglWindowObj->nativeDisplay))
        {
            printf ("Cannot create native display\n");
            status = -1;
        }
    }

    /* Setup the windowing system, create a window */
    if (status == 0)
    {
        if (!createNativeWindow(pEglWindowObj->nativeDisplay, &pEglWindowObj->nativeWindow))
        {
            printf ("Cannot create native Window\n");
            status = -1;
        }
    }

    pEglWindowObj->display = eglGetDisplay((EGLNativeDisplayType)pEglWindowObj->nativeDisplay);
    appEglCheckEglError("eglGetDisplay", EGL_TRUE);

    if (pEglWindowObj->display == EGL_NO_DISPLAY) {
       printf("APP EGL: ERROR: eglGetDisplay() returned EGL_NO_DISPLAY !!!\n");
       status = -1;
    }

    if(status == 0)
    {
      ret = eglInitialize(pEglWindowObj->display, &majorVersion, &minorVersion);
      appEglCheckEglError("eglInitialize", ret);
      printf("APP EGL: version %d.%d\n", majorVersion, minorVersion);
      if (ret != EGL_TRUE) {
         printf("APP EGL: eglInitialize() failed !!!\n");
         status = -1;
      }
    }

    if(status == 0)
    {
      if (!eglChooseConfig(pEglWindowObj->display, attribs, &pEglWindowObj->config, 1, &num_configs))
      {
         printf("APP EGL: ERROR: eglChooseConfig() failed. Couldn't get an EGL visual config !!!\n");
         status = -1;
      }
    }

    if(status == 0)
    {
      pEglWindowObj->surface = eglCreateWindowSurface(pEglWindowObj->display, pEglWindowObj->config, (EGLNativeWindowType)pEglWindowObj->nativeWindow, NULL);
      appEglCheckEglError("eglCreateWindowSurface", EGL_TRUE);
      if (pEglWindowObj->surface == EGL_NO_SURFACE)
      {
         printf("APP EGL: eglCreateWindowSurface() failed !!!\n");
         status = -1;
      }
    }

    if(status == 0)
    {
      if (!eglBindAPI(EGL_OPENGL_ES_API)) { // Shiju - add in VSDK
          printf("APP EGL: ERROR - failed to bind api EGL_OPENGL_ES_API\n");
          status = -1;
      }
    }

    if(status == 0)
    {
      pEglWindowObj->context = eglCreateContext(pEglWindowObj->display, pEglWindowObj->config, EGL_NO_CONTEXT, context_attribs);
      appEglCheckEglError("eglCreateContext", EGL_TRUE);
      if (pEglWindowObj->context == EGL_NO_CONTEXT) {
         printf("APP EGL: eglCreateContext() failed !!!\n");
         status = -1;
      }
    }

    if(status == 0)
    {

      if(status == 0)
      {
        ret = eglMakeCurrent(pEglWindowObj->display, pEglWindowObj->surface, pEglWindowObj->surface, pEglWindowObj->context);
        appEglCheckEglError("eglMakeCurrent", ret);
        if (ret != EGL_TRUE)
        {
           printf("APP EGL: eglMakeCurrent() failed !!!\n");
           status = -1;
        }
      }
    }

    return pEglWindowObj;
}

void appEglSwap(void *eglWindow)
{
    App_EglWindowObj *pEglWindowObj = (App_EglWindowObj*)eglWindow;

    eglSwapBuffers(pEglWindowObj->display, pEglWindowObj->surface);
}

static GLuint appEglWindowSetupYuvTexSurface(App_EglWindowObj *pObj, app_egl_tex_prop_t *pProp, int64_t dmaBufFd, int32_t texIndex)
{
    int32_t attrIdx, status;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
    GLenum target = GL_TEXTURE_2D;
    GLint param = GL_NEAREST;
    status = 0;

    if (status == 0)
    {
        char filename[MAX_ABS_FILENAME];

        glGenTextures(1, &pObj->texYuv[texIndex]);
        appEglCheckGlError("glGenTextures");

        glBindTexture(GL_TEXTURE_2D, pObj->texYuv[texIndex]);
        appEglCheckGlError("glBindTexture");

        if (pProp->dataFormat == APP_EGL_DF_RGB)
        {
            load_texture(pObj->texYuv[texIndex], pProp->width, pProp->height, GL_RGB, dmaBufFd);
        }
        else if (pProp->dataFormat == APP_EGL_DF_RGBX)
        {
            load_texture(pObj->texYuv[texIndex], pProp->width, pProp->height, GL_RGBA, dmaBufFd);
        }
        else
        {
            printf("APP EGL: ERROR: texture format invalid!\n");
        }

        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, param);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, param);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        appEglCheckGlError("glTexParameteri");

        pObj->dmaBufFd[texIndex] = dmaBufFd;
    }

    return status;
}

uint32_t appEglWindowGetTexYuv(void *eglWindow, app_egl_tex_prop_t *pProp)
{
    GLuint texYuv = 0;
    int32_t texFound = 0, i, status;

    App_EglWindowObj *pEglWindowObj = (App_EglWindowObj*)eglWindow;

    for(i=0; i<pEglWindowObj->numBuf; i++)
    {
        if(pEglWindowObj->dmaBufFd[i]==pProp->dmaBufFd[0])
        {
            texYuv = pEglWindowObj->texYuv[i];
            texFound = 1;
            break;
        }
    }
    if((texFound==0) && (i < APP_EGL_MAX_TEXTURES))
    {
        status = appEglWindowSetupYuvTexSurface(
                        pEglWindowObj,
                        pProp,
                        pProp->dmaBufFd[0],
                        i
                        );
        if(status!=0)
        {
            printf("APP EGL: ERROR: Unable to bind texture[%d] to dmabuf fd [%d] !!!\n", i, pProp->dmaBufFd[0]);
        }
        else
        {
            texYuv = pEglWindowObj->texYuv[i];
            pEglWindowObj->numBuf++;
        }
    }

    return texYuv;
}

void appEglBindFrameBuffer(void *eglWindow, app_egl_tex_prop_t *prop)
{
    return;
}

int appEglWindowClose(void *eglWindow)
{
    uint32_t count;
    App_EglVideoFrameBuffer *eglBuf;

    App_EglWindowObj *obj = (App_EglWindowObj*)eglWindow;

    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

    for(count = 0; count < obj->numBuf; count++)
    {
        if(obj->texYuv[count] != 0)
        {
            glDeleteTextures(1, &obj->texYuv[count]);
            obj->texYuv[count] = 0;
        }
        if(obj->texImg[count] != NULL)
        {
            eglDestroyImageKHR(obj->display, obj->texImg[count]);
            obj->texImg[count] = 0;
        }
    }
    eglMakeCurrent(obj->display,EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroySurface(obj->display, obj->surface);

    eglDestroyContext(obj->display, obj->context);
    eglTerminate(obj->display);

    XDestroyWindow(obj->nativeDisplay, obj->nativeWindow);
    XCloseDisplay(obj->nativeDisplay);
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

