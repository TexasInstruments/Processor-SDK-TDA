/*
 *
 * Copyright (c) 2025-2026 Texas Instruments Incorporated
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

#include <algorithm>

#include <graphics/include/Renderer.h>

namespace ti::graphics
{

Renderer::Renderer(uint32_t width, uint32_t height)
: m_width(width), m_height(height)
{
    LOG_DEBUG("Renderer created with viewport %ux%u", width, height);
}

Renderer::~Renderer()
{
    LOG_DEBUG("Renderer destroyed");
}

int32_t Renderer::initialize()
{
    int32_t status = 0;

    LOG_DEBUG("Initializing renderer...");

    if (status == 0)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glViewport(0, 0, m_width, m_height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        status = checkGLError("Renderer initialization");
    }

    return status;
}

void Renderer::beginFrame()
{
    clear();
}

int32_t Renderer::endFrame()
{
    int32_t status = 0;

    status = checkGLError("Frame end");

    return status;
}


int32_t Renderer::render()
{
    int32_t status = 0;

    // Update view-projection matrix
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

    // Render all visible objects
    for (auto &renderable : m_renderables)
    {
        if (renderable && renderable->isVisible())
        {
            renderable->render(m_viewProjectionMatrix);
        }
    }

    status = checkGLError("Render");

    return status;
}

int32_t Renderer::addRenderable(std::shared_ptr<Renderable> renderable)
{
    int32_t status = 0;

    if (renderable)
    {
        m_renderables.push_back(renderable);
        LOG_DEBUG("Added renderable to renderer");
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t Renderer::removeRenderable(std::shared_ptr<Renderable> renderable)
{
    int32_t status = 0;
    auto it = std::find(m_renderables.begin(), m_renderables.end(), renderable);

    if (it != m_renderables.end())
    {
        (*it)->cleanup();
        m_renderables.erase(it);
        LOG_DEBUG("Removed renderable from renderer");
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t Renderer::setViewport(uint32_t width, uint32_t height)
{
    int32_t status = 0;

    m_width  = width;
    m_height = height;
    glViewport(0, 0, width, height);
    LOG_DEBUG("Viewport resized to %ux%u", width, height);

    return status;
}

int32_t Renderer::setViewMatrix(const glm::mat4 &view)
{
    int32_t status = 0;

    m_viewMatrix = view;

    return status;
}

int32_t Renderer::setProjectionMatrix(const glm::mat4 &projection)
{
    int32_t status = 0;

    m_projectionMatrix = projection;

    return status;
}

int32_t Renderer::setDepthTest(bool enable)
{
    int32_t status = 0;

    if (enable != m_depthTestEnabled)
    {
        if (enable)
        {
            glEnable(GL_DEPTH_TEST);
            LOG_DEBUG("Depth testing enabled");
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
            LOG_DEBUG("Depth testing disabled");
        }

        m_depthTestEnabled = enable;
    }

    return status;
}

int32_t Renderer::setBlending(bool enable)
{
    int32_t status = 0;

    if (enable != m_blendingEnabled)
    {
        if (enable)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            LOG_DEBUG("Blending enabled");
        }
        else
        {
            glDisable(GL_BLEND);
            LOG_DEBUG("Blending disabled");
        }

        m_blendingEnabled = enable;
    }

    return status;
}

int32_t Renderer::clear(bool color, bool depth, bool stencil)
{
    int32_t    status = 0;
    GLbitfield mask   = 0;

    if (color)
    {
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (depth)
    {
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (stencil)
    {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);

    return status;
}

int32_t Renderer::checkGLError(const char *location)
{
    int32_t status = 0;
    GLenum  error  = glGetError();

    if (error != GL_NO_ERROR)
    {
        const char *errorString;

        switch (error)
        {
            case GL_INVALID_ENUM:
                errorString = "GL_INVALID_ENUM";
                break;

            case GL_INVALID_VALUE:
                errorString = "GL_INVALID_VALUE";
                break;

            case GL_INVALID_OPERATION:
                errorString = "GL_INVALID_OPERATION";
                break;

            case GL_OUT_OF_MEMORY:
                errorString = "GL_OUT_OF_MEMORY";
                break;

            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;

            default:
                errorString = "UNKNOWN";
                break;
        }

        LOG_ERROR("OpenGL error at %s: %s (0x%x)", location, errorString, error);
        status = -1;
    }

    return status;
}

} // namespace ti::graphics
