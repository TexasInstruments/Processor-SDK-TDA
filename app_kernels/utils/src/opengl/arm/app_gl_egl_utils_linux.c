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

 THIS SOFTWARE IS PROVIDED BY TI AND TI?S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI?S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef x86_64
#include <fcntl.h>
#include <unistd.h>
#include <gbm.h>
#include <xf86drm.h>
#include <drm.h>
#include <drm_fourcc.h>
#endif

#include <app_gl_egl_utils.h>

#define APP_EGL_MAX_TEXTURES        (20u)
#define APP_EGL_MAX_RENDER_TEXTURES (20u)

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])

typedef struct {

    uint32_t isAlloc;
    GLuint tex;
    EGLImageKHR img;
    GLuint   fboId;
    uint32_t dmaBufFd;
    uint32_t dmaBufFdOffset;

} app_egl_tex_obj_t;

typedef struct
{
    EGLDisplay display;
    EGLNativeDisplayType nativeDisplay;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;

    app_egl_tex_obj_t texRender[APP_EGL_MAX_TEXTURES];
    app_egl_tex_obj_t tex[APP_EGL_MAX_TEXTURES];

    int drm_fd;
    struct gbm_device *gbm_dev;
    struct gbm_surface *gbm_surface;
    PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display;
    PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC create_platform_window_surface;

} app_egl_obj_t;


void appEglPrintGLString(const char *name, GLenum s)
{
   const char *v = (const char *) glGetString(s);

   printf("EGL: GL %s = %s\n", name, v);
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
       fprintf(stderr, "EGL: %s() returned %d\n", op, returnVal);
   }

   for (error = eglGetError(); error != EGL_SUCCESS; error = eglGetError())
   {
       fprintf(stderr, "EGL: after %s() eglError (0x%x)\n", op, error);
   }
}

static EGLBoolean has_extension(const char *const extensions_list,
        const char *const extension_searched)
{
    const char *extension = extensions_list;
    const size_t extension_searched_length = strlen(extension_searched);

    if (!extension)
        return EGL_FALSE;
    if (!extension_searched_length)
        return EGL_TRUE;

    while (EGL_TRUE)
    {
        const size_t extension_length = strcspn(extension, " ");

        if (extension_length == extension_searched_length &&
                strncmp(extension, extension_searched, extension_length) == 0)
        {
            return EGL_TRUE;
        }

        extension += extension_length;

        if ('\0' == *extension)
            return EGL_FALSE;

        extension += 1;
    }
}

static void appEglWindowResetTex(app_egl_tex_obj_t *tex_obj)
{
    tex_obj->isAlloc = 0;
    tex_obj->img = 0;
    tex_obj->tex = 0;
    tex_obj->fboId = -1;
    tex_obj->dmaBufFd = -1;
    tex_obj->dmaBufFdOffset = 0;
}

void *appEglWindowOpen()
{
    const char *egl_platform_extensions;
    EGLint num_configs;
    EGLint majorVersion;
    EGLint minorVersion;
    int32_t ret = 0;
    uint32_t count;

    const EGLint attribs[] = {
       EGL_RED_SIZE, 8,
       EGL_GREEN_SIZE, 8,
       EGL_BLUE_SIZE, 8,
       EGL_ALPHA_SIZE, 8,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
       EGL_DEPTH_SIZE, 16,
       EGL_NONE
    };

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    app_egl_obj_t *obj;

    obj = malloc(sizeof(app_egl_obj_t));
    if(obj==NULL)
    {
        goto goto_error;
    }
    obj->drm_fd = -1;
    obj->gbm_dev = NULL;
    obj->gbm_surface = NULL;
    obj->surface = EGL_NO_SURFACE;

    for(count=0; count < APP_EGL_MAX_TEXTURES; count++)
    {
        appEglWindowResetTex(&obj->tex[count]);
    }
    for(count=0; count < APP_EGL_MAX_RENDER_TEXTURES; count++)
    {
        appEglWindowResetTex(&obj->texRender[count]);
    }

    obj->get_platform_display = (void *) eglGetProcAddress("eglGetPlatformDisplayEXT");
    if(!obj->get_platform_display)
    {
        printf("EGL: ERROR: eglGetProcAddress(\"eglGetPlatformDisplayEXT\") failed !!!\n");
        goto destroy_gbm_surface;
    }

    egl_platform_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

    if (has_extension(egl_platform_extensions, "EGL_MESA_platform_surfaceless"))
    {
        obj->display = obj->get_platform_display(EGL_PLATFORM_SURFACELESS_MESA,
                EGL_DEFAULT_DISPLAY, NULL);
    }
    else
    {
        obj->drm_fd = open("/dev/dri/by-path/platform-4a00000.dss-card", O_RDWR);
        if(obj->drm_fd < 0)
        {
            printf("EGL: ERROR: drmOpen() failed !!!\n");
            goto goto_error;
        }

        obj->gbm_dev = gbm_create_device(obj->drm_fd);
        if(!obj->gbm_dev)
        {
            printf("EGL: ERROR: gbm_create_device() failed !!!\n");
            goto close_fd;
        }

        obj->gbm_surface = gbm_surface_create(obj->gbm_dev,
                1920, 1080, GBM_FORMAT_XRGB8888,
                GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
        if(!obj->gbm_surface)
        {
            printf("EGL: ERROR: gbm_urface_create() failed !!!\n");
            goto destroy_gbm_device;
        }


        obj->create_platform_window_surface = (void *) eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
        if(!obj->create_platform_window_surface)
        {
            printf("EGL: ERROR: eglGetProcAddress(\"eglCreatePlatformWindowSurfaceEXT\") failed !!!\n");
            goto destroy_gbm_surface;
        }

        obj->display = obj->get_platform_display(EGL_PLATFORM_GBM_KHR, obj->gbm_dev, NULL);
        appEglCheckEglError("eglGetDisplay", EGL_TRUE);
        if (obj->display == EGL_NO_DISPLAY)
        {
            printf("EGL: ERROR: eglGetDisplay() returned EGL_NO_DISPLAY !!!\n");
            goto destroy_gbm_surface;
        }
    }

    ret = eglInitialize(obj->display, &majorVersion, &minorVersion);
    appEglCheckEglError("eglInitialize", ret);
    if (ret != EGL_TRUE)
    {
        printf("EGL: ERROR: eglInitialize() failed !!!\n");
        goto terminate_display;
    }
    printf("EGL: version %d.%d\n", majorVersion, minorVersion);

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        printf("EGL: ERROR: eglBindAPI(EGL_OPENGL_ES_API) failed !!!\n");
        goto terminate_display;
    }

    if (obj->gbm_surface)
    {
        if (!eglChooseConfig(obj->display, attribs, &obj->config, 1, &num_configs))
        {
            printf("EGL: eglChooseConfig() failed. Couldn't get an EGL visual config !!!\n");
            goto terminate_display;
        }

        obj->context = eglCreateContext(obj->display, obj->config, EGL_NO_CONTEXT, context_attribs);
    }
    else
    {
        /* with surfaceless platform, there is no need for config */
        obj->context = eglCreateContext(obj->display, EGL_NO_CONFIG_KHR,
                EGL_NO_CONTEXT, context_attribs);
    }
    appEglCheckEglError("eglCreateContext", EGL_TRUE);
    if (obj->context == EGL_NO_CONTEXT)
    {
        printf("EGL: ERROR: eglCreateContext() failed !!!\n");
        goto terminate_display;
    }

    appEglPrintGLString("Version", GL_VERSION);
    appEglPrintGLString("Vendor", GL_VENDOR);
    appEglPrintGLString("Renderer", GL_RENDERER);
    appEglPrintGLString("Extensions", GL_EXTENSIONS);

    if (obj->gbm_surface)
    {
        obj->surface = obj->create_platform_window_surface(obj->display, obj->config, obj->gbm_surface, NULL);
        appEglCheckEglError("eglCreateWindowSurface", EGL_TRUE);
        if (obj->surface == EGL_NO_SURFACE)
        {
            printf("EGL: ERROR: eglCreateWindowSurface() failed !!!\n");
            goto destroy_context;
        }
    }

    ret = eglMakeCurrent(obj->display, obj->surface,
                        obj->surface, obj->context);
    appEglCheckEglError("eglMakeCurrent", ret);
    if (ret != EGL_TRUE)
    {
        printf("EGL: ERROR: eglMakeCurrent() failed !!!\n");
        goto destroy_surface;
    }

    return obj;

destroy_surface:
    if (obj->surface != EGL_NO_SURFACE)
        eglDestroySurface(obj->display, obj->surface);
destroy_context:
    eglDestroyContext(obj->display, obj->context);
terminate_display:
    eglTerminate(obj->display);
destroy_gbm_surface:
    if (obj->gbm_surface)
        gbm_surface_destroy(obj->gbm_surface);
destroy_gbm_device:
    if (obj->gbm_dev)
        gbm_device_destroy(obj->gbm_dev);
close_fd:
    if (obj->drm_fd != -1)
        close(obj->drm_fd);
goto_error:
    if (obj)
        free(obj);

    return NULL;
}

void appEglSwap(void *eglWindow)
{
    app_egl_obj_t *obj = (app_egl_obj_t*)eglWindow;

    if(obj)
    {
        eglSwapBuffers(obj->display, obj->surface);
        appEglCheckGlError("eglSwapBuffers");

        if (obj->gbm_surface)
        {
            struct gbm_bo *bo = gbm_surface_lock_front_buffer(obj->gbm_surface);
            gbm_surface_release_buffer(obj->gbm_surface, bo);
        }
    }
}

static EGLImageKHR appEglWindowCreateIMG(app_egl_obj_t *obj,
        app_egl_tex_prop_t *prop)
{
    int32_t attrIdx = 0;
    EGLImageKHR image;
    EGLint attr[32];
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;

    uint32_t num_planes = 1;

    if(prop->dataFormat==APP_EGL_DF_NV12)
    {
        num_planes = 2;
    }

    attr[attrIdx++] = EGL_LINUX_DRM_FOURCC_EXT;
    if (prop->dataFormat == APP_EGL_DF_NV12)
        attr[attrIdx++] = FOURCC_STR("NV12");
    else if (prop->dataFormat == APP_EGL_DF_YUYV)
        attr[attrIdx++] = FOURCC_STR("YUYV");
    else if (prop->dataFormat == APP_EGL_DF_UYVY)
        attr[attrIdx++] = FOURCC_STR("UYVY");
    else
        attr[attrIdx++] = FOURCC_STR("AB24");

    attr[attrIdx++] = EGL_WIDTH;
    attr[attrIdx++] = prop->width;

    attr[attrIdx++] = EGL_HEIGHT;
    attr[attrIdx++] = prop->height;

    attr[attrIdx++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attr[attrIdx++] = prop->pitch[0];

    if (num_planes>1)
    {
        attr[attrIdx++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attr[attrIdx++] = prop->pitch[1];
    }

    attr[attrIdx++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attr[attrIdx++] = prop->dmaBufFdOffset[0];

    if (num_planes>1)
    {
        attr[attrIdx++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attr[attrIdx++] = prop->dmaBufFdOffset[1];
    }

    attr[attrIdx++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attr[attrIdx++] = prop->dmaBufFd[0];

    if (num_planes>1)
    {
        attr[attrIdx++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attr[attrIdx++] = prop->dmaBufFd[1];
    }

    attr[attrIdx++] = EGL_NONE;

    eglCreateImageKHR =
        (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");

    EGLDisplay disp = eglGetCurrentDisplay();
    appEglCheckEglError("eglGetCurrentDisplay()", EGL_TRUE);
    image = eglCreateImageKHR(disp, EGL_NO_CONTEXT,
                                EGL_LINUX_DMA_BUF_EXT, NULL, attr);
    appEglCheckEglError("eglCreateImageKHR", EGL_TRUE);

    if (image == EGL_NO_IMAGE_KHR) {
        printf("EGL: ERROR: eglCreateImageKHR failed !!!\n");
    }

    return image;
}

/* target_type should be one of
 *   - GL_TEXTURE_2D
 *   - GL_TEXTURE_EXTERNAL_OES
 */
static GLuint appEglWindowCreateTexture(GLint target_type)
{
    GLuint texId;

    glGenTextures(1, &texId);
    appEglCheckGlError("glGenTextures");

    glBindTexture(target_type, texId);
    appEglCheckGlError("glBindTexture");

    glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    appEglCheckGlError("glTexParameteri");

    return texId;
}

static int32_t appEglWindowSetupRenderTex(app_egl_obj_t *obj,
                        app_egl_tex_prop_t *prop, int32_t index)
{
    int32_t status = 0;
    app_egl_tex_obj_t *tex_obj;

    tex_obj = &obj->texRender[index];

    appEglWindowResetTex(tex_obj);

    tex_obj->img = appEglWindowCreateIMG(obj, prop);
    if (tex_obj->img == EGL_NO_IMAGE_KHR) {
        printf("EGL: ERROR: appEglWindowCreateIMG failed !!!\n");
        status = -1;
    }

    if(status==0)
    {
        tex_obj->tex = appEglWindowCreateTexture(GL_TEXTURE_2D);

        glGenFramebuffers(1, &tex_obj->fboId);
        appEglCheckEglError("glGenFramebuffers", EGL_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, tex_obj->fboId);
        appEglCheckEglError("glBindFramebuffer", EGL_TRUE);

        GLuint rboDepthStencil;
        glGenRenderbuffers(1, &rboDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, prop->width, prop->height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

        tex_obj->dmaBufFd = prop->dmaBufFd[0];
        tex_obj->dmaBufFdOffset = prop->dmaBufFdOffset[0];
        tex_obj->isAlloc = 1;
    }

    return status;
}

static int32_t appEglWindowSetupTex(app_egl_obj_t *obj, app_egl_tex_prop_t *prop, int32_t index)
{
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
    int32_t status = 0;
    app_egl_tex_obj_t *tex_obj;

    tex_obj = &obj->tex[index];

    appEglWindowResetTex(tex_obj);

    glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

    tex_obj->img = appEglWindowCreateIMG(obj, prop);
    if (tex_obj->img == EGL_NO_IMAGE_KHR) {
        printf("EGL: ERROR: appEglWindowCreateIMG failed !!!\n");
        status = -1;
    }

    if(status==0)
    {
        tex_obj->tex = appEglWindowCreateTexture(GL_TEXTURE_EXTERNAL_OES);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex_obj->tex);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)tex_obj->img);
        appEglCheckGlError("glEGLImageTargetTexture2DOES");

        tex_obj->dmaBufFd = prop->dmaBufFd[0];
        tex_obj->dmaBufFdOffset = prop->dmaBufFdOffset[0];
        tex_obj->isAlloc = 1;
    }

    return status;
}

/* Creates texture and eglimage objects
 */
void appEglBindFrameBuffer(void *eglWindow, app_egl_tex_prop_t *prop)
{
    int32_t texFound;
    int32_t texIndex;
    uint32_t i;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
    PFNGLCLIPCONTROLEXTPROC glClipControlEXT;
    app_egl_obj_t *obj = (app_egl_obj_t*)eglWindow;
    app_egl_tex_obj_t *tex_obj;

    glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    glClipControlEXT =
            (PFNGLCLIPCONTROLEXTPROC)eglGetProcAddress("glClipControlEXT");

    texIndex = -1;
    texFound = 0;

    for(i = 0; i < APP_EGL_MAX_RENDER_TEXTURES; i++)
    {
        tex_obj = &obj->texRender[i];

        if(tex_obj->isAlloc
            && tex_obj->dmaBufFd == prop->dmaBufFd[0]
            && tex_obj->dmaBufFdOffset == prop->dmaBufFdOffset[0]
            )
        {
            texIndex = i;
            texFound = 1;
            break;
        }
    }

    if(!texFound)
    {
        /* find free slot amd create texture */
        for(i = 0; i < APP_EGL_MAX_RENDER_TEXTURES; i++)
        {
            tex_obj = &obj->texRender[i];

            if(!tex_obj->isAlloc)
            {
                int32_t status;

                status = appEglWindowSetupRenderTex(obj, prop, i);
                if(status==0)
                {
                    texIndex = i;
                    texFound = 1;
                }
                break;
            }
        }
    }

    if(texFound)
    {
        tex_obj = &obj->texRender[texIndex];

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_obj->tex);
        appEglCheckEglError("glBindTexture", EGL_TRUE);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)tex_obj->img);
        appEglCheckEglError("glEGLImageTargetTexture2DOES", EGL_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, tex_obj->fboId);
        appEglCheckEglError("glBindFramebuffer", EGL_TRUE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, tex_obj->tex, 0);
        appEglCheckEglError("glFramebufferTexture2D", EGL_TRUE);

        GLenum fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fbstatus != GL_FRAMEBUFFER_COMPLETE)
            printf("EGL: ERROR: Frambuffer complete check failed 0x%x\n", fbstatus);
    }
    /* Binding FBO: move the origin to upper left */
    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
}

GLuint appEglWindowGetTexYuv(void *eglWindow, app_egl_tex_prop_t *prop)
{
    GLuint tex;
    int32_t texFound;
    uint32_t i;
    app_egl_obj_t *obj = (app_egl_obj_t*)eglWindow;
    app_egl_tex_obj_t *tex_obj;

    tex = 0;
    texFound = 0;

    for(i = 0; i < APP_EGL_MAX_TEXTURES; i++)
    {
        tex_obj = &obj->tex[i];

        if(tex_obj->isAlloc
            && tex_obj->dmaBufFd == prop->dmaBufFd[0]
            && tex_obj->dmaBufFdOffset == prop->dmaBufFdOffset[0]
            )
        {
            tex = tex_obj->tex;
            texFound = 1;
            break;
        }
    }

    if(!texFound)
    {
        /* find free slot amd create texture */
        for(i = 0; i < APP_EGL_MAX_TEXTURES; i++)
        {
            tex_obj = &obj->tex[i];

            if(!tex_obj->isAlloc)
            {
                int32_t status;

                status = appEglWindowSetupTex(obj, prop, i);
                if(status==0)
                {
                    tex = tex_obj->tex;
                }
                break;
            }
        }
    }

    return tex;
}

static void appEglWindowDestroyTex(app_egl_obj_t *obj, app_egl_tex_obj_t *tex_obj)
{
    if(tex_obj->tex!=0)
    {
        glDeleteTextures(1, &tex_obj->tex);
        tex_obj->tex = 0;
    }
    if (tex_obj->img != 0)
    {
        PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
        eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

        eglDestroyImageKHR(obj->display, tex_obj->img);
        tex_obj->img = 0;
    }
}

int32_t appEglWindowClose(void *eglWindow)
{
    uint32_t count;

    app_egl_obj_t *obj = (app_egl_obj_t*)eglWindow;

    if(obj==NULL)
    {
        return -1;
    }

    eglMakeCurrent(obj->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    for (count = 0; count < APP_EGL_MAX_RENDER_TEXTURES; count++)
    {
        appEglWindowDestroyTex(obj, &obj->texRender[count]);
    }
    for (count = 0; count < APP_EGL_MAX_TEXTURES; count++)
    {
        appEglWindowDestroyTex(obj, &obj->tex[count]);
    }

    if (obj->surface != EGL_NO_SURFACE)
        eglDestroySurface(obj->display, obj->surface);
    eglDestroyContext(obj->display, obj->context);
    eglTerminate(obj->display);

    if (obj->gbm_surface)
    {
        gbm_surface_destroy(obj->gbm_surface);
        gbm_device_destroy(obj->gbm_dev);
        close(obj->drm_fd);
    }

    free(obj);

    return 0;
}
