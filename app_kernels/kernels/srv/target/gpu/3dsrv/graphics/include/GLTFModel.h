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

#ifndef GRAPHICS_GLTF_MODEL_H_
#define GRAPHICS_GLTF_MODEL_H_

#include <graphics/include/Model.h>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <unordered_map>
#include <vector>

namespace ti::graphics
{

/**
 * @brief Implementation of Model for the glTF format
 *
 * glTF (GL Transmission Format) is the standard 3D scene format
 * supporting geometry, materials, textures, and animations.
 *
 * This implementation supports:
 * - Mesh geometry with multiple primitives
 * - PBR materials with base color, metallic, roughness
 * - Textures (diffuse, normal, metallic-roughness)
 * - Multiple material groups per model
 */
class GLTFModel : public Model
{
  public:
    /**
     * @brief Constructor
     */
    GLTFModel() = default;

    /**
     * @brief Destructor
     */
    ~GLTFModel() override = default;

    /**
     * @brief Load a glTF model from file
     * @param path Path to glTF file (relative to resource directory)
     * @return Status code indicating success or failure
     */
    Status load(const std::string& path) override;

    /**
     * @brief Save model data to glTF format
     * @param path Path to save file (relative to resource directory)
     * @return Status code indicating success or failure
     */
    Status save(const std::string& path) const override;

    /**
     * @brief Set model data directly
     * @param vertices Vertex data
     * @param indices Index data
     */
    void setData(const std::vector<Mesh3DRenderer::Vertex>& vertices,
                 const std::vector<uint32_t>&               indices) override;

    /**
     * @brief Set model data with material groups
     * @param materialGroups Material groups with vertices, indices, and
     * material info
     */
    void setDataWithMaterials(const std::vector<Model::MaterialGroup>& materialGroups);
  public:
    /**
     * @brief glTF component types
     */
    enum class ComponentType : uint32_t
    {
        BYTE           = 5120,
        UNSIGNED_BYTE  = 5121,
        SHORT          = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT   = 5125,
        FLOAT          = 5126
    };

    /**
     * @brief glTF accessor types
     */
    enum class AccessorType
    {
        SCALAR,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4
    };

    /**
     * @brief glTF primitive modes
     */
    enum class PrimitiveMode : uint32_t
    {
        POINTS         = 0,
        LINES          = 1,
        LINE_LOOP      = 2,
        LINE_STRIP     = 3,
        TRIANGLES      = 4,
        TRIANGLE_STRIP = 5,
        TRIANGLE_FAN   = 6
    };

    /**
     * @brief glTF buffer structure
     */
    struct Buffer
    {
        std::string          uri;
        size_t               byteLength = 0;
        std::vector<uint8_t> data;
    };

    /**
     * @brief glTF buffer view structure
     */
    struct BufferView
    {
        size_t   buffer     = 0;
        size_t   byteOffset = 0;
        size_t   byteLength = 0;
        size_t   byteStride = 0;
        uint32_t target     = 0; // GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
    };

    /**
     * @brief glTF accessor structure
     */
    struct Accessor
    {
        size_t             bufferView    = 0;
        size_t             byteOffset    = 0;
        ComponentType      componentType = ComponentType::FLOAT;
        bool               normalized    = false;
        size_t             count         = 0;
        AccessorType       type          = AccessorType::SCALAR;
        std::vector<float> min;
        std::vector<float> max;
    };

    /**
     * @brief glTF texture info
     */
    struct TextureInfo
    {
        size_t index    = 0;
        size_t texCoord = 0;
    };

    /**
     * @brief glTF material structure
     */
    struct Material
    {
        std::string name;

        // PBR Metallic Roughness
        glm::vec4   baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
        TextureInfo baseColorTexture;
        float       metallicFactor  = 1.0f;
        float       roughnessFactor = 1.0f;
        TextureInfo metallicRoughnessTexture;

        // Additional material properties
        TextureInfo normalTexture;
        TextureInfo occlusionTexture;
        TextureInfo emissiveTexture;
        glm::vec3   emissiveFactor{0.0f, 0.0f, 0.0f};

        // Legacy properties for compatibility
        glm::vec3 ambient{0.2f, 0.2f, 0.2f};
        glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
        glm::vec3 specular{0.0f, 0.0f, 0.0f};
        float     shininess{0.0f};

        // Texture paths (resolved from indices)
        std::string diffuseTexturePath;
        std::string metallicRoughnessTexturePath;
        std::string normalTexturePath;

        // Texture transform properties (from KHR_texture_transform extension)
        float     texRotation{0.0f};     // Rotation in radians
        glm::vec2 texScale{1.0f, 1.0f};  // Scale factors for U and V
        glm::vec2 texOffset{0.0f, 0.0f}; // Offset for U and V
    };

    /**
     * @brief glTF primitive structure
     */
    struct Primitive
    {
        std::unordered_map<std::string, size_t> attributes; // attribute name -> accessor index
        size_t                                  indices  = SIZE_MAX; // accessor index for indices
        size_t                                  material = SIZE_MAX;
        PrimitiveMode                           mode     = PrimitiveMode::TRIANGLES;
    };

    /**
     * @brief glTF mesh structure
     */
    struct Mesh
    {
        std::string            name;
        std::vector<Primitive> primitives;
    };

    /**
     * @brief glTF image structure
     */
    struct Image
    {
        std::string uri;
        std::string mimeType;
        size_t      bufferView = SIZE_MAX;
    };

    /**
     * @brief glTF texture structure
     */
    struct Texture
    {
        size_t source  = SIZE_MAX; // image index
        size_t sampler = SIZE_MAX;
    };

    /**
     * @brief glTF node structure
     */
    struct Node
    {
        std::string         name;
        std::vector<size_t> children;
        size_t              mesh = SIZE_MAX;
        glm::mat4           matrix{1.0f};
        glm::vec3           translation{0.0f};
        glm::vec4           rotation{0.0f, 0.0f, 0.0f, 1.0f}; // quaternion
        glm::vec3           scale{1.0f};
    };

    /**
     * @brief glTF scene structure
     */
    struct Scene
    {
        std::string         name;
        std::vector<size_t> nodes;
    };

    /**
     * @brief Get material groups (separate vertex/index data for each
     * material)
     * @return Vector of material groups
     */
    std::vector<Model::MaterialGroup> getMaterialGroups() const;
  private:
    // Full glTF parsing methods
    /**
     * @brief Parse the main glTF JSON document
     * @param gltfContent JSON content as string
     * @return true if parsing successful
     */
    bool parseGLTFDocument(const std::string& gltfContent);
    /**
     * @brief Parse buffer definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseBuffers(const nlohmann::json& gltfJson);
    /**
     * @brief Parse buffer view definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseBufferViews(const nlohmann::json& gltfJson);
    /**
     * @brief Parse accessor definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseAccessors(const nlohmann::json& gltfJson);
    /**
     * @brief Parse material definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseMaterials(const nlohmann::json& gltfJson);
    /**
     * @brief Parse mesh definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseMeshes(const nlohmann::json& gltfJson);
    /**
     * @brief Parse node definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseNodes(const nlohmann::json& gltfJson);
    /**
     * @brief Parse scene definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseScenes(const nlohmann::json& gltfJson);
    /**
     * @brief Parse image definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseImages(const nlohmann::json& gltfJson);
    /**
     * @brief Parse texture definitions from glTF JSON
     * @param gltfJson The parsed glTF JSON document
     * @return true if parsing successful
     */
    bool parseTextures(const nlohmann::json& gltfJson);

    // Buffer and data access methods
    /**
     * @brief Load buffer data from file or embedded data
     * @param bufferIndex Index of buffer to load
     * @param baseDir Base directory for resolving relative paths
     * @return true if loading successful
     */
    bool loadBuffer(size_t bufferIndex, const std::string& baseDir);
    /**
     * @brief Get typed data from a glTF accessor
     * @tparam T Data type to extract
     * @param accessorIndex Index of accessor to read from
     * @return Vector of typed data
     */
    template <typename T> std::vector<T> getAccessorData(size_t accessorIndex);
    /**
     * @brief Get the size in bytes of a component type
     * @param componentType glTF component type
     * @return Size in bytes
     */
    size_t getComponentSize(ComponentType componentType) const;
    /**
     * @brief Get the number of components for an accessor type
     * @param type glTF accessor type (SCALAR, VEC2, VEC3, etc.)
     * @return Number of components
     */
    size_t getTypeSize(AccessorType type) const;
    /**
     * @brief Parse accessor type from string
     * @param typeStr Type string from glTF JSON
     * @return Parsed accessor type
     */
    AccessorType parseAccessorType(const std::string& typeStr);

    // Mesh processing methods
    /**
     * @brief Process a glTF scene and its nodes
     * @param sceneIndex Index of scene to process
     */
    void processScene(size_t sceneIndex);
    /**
     * @brief Process a glTF node recursively
     * @param nodeIndex Index of node to process
     * @param parentTransform Transformation matrix from parent nodes
     */
    void processNode(size_t nodeIndex, const glm::mat4& parentTransform = glm::mat4(1.0f));
    /**
     * @brief Process a mesh primitive and extract geometry
     * @param primitive Primitive definition from glTF
     * @param meshName Name of the parent mesh
     * @param transform World transformation matrix
     */
    void processPrimitive(const Primitive& primitive, const std::string& meshName,
                          const glm::mat4& transform);

    // Material processing methods
    /**
     * @brief Process material properties and resolve textures
     * @param materialIndex Index of material to process
     * @param material Material structure to populate
     */
    void processMaterial(size_t materialIndex, Material& material);
    /**
     * @brief Resolve texture file path from texture index
     * @param textureIndex Index of texture in glTF
     * @return Resolved file path or empty string if not found
     */
    std::string resolveTexturePath(size_t textureIndex);
    /**
     * @brief Parse texture transform extension if present
     * @param textureJson Texture JSON object
     * @param texInfo Texture info to update
     * @param material Material to store transform data
     * @return true if transform was parsed
     */
    bool parseTextureTransform(const nlohmann::json& textureJson, TextureInfo& texInfo,
                               Material& material);

    /**
     * @brief Apply intelligent material enhancements based on material
     * properties and names
     * @param group Material group to enhance
     */
    void applyMaterialIntelligence(Model::MaterialGroup& group) const;

    /**
     * @brief Create default material
     */
    void createDefaultMaterial();

    /**
     * @brief Create placeholder geometry for testing
     */
    void createPlaceholderGeometry();

    /**
     * @brief glTF buffer data
     */
    std::vector<Buffer> m_buffers;

    /**
     * @brief glTF buffer views
     */
    std::vector<BufferView> m_bufferViews;

    /**
     * @brief glTF accessors
     */
    std::vector<Accessor> m_accessors;

    /**
     * @brief glTF materials
     */
    std::vector<Material> m_materials;

    /**
     * @brief glTF meshes
     */
    std::vector<Mesh> m_meshes;

    /**
     * @brief glTF nodes
     */
    std::vector<Node> m_nodes;

    /**
     * @brief glTF scenes
     */
    std::vector<Scene> m_scenes;

    /**
     * @brief glTF images
     */
    std::vector<Image> m_images;

    /**
     * @brief glTF textures
     */
    std::vector<Texture> m_textures;

    /**
     * @brief Default scene index
     */
    size_t m_defaultScene{0};

    /**
     * @brief Base directory for relative paths
     */
    std::string m_baseDirectory;

    /**
     * @brief Material vertices grouped by material name
     */
    std::unordered_map<std::string, std::vector<Mesh3DRenderer::Vertex>> m_materialVertices;

    /**
     * @brief Material indices grouped by material name
     */
    std::unordered_map<std::string, std::vector<uint32_t>> m_materialIndices;
};

} // namespace ti::graphics

#endif // GRAPHICS_GLTF_MODEL_H_
