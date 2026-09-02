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

#ifndef GRAPHICS_RENDERER_H_
#define GRAPHICS_RENDERER_H_

// C++ includes
#include <memory>
#include <vector>

// Graphics library includes
#include <GLES3/gl32.h>

// Local includes
#include <graphics/include/Renderable.h>
#include <utils/include/logger.h>

namespace ti::graphics
{

/**
 * @brief Core renderer class managing OpenGL ES 3.1 rendering pipeline
 *
 * This class manages the graphics context, rendering state, and coordinates
 * the rendering of all Renderable objects. It provides a robust rendering
 * pipeline with support for various rendering techniques and optimizations.
 */
class Renderer
{
  public:
    /**
     * @brief Constructor
     * @param width Initial viewport width
     * @param height Initial viewport height
     */
    Renderer(uint32_t width, uint32_t height);

    /**
     * @brief Destructor
     */
    ~Renderer();

    /**
     * @brief Initialize the renderer
     * @return 0 on success, negative error code on failure
     */
    int32_t initialize();

    /**
     * @brief Begin a new frame
     */
    void beginFrame();

    /**
     * @brief End the current frame and swap buffers
     * @return 0 on success, negative error code on failure
     */
    int32_t endFrame();

    /**
     * @brief Render all visible renderables
     * @return 0 on success, negative error code on failure
     */
    int32_t render();

    /**
     * @brief Add a renderable object to the renderer
     * @param renderable Shared pointer to the renderable object
     * @return 0 on success, negative error code on failure
     */
    int32_t addRenderable(std::shared_ptr<Renderable> renderable);

    /**
     * @brief Remove a renderable object from the renderer
     * @param renderable Shared pointer to the renderable to remove
     * @return 0 on success, negative error code on failure
     */
    int32_t removeRenderable(std::shared_ptr<Renderable> renderable);

    /**
     * @brief Set the viewport size
     * @param width New viewport width
     * @param height New viewport height
     * @return 0 on success, negative error code on failure
     */
    int32_t setViewport(uint32_t width, uint32_t height);

    /**
     * @brief Set the camera view matrix
     * @param view New view matrix
     * @return 0 on success, negative error code on failure
     */
    int32_t setViewMatrix(const glm::mat4& view);

    /**
     * @brief Set the projection matrix
     * @param projection New projection matrix
     * @return 0 on success, negative error code on failure
     */
    int32_t setProjectionMatrix(const glm::mat4& projection);

    /**
     * @brief Enable or disable depth testing
     * @param enable true to enable depth testing, false to disable
     * @return 0 on success, negative error code on failure
     */
    int32_t setDepthTest(bool enable);

    /**
     * @brief Enable or disable alpha blending
     * @param enable true to enable blending, false to disable
     * @return 0 on success, negative error code on failure
     */
    int32_t setBlending(bool enable);

    /**
     * @brief Clear the render buffers
     * @param color Clear the color buffer
     * @param depth Clear the depth buffer
     * @param stencil Clear the stencil buffer
     * @return 0 on success, negative error code on failure
     */
    int32_t clear(bool color = true, bool depth = true, bool stencil = true);
  private:
    /**
     * @brief Check for OpenGL errors and log them
     * @param location String identifying where the check was made
     * @return 0 if no error, negative error code if error found
     */
    int32_t checkGLError(const char* location);
  private:
    /**
     * @brief Viewport width
     */
    uint32_t m_width{0};

    /**
     * @brief Viewport height
     */
    uint32_t m_height{0};

    /**
     * @brief Camera view matrix
     */
    glm::mat4 m_viewMatrix{1.0f};

    /**
     * @brief Projection matrix
     */
    glm::mat4 m_projectionMatrix{1.0f};

    /**
     * @brief Combined view-projection matrix
     */
    glm::mat4 m_viewProjectionMatrix{1.0f};

    /**
     * @brief List of renderables
     */
    std::vector<std::shared_ptr<Renderable>> m_renderables{};

    /**
     * @brief Depth test state
     */
    bool m_depthTestEnabled{true};

    /**
     * @brief Blending state
     */
    bool m_blendingEnabled{false};
};

} // namespace ti::graphics

#endif // GRAPHICS_RENDERER_H_
