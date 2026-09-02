/*
 *
 * Copyright (c) 2025-2026 Texas Instruments Incorporated
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

#ifndef GRAPHICS_GL_ERROR_H_
#define GRAPHICS_GL_ERROR_H_

// Graphics library includes
#include <EGL/egl.h>
#include <GLES3/gl32.h>

// Local includes
#include <utils/include/logger.h>

namespace ti::graphics
{

/**
 * @brief Check for OpenGL errors and log them
 * @param file Source file where check is performed
 * @param line Line number where check is performed
 * @param func Function name where check is performed
 * @return true if no errors, false if errors found
 */
inline bool checkGLErrors(const char* file, int line, const char* func)
{
    bool   success = true;
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        success = false;
        const char* errorStr;
        switch (error)
        {
        case GL_INVALID_ENUM:
            errorStr = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorStr = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorStr = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorStr = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorStr = "UNKNOWN";
            break;
        }
        LOG_ERROR("OpenGL error in %s at %s:%d: %s (0x%x)", func, file, line, errorStr, error);
    }
    return success;
}

/**
 * @brief Check for EGL errors and log them
 * @param file Source file where check is performed
 * @param line Line number where check is performed
 * @param func Function name where check is performed
 * @return true if no errors, false if errors found
 */
inline bool checkEGLErrors(const char* file, int line, const char* func)
{
    bool   success = true;
    EGLint error;
    while ((error = eglGetError()) != EGL_SUCCESS)
    {
        success = false;
        const char* errorStr;
        switch (error)
        {
        case EGL_NOT_INITIALIZED:
            errorStr = "EGL_NOT_INITIALIZED";
            break;
        case EGL_BAD_ACCESS:
            errorStr = "EGL_BAD_ACCESS";
            break;
        case EGL_BAD_ALLOC:
            errorStr = "EGL_BAD_ALLOC";
            break;
        case EGL_BAD_ATTRIBUTE:
            errorStr = "EGL_BAD_ATTRIBUTE";
            break;
        case EGL_BAD_CONFIG:
            errorStr = "EGL_BAD_CONFIG";
            break;
        case EGL_BAD_CONTEXT:
            errorStr = "EGL_BAD_CONTEXT";
            break;
        case EGL_BAD_CURRENT_SURFACE:
            errorStr = "EGL_BAD_CURRENT_SURFACE";
            break;
        case EGL_BAD_DISPLAY:
            errorStr = "EGL_BAD_DISPLAY";
            break;
        case EGL_BAD_MATCH:
            errorStr = "EGL_BAD_MATCH";
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            errorStr = "EGL_BAD_NATIVE_PIXMAP";
            break;
        case EGL_BAD_NATIVE_WINDOW:
            errorStr = "EGL_BAD_NATIVE_WINDOW";
            break;
        case EGL_BAD_PARAMETER:
            errorStr = "EGL_BAD_PARAMETER";
            break;
        case EGL_BAD_SURFACE:
            errorStr = "EGL_BAD_SURFACE";
            break;
        default:
            errorStr = "UNKNOWN";
            break;
        }
        LOG_ERROR("EGL error in %s at %s:%d: %s (0x%x)", func, file, line, errorStr, error);
    }
    return success;
}

// Convenience macros for error checking
#define CHECK_GL_ERRORS() checkGLErrors(__FILE__, __LINE__, __func__)
#define CHECK_EGL_ERRORS() checkEGLErrors(__FILE__, __LINE__, __func__)

} // namespace ti::graphics

#endif // GRAPHICS_GL_ERROR_H_
