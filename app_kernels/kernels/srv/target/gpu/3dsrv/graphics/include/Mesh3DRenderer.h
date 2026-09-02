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

#ifndef GRAPHICS_MESH3D_RENDERER_H_
#define GRAPHICS_MESH3D_RENDERER_H_

#include <GLES3/gl32.h>
#include <graphics/include/Renderable.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ti::graphics
{

/**
 * @brief Specialized renderer for 3D mesh objects
 *
 * This class handles the rendering of 3D primitives like cubes, spheres,
 * pyramids, and their textured variants. It inherits from Renderable to
 * integrate with the main rendering pipeline.
 */
class Mesh3DRenderer : public Renderable
{
  public:
    /**
     * @brief Enumeration of supported 3D mesh types
     */
    enum class MeshType
    {
        Cube,
        Sphere,
        Pyramid,
        Custom
    };

    /**
     * @brief Structure representing vertex data
     */
    struct Vertex
    {
        glm::vec3 position; ///< Vertex position
        glm::vec3 normal;   ///< Vertex normal
        glm::vec2 texCoord; ///< Texture coordinates
    };

    /**
     * @brief Structure representing material properties
     *
     * Enhanced with PBR (Physically Based Rendering) properties for realistic
     * rendering of complex materials like car paint, chrome, glass, and
     * rubber.
     */
    struct Material
    {
        // Traditional Blinn-Phong properties (for backward compatibility)
        glm::vec3 ambient;   ///< Ambient color
        glm::vec3 diffuse;   ///< Diffuse color/base color
        glm::vec3 specular;  ///< Specular color
        float     shininess; ///< Specular shininess

        // PBR properties
        float metallic;     ///< Metallic factor (0-1)
        float roughness;    ///< Surface roughness (0-1)
        float ior;          ///< Index of refraction (for glass, water)
        float transmission; ///< Transmission factor for transparent materials

        // Car paint specific properties
        float clearcoat;            ///< Clear coat layer strength
        float clearcoatRoughness;   ///< Clear coat layer roughness
        float anisotropy;           ///< Anisotropic reflection strength (for brushed
                                    ///< metals)
        glm::vec3 emissive;         ///< Emissive color (for lights, glowing parts)
        float     emissiveStrength; ///< Strength of emissive effect

        // Texture maps
        GLuint normalMap;            ///< Normal mapping texture
        GLuint metallicRoughnessMap; ///< Combined metallic-roughness map
        GLuint aoMap;                ///< Ambient occlusion map
        GLuint emissiveMap;          ///< Emissive map for lights

        /**
         * @brief Default constructor with reasonable defaults
         */
        Material()
            : ambient(0.2f, 0.2f, 0.2f), diffuse(0.8f, 0.8f, 0.8f), specular(0.5f, 0.5f, 0.5f),
              shininess(32.0f), metallic(0.0f), roughness(0.5f), ior(1.45f), transmission(0.0f),
              clearcoat(0.0f), clearcoatRoughness(0.0f), anisotropy(0.0f),
              emissive(0.0f, 0.0f, 0.0f), emissiveStrength(1.0f), normalMap(0),
              metallicRoughnessMap(0), aoMap(0), emissiveMap(0)
        {
        }
    };

    /**
     * @brief Structure representing a submesh with its own material
     */
    struct SubMesh
    {
        std::vector<uint32_t> indices;       ///< Index data for this submesh
        int                   materialIndex; ///< Index into materials array (-1 if none)
    };

    /**
     * @brief Structure representing a 3D mesh element
     */
    struct MeshElement
    {
        MeshType              type;      ///< Type of mesh
        std::vector<Vertex>   vertices;  ///< Vertex data
        std::vector<uint32_t> indices;   ///< Index data (used when no submeshes defined)
        std::vector<SubMesh>  submeshes; ///< Submeshes with different materials
        GLuint                textureId; ///< Default texture ID (0 if untextured)
        glm::vec3             position;  ///< Position in world space
        glm::vec3             rotation;  ///< Rotation (Euler angles)
        glm::vec3             scale;     ///< Scale factors
        glm::vec4             color;     ///< Base color/tint (alpha < 1.0 indicates transparency)
        bool                  visible;   ///< Visibility state
        GLuint                vao;       ///< Vertex array object
        GLuint                vbo;       ///< Vertex buffer object
        GLuint                ebo;       ///< Element buffer object
        int                   materialIndex;  ///< Default material index (-1 if none)
        bool                  castsShadows;   ///< Whether this mesh casts shadows
        bool                  receiveShadows; ///< Whether this mesh receives shadows
    };

    /**
     * @brief Constructor
     * @param width Viewport width
     * @param height Viewport height
     */
    Mesh3DRenderer(uint32_t width, uint32_t height);

    /**
     * @brief Destructor
     */
    ~Mesh3DRenderer() override;

    /**
     * @brief Initialize mesh rendering resources
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Render all mesh elements
     * @param viewProjection View-projection matrix
     */
    void render(const glm::mat4& viewProjection) override;

    /**
     * @brief Clean up mesh rendering resources
     */
    void cleanup() override;

    /**
     * @brief Add a cube mesh
     * @param position Position in world space
     * @param size Size in each dimension
     * @param color Base color
     * @param textureId Optional texture ID
     * @return ID of the created mesh
     */
    uint32_t addCube(const glm::vec3& position, const glm::vec3& size,
                     const glm::vec4& color = glm::vec4(1.0f), GLuint textureId = 0);

    /**
     * @brief Add a sphere mesh
     * @param position Position in world space
     * @param radius Sphere radius
     * @param segments Number of segments (detail level)
     * @param color Base color
     * @param textureId Optional texture ID
     * @return ID of the created mesh
     */
    uint32_t addSphere(const glm::vec3& position, float radius, uint32_t segments = 32,
                       const glm::vec4& color = glm::vec4(1.0f), GLuint textureId = 0);

    /**
     * @brief Add a pyramid mesh
     * @param position Position in world space
     * @param baseSize Base width and depth
     * @param height Pyramid height
     * @param color Base color
     * @param textureId Optional texture ID
     * @return ID of the created mesh
     */
    uint32_t addPyramid(const glm::vec3& position, const glm::vec2& baseSize, float height,
                        const glm::vec4& color = glm::vec4(1.0f), GLuint textureId = 0);

    /**
     * @brief Add a custom mesh
     * @param vertices Vertex data
     * @param indices Index data
     * @param position Position in world space
     * @param color Base color
     * @param textureId Optional texture ID
     * @param materialIndex Optional material index (-1 for default material)
     * @return ID of the created mesh
     */
    uint32_t addCustomMesh(const std::vector<Vertex>&   vertices,
                           const std::vector<uint32_t>& indices, const glm::vec3& position,
                           const glm::vec4& color = glm::vec4(1.0f), GLuint textureId = 0,
                           int materialIndex = -1);

    /**
     * @brief Add a material to the renderer (legacy version)
     * @param ambient Ambient color
     * @param diffuse Diffuse color
     * @param specular Specular color
     * @param shininess Specular shininess
     * @return Index of the added material
     */
    int addMaterial(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
                    float shininess);

    /**
     * @brief Add a PBR material to the renderer
     * @param diffuse Base color
     * @param metallic Metallic factor (0-1)
     * @param roughness Surface roughness (0-1)
     * @param textureId Diffuse texture ID (0 if none)
     * @return Index of the added material
     */
    int addPBRMaterial(const glm::vec3& diffuse, float metallic, float roughness,
                       GLuint textureId = 0);

    /**
     * @brief Add a car paint material to the renderer
     * @param baseColor Base color
     * @param metallic Metallic factor (0-1)
     * @param roughness Surface roughness (0-1)
     * @param clearcoat Clear coat layer strength (0-1)
     * @param clearcoatRoughness Clear coat layer roughness (0-1)
     * @return Index of the added material
     */
    int addCarPaintMaterial(const glm::vec3& baseColor, float metallic, float roughness,
                            float clearcoat, float clearcoatRoughness);

    /**
     * @brief Set material texture maps
     * @param materialIndex Index of the material to modify
     * @param normalMapId Normal map texture ID (0 if none)
     * @param metallicRoughnessMapId Metallic-roughness map texture ID (0 if
     * none)
     * @param aoMapId Ambient occlusion map texture ID (0 if none)
     * @param emissiveMapId Emissive map texture ID (0 if none)
     */
    void setMaterialTextureMaps(int materialIndex, GLuint normalMapId = 0,
                                GLuint metallicRoughnessMapId = 0, GLuint aoMapId = 0,
                                GLuint emissiveMapId = 0);

    /**
     * @brief Update mesh transform
     * @param id Mesh element ID
     * @param position New position
     * @param rotation New rotation
     * @param scale New scale
     */
    void updateTransform(uint32_t id, const glm::vec3& position,
                         const glm::vec3& rotation = glm::vec3(0.0f),
                         const glm::vec3& scale    = glm::vec3(1.0f));

    /**
     * @brief Load a texture for use with meshes
     * @param path Path to texture file
     * @param flipVertically Whether to flip the texture vertically during
     * loading
     * @return Texture ID if successful, 0 if failed
     */
    GLuint loadTexture(const std::string& path, bool flipVertically = false);

    /**
     * @brief Load a cubemap texture for environment mapping
     * @param paths Array of 6 paths to cubemap face textures (right, left,
     * top, bottom, front, back)
     * @return Cubemap texture ID if successful, 0 if failed
     */
    GLuint loadCubemap(const std::vector<std::string>& paths);

    /**
     * @brief Set the environment map for reflections
     * @param cubemapId Cubemap texture ID
     */
    void setEnvironmentMap(GLuint cubemapId);

    /**
     * @brief Add a submesh to an existing mesh
     * @param meshId ID of the mesh to add submesh to
     * @param indices Index data for the submesh
     * @param materialIndex Material index for the submesh
     * @return Index of the added submesh
     */
    int addSubmesh(uint32_t meshId, const std::vector<uint32_t>& indices, int materialIndex);

    /**
     * @brief Set lighting parameters
     * @param lightPositions Array of light positions
     * @param lightColors Array of light colors
     * @param lightIntensities Array of light intensities
     */
    void setLights(const std::vector<glm::vec3>& lightPositions,
                   const std::vector<glm::vec3>& lightColors,
                   const std::vector<float>&     lightIntensities);

    /**
     * @brief Set viewport size
     * @param width New viewport width
     * @param height New viewport height
     */
    void setViewport(uint32_t width, uint32_t height);

    /**
     * @brief Enable or disable blending for a specific mesh
     * @param meshId ID of the mesh to set blending for
     * @param enable Whether to enable blending
     */
    void setMeshBlendMode(uint32_t meshId, bool enable);

    /**
     * @brief Get the number of materials
     * @return Number of materials
     */
    size_t getMaterialCount() const
    {
        return mMaterials.size();
    }

    /**
     * @brief Get material by index
     * @param index Material index
     * @return Reference to material
     */
    Material& getMaterial(int index)
    {
        return mMaterials[index];
    }

    /**
     * @brief Get material by index (const version)
     * @param index Material index
     * @return Const reference to material
     */
    const Material& getMaterial(int index) const
    {
        return mMaterials[index];
    }
  private:
    /**
     * @brief Initialize shader programs
     * @return true if initialization was successful
     */
    bool initializeShaders();

    /**
     * @brief Initialize PBR shader program
     * @return true if initialization was successful
     */
    bool initializePBRShaders();

    /**
     * @brief Generate cube mesh data
     * @param size Size in each dimension
     * @return Pair of vertices and indices
     */
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> generateCubeMesh(const glm::vec3& size);

    /**
     * @brief Generate sphere mesh data
     * @param radius Sphere radius
     * @param segments Number of segments
     * @return Pair of vertices and indices
     */
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> generateSphereMesh(float    radius,
                                                                             uint32_t segments);

    /**
     * @brief Generate pyramid mesh data
     * @param baseSize Base width and depth
     * @param height Pyramid height
     * @return Pair of vertices and indices
     */
    std::pair<std::vector<Vertex>, std::vector<uint32_t>>
    generatePyramidMesh(const glm::vec2& baseSize, float height);

    /**
     * @brief Create buffers for mesh data
     * @param mesh Mesh element to create buffers for
     */
    void createMeshBuffers(MeshElement& mesh);

    uint32_t                                m_width;           ///< Viewport width
    uint32_t                                m_height;          ///< Viewport height
    std::vector<MeshElement>                mMeshElements;     ///< Mesh elements
    std::vector<Material>                   mMaterials;        ///< Material properties
    std::unordered_map<std::string, GLuint> mLoadedTextures;   ///< Cached textures
    GLuint                                  mShaderProgram;    ///< Traditional shader program
    GLuint                                  mPBRShaderProgram; ///< PBR shader program
    GLuint    mShadowMapShaderProgram;                         ///< Shadow mapping shader program
    glm::mat4 mProjection;                                     ///< Projection matrix

    // Environment mapping
    GLuint mEnvironmentMapId; ///< Environment cubemap for reflections

    // Lighting
    std::vector<glm::vec3> mLightPositions;   ///< Light positions
    std::vector<glm::vec3> mLightColors;      ///< Light colors
    std::vector<float>     mLightIntensities; ///< Light intensities

    // Shadow mapping
    GLuint    mShadowMapFBO;     ///< Framebuffer for shadow mapping
    GLuint    mShadowMap;        ///< Shadow map texture
    glm::mat4 mLightSpaceMatrix; ///< Light space transformation matrix

    // Performance optimization flags
    bool mEnableShadows;            ///< Whether shadow mapping is enabled
    bool mEnableEnvironmentMapping; ///< Whether environment mapping is enabled
};

} // namespace ti::graphics

#endif // GRAPHICS_MESH3D_RENDERER_H_
