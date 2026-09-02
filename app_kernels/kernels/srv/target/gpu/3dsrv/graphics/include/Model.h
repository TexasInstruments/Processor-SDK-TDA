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

#ifndef GRAPHICS_MODEL_H_
#define GRAPHICS_MODEL_H_

#include <graphics/include/Mesh3DRenderer.h>
#include <memory>
#include <string>
#include <utils/include/logger.h>
#include <vector>

namespace ti::graphics
{

/**
 * @brief Abstract base class for 3D model loading and management
 *
 * This class defines the interface for loading and managing 3D models
 * in GLTF/GLB format. It uses the existing Mesh3DRenderer
 * vertex structure for compatibility with the rendering system.
 */
class Model
{
  public:
    /**
     * @brief Status codes for model operations
     */
    enum class Status
    {
        Success,           ///< Operation completed successfully
        FileNotFound,      ///< Model file not found
        InvalidFormat,     ///< File format is invalid
        UnsupportedFormat, ///< File format not supported
        LoadError          ///< General loading error
    };

    /**
     * @brief Virtual destructor
     */
    virtual ~Model() = default;

    /**
     * @brief Load a model from file
     * @param path Path to model file (relative to resource directory)
     * @return Status code indicating success or failure
     */
    virtual Status load(const std::string& path) = 0;

    /**
     * @brief Save model data to file
     * @param path Path to save file (relative to resource directory)
     * @return Status code indicating success or failure
     */
    virtual Status save(const std::string& path) const
    {
        LOG_ERROR("Save not implemented for this model type");
        return Status::UnsupportedFormat;
    }

    /**
     * @brief Set model data directly
     * @param vertices Vertex data
     * @param indices Index data
     */
    virtual void setData(const std::vector<Mesh3DRenderer::Vertex>& vertices,
                         const std::vector<uint32_t>&               indices)
    {
        LOG_DEBUG("Setting model data with %zu vertices and %zu indices", vertices.size(),
                  indices.size());
        m_vertices = vertices;
        m_indices  = indices;
        m_loaded   = true;
    }

    /**
     * @brief Get the loaded vertex data
     * @return Vector of vertices
     */
    const std::vector<Mesh3DRenderer::Vertex>& getVertices() const
    {
        if (!m_loaded)
        {
            LOG_WARN("Attempting to get vertices from unloaded model");
        }
        return m_vertices;
    }

    /**
     * @brief Get the loaded index data
     * @return Vector of indices
     */
    const std::vector<uint32_t>& getIndices() const
    {
        if (!m_loaded)
        {
            LOG_WARN("Attempting to get indices from unloaded model");
        }
        return m_indices;
    }

    /**
     * @brief Check if model data is loaded
     * @return true if model is loaded, false otherwise
     */
    bool isLoaded() const
    {
        if (!m_loaded)
        {
            LOG_DEBUG("Model is not loaded");
        }
        return m_loaded;
    }

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    const std::string& getError() const
    {
        if (!m_error.empty())
        {
            LOG_DEBUG("Retrieving error: %s", m_error.c_str());
        }
        return m_error;
    }

    /**
     * @brief Get the material color for rendering
     * @return Material color as vec4 (RGB + alpha)
     */
    virtual glm::vec4 getMaterialColor() const
    {
        return glm::vec4(1.0f); // Default white color
    }

    /**
     * @brief Get the material ambient color
     * @return Ambient color as vec3 (RGB)
     */
    virtual glm::vec3 getMaterialAmbient() const
    {
        return glm::vec3(0.2f); // Default ambient color
    }

    /**
     * @brief Get the material diffuse color
     * @return Diffuse color as vec3 (RGB)
     */
    virtual glm::vec3 getMaterialDiffuse() const
    {
        return glm::vec3(0.8f); // Default diffuse color
    }

    /**
     * @brief Get the material specular color
     * @return Specular color as vec3 (RGB)
     */
    virtual glm::vec3 getMaterialSpecular() const
    {
        return glm::vec3(0.0f); // Default specular color
    }

    /**
     * @brief Get the material shininess
     * @return Shininess value
     */
    virtual float getMaterialShininess() const
    {
        return 0.0f; // Default shininess
    }

    /**
     * @brief Get the diffuse texture path
     * @return Path to diffuse texture, empty if none
     */
    virtual std::string getDiffuseTexturePath() const
    {
        return ""; // Default no texture
    }

    /**
     * @brief Configure texture flipping behavior
     * @param flipImageData True to flip image pixel data when loading (affects
     * stbi_set_flip_vertically_on_load)
     * @param flipTexCoords True to flip texture V coordinates in model data
     * (1.0 - v)
     */
    virtual void setFlipTextures(bool flipImageData, bool flipTexCoords)
    {
        m_flipImageData = flipImageData;
        m_flipTexCoords = flipTexCoords;
        LOG_DEBUG("Texture flipping configured: image=%s, coordinates=%s",
                  flipImageData ? "enabled" : "disabled", flipTexCoords ? "enabled" : "disabled");
    }

    /**
     * @brief Get current image data flipping setting
     * @return True if image data should be flipped when loading
     */
    bool getFlipImageData() const
    {
        return m_flipImageData;
    }

    /**
     * @brief Get current texture coordinate flipping setting
     * @return True if texture coordinates should be flipped in model data
     */
    bool getFlipTexCoords() const
    {
        return m_flipTexCoords;
    }

    // Convenience methods for common texture flipping scenarios

    /**
     * @brief Use standard texture flipping (flip image data, keep
     * coordinates) Good for most OBJ files with standard image formats
     */
    void useStandardTextureFlipping()
    {
        setFlipTextures(true, false);
    }

    /**
     * @brief Disable all texture flipping
     * Good for pre-processed assets or specific workflows
     */
    void useNoTextureFlipping()
    {
        setFlipTextures(false, false);
    }

    /**
     * @brief Use coordinate flipping only (keep image data, flip
     * coordinates) Good for models with inverted UV coordinates
     */
    void useCoordinateFlippingOnly()
    {
        setFlipTextures(false, true);
    }
  public:
    /**
     * @brief Structure to hold material properties shared by all model types
     */
    struct Material
    {
        std::string name;                       ///< Material name
        glm::vec3   ambient{0.2f, 0.2f, 0.2f};  ///< Ambient color
        glm::vec3   diffuse{0.8f, 0.8f, 0.8f};  ///< Diffuse color
        glm::vec3   specular{0.0f, 0.0f, 0.0f}; ///< Specular color
        float       shininess{0.0f};            ///< Specular exponent
        float       alpha{1.0f};     ///< Alpha/transparency (1.0 = opaque, 0.0 = transparent)
        float       metallic{0.0f};  ///< Metallic factor for PBR (0.0 = non-metal, 1.0 = metal)
        float       roughness{0.5f}; ///< Roughness factor for PBR (0.0 = mirror, 1.0 = rough)
        std::string ambientTexture;  ///< Ambient texture map
        std::string diffuseTexture;  ///< Diffuse texture map
        std::string specularTexture; ///< Specular texture map
        std::string normalTexture;   ///< Normal map
    };

    /**
     * @brief Structure to hold a material group (vertices using the same
     * material)
     */
    struct MaterialGroup
    {
        std::string                         materialName;
        std::vector<Mesh3DRenderer::Vertex> vertices;
        std::vector<uint32_t>               indices;
        Material                            material;
    };

    /**
     * @brief Material type classification based on properties
     */
    enum class MaterialType
    {
        Standard,  ///< Use traditional Blinn-Phong rendering
        Glass,     ///< Transparent material requiring PBR
        Reflective ///< Highly reflective material requiring PBR
    };

    /**
     * @brief Analyze a material and determine the best rendering approach
     * @param material Material to analyze
     * @return Recommended material type for rendering
     */
    static MaterialType classifyMaterial(const Material& material);

    /**
     * @brief Render the model using the provided renderer
     * @param renderer The renderer to use
     * @return Vector of mesh IDs created during rendering
     */
    virtual std::vector<uint32_t> render(Mesh3DRenderer& renderer) const;

    /**
     * @brief Get material groups (separate vertex/index data for each
     * material)
     * @return Vector of material groups
     */
    virtual std::vector<MaterialGroup> getMaterialGroups() const
    {
        return {};
    }

    /**
     * @brief Log material properties at INFO level
     * This logs material color, ambient, diffuse, specular, shininess, and
     * texture information
     */
    void logMaterialProperties() const;

    /**
     * @brief Check if a material name suggests glass-like properties
     * @param materialName Name of the material
     * @return True if material appears to be glass-like
     */
    static bool isGlassLikeName(const std::string& materialName);

    /**
     * @brief Check if a material has transparent properties
     * @param material Material to check
     * @return True if material has transparency (alpha < 1.0)
     */
    static bool isTransparent(const Material& material);

    /**
     * @brief Check if a material has highly reflective properties
     * @param material Material to check
     * @return True if material appears highly reflective
     */
    static bool isHighlyReflective(const Material& material);

    /**
     * @brief Enhance material properties for glass-like materials
     * @param material Material to enhance (modified in place)
     */
    static void enhanceGlassMaterial(Material& material);

    /**
     * @brief Enhance material properties for reflective materials
     * @param material Material to enhance (modified in place)
     */
    static void enhanceReflectiveMaterial(Material& material);

    /**
     * @brief Apply intelligent material enhancement to a material group
     * This method analyzes the material and applies appropriate enhancements
     * @param group Material group to enhance (material modified in place)
     */
    static void applyMaterialIntelligence(MaterialGroup& group);
  protected:
    /**
     * @brief Set error message with logging
     * @param error Error message to set
     */
    void setError(const std::string& error)
    {
        m_error = error;
        LOG_ERROR("%s", m_error.c_str());
    }

    /**
     * @brief Model vertex data
     */
    std::vector<Mesh3DRenderer::Vertex> m_vertices;

    /**
     * @brief Model index data
     */
    std::vector<uint32_t> m_indices;

    /**
     * @brief Loading state
     */
    bool m_loaded{false};

    /**
     * @brief Last error message
     */
    std::string m_error;

    /**
     * @brief Whether to flip image pixel data when loading
     */
    bool m_flipImageData{true};

    /**
     * @brief Whether to flip texture V coordinates (1.0 - v)
     */
    bool m_flipTexCoords{false};
};

} // namespace ti::graphics

#endif // GRAPHICS_MODEL_H_
