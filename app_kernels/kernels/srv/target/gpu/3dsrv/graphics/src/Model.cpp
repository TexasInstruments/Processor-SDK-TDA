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
#include <cctype>
#include <graphics/include/Model.h>
#include <utils/include/logger.h>

namespace ti::graphics
{

std::vector<uint32_t> Model::render(Mesh3DRenderer &renderer) const
{
    std::vector<uint32_t> meshIds;

    if (!isLoaded())
    {
        LOG_ERROR("Cannot render - model is not loaded");
        return meshIds;
    }

    // Log model properties before rendering
    logMaterialProperties();

    // Get all material groups from the model
    auto materialGroups = getMaterialGroups();

    LOG_INFO("Model has %zu material groups", materialGroups.size());

    for (const auto &group : materialGroups)
    {
        if (group.vertices.empty())
            continue;

        // Check if this material should use PBR rendering
        bool usePBR =
        (group.material.metallic > 0.0f) || (group.material.roughness < 1.0f) ||
        (group.material.alpha < 1.0f); // Use PBR for transparent materials too

        int materialIndex;
        if (usePBR)
        {
            // Use PBR material for enhanced materials
            materialIndex = renderer.addPBRMaterial(group.material.diffuse,
                                                    group.material.metallic,
                                                    group.material.roughness);

            LOG_INFO("Created PBR material for '%s' (metallic=%.2f, "
                     "roughness=%.2f)",
                     group.materialName.c_str(),
                     group.material.metallic,
                     group.material.roughness);
        }
        else
        {
            // Create standard material for this group
            materialIndex = renderer.addMaterial(group.material.ambient,
                                                 group.material.diffuse,
                                                 group.material.specular,
                                                 group.material.shininess);

            LOG_INFO("Created standard material for '%s'", group.materialName.c_str());
        }

        // Load textures for this material if it has them
        GLuint groupTextureId = 0;
        GLuint normalMapId    = 0;

        // Load diffuse texture if available
        if (!group.material.diffuseTexture.empty())
        {
            groupTextureId =
            renderer.loadTexture(group.material.diffuseTexture, getFlipImageData());
            if (!groupTextureId)
            {
                LOG_WARN("Failed to load texture: %s",
                         group.material.diffuseTexture.c_str());
            }
            else
            {
                LOG_INFO("Loaded diffuse texture: %s",
                         group.material.diffuseTexture.c_str());
            }
        }

        // Load normal map if available
        if (!group.material.normalTexture.empty())
        {
            normalMapId =
            renderer.loadTexture(group.material.normalTexture, getFlipImageData());
            if (!normalMapId)
            {
                LOG_WARN("Failed to load normal map: %s",
                         group.material.normalTexture.c_str());
            }
            else
            {
                LOG_INFO("Loaded normal map: %s", group.material.normalTexture.c_str());
            }
        }

        // Add mesh for this material group
        uint32_t meshId =
        renderer.addCustomMesh(group.vertices,
                               group.indices,
                               glm::vec3(0.0f), // Center position
                               glm::vec4(group.material.diffuse, group.material.alpha), // Use material diffuse color with alpha
                               groupTextureId,
                               materialIndex);

        // Set material texture maps if needed
        if (normalMapId && materialIndex >= 0)
        {
            renderer.setMaterialTextureMaps(materialIndex, normalMapId);
            LOG_INFO("Set normal map for material '%s'", group.materialName.c_str());
        }

        // Enable blending for transparent materials
        if (group.material.alpha < 1.0f)
        {
            renderer.setMeshBlendMode(meshId, true);

            LOG_INFO("Enabled blending for mesh with material '%s' "
                     "(alpha=%.2f)",
                     group.materialName.c_str(),
                     group.material.alpha);
        }

        meshIds.push_back(meshId);

        LOG_INFO("Created mesh for material '%s' with %zu vertices, %zu "
                 "indices",
                 group.materialName.c_str(),
                 group.vertices.size(),
                 group.vertices.size());
    }

    // If no material groups were found but the model is loaded,
    // create a basic material with the model's default properties
    if (meshIds.empty() && !getVertices().empty())
    {
        int materialIndex = renderer.addMaterial(getMaterialAmbient(),
                                                 getMaterialDiffuse(),
                                                 getMaterialSpecular(),
                                                 getMaterialShininess());

        LOG_INFO("Created default material for model");

        // Add model to renderer with default material
        uint32_t meshId =
        renderer.addCustomMesh(getVertices(),
                               getIndices(),
                               glm::vec3(0.0f),    // Center position
                               getMaterialColor(), // Use default material color
                               0,                  // No texture
                               materialIndex);

        meshIds.push_back(meshId);

        LOG_INFO("Created default mesh with %zu vertices and %zu indices",
                 getVertices().size(),
                 getIndices().size());
    }

    return meshIds;
}

Model::MaterialType Model::classifyMaterial(const Material &material)
{
    // Check for glass-like materials first
    if (isGlassLikeName(material.name) || isTransparent(material))
    {
        return MaterialType::Glass;
    }

    // Check for highly reflective materials
    if (isHighlyReflective(material))
    {
        return MaterialType::Reflective;
    }

    // Default to standard material
    return MaterialType::Standard;
}

bool Model::isGlassLikeName(const std::string &materialName)
{
    std::string lowerName = materialName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    return lowerName.find("glass") != std::string::npos ||
           lowerName.find("window") != std::string::npos ||
           lowerName.find("windshield") != std::string::npos ||
           lowerName.find("headlight") != std::string::npos ||
           lowerName.find("transparent") != std::string::npos;
}

bool Model::isTransparent(const Material &material)
{
    return material.alpha < 1.0f;
}

bool Model::isHighlyReflective(const Material &material)
{
    // Check if material has high specularity
    float specularSum =
    material.specular.r + material.specular.g + material.specular.b;
    bool highSpecular = specularSum > 1.5f || material.shininess > 50.0f;

    // Check if material already has high metallic value
    bool highMetallic = material.metallic > 0.5f;

    return highSpecular || highMetallic;
}

void Model::enhanceGlassMaterial(Material &material)
{
    // Set appropriate glass color - slightly blue-tinted
    material.diffuse = glm::vec3(0.6f, 0.7f, 0.8f);

    // Set appropriate PBR properties for glass
    material.metallic = 0.1f; // Glass has slight metallic property for reflections
    material.roughness = 0.05f; // Very smooth for transparency/refraction

    // Ensure proper transparency
    material.alpha = 0.2f; // Make it more transparent

    LOG_DEBUG("Enhanced glass material '%s'", material.name.c_str());
}

void Model::enhanceReflectiveMaterial(Material &material)
{
    // Use existing metallic value if reasonable, otherwise default to high
    if (material.metallic <= 0.0f)
    {
        material.metallic = 0.8f; // High metallic for reflective surfaces
    }

    // Use existing roughness if reasonable, otherwise make smooth
    if (material.roughness >= 1.0f)
    {
        material.roughness = 0.05f; // Low roughness for reflection
    }

    LOG_DEBUG("Enhanced reflective material '%s' (metallic=%.2f, "
              "roughness=%.2f)",
              material.name.c_str(),
              material.metallic,
              material.roughness);
}

void Model::applyMaterialIntelligence(MaterialGroup &group)
{
    // Apply intelligent material enhancement based on material properties
    MaterialType materialType = classifyMaterial(group.material);
    switch (materialType)
    {
        case MaterialType::Glass:
            enhanceGlassMaterial(group.material);
            break;
        case MaterialType::Reflective:
            enhanceReflectiveMaterial(group.material);
            break;
        case MaterialType::Standard:
            // Keep original properties for standard materials
            break;
    }
}

void Model::logMaterialProperties() const
{
    // Report basic model info
    LOG_INFO("Model loaded successfully:");
    LOG_INFO("  Vertices: %zu", getVertices().size());
    LOG_INFO("  Indices: %zu", getIndices().size());

    // Report texture information
    std::string diffuseTexture = getDiffuseTexturePath();
    if (!diffuseTexture.empty())
    {
        LOG_INFO("Model has diffuse texture: %s", diffuseTexture.c_str());
    }

    // Report material properties
    LOG_INFO("Material properties:");
    LOG_INFO("  Color: %.2f, %.2f, %.2f, %.2f",
             getMaterialColor().r,
             getMaterialColor().g,
             getMaterialColor().b,
             getMaterialColor().a);
    LOG_INFO("  Ambient: %.2f, %.2f, %.2f",
             getMaterialAmbient().r,
             getMaterialAmbient().g,
             getMaterialAmbient().b);
    LOG_INFO("  Diffuse: %.2f, %.2f, %.2f",
             getMaterialDiffuse().r,
             getMaterialDiffuse().g,
             getMaterialDiffuse().b);
    LOG_INFO("  Specular: %.2f, %.2f, %.2f",
             getMaterialSpecular().r,
             getMaterialSpecular().g,
             getMaterialSpecular().b);
    LOG_INFO("  Shininess: %.2f", getMaterialShininess());

    if (!diffuseTexture.empty())
    {
        LOG_INFO("  Diffuse Texture: %s", diffuseTexture.c_str());
    }
}

} // namespace ti::graphics
