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

#include <stdlib.h>
#include <stdio.h>

#ifndef x86_64
#include <screen/screen.h>
#include <GLES3/gl3.h>
#include <errno.h>
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
    void* bufAddr[APP_EGL_TEX_MAX_PLANES];
    screen_pixmap_t screen_pix; // Note: is this the same thing as EGLPixmap...
    /**< Screen pixmap (QNX) */
    screen_buffer_t screen_pbuf;
    /**< Screen buffer (QNX) */
} app_egl_tex_obj_t;

typedef struct
{
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    screen_context_t screen_ctx;         /* connection to screen windowing system */
    screen_window_t screen_win;          /* native handle for our window */

    app_egl_tex_obj_t texRender[APP_EGL_MAX_TEXTURES];
    app_egl_tex_obj_t tex[APP_EGL_MAX_TEXTURES];

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

static void appEglWindowResetTex(app_egl_tex_obj_t *tex_obj)
{
    int i;

    tex_obj->isAlloc = 0;
    tex_obj->img = 0;
    tex_obj->tex = 0;
    tex_obj->fboId = -1;
    tex_obj->dmaBufFd = -1;
    tex_obj->dmaBufFdOffset = 0;
    tex_obj->screen_pix = NULL;
    tex_obj->screen_pbuf = NULL;

    for (i = 0; i < APP_EGL_TEX_MAX_PLANES; i++)
    {
        tex_obj->bufAddr[i] = NULL;
    }
}

static int choose_format(EGLDisplay egl_disp, EGLConfig egl_conf)
{
    EGLint buffer_bit_depth, alpha_bit_depth;

    eglGetConfigAttrib(egl_disp, egl_conf, EGL_BUFFER_SIZE, &buffer_bit_depth);
    eglGetConfigAttrib(egl_disp, egl_conf, EGL_ALPHA_SIZE, &alpha_bit_depth);
    switch (buffer_bit_depth) {
        case 32: {
            return SCREEN_FORMAT_RGBA8888;
        }
        case 24: {
            return SCREEN_FORMAT_RGB888;
        }
        case 16: {
            switch (alpha_bit_depth) {
                case 4: {
                    return SCREEN_FORMAT_RGBA4444;
                }
                case 1: {
                    return SCREEN_FORMAT_RGBA5551;
                }
                default: {
                    return SCREEN_FORMAT_RGB565;
                }
            }
            break;
        }
        default: {
            return SCREEN_FORMAT_BYTE;
        }
    }
}

void *appEglWindowOpen()
{
    EGLint num_configs;
    EGLint majorVersion;
    EGLint minorVersion;
    int32_t ret;
    uint32_t count;

#if 1
    EGLint interval = 1;                 /* EGL swap interval */
    int format;                          /* native visual type / screen format */
    int usage = SCREEN_USAGE_OPENGL_ES3; /* we will use OpenGL ES 3.X to do our rendering */
    int size[2] = { 1920, 1080 };            /* width and height of our window */
    int pos[2] = { 0, 0 };               /* x,y position of our window */
    int nbuffers = 2;                    /* number of buffers backing the window */
#endif

    const EGLint attribs[] = {
       EGL_RED_SIZE, 8,
       EGL_GREEN_SIZE, 8,
       EGL_BLUE_SIZE, 8,
       EGL_ALPHA_SIZE, 8,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
       EGL_DEPTH_SIZE, 8,
       EGL_NONE
    };

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    app_egl_obj_t *obj;

    obj = malloc(sizeof(app_egl_obj_t));
    if(obj==NULL)
    {
        goto goto_error;
    }

    for(count=0; count < APP_EGL_MAX_TEXTURES; count++)
    {
        appEglWindowResetTex(&obj->tex[count]);
    }
    for(count=0; count < APP_EGL_MAX_RENDER_TEXTURES; count++)
    {
        appEglWindowResetTex(&obj->texRender[count]);
    }

    obj->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    appEglCheckEglError("eglGetDisplay", EGL_TRUE);
    if (obj->display == EGL_NO_DISPLAY) {
       printf("EGL: ERROR: eglGetDisplay() returned EGL_NO_DISPLAY !!!\n");
       goto goto_error;
    }

    ret = eglInitialize(obj->display, &majorVersion, &minorVersion);
    appEglCheckEglError("eglInitialize", ret);
    if (ret != EGL_TRUE) {
        printf("EGL: ERROR: eglInitialize() failed !!!\n");
        goto goto_error;
    }

    printf("EGL: version %d.%d\n", majorVersion, minorVersion);

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        printf("EGL: ERROR: eglBindAPI(EGL_OPENGL_ES_API) failed !!!\n");
        goto goto_error;
    }

    if (!eglChooseConfig(obj->display, attribs, &obj->config, 1, &num_configs))
    {
        printf("EGL: eglChooseConfig() failed. Couldn't get an EGL visual config !!!\n");
        goto goto_error;
    }

    obj->context = eglCreateContext(obj->display, obj->config, EGL_NO_CONTEXT, context_attribs);
    appEglCheckEglError("eglCreateContext", EGL_TRUE);
    if (obj->context == EGL_NO_CONTEXT) {
        printf("EGL: ERROR: eglCreateContext() failed !!!\n");
        goto goto_error;
    }

    ret = screen_create_context(&obj->screen_ctx, SCREEN_BUFFER_PROVIDER_CONTEXT);
    if (ret) {
        printf("screen_context_create failed\n");
        goto goto_error;
    }

    ret = screen_create_window(&obj->screen_win, obj->screen_ctx);
    if (ret) {
        printf("screen_create_window failed\n");
        goto goto_error;
    }

    format = choose_format(obj->display, obj->config);
    ret = screen_set_window_property_iv(obj->screen_win, SCREEN_PROPERTY_FORMAT, &format);
    if (ret) {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT) failed\n");
        goto goto_error;
    }

    ret = screen_set_window_property_iv(obj->screen_win, SCREEN_PROPERTY_USAGE, &usage);
    if (ret) {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed\n");
        goto goto_error;
    }

    ret = screen_set_window_property_iv(obj->screen_win, SCREEN_PROPERTY_SWAP_INTERVAL, &interval);
    if (ret) {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_SWAP_INTERVAL) failed\n");
        goto goto_error;
    }

    if (size[0] > 0 && size[1] > 0) {
        ret = screen_set_window_property_iv(obj->screen_win, SCREEN_PROPERTY_SIZE, size);
        if (ret) {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) failed\n");
            goto goto_error;
        }
    } else {
        ret = screen_get_window_property_iv(obj->screen_win, SCREEN_PROPERTY_SIZE, size);
        if (ret) {
            printf("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE) failed\n");
            goto goto_error;
        }
    }

    if (pos[0] != 0 || pos[1] != 0) {
        ret = screen_set_window_property_iv(obj->screen_win, SCREEN_PROPERTY_POSITION, pos);
        if (ret) {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_POSITION) failed\n");
            goto goto_error;
        }
    }

    ret = screen_create_window_buffers(obj->screen_win, nbuffers);
    if (ret) {
        printf("screen_create_window_buffers");
        goto goto_error;
    }

    obj->surface = eglCreateWindowSurface(obj->display, obj->config, (EGLNativeWindowType)obj->screen_win, NULL);
    appEglCheckEglError("eglCreateWindowSurface", EGL_TRUE);

    ret = eglMakeCurrent(obj->display, obj->surface,
                        obj->surface, obj->context);
    appEglCheckEglError("eglMakeCurrent", ret);
    if (ret != EGL_TRUE)
    {
        printf("EGL: ERROR: eglMakeCurrent() failed !!!\n");
        goto goto_error;
    }

    return obj;
goto_error:
    if(obj)
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
    }

    return;
}

static int screen_buf_attach (app_egl_obj_t *obj, app_egl_tex_obj_t *tex_obj,
                        app_egl_tex_prop_t *prop)
{
    int rc, val, stride, format, offset[3];
    int size[2],buf_size;

    if (prop->bufAddr[0] == NULL) {
        printf("bufAddr0 is NULL\n");
        return -1;
    }

    if ((prop->dataFormat == APP_EGL_DF_NV12) &&
        (prop->bufAddr[1] == NULL)) {
        printf("bufAddr1 is NULL\n");
        return -1;
    }

    rc = screen_create_pixmap(&tex_obj->screen_pix, obj->screen_ctx);
    if (rc) {
        printf("screen_create_pixmap failed\n");
        return -1;
    }

    val = SCREEN_USAGE_OPENGL_ES3 | SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE | SCREEN_USAGE_READ ;
    rc = screen_set_pixmap_property_iv(tex_obj->screen_pix, SCREEN_PROPERTY_USAGE, &val);
    if (rc) {
        printf("screen_set_pixmap_property_iv(SCREEN_PROPERTY_USAGE) failed\n");
        goto fail4;
    }

    size[0] = prop->width;
    size[1] = prop->height;

    switch(prop->dataFormat) {
        case APP_EGL_DF_NV12:
            format = SCREEN_FORMAT_NV12;
            stride = prop->pitch[0];
            buf_size = (stride * prop->height * 3) / 2;
            size[0] = stride;
            size[1] = prop->height;
            break;
        case APP_EGL_DF_RGB:
            format = SCREEN_FORMAT_RGB888;
            stride = prop->pitch[0];
            buf_size = (stride * prop->height);
            break;
        case APP_EGL_DF_RGBX:
            format = SCREEN_FORMAT_RGBA8888;
            stride = prop->pitch[0];
            buf_size = (stride * prop->height);
            break;
        case APP_EGL_DF_YUYV:
            format = SCREEN_FORMAT_YUY2;
            stride = prop->pitch[0];
            buf_size = (stride * prop->height);
            break;
        case APP_EGL_DF_UYVY:
            format = SCREEN_FORMAT_UYVY;
            stride = prop->pitch[0];
            buf_size = (stride * prop->height);
            break;
        default:
            printf("Couldn't find format %d\n", prop->dataFormat);
            goto fail4;
    }

    rc = screen_set_pixmap_property_iv(tex_obj->screen_pix, SCREEN_PROPERTY_FORMAT, &format);
    if (rc) {
        printf("Error: screen_set_pixmap_property_iv(SCREEN_PROPERTY_FORMAT) failed\n");
        goto fail4;
    }

    rc = screen_set_pixmap_property_iv(tex_obj->screen_pix, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) {
        printf("Error: screen_set_pixmap_property_iv(SCREEN_PROPERTY_BUFFER_SIZE) failed\n");
        goto fail4;
    }

    rc = screen_create_buffer(&tex_obj->screen_pbuf);
    if (rc) {
        printf("Error: screen_create_buffer failed\n");
        goto fail4;
    }

    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_SIZE, &buf_size);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_SIZE)\n");
        goto fail4;
    }

    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_BUFFER_SIZE)\n");
        goto fail4;
    }

    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_FORMAT, &format);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_FORMAT)\n");
        goto fail4;
    }

    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_STRIDE, &stride);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_STRIDE)\n");
        goto fail4;
    }

    val = 1;
    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_PHYSICALLY_CONTIGUOUS, &val);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_PHYSICALLY_CONTIGUOUS)\n");
        goto fail4;
    }

    offset[0] = 0;
    offset[1] = stride * prop->height;
    offset[2] = 0;

    if (NULL != prop->bufAddr[1])
    {
        offset[1] = prop->bufAddr[1] - prop->bufAddr[0];
    }

    rc = screen_set_buffer_property_iv(tex_obj->screen_pbuf, SCREEN_PROPERTY_PLANAR_OFFSETS, offset);
    if (rc) {
        printf("Error: screen_set_buffer_property_iv(SCREEN_PROPERTY_PLANAR_OFFSETS)\n");
        goto fail4;
    }

    rc = screen_set_buffer_property_pv(tex_obj->screen_pbuf, SCREEN_PROPERTY_POINTER, (void **)&prop->bufAddr[0]);
    if (rc) {
        printf("Error: screen_set_buffer_property_pv(SCREEN_PROPERTY_POINTER)\n");
        goto fail4;
    }

    rc = screen_attach_pixmap_buffer(tex_obj->screen_pix, tex_obj->screen_pbuf);
    if (rc) {
        printf("Error: screen_attach_pixmap_buffer failed %d\n", errno);
        goto fail4;
    }

    return 0;
fail4:
    if (tex_obj->screen_pix)
        screen_destroy_pixmap(tex_obj->screen_pix);
    return -1;
}

static EGLImageKHR appEglWindowCreateIMG(app_egl_obj_t *obj,
        app_egl_tex_prop_t *prop, screen_pixmap_t screen_pix)
{
    EGLImageKHR image;
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;

    eglCreateImageKHR =
        (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");

    EGLDisplay disp = eglGetCurrentDisplay();
    appEglCheckEglError("eglGetCurrentDisplay()", EGL_TRUE);

    image = eglCreateImageKHR(
                                disp,
                                EGL_NO_CONTEXT,
                                EGL_NATIVE_PIXMAP_KHR,
                                (EGLClientBuffer)(screen_pix),
                                NULL
                              );
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

    status = screen_buf_attach(obj, tex_obj, prop);

    if (0 == status)
    {
        tex_obj->img = appEglWindowCreateIMG(obj, prop, tex_obj->screen_pix);
        if (tex_obj->img == EGL_NO_IMAGE_KHR) {
            printf("EGL: ERROR: appEglWindowCreateIMG failed !!!\n");
            status = -1;
        } 
    }
    else
    {
        printf("screen_buf_attach failed!\n");
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


        tex_obj->bufAddr[0] = prop->bufAddr[0];
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

    status = screen_buf_attach(obj, tex_obj, prop);

    if (0 == status)
    {
        tex_obj->img = appEglWindowCreateIMG(obj, prop, tex_obj->screen_pix);
        if (tex_obj->img == EGL_NO_IMAGE_KHR) {
            printf("EGL: ERROR: appEglWindowCreateIMG failed !!!\n");
            status = -1;
        }
    }
    else
    {
        printf("screen_buf_attach failed!\n");
    }

    if(status==0)
    {
        tex_obj->tex = appEglWindowCreateTexture(GL_TEXTURE_EXTERNAL_OES);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex_obj->tex);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)tex_obj->img);
        appEglCheckGlError("glEGLImageTargetTexture2DOES");

        tex_obj->bufAddr[0] = prop->bufAddr[0];
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
    // TODO: Note: I am including the newer ext.h file
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
            && (tex_obj->bufAddr[0] == prop->bufAddr[0])
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
            && (tex_obj->bufAddr[0] == prop->bufAddr[0])
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

        screen_destroy_buffer(tex_obj->screen_pbuf);
        tex_obj->screen_pbuf = NULL;

        screen_destroy_pixmap(tex_obj->screen_pix);
        tex_obj->screen_pix = NULL;
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

    eglDestroyContext(obj->display, obj->context);
    eglTerminate(obj->display);

    screen_destroy_window(obj->screen_win);
    screen_destroy_context(obj->screen_ctx);

    free(obj);

    return 0;
}
