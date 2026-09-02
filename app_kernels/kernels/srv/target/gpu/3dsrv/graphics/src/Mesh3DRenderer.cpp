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

#include <fstream>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iterator>

#include <graphics/include/Mesh3DRenderer.h>
#include <graphics/include/ShaderManager.h>
#include <stb_image.h>
#include <utils/include/logger.h>
#include <utils/include/resources.h>

namespace ti::graphics
{

namespace
{
// Vertex shader for 3D mesh rendering
const char *MESH_VERTEX_SHADER = R"(#version 300 es
        precision highp float;
        
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        layout(location = 2) in vec2 texCoord;
        
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        uniform mat4 normalMatrix;
        
        out vec3 vNormal;
        out vec2 vTexCoord;
        out vec3 vFragPos;
        
        void main()
        {
            vec4 worldPos = model * vec4(position, 1.0);
            gl_Position = projection * view * worldPos;
            vFragPos = worldPos.xyz;
            vNormal = mat3(normalMatrix) * normal;
            vTexCoord = texCoord;
        }
    )";

// Fragment shader for 3D mesh rendering with proper material properties
const char *MESH_FRAGMENT_SHADER = R"(#version 300 es
        precision highp float;

        in vec3 vNormal;
        in vec2 vTexCoord;
        in vec3 vFragPos;

        // Material properties
        uniform vec4 baseColor;      // Base color or diffuse color
        uniform vec3 ambientColor;   // Ambient color
        uniform vec3 specularColor;  // Specular color
        uniform float shininess;     // Specular shininess
        uniform vec3 emissiveColor;  // Emissive color for glowing parts
        uniform float emissiveStrength; // Emissive intensity multiplier

        // Textures
        uniform sampler2D textureSampler;    // Base color/diffuse texture
        uniform bool hasTexture;
        uniform sampler2D normalSampler;     // Normal map texture
        uniform bool hasNormalMap;           // Whether normal map is available
        uniform float normalScale;           // Normal map strength

        // Lighting
        #define MAX_LIGHTS 8
        uniform int numLights;
        uniform vec3 lightPositions[MAX_LIGHTS];
        uniform vec3 lightColors[MAX_LIGHTS];
        uniform float lightIntensities[MAX_LIGHTS];
        uniform vec3 viewPos;

        out vec4 fragColor;

        // TBN matrix calculation for normal mapping
        mat3 calculateTBN(vec3 normal)
        {
            // Calculate tangent and bitangent
            vec3 Q1 = dFdx(vFragPos);
            vec3 Q2 = dFdy(vFragPos);
            vec2 st1 = dFdx(vTexCoord);
            vec2 st2 = dFdy(vTexCoord);

            vec3 N = normalize(normal);
            vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
            T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
            vec3 B = -normalize(cross(N, T));

            return mat3(T, B, N);
        }

        void main()
        {
            // Get diffuse color from texture or uniform
            vec4 diffuseColor;
            if (hasTexture)
            {
                diffuseColor = texture(textureSampler, vTexCoord);
            }
            else
            {
                diffuseColor = baseColor;
            }

            // Ensure minimum brightness for diffuse color
            float brightness = max(max(diffuseColor.r, diffuseColor.g), diffuseColor.b);
            if (brightness < 0.1)
            {
                // Boost brightness while preserving hue
                float boost = 0.3 / max(brightness, 0.01);
                diffuseColor.rgb *= boost;
            }

            // Calculate normal
            vec3 norm = normalize(vNormal);

            // Apply normal mapping if available
            if (hasNormalMap)
            {
                // Sample normal map and convert from [0,1] to [-1,1] range
                vec3 normalMap = texture(normalSampler, vTexCoord).rgb * 2.0 - 1.0;
                normalMap.xy *= normalScale;

                // Calculate TBN matrix for normal mapping
                mat3 TBN = calculateTBN(norm);

                // Apply normal map
                norm = normalize(TBN * normalMap);
            }

            vec3 viewDir = normalize(viewPos - vFragPos);

            // Ambient component (only calculated once)
            vec3 ambient = ambientColor * diffuseColor.rgb;

            // Initialize lighting accumulators
            vec3 totalDiffuse = vec3(0.0);
            vec3 totalSpecular = vec3(0.0);

            // Calculate lighting contribution from each light
            for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
                vec3 lightDir = normalize(lightPositions[i] - vFragPos);
                vec3 halfDir = normalize(lightDir + viewDir);

                // Diffuse component
                float diff = max(dot(norm, lightDir), 0.0);
                totalDiffuse += diff * diffuseColor.rgb * lightColors[i] * lightIntensities[i];

                // Specular component (Blinn-Phong)
                float spec = pow(max(dot(norm, halfDir), 0.0), shininess);
                totalSpecular += spec * specularColor * lightColors[i] * lightIntensities[i];
            }

            vec3 diffuse = totalDiffuse;
            vec3 specular = totalSpecular;

            // Add a second light source (directional light from above)
            vec3 topLightDir = normalize(vec3(0.0, 1.0, 0.0));
            float topDiff = max(dot(norm, topLightDir), 0.0);
            vec3 topDiffuse = topDiff * diffuseColor.rgb * 0.4;

            // Add emissive contribution (for lights, screens, etc.)
            vec3 emission = emissiveColor * emissiveStrength;

            // Combine lighting components
            vec3 result = ambient + diffuse + specular + topDiffuse + emission;

            // Ensure result is in valid range
            result = clamp(result, 0.0, 1.0);
            fragColor = vec4(result, diffuseColor.a);
        }
    )";
} // namespace

Mesh3DRenderer::Mesh3DRenderer(uint32_t width, uint32_t height)
: m_width(width),
  m_height(height),
  mShaderProgram(0),
  mPBRShaderProgram(0),
  mShadowMapShaderProgram(0),
  mProjection(
  glm::perspective(glm::radians(45.0f),
                   height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f,
                   0.1f,
                   100.0f)),
  mEnvironmentMapId(0),
  mShadowMapFBO(0),
  mShadowMap(0),
  mLightSpaceMatrix(1.0f),
  mEnableShadows(false),
  mEnableEnvironmentMapping(false)
{
    LOG_DEBUG("Mesh3DRenderer created with viewport %ux%u", width, height);

    // Default lights are now handled in render() function when no lights are
    // configured mLightPositions.push_back(glm::vec3(0.0f, 5.0f, 0.0f)); mLightColors.push_back(glm::vec3(1.0f,
    // 1.0f, 1.0f)); mLightIntensities.push_back(1.0f);
}

Mesh3DRenderer::~Mesh3DRenderer()
{
    cleanup();
    LOG_DEBUG("Mesh3DRenderer destroyed");
}

bool Mesh3DRenderer::initialize()
{
    LOG_DEBUG("Initializing mesh renderer...");

    if (!initializeShaders())
    {
        LOG_ERROR("Failed to initialize traditional mesh shaders");
        return false;
    }

    if (!initializePBRShaders())
    {
        LOG_ERROR("Failed to initialize PBR shaders");
        // Continue anyway, we can fall back to traditional shaders
    }

    // Initialize shadow mapping resources
    // Note: This shadow mapping implementation follows standard OpenGL shadow mapping practices
    // with inspiration from multiple OpenGL resources including:
    // - OpenGL SuperBible by Graham Sellers
    // - LearnOpenGL.com by Joey de Vries
    // - ogldev tutorials by Etay Meiri (https://github.com/emeiri/ogldev)
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    // Create framebuffer
    glGenFramebuffers(1, &mShadowMapFBO);

    // Create depth texture
    glGenTextures(1, &mShadowMap);
    glBindTexture(GL_TEXTURE_2D, mShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowMap, 0);
    glDrawBuffers(0, NULL); // No color buffer is drawn to
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    LOG_DEBUG("Shadow mapping resources initialized");

    return true;
}


void Mesh3DRenderer::render(const glm::mat4 &viewProjection)
{
    glUseProgram(mShaderProgram);

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Set common uniforms
    GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(mShaderProgram, "view");

    // Extract view and projection matrices
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view       = glm::mat4(1.0f);

    // Decompose view-projection matrix
    // This assumes the matrix is a standard view-projection matrix
    // where projection is applied after view
    projection = viewProjection * glm::inverse(view);
    view       = glm::inverse(glm::inverse(viewProjection) * projection);

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    // Set lighting uniforms
    GLint numLightsLoc = glGetUniformLocation(mShaderProgram, "numLights");
    GLint viewPosLoc   = glGetUniformLocation(mShaderProgram, "viewPos");

    // Use the lights configured via setLights()
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);

    int       numLights  = 0;
    const int MAX_LIGHTS = 8;

    if (!mLightPositions.empty())
    {
        numLights = std::min(static_cast<int>(mLightPositions.size()), MAX_LIGHTS);

        // Set light arrays
        for (int i = 0; i < numLights; ++i)
        {
            // Light positions
            std::string posUniform = "lightPositions[" + std::to_string(i) + "]";
            GLint posLoc = glGetUniformLocation(mShaderProgram, posUniform.c_str());
            glUniform3f(posLoc,
                        mLightPositions[i].x,
                        mLightPositions[i].y,
                        mLightPositions[i].z);

            // Light colors (use default if not enough colors provided)
            glm::vec3 lightColor = (i < static_cast<int>(mLightColors.size())) ?
                                   mLightColors[i] :
                                   glm::vec3(1.0f, 1.0f, 1.0f);
            std::string colorUniform = "lightColors[" + std::to_string(i) + "]";
            GLint colorLoc = glGetUniformLocation(mShaderProgram, colorUniform.c_str());
            glUniform3f(colorLoc, lightColor.r, lightColor.g, lightColor.b);

            // Light intensities (use default if not enough intensities provided)
            float lightIntensity =
            (i < static_cast<int>(mLightIntensities.size())) ? mLightIntensities[i] : 1.0f;
            std::string intensityUniform = "lightIntensities[" + std::to_string(i) + "]";
            GLint intensityLoc =
            glGetUniformLocation(mShaderProgram, intensityUniform.c_str());
            glUniform1f(intensityLoc, lightIntensity);
        }

        LOG_DEBUG("Using %d configured lights", numLights);
    }
    else
    {
        // Use default light if no lights configured
        numLights    = 1;
        GLint posLoc = glGetUniformLocation(mShaderProgram, "lightPositions[0]");
        GLint colorLoc = glGetUniformLocation(mShaderProgram, "lightColors[0]");
        GLint intensityLoc = glGetUniformLocation(mShaderProgram, "lightIntensities[0]");

        glUniform3f(posLoc, 0.0f, 5.0f, 0.0f);   // Default position
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // Default white
        glUniform1f(intensityLoc, 1.0f);         // Default intensity

        LOG_DEBUG("No lights configured, using default light");
    }

    glUniform1i(numLightsLoc, numLights);
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

    // Debug the material color of each mesh
    LOG_DEBUG("Number of mesh elements: %zu", mMeshElements.size());
    for (size_t i = 0; i < mMeshElements.size(); ++i)
    {
        const auto &mesh = mMeshElements[i];
        LOG_DEBUG("Mesh %zu color: (%.2f, %.2f, %.2f, %.2f), texture ID: %u",
                  i,
                  mesh.color.r,
                  mesh.color.g,
                  mesh.color.b,
                  mesh.color.a,
                  mesh.textureId);
    }

    // First, render all opaque meshes
    for (const auto &mesh : mMeshElements)
    {
        if (!mesh.visible)
            continue;

        // Skip transparent meshes in this pass
        if (mesh.color.a < 1.0f)
            continue;

        // Calculate model and normal matrices
        glm::mat4 model = glm::translate(glm::mat4(1.0f), mesh.position);
        model = glm::rotate(model, mesh.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, mesh.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, mesh.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, mesh.scale);

        glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

        // Set mesh-specific uniforms
        GLint modelLoc     = glGetUniformLocation(mShaderProgram, "model");
        GLint normalMatLoc = glGetUniformLocation(mShaderProgram, "normalMatrix");
        GLint colorLoc     = glGetUniformLocation(mShaderProgram, "baseColor");
        GLint ambientLoc = glGetUniformLocation(mShaderProgram, "ambientColor");
        GLint specularLoc = glGetUniformLocation(mShaderProgram, "specularColor");
        GLint shininessLoc = glGetUniformLocation(mShaderProgram, "shininess");
        GLint hasTexLoc    = glGetUniformLocation(mShaderProgram, "hasTexture");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(normalMatLoc, 1, GL_FALSE, &normalMatrix[0][0]);

        // Set material properties
        glUniform4fv(colorLoc, 1, &mesh.color[0]);

        // Default material properties if not specified
        glm::vec3 ambientColor     = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 specularColor    = glm::vec3(0.5f, 0.5f, 0.5f);
        float     shininess        = 32.0f;
        glm::vec3 emissiveColor    = glm::vec3(0.0f);
        float     emissiveStrength = 0.0f;

        // Material texture properties
        GLuint normalMapId = 0;
        float  normalScale = 1.0f;

        // Use material properties from the model if available
        if (mesh.materialIndex >= 0 &&
            static_cast<size_t>(mesh.materialIndex) < mMaterials.size())
        {
            const auto &material = mMaterials[mesh.materialIndex];
            ambientColor         = material.ambient;
            specularColor        = material.specular;
            shininess            = material.shininess;
            normalMapId          = material.normalMap;
            emissiveColor        = material.emissive;
            emissiveStrength     = material.emissiveStrength;
        }

        glUniform3fv(ambientLoc, 1, &ambientColor[0]);
        glUniform3fv(specularLoc, 1, &specularColor[0]);
        glUniform1f(shininessLoc, shininess);
        glUniform1i(hasTexLoc, mesh.textureId != 0);

        // Set emissive properties
        GLint emissiveColorLoc = glGetUniformLocation(mShaderProgram, "emissiveColor");
        GLint emissiveStrengthLoc = glGetUniformLocation(mShaderProgram, "emissiveStrength");
        glUniform3fv(emissiveColorLoc, 1, &emissiveColor[0]);
        glUniform1f(emissiveStrengthLoc, emissiveStrength);

        // Get locations for normal map uniforms
        GLint hasNormalMapLoc = glGetUniformLocation(mShaderProgram, "hasNormalMap");
        GLint normalScaleLoc = glGetUniformLocation(mShaderProgram, "normalScale");
        GLint normalSamplerLoc = glGetUniformLocation(mShaderProgram, "normalSampler");

        // Set normal map uniforms
        glUniform1i(hasNormalMapLoc, normalMapId != 0);
        glUniform1f(normalScaleLoc, normalScale);

        // Bind texture if available
        if (mesh.textureId)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureId);
            glUniform1i(glGetUniformLocation(mShaderProgram, "textureSampler"), 0);
        }

        // Bind normal map if available
        if (normalMapId)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalMapId);
            glUniform1i(normalSamplerLoc, 1);
        }

        // Render mesh
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    // Now render transparent meshes with blending enabled
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto &mesh : mMeshElements)
    {
        if (!mesh.visible)
            continue;

        // Only render transparent meshes in this pass
        if (mesh.color.a >= 1.0f)
            continue;

        // Calculate model and normal matrices
        glm::mat4 model = glm::translate(glm::mat4(1.0f), mesh.position);
        model = glm::rotate(model, mesh.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, mesh.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, mesh.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, mesh.scale);

        glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

        // Set mesh-specific uniforms
        GLint modelLoc     = glGetUniformLocation(mShaderProgram, "model");
        GLint normalMatLoc = glGetUniformLocation(mShaderProgram, "normalMatrix");
        GLint colorLoc     = glGetUniformLocation(mShaderProgram, "baseColor");
        GLint ambientLoc = glGetUniformLocation(mShaderProgram, "ambientColor");
        GLint specularLoc = glGetUniformLocation(mShaderProgram, "specularColor");
        GLint shininessLoc = glGetUniformLocation(mShaderProgram, "shininess");
        GLint hasTexLoc    = glGetUniformLocation(mShaderProgram, "hasTexture");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(normalMatLoc, 1, GL_FALSE, &normalMatrix[0][0]);

        // Set material properties
        glUniform4fv(colorLoc, 1, &mesh.color[0]);

        // Default material properties if not specified
        glm::vec3 ambientColor  = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
        float     shininess     = 32.0f;

        // Use material properties from the model if available
        if (mesh.materialIndex >= 0 &&
            static_cast<size_t>(mesh.materialIndex) < mMaterials.size())
        {
            const auto &material = mMaterials[mesh.materialIndex];
            ambientColor         = material.ambient;
            specularColor        = material.specular;
            shininess            = material.shininess;
        }

        glUniform3fv(ambientLoc, 1, &ambientColor[0]);
        glUniform3fv(specularLoc, 1, &specularColor[0]);
        glUniform1f(shininessLoc, shininess);
        glUniform1i(hasTexLoc, mesh.textureId != 0);

        // Bind texture if available
        if (mesh.textureId)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureId);
        }

        // Render mesh
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    // Restore state
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Mesh3DRenderer::cleanup()
{
    LOG_DEBUG("Cleaning up Mesh3DRenderer resources...");

    // Clean up shader programs
    if (mShaderProgram)
    {
        LOG_DEBUG("Deleting traditional shader program %u", mShaderProgram);
        glDeleteProgram(mShaderProgram);
    }

    if (mPBRShaderProgram)
    {
        LOG_DEBUG("Deleting PBR shader program %u", mPBRShaderProgram);
        glDeleteProgram(mPBRShaderProgram);
    }

    if (mShadowMapShaderProgram)
    {
        LOG_DEBUG("Deleting shadow map shader program %u", mShadowMapShaderProgram);
        glDeleteProgram(mShadowMapShaderProgram);
    }

    // Clean up mesh resources
    LOG_DEBUG("Cleaning up %zu mesh elements", mMeshElements.size());
    for (auto &mesh : mMeshElements)
    {
        if (mesh.vao)
        {
            LOG_DEBUG("Deleting mesh buffers (VAO: %u, VBO: %u, EBO: %u)",
                      mesh.vao,
                      mesh.vbo,
                      mesh.ebo);
            glDeleteVertexArrays(1, &mesh.vao);
            glDeleteBuffers(1, &mesh.vbo);
            glDeleteBuffers(1, &mesh.ebo);
        }
    }

    // Clean up textures
    LOG_DEBUG("Cleaning up %zu textures", mLoadedTextures.size());
    for (const auto &pair : mLoadedTextures)
    {
        LOG_DEBUG("Deleting texture %u for '%s'", pair.second, pair.first.c_str());
        glDeleteTextures(1, &pair.second);
    }

    // Clean up shadow mapping resources
    if (mShadowMapFBO)
    {
        glDeleteFramebuffers(1, &mShadowMapFBO);
        mShadowMapFBO = 0;
    }

    if (mShadowMap)
    {
        glDeleteTextures(1, &mShadowMap);
        mShadowMap = 0;
    }

    // Clean up environment map if we own it
    if (mEnvironmentMapId)
    {
        glDeleteTextures(1, &mEnvironmentMapId);
        mEnvironmentMapId = 0;
    }

    mMeshElements.clear();
    mLoadedTextures.clear();
}

uint32_t Mesh3DRenderer::addCube(const glm::vec3 &position,
                                 const glm::vec3 &size,
                                 const glm::vec4 &color,
                                 GLuint           textureId)
{
    auto [vertices, indices] = generateCubeMesh(size);

    MeshElement element;
    element.type          = MeshType::Cube;
    element.vertices      = std::move(vertices);
    element.indices       = std::move(indices);
    element.position      = position;
    element.rotation      = glm::vec3(0.0f);
    element.scale         = glm::vec3(1.0f);
    element.color         = color;
    element.textureId     = textureId;
    element.materialIndex = -1; // No material by default
    element.visible       = true;

    createMeshBuffers(element);

    mMeshElements.push_back(std::move(element));
    return static_cast<uint32_t>(mMeshElements.size() - 1);
}

uint32_t Mesh3DRenderer::addSphere(const glm::vec3 &position,
                                   float            radius,
                                   uint32_t         segments,
                                   const glm::vec4 &color,
                                   GLuint           textureId)
{
    auto [vertices, indices] = generateSphereMesh(radius, segments);

    MeshElement element;
    element.type          = MeshType::Sphere;
    element.vertices      = std::move(vertices);
    element.indices       = std::move(indices);
    element.position      = position;
    element.rotation      = glm::vec3(0.0f);
    element.scale         = glm::vec3(1.0f);
    element.color         = color;
    element.textureId     = textureId;
    element.materialIndex = -1; // No material by default
    element.visible       = true;

    createMeshBuffers(element);

    mMeshElements.push_back(std::move(element));
    return static_cast<uint32_t>(mMeshElements.size() - 1);
}

uint32_t Mesh3DRenderer::addPyramid(const glm::vec3 &position,
                                    const glm::vec2 &baseSize,
                                    float            height,
                                    const glm::vec4 &color,
                                    GLuint           textureId)
{
    auto [vertices, indices] = generatePyramidMesh(baseSize, height);

    MeshElement element;
    element.type          = MeshType::Pyramid;
    element.vertices      = std::move(vertices);
    element.indices       = std::move(indices);
    element.position      = position;
    element.rotation      = glm::vec3(0.0f);
    element.scale         = glm::vec3(1.0f);
    element.color         = color;
    element.textureId     = textureId;
    element.materialIndex = -1; // No material by default
    element.visible       = true;

    createMeshBuffers(element);

    mMeshElements.push_back(std::move(element));
    return static_cast<uint32_t>(mMeshElements.size() - 1);
}

uint32_t Mesh3DRenderer::addCustomMesh(const std::vector<Vertex>   &vertices,
                                       const std::vector<uint32_t> &indices,
                                       const glm::vec3             &position,
                                       const glm::vec4             &color,
                                       GLuint                       textureId,
                                       int materialIndex)
{
    MeshElement element;
    element.type           = MeshType::Custom;
    element.vertices       = vertices;
    element.indices        = indices;
    element.position       = position;
    element.rotation       = glm::vec3(0.0f);
    element.scale          = glm::vec3(1.0f);
    element.color          = color;
    element.textureId      = textureId;
    element.materialIndex  = materialIndex;
    element.visible        = true;
    element.castsShadows   = true;
    element.receiveShadows = true;

    createMeshBuffers(element);

    mMeshElements.push_back(std::move(element));
    return static_cast<uint32_t>(mMeshElements.size() - 1);
}

int Mesh3DRenderer::addMaterial(const glm::vec3 &ambient,
                                const glm::vec3 &diffuse,
                                const glm::vec3 &specular,
                                float            shininess)
{
    Material material;
    material.ambient   = ambient;
    material.diffuse   = diffuse;
    material.specular  = specular;
    material.shininess = shininess;

    // Set reasonable defaults for PBR properties
    material.metallic           = 0.0f;
    material.roughness          = 0.5f;
    material.ior                = 1.45f;
    material.transmission       = 0.0f;
    material.clearcoat          = 0.0f;
    material.clearcoatRoughness = 0.0f;
    material.anisotropy         = 0.0f;
    material.emissive           = glm::vec3(0.0f);
    material.emissiveStrength   = 1.0f;

    mMaterials.push_back(material);
    return static_cast<int>(mMaterials.size() - 1);
}

int Mesh3DRenderer::addPBRMaterial(const glm::vec3 &diffuse, float metallic, float roughness, GLuint textureId)
{
    Material material;

    // Set traditional properties for backward compatibility
    material.ambient = diffuse * 0.2f;
    material.diffuse = diffuse;

    // Calculate specular based on metallic and roughness
    // For more metallic surfaces, increase specular but decrease it for rougher surfaces
    float specularIntensity = metallic * (1.0f - roughness) * 0.5f;
    material.specular       = glm::vec3(specularIntensity);

    // Higher shininess for smoother surfaces
    material.shininess = (1.0f - roughness) * 128.0f + 4.0f;

    // Set PBR properties
    material.metallic           = metallic;
    material.roughness          = roughness;
    material.ior                = 1.45f;
    material.transmission       = 0.0f;
    material.clearcoat          = 0.0f;
    material.clearcoatRoughness = 0.0f;
    material.anisotropy         = 0.0f;
    material.emissive           = glm::vec3(0.0f);
    material.emissiveStrength   = 1.0f;

    mMaterials.push_back(material);
    return static_cast<int>(mMaterials.size() - 1);
}

int Mesh3DRenderer::addCarPaintMaterial(const glm::vec3 &baseColor,
                                        float            metallic,
                                        float            roughness,
                                        float            clearcoat,
                                        float            clearcoatRoughness)
{
    Material material;

    // Set traditional properties for backward compatibility
    material.ambient = baseColor * 0.2f;
    material.diffuse = baseColor;

    // Car paint typically has high specular reflection with a tight highlight
    // Higher clearcoat means more pronounced specular highlights
    float specularIntensity = 0.5f + clearcoat * 0.5f;
    material.specular       = glm::vec3(specularIntensity);

    // Car paint usually has a sharp highlight, less roughness means higher shininess
    material.shininess = (1.0f - clearcoatRoughness) * 256.0f + 32.0f;

    // Set PBR properties
    material.metallic     = metallic;
    material.roughness    = roughness;
    material.ior          = 1.45f;
    material.transmission = 0.0f;

    // Set car paint specific properties
    material.clearcoat          = clearcoat;
    material.clearcoatRoughness = clearcoatRoughness;
    material.anisotropy         = 0.0f;
    material.emissive           = glm::vec3(0.0f);
    material.emissiveStrength   = 1.0f;

    mMaterials.push_back(material);
    return static_cast<int>(mMaterials.size() - 1);
}

void Mesh3DRenderer::setMaterialTextureMaps(int    materialIndex,
                                            GLuint normalMapId,
                                            GLuint metallicRoughnessMapId,
                                            GLuint aoMapId,
                                            GLuint emissiveMapId)
{
    if (materialIndex >= 0 && materialIndex < static_cast<int>(mMaterials.size()))
    {
        Material &material            = mMaterials[materialIndex];
        material.normalMap            = normalMapId;
        material.metallicRoughnessMap = metallicRoughnessMapId;
        material.aoMap                = aoMapId;
        material.emissiveMap          = emissiveMapId;
    }
}

void Mesh3DRenderer::updateTransform(uint32_t         id,
                                     const glm::vec3 &position,
                                     const glm::vec3 &rotation,
                                     const glm::vec3 &scale)
{
    if (id < mMeshElements.size())
    {
        auto &mesh    = mMeshElements[id];
        mesh.position = position;
        mesh.rotation = rotation;
        mesh.scale    = scale;
    }
}

GLuint Mesh3DRenderer::loadTexture(const std::string &path, bool flipVertically)
{
    LOG_DEBUG("Loading texture from '%s'", path.c_str());

    // Check if texture is already loaded
    auto it = mLoadedTextures.find(path);
    if (it != mLoadedTextures.end())
    {
        LOG_DEBUG("Using cached texture %u for '%s'", it->second, path.c_str());
        return it->second;
    }

    LOG_DEBUG("Resolved path: '%s'", path.c_str());

    // Load image data
    int width, height, channels;
    stbi_set_flip_vertically_on_load(flipVertically);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        LOG_ERROR("Failed to load texture image: %s", stbi_failure_reason());
        return 0;
    }

    LOG_DEBUG("Loaded image: %dx%d with %d channels", width, height, channels);

    // Create OpenGL texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data
    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free image data
    stbi_image_free(data);

    // Cache texture
    mLoadedTextures[path] = texture;
    LOG_DEBUG("Created texture %u for '%s'", texture, path.c_str());

    return texture;
}

GLuint Mesh3DRenderer::loadCubemap(const std::vector<std::string> &paths)
{
    if (paths.size() != 6)
    {
        LOG_ERROR("Cubemap requires exactly 6 texture paths");
        return 0;
    }

    // Generate cubemap texture
    GLuint cubemapId;
    glGenTextures(1, &cubemapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

    // Load each face
    for (unsigned int i = 0; i < paths.size(); i++)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(false); // Don't flip cubemap textures
        unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);

        if (data)
        {
            GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            LOG_ERROR("Failed to load cubemap texture: %s", paths[i].c_str());
            glDeleteTextures(1, &cubemapId);
            return 0;
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    LOG_DEBUG("Created cubemap texture %u", cubemapId);
    return cubemapId;
}

void Mesh3DRenderer::setEnvironmentMap(GLuint cubemapId)
{
    mEnvironmentMapId         = cubemapId;
    mEnableEnvironmentMapping = (cubemapId != 0);
    LOG_DEBUG("Environment map set to %u, environment mapping %s",
              cubemapId,
              mEnableEnvironmentMapping ? "enabled" : "disabled");
}

int Mesh3DRenderer::addSubmesh(uint32_t meshId, const std::vector<uint32_t> &indices, int materialIndex)
{
    if (meshId >= mMeshElements.size())
    {
        LOG_ERROR("Invalid mesh ID: %u", meshId);
        return -1;
    }

    MeshElement &mesh = mMeshElements[meshId];

    // Create submesh
    SubMesh submesh;
    submesh.indices       = indices;
    submesh.materialIndex = materialIndex;

    // Add to mesh
    mesh.submeshes.push_back(submesh);

    LOG_DEBUG("Added submesh with %zu indices and material %d to mesh %u",
              indices.size(),
              materialIndex,
              meshId);

    return static_cast<int>(mesh.submeshes.size() - 1);
}

void Mesh3DRenderer::setLights(const std::vector<glm::vec3> &lightPositions,
                               const std::vector<glm::vec3> &lightColors,
                               const std::vector<float>     &lightIntensities)
{
    // Validate input
    if (lightPositions.size() != lightColors.size() ||
        lightPositions.size() != lightIntensities.size())
    {
        LOG_ERROR("Light arrays must have the same size");
        return;
    }

    // Store light data
    mLightPositions   = lightPositions;
    mLightColors      = lightColors;
    mLightIntensities = lightIntensities;

    LOG_DEBUG("Set %zu lights", mLightPositions.size());
}

void Mesh3DRenderer::setViewport(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;
    mProjection =
    glm::perspective(glm::radians(45.0f),
                     height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f,
                     0.1f,
                     100.0f);
}

bool Mesh3DRenderer::initializeShaders()
{
    // Use ShaderManager to create the traditional mesh shader program
    mShaderProgram = ShaderManager::getInstance().createGraphicsProgramFromSource(
    "mesh3d_traditional", MESH_VERTEX_SHADER, MESH_FRAGMENT_SHADER);

    if (mShaderProgram == 0)
    {
        LOG_ERROR("Failed to create traditional mesh shader program");
        return false;
    }

    LOG_DEBUG("Traditional mesh shaders initialized successfully");

    // Initialize shadow mapping shaders using ShaderManager
    std::string shaderPath = utils::getResourcePath() + "/shaders/pbr/";

    mShadowMapShaderProgram = ShaderManager::getInstance().createGraphicsProgramFromFiles(
    "shadow_mapping", shaderPath + "shadow.vert", shaderPath + "shadow.frag");

    if (mShadowMapShaderProgram == 0)
    {
        LOG_ERROR("Failed to create shadow mapping shader program");
        LOG_DEBUG("Continuing without shadow mapping support");
        mEnableShadows = false;
        return true; // Continue without shadow mapping
    }

    LOG_DEBUG("Shadow mapping shaders initialized successfully");
    mEnableShadows = true;

    return true;
}

bool Mesh3DRenderer::initializePBRShaders()
{
    LOG_DEBUG("Initializing PBR shaders...");

    std::string shaderPath = utils::getResourcePath() + "/shaders/pbr/";

    // Use ShaderManager to create the PBR shader program
    mPBRShaderProgram = ShaderManager::getInstance().createGraphicsProgramFromFiles(
    "pbr_renderer", shaderPath + "pbr.vert", shaderPath + "pbr.frag");

    if (mPBRShaderProgram == 0)
    {
        LOG_ERROR("Failed to create PBR shader program");
        return false;
    }

    LOG_DEBUG("PBR shaders initialized successfully");
    return true;
}

std::pair<std::vector<Mesh3DRenderer::Vertex>, std::vector<uint32_t>>
Mesh3DRenderer::generateCubeMesh(const glm::vec3 &size)
{
    LOG_DEBUG("Generating cube mesh with size (%.2f, %.2f, %.2f)",
              size.x,
              size.y,
              size.z);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(24); // 6 faces * 4 vertices
    indices.reserve(36);  // 6 faces * 2 triangles * 3 vertices

    // Half dimensions
    float w = size.x * 0.5f;
    float h = size.y * 0.5f;
    float d = size.z * 0.5f;

    // Front face
    vertices.push_back({{-w, -h, d}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}); // 0
    vertices.push_back({{w, -h, d}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}});  // 1
    vertices.push_back({{w, h, d}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}});   // 2
    vertices.push_back({{-w, h, d}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}});  // 3

    // Back face
    vertices.push_back({{w, -h, -d}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});  // 4
    vertices.push_back({{-w, -h, -d}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}); // 5
    vertices.push_back({{-w, h, -d}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}});  // 6
    vertices.push_back({{w, h, -d}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}});   // 7

    // Right face
    vertices.push_back({{w, -h, d}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});  // 8
    vertices.push_back({{w, -h, -d}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}); // 9
    vertices.push_back({{w, h, -d}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}});  // 10
    vertices.push_back({{w, h, d}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}});   // 11

    // Left face
    vertices.push_back({{-w, -h, -d}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}); // 12
    vertices.push_back({{-w, -h, d}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}});  // 13
    vertices.push_back({{-w, h, d}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}});   // 14
    vertices.push_back({{-w, h, -d}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}});  // 15

    // Top face
    vertices.push_back({{-w, h, d}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});  // 16
    vertices.push_back({{w, h, d}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}});   // 17
    vertices.push_back({{w, h, -d}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});  // 18
    vertices.push_back({{-w, h, -d}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}); // 19

    // Bottom face
    vertices.push_back({{-w, -h, -d}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}); // 20
    vertices.push_back({{w, -h, -d}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});  // 21
    vertices.push_back({{w, -h, d}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}});   // 22
    vertices.push_back({{-w, -h, d}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}});  // 23

    // Indices for all faces (6 faces * 2 triangles * 3 vertices)
    indices = {
    0,  1,  2,  2,  3,  0,  // Front
    4,  5,  6,  6,  7,  4,  // Back
    8,  9,  10, 10, 11, 8,  // Right
    12, 13, 14, 14, 15, 12, // Left
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
    };

    LOG_DEBUG("Generated cube mesh with %zu vertices and %zu indices",
              vertices.size(),
              indices.size());
    return {vertices, indices};
}

std::pair<std::vector<Mesh3DRenderer::Vertex>, std::vector<uint32_t>>
Mesh3DRenderer::generateSphereMesh(float radius, uint32_t segments)
{
    LOG_DEBUG("Generating sphere mesh with radius %.2f and %u segments", radius, segments);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    // Pre-allocate memory
    vertices.reserve((segments + 1) * (segments + 1));
    indices.reserve(segments * segments * 6);

    // Generate vertices
    for (uint32_t y = 0; y <= segments; ++y)
    {
        float phi = glm::pi<float>() * static_cast<float>(y) / static_cast<float>(segments);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (uint32_t x = 0; x <= segments; ++x)
        {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(x) /
                          static_cast<float>(segments);
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            // Position
            glm::vec3 position(radius * cosTheta * sinPhi,
                               radius * cosPhi,
                               radius * sinTheta * sinPhi);

            // Normal (normalized position for sphere)
            glm::vec3 normal = glm::normalize(position);

            // Texture coordinates
            glm::vec2 texCoord(static_cast<float>(x) / static_cast<float>(segments),
                               static_cast<float>(y) / static_cast<float>(segments));

            vertices.push_back({position, normal, texCoord});
        }
    }

    LOG_DEBUG("Generated %zu vertices", vertices.size());

    // Generate indices
    for (uint32_t y = 0; y < segments; ++y)
    {
        for (uint32_t x = 0; x < segments; ++x)
        {
            uint32_t v0 = y * (segments + 1) + x;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = (y + 1) * (segments + 1) + x;
            uint32_t v3 = v2 + 1;

            indices.push_back(v0);
            indices.push_back(v2);
            indices.push_back(v1);

            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);
        }
    }

    LOG_DEBUG("Generated %zu indices", indices.size());
    return {vertices, indices};
}

std::pair<std::vector<Mesh3DRenderer::Vertex>, std::vector<uint32_t>>
Mesh3DRenderer::generatePyramidMesh(const glm::vec2 &baseSize, float height)
{
    LOG_DEBUG("Generating pyramid mesh with base size (%.2f, %.2f) and height "
              "%.2f",
              baseSize.x,
              baseSize.y,
              height);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(17); // 4 base + 1 apex + 12 side vertices
    indices.reserve(18);  // 2 base triangles + 4 side triangles * 3 vertices

    // Half dimensions
    float w = baseSize.x * 0.5f;
    float d = baseSize.y * 0.5f;
    float h = height;

    // Base vertices
    vertices.push_back({{-w, 0.0f, d}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}); // 0
    vertices.push_back({{w, 0.0f, d}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});  // 1
    vertices.push_back({{w, 0.0f, -d}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}); // 2
    vertices.push_back({{-w, 0.0f, -d}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}); // 3

    // Apex
    vertices.push_back({{0.0f, h, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}}); // 4

    // Calculate face normals
    glm::vec3 frontNormal =
    glm::normalize(glm::cross(glm::vec3(2.0f * w, 0.0f, 0.0f), glm::vec3(-w, h, -d)));
    glm::vec3 rightNormal =
    glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, -2.0f * d), glm::vec3(-w, h, -d)));
    glm::vec3 backNormal =
    glm::normalize(glm::cross(glm::vec3(-2.0f * w, 0.0f, 0.0f), glm::vec3(w, h, d)));
    glm::vec3 leftNormal =
    glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 2.0f * d), glm::vec3(w, h, d)));

    // Add vertices for sides with correct normals
    // Front face
    vertices.push_back({{-w, 0.0f, d}, frontNormal, {0.0f, 0.0f}});   // 5
    vertices.push_back({{w, 0.0f, d}, frontNormal, {1.0f, 0.0f}});    // 6
    vertices.push_back({{0.0f, h, 0.0f}, frontNormal, {0.5f, 1.0f}}); // 7

    // Right face
    vertices.push_back({{w, 0.0f, d}, rightNormal, {0.0f, 0.0f}});    // 8
    vertices.push_back({{w, 0.0f, -d}, rightNormal, {1.0f, 0.0f}});   // 9
    vertices.push_back({{0.0f, h, 0.0f}, rightNormal, {0.5f, 1.0f}}); // 10

    // Back face
    vertices.push_back({{w, 0.0f, -d}, backNormal, {0.0f, 0.0f}});   // 11
    vertices.push_back({{-w, 0.0f, -d}, backNormal, {1.0f, 0.0f}});  // 12
    vertices.push_back({{0.0f, h, 0.0f}, backNormal, {0.5f, 1.0f}}); // 13

    // Left face
    vertices.push_back({{-w, 0.0f, -d}, leftNormal, {0.0f, 0.0f}});  // 14
    vertices.push_back({{-w, 0.0f, d}, leftNormal, {1.0f, 0.0f}});   // 15
    vertices.push_back({{0.0f, h, 0.0f}, leftNormal, {0.5f, 1.0f}}); // 16

    // Indices
    // Base
    indices = {0,
               1,
               2, // Base triangle 1
               0,
               2,
               3, // Base triangle 2
               // Front face
               5,
               6,
               7,
               // Right face
               8,
               9,
               10,
               // Back face
               11,
               12,
               13,
               // Left face
               14,
               15,
               16};

    LOG_DEBUG("Generated pyramid mesh with %zu vertices and %zu indices",
              vertices.size(),
              indices.size());
    return {vertices, indices};
}

void Mesh3DRenderer::createMeshBuffers(MeshElement &mesh)
{
    LOG_DEBUG("Creating mesh buffers for %zu vertices and %zu indices",
              mesh.vertices.size(),
              mesh.indices.size());

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    // Buffer vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    // Buffer index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(uint32_t),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

    glBindVertexArray(0);

    LOG_DEBUG("Mesh buffers created successfully (VAO: %u, VBO: %u, EBO: %u)",
              mesh.vao,
              mesh.vbo,
              mesh.ebo);
}

void Mesh3DRenderer::setMeshBlendMode(uint32_t meshId, bool enable)
{
    if (meshId >= mMeshElements.size())
    {
        LOG_ERROR("Invalid mesh ID: %u", meshId);
        return;
    }

    // Store blending state in the mesh color's alpha component for later use
    // We'll set a flag in the alpha channel that the render function will check
    MeshElement &mesh = mMeshElements[meshId];

    if (enable)
    {
        // Keep original alpha but mark it for blending
        if (mesh.color.a >= 1.0f)
        {
            // If it was originally opaque, make it slightly transparent for blending
            mesh.color.a = 0.95f;
        }

        LOG_DEBUG("Enabled blending for mesh %u with alpha %.2f",
                  meshId,
                  mesh.color.a);
    }

    // In the render method, we'll check mesh.color.a < 1.0f to enable blending
}

} // namespace ti::graphics
