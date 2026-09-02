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
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>
#include <type_traits>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include <GLES3/gl32.h>
#include <graphics/include/GLTFModel.h>
#include <utils/include/logger.h>
#include <utils/include/resources.h>

using json = nlohmann::json;

namespace ti::graphics
{

Model::Status GLTFModel::load(const std::string &path)
{
    LOG_DEBUG("Loading glTF model from '%s'", path.c_str());

    // Clear previous data
    m_vertices.clear();
    m_indices.clear();
    m_buffers.clear();
    m_bufferViews.clear();
    m_accessors.clear();
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_scenes.clear();
    m_images.clear();
    m_textures.clear();
    m_materialVertices.clear();
    m_materialIndices.clear();

    // Store base directory for resolving relative paths
    std::filesystem::path gltfPath(path);
    m_baseDirectory = gltfPath.parent_path().string();
    if (!m_baseDirectory.empty() && m_baseDirectory.back() != '/' &&
        m_baseDirectory.back() != '\\')
    {
        m_baseDirectory += '/';
    }

    // Open and read the entire glTF file
    std::ifstream file(path);
    if (!file.is_open())
    {
        setError("Failed to open glTF file: " + path);
        return Model::Status::FileNotFound;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string gltfContent = buffer.str();
    file.close();

    // Parse the glTF document
    if (!parseGLTFDocument(gltfContent))
    {
        setError("Failed to parse glTF document");
        return Model::Status::InvalidFormat;
    }

    // Process the default scene
    if (m_defaultScene < m_scenes.size())
    {
        processScene(m_defaultScene);
    }
    else if (!m_scenes.empty())
    {
        processScene(0);
    }

    m_loaded = true;
    LOG_DEBUG("Successfully loaded glTF model with %zu total vertices and %zu "
              "total indices",
              m_vertices.size(),
              m_indices.size());
    return Model::Status::Success;
}

bool GLTFModel::parseGLTFDocument(const std::string &gltfContent)
{
    LOG_DEBUG("Parsing glTF document structure");

    try
    {
        json gltfJson = json::parse(gltfContent);

        // Parse each section of the glTF file
        if (!parseBuffers(gltfJson))
        {
            LOG_WARN(
            "Failed to parse buffers, continuing without external buffers");
        }

        if (!parseBufferViews(gltfJson))
        {
            LOG_WARN("Failed to parse buffer views");
            return false;
        }

        if (!parseAccessors(gltfJson))
        {
            LOG_WARN("Failed to parse accessors");
            return false;
        }

        if (!parseImages(gltfJson))
        {
            LOG_WARN("Failed to parse images, continuing without textures");
        }

        if (!parseTextures(gltfJson))
        {
            LOG_WARN("Failed to parse textures, continuing without textures");
        }

        if (!parseMaterials(gltfJson))
        {
            LOG_WARN("Failed to parse materials, using defaults");
            createDefaultMaterial();
        }

        if (!parseMeshes(gltfJson))
        {
            LOG_ERROR("Failed to parse meshes");
            return false;
        }

        if (!parseNodes(gltfJson))
        {
            LOG_WARN("Failed to parse nodes, using default structure");
        }

        if (!parseScenes(gltfJson))
        {
            LOG_WARN("Failed to parse scenes, using default scene");
        }

        // Parse default scene
        if (gltfJson.contains("scene") && gltfJson["scene"].is_number())
        {
            m_defaultScene = gltfJson["scene"].get<size_t>();
        }

        LOG_DEBUG("Parsed glTF document: %zu buffers, %zu materials, %zu "
                  "meshes, %zu nodes, %zu scenes",
                  m_buffers.size(),
                  m_materials.size(),
                  m_meshes.size(),
                  m_nodes.size(),
                  m_scenes.size());

        return true;
    }
    catch (const json::exception &e)
    {
        LOG_ERROR("JSON parsing error: %s", e.what());
        return false;
    }
}

bool GLTFModel::parseBuffers(const json &gltfJson)
{
    if (!gltfJson.contains("buffers") || !gltfJson["buffers"].is_array())
    {
        return false;
    }

    for (const auto &bufferJson : gltfJson["buffers"])
    {
        Buffer buffer;

        if (bufferJson.contains("uri") && bufferJson["uri"].is_string())
        {
            buffer.uri = bufferJson["uri"].get<std::string>();
        }

        if (bufferJson.contains("byteLength") && bufferJson["byteLength"].is_number())
        {
            buffer.byteLength = bufferJson["byteLength"].get<size_t>();
        }

        m_buffers.push_back(buffer);

        if (!loadBuffer(m_buffers.size() - 1, m_baseDirectory))
        {
            LOG_WARN("Failed to load buffer %zu", m_buffers.size() - 1);
        }

        LOG_DEBUG("Parsed buffer: uri='%s', length=%zu",
                  buffer.uri.c_str(),
                  buffer.byteLength);
    }

    return true;
}

bool GLTFModel::loadBuffer(size_t bufferIndex, const std::string &baseDir)
{
    if (bufferIndex >= m_buffers.size())
        return false;

    Buffer &buffer = m_buffers[bufferIndex];

    // Handle data URIs (base64 embedded data)
    if (buffer.uri.substr(0, 5) == "data:")
    {
        // Find base64 data after "base64,"
        size_t base64Start = buffer.uri.find("base64,");
        if (base64Start != std::string::npos)
        {
            base64Start += 7; // Skip "base64,"
            std::string base64Data = buffer.uri.substr(base64Start);

            // Decode base64 (simplified implementation)
            // A full implementation would use a proper base64 decoder
            LOG_WARN(
            "Base64 data URIs not fully implemented, using placeholder data");
            buffer.data.resize(buffer.byteLength, 0);
            return true;
        }
    }

    // Load external file
    std::string   fullPath = baseDir + buffer.uri;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open buffer file: %s", fullPath.c_str());
        return false;
    }

    buffer.data.resize(buffer.byteLength);
    file.read(reinterpret_cast<char *>(buffer.data.data()), buffer.byteLength);

    if (!file.good())
    {
        LOG_ERROR("Failed to read buffer data from: %s", fullPath.c_str());
        return false;
    }

    LOG_DEBUG("Loaded buffer %zu: %zu bytes from %s",
              bufferIndex,
              buffer.data.size(),
              fullPath.c_str());
    return true;
}

bool GLTFModel::parseBufferViews(const json &gltfJson)
{
    if (!gltfJson.contains("bufferViews") || !gltfJson["bufferViews"].is_array())
    {
        return false;
    }

    for (const auto &viewJson : gltfJson["bufferViews"])
    {
        BufferView view;

        if (viewJson.contains("buffer") && viewJson["buffer"].is_number())
        {
            view.buffer = viewJson["buffer"].get<size_t>();
        }

        if (viewJson.contains("byteOffset") && viewJson["byteOffset"].is_number())
        {
            view.byteOffset = viewJson["byteOffset"].get<size_t>();
        }

        if (viewJson.contains("byteLength") && viewJson["byteLength"].is_number())
        {
            view.byteLength = viewJson["byteLength"].get<size_t>();
        }

        if (viewJson.contains("byteStride") && viewJson["byteStride"].is_number())
        {
            view.byteStride = viewJson["byteStride"].get<size_t>();
        }

        if (viewJson.contains("target") && viewJson["target"].is_number())
        {
            view.target = viewJson["target"].get<uint32_t>();
        }

        m_bufferViews.push_back(view);
        LOG_DEBUG("Parsed buffer view: buffer=%zu, offset=%zu, length=%zu",
                  view.buffer,
                  view.byteOffset,
                  view.byteLength);
    }

    return true;
}

bool GLTFModel::parseAccessors(const json &gltfJson)
{
    if (!gltfJson.contains("accessors") || !gltfJson["accessors"].is_array())
    {
        return false;
    }

    for (const auto &accessorJson : gltfJson["accessors"])
    {
        Accessor accessor;

        if (accessorJson.contains("bufferView") && accessorJson["bufferView"].is_number())
        {
            accessor.bufferView = accessorJson["bufferView"].get<size_t>();
        }

        if (accessorJson.contains("byteOffset") && accessorJson["byteOffset"].is_number())
        {
            accessor.byteOffset = accessorJson["byteOffset"].get<size_t>();
        }

        if (accessorJson.contains("componentType") &&
            accessorJson["componentType"].is_number())
        {
            accessor.componentType =
            static_cast<ComponentType>(accessorJson["componentType"].get<uint32_t>());
        }

        if (accessorJson.contains("normalized") && accessorJson["normalized"].is_boolean())
        {
            accessor.normalized = accessorJson["normalized"].get<bool>();
        }

        if (accessorJson.contains("count") && accessorJson["count"].is_number())
        {
            accessor.count = accessorJson["count"].get<size_t>();
        }

        if (accessorJson.contains("type") && accessorJson["type"].is_string())
        {
            accessor.type = parseAccessorType(accessorJson["type"].get<std::string>());
        }

        m_accessors.push_back(accessor);
        LOG_DEBUG("Parsed accessor: bufferView=%zu, count=%zu",
                  accessor.bufferView,
                  accessor.count);
    }

    return true;
}

GLTFModel::AccessorType GLTFModel::parseAccessorType(const std::string &typeStr)
{
    if (typeStr == "SCALAR")
        return AccessorType::SCALAR;
    if (typeStr == "VEC2")
        return AccessorType::VEC2;
    if (typeStr == "VEC3")
        return AccessorType::VEC3;
    if (typeStr == "VEC4")
        return AccessorType::VEC4;
    if (typeStr == "MAT2")
        return AccessorType::MAT2;
    if (typeStr == "MAT3")
        return AccessorType::MAT3;
    if (typeStr == "MAT4")
        return AccessorType::MAT4;
    return AccessorType::SCALAR;
}

size_t GLTFModel::getComponentSize(ComponentType componentType) const
{
    switch (componentType)
    {
        case ComponentType::BYTE:
        case ComponentType::UNSIGNED_BYTE:
            return 1;
        case ComponentType::SHORT:
        case ComponentType::UNSIGNED_SHORT:
            return 2;
        case ComponentType::UNSIGNED_INT:
        case ComponentType::FLOAT:
            return 4;
        default:
            return 4;
    }
}

size_t GLTFModel::getTypeSize(AccessorType type) const
{
    switch (type)
    {
        case AccessorType::SCALAR:
            return 1;
        case AccessorType::VEC2:
            return 2;
        case AccessorType::VEC3:
            return 3;
        case AccessorType::VEC4:
            return 4;
        case AccessorType::MAT2:
            return 4;
        case AccessorType::MAT3:
            return 9;
        case AccessorType::MAT4:
            return 16;
        default:
            return 1;
    }
}

template <typename T>
std::vector<T> GLTFModel::getAccessorData(size_t accessorIndex)
{
    std::vector<T> result;

    if (accessorIndex >= m_accessors.size())
    {
        LOG_ERROR("Invalid accessor index: %zu", accessorIndex);
        return result;
    }

    const Accessor &accessor = m_accessors[accessorIndex];
    if (accessor.bufferView >= m_bufferViews.size())
    {
        LOG_ERROR("Invalid buffer view index: %zu", accessor.bufferView);
        return result;
    }

    const BufferView &bufferView = m_bufferViews[accessor.bufferView];
    if (bufferView.buffer >= m_buffers.size())
    {
        LOG_ERROR("Invalid buffer index: %zu", bufferView.buffer);
        return result;
    }

    const Buffer &buffer = m_buffers[bufferView.buffer];
    if (buffer.data.empty())
    {
        LOG_ERROR("Buffer data is empty for buffer: %zu", bufferView.buffer);
        return result;
    }

    size_t componentSize = getComponentSize(accessor.componentType);
    size_t typeSize      = getTypeSize(accessor.type);
    size_t elementSize   = componentSize * typeSize;

    size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
    size_t stride = bufferView.byteStride > 0 ? bufferView.byteStride : elementSize;

    size_t totalComponents = accessor.count * typeSize;
    result.reserve(totalComponents);

    for (size_t i = 0; i < accessor.count; ++i)
    {
        size_t offset = dataOffset + i * stride;

        if (offset + elementSize > buffer.data.size())
        {
            LOG_ERROR("Buffer access out of bounds: offset=%zu, size=%zu",
                      offset,
                      buffer.data.size());
            break;
        }

        // Read all components for this element
        for (size_t component = 0; component < typeSize; ++component)
        {
            size_t componentOffset = offset + component * componentSize;

            // Convert based on component type
            T value = T{};
            if (std::is_same_v<T, float>)
            {
                if (accessor.componentType == ComponentType::FLOAT)
                {
                    const float *ptr =
                    reinterpret_cast<const float *>(&buffer.data[componentOffset]);
                    value = static_cast<T>(*ptr);
                }
                else if (accessor.componentType == ComponentType::UNSIGNED_SHORT)
                {
                    const uint16_t *ptr =
                    reinterpret_cast<const uint16_t *>(&buffer.data[componentOffset]);
                    value = static_cast<T>(*ptr) / static_cast<float>(UINT16_MAX);
                }
                else if (accessor.componentType == ComponentType::UNSIGNED_BYTE)
                {
                    const uint8_t *ptr =
                    reinterpret_cast<const uint8_t *>(&buffer.data[componentOffset]);
                    value = static_cast<T>(*ptr) / static_cast<float>(UINT8_MAX);
                }
            }
            else if (std::is_same_v<T, uint32_t>)
            {
                if (accessor.componentType == ComponentType::UNSIGNED_INT)
                {
                    const uint32_t *ptr =
                    reinterpret_cast<const uint32_t *>(&buffer.data[componentOffset]);
                    value = *ptr;
                }
                else if (accessor.componentType == ComponentType::UNSIGNED_SHORT)
                {
                    const uint16_t *ptr =
                    reinterpret_cast<const uint16_t *>(&buffer.data[componentOffset]);
                    value = static_cast<uint32_t>(*ptr);
                }
                else if (accessor.componentType == ComponentType::UNSIGNED_BYTE)
                {
                    const uint8_t *ptr =
                    reinterpret_cast<const uint8_t *>(&buffer.data[componentOffset]);
                    value = static_cast<uint32_t>(*ptr);
                }
            }

            result.push_back(value);
        }
    }

    return result;
}

// Continue with the rest of the implementation...
// This is getting quite long, so I'll implement it in stages

Model::Status GLTFModel::save(const std::string &path) const
{
    LOG_DEBUG("Saving glTF model to '%s'", path.c_str());

    try
    {
        json gltfJson;

        // Asset information
        gltfJson["asset"]["version"]   = "2.0";
        gltfJson["asset"]["generator"] = "Graphics Engine glTF Exporter";

        // Check if we have material groups for multi-material export
        bool isMultiMaterial =
        !m_materialVertices.empty() && m_materialVertices.size() > 1;

        std::vector<uint8_t> bufferData;
        size_t               bufferViewIndex = 0;
        size_t               accessorIndex   = 0;

        if (isMultiMaterial)
        {
            LOG_DEBUG("Exporting multi-material model with %zu material groups",
                      m_materialVertices.size());

            // Process each material group separately
            for (size_t matIndex = 0; matIndex < m_materials.size(); ++matIndex)
            {
                const Material &material = m_materials[matIndex];

                // Find vertices and indices for this material
                auto verticesIt = m_materialVertices.find(material.name);
                auto indicesIt  = m_materialIndices.find(material.name);

                if (verticesIt == m_materialVertices.end() ||
                    indicesIt == m_materialIndices.end())
                {
                    LOG_WARN("No geometry found for material: %s", material.name.c_str());
                    continue;
                }

                const auto &vertices = verticesIt->second;
                const auto &indices  = indicesIt->second;

                if (vertices.empty() || indices.empty())
                {
                    LOG_WARN("Empty geometry for material: %s", material.name.c_str());
                    continue;
                }

                LOG_DEBUG("Processing material %s: %zu vertices, %zu indices",
                          material.name.c_str(),
                          vertices.size(),
                          indices.size());

                // Calculate data sizes for this material group
                size_t posDataSize  = vertices.size() * 3 * sizeof(float);
                size_t normDataSize = vertices.size() * 3 * sizeof(float);
                size_t texDataSize  = vertices.size() * 2 * sizeof(float);
                size_t idxDataSize  = indices.size() * sizeof(uint32_t);

                // Store offsets for buffer views
                size_t posOffset  = bufferData.size();
                size_t normOffset = posOffset + posDataSize;
                size_t texOffset  = normOffset + normDataSize;
                size_t idxOffset  = texOffset + texDataSize;

                // Write position data
                for (const auto &vertex : vertices)
                {
                    const uint8_t *posBytes =
                    reinterpret_cast<const uint8_t *>(&vertex.position.x);
                    bufferData.insert(bufferData.end(), posBytes, posBytes + 12);
                }

                // Write normal data
                for (const auto &vertex : vertices)
                {
                    const uint8_t *normBytes =
                    reinterpret_cast<const uint8_t *>(&vertex.normal.x);
                    bufferData.insert(bufferData.end(), normBytes, normBytes + 12);
                }

                // Write texture coordinate data
                for (const auto &vertex : vertices)
                {
                    float texCoords[2] = {vertex.texCoord.x, vertex.texCoord.y};
                    const uint8_t *texBytes = reinterpret_cast<const uint8_t *>(texCoords);
                    bufferData.insert(bufferData.end(), texBytes, texBytes + 8);
                }

                // Write index data
                const uint8_t *idxBytes =
                reinterpret_cast<const uint8_t *>(indices.data());
                bufferData.insert(bufferData.end(), idxBytes, idxBytes + idxDataSize);

                // Create buffer views for this material group
                gltfJson["bufferViews"][bufferViewIndex]["buffer"]     = 0;
                gltfJson["bufferViews"][bufferViewIndex]["byteOffset"] = posOffset;
                gltfJson["bufferViews"][bufferViewIndex]["byteLength"] = posDataSize;
                gltfJson["bufferViews"][bufferViewIndex]["target"] = GL_ARRAY_BUFFER;
                size_t posBufferView = bufferViewIndex++;

                gltfJson["bufferViews"][bufferViewIndex]["buffer"]     = 0;
                gltfJson["bufferViews"][bufferViewIndex]["byteOffset"] = normOffset;
                gltfJson["bufferViews"][bufferViewIndex]["byteLength"] = normDataSize;
                gltfJson["bufferViews"][bufferViewIndex]["target"] = GL_ARRAY_BUFFER;
                size_t normBufferView = bufferViewIndex++;

                gltfJson["bufferViews"][bufferViewIndex]["buffer"]     = 0;
                gltfJson["bufferViews"][bufferViewIndex]["byteOffset"] = texOffset;
                gltfJson["bufferViews"][bufferViewIndex]["byteLength"] = texDataSize;
                gltfJson["bufferViews"][bufferViewIndex]["target"] = GL_ARRAY_BUFFER;
                size_t texBufferView = bufferViewIndex++;

                gltfJson["bufferViews"][bufferViewIndex]["buffer"]     = 0;
                gltfJson["bufferViews"][bufferViewIndex]["byteOffset"] = idxOffset;
                gltfJson["bufferViews"][bufferViewIndex]["byteLength"] = idxDataSize;
                gltfJson["bufferViews"][bufferViewIndex]["target"] = GL_ELEMENT_ARRAY_BUFFER;
                size_t idxBufferView = bufferViewIndex++;

                // Create accessors for this material group
                gltfJson["accessors"][accessorIndex]["bufferView"] = posBufferView;
                gltfJson["accessors"][accessorIndex]["componentType"] = GL_FLOAT;
                gltfJson["accessors"][accessorIndex]["count"] = vertices.size();
                gltfJson["accessors"][accessorIndex]["type"]  = "VEC3";
                size_t posAccessor                            = accessorIndex++;

                gltfJson["accessors"][accessorIndex]["bufferView"] = normBufferView;
                gltfJson["accessors"][accessorIndex]["componentType"] = GL_FLOAT;
                gltfJson["accessors"][accessorIndex]["count"] = vertices.size();
                gltfJson["accessors"][accessorIndex]["type"]  = "VEC3";
                size_t normAccessor                           = accessorIndex++;

                gltfJson["accessors"][accessorIndex]["bufferView"] = texBufferView;
                gltfJson["accessors"][accessorIndex]["componentType"] = GL_FLOAT;
                gltfJson["accessors"][accessorIndex]["count"] = vertices.size();
                gltfJson["accessors"][accessorIndex]["type"]  = "VEC2";
                size_t texAccessor                            = accessorIndex++;

                gltfJson["accessors"][accessorIndex]["bufferView"] = idxBufferView;
                gltfJson["accessors"][accessorIndex]["componentType"] = GL_UNSIGNED_INT;
                gltfJson["accessors"][accessorIndex]["count"] = indices.size();
                gltfJson["accessors"][accessorIndex]["type"]  = "SCALAR";
                size_t idxAccessor                            = accessorIndex++;

                // Create primitive for this material group
                size_t primitiveIndex = gltfJson["meshes"][0]["primitives"].size();
                gltfJson["meshes"][0]["primitives"][primitiveIndex]
                        ["attributes"]["POSITION"] = posAccessor;
                gltfJson["meshes"][0]["primitives"][primitiveIndex]["attribute"
                                                                    "s"]["NORMA"
                                                                         "L"] = normAccessor;
                gltfJson["meshes"][0]["primitives"][primitiveIndex]
                        ["attributes"]["TEXCOORD_0"] = texAccessor;
                gltfJson["meshes"][0]["primitives"][primitiveIndex]["indices"] = idxAccessor;
                gltfJson["meshes"][0]["primitives"][primitiveIndex]["materia"
                                                                    "l"] = matIndex;
            }
        }
        else
        {
            // Single material export (fallback to original logic)
            LOG_DEBUG("Exporting single-material model");

            size_t posDataSize  = m_vertices.size() * 3 * sizeof(float);
            size_t normDataSize = m_vertices.size() * 3 * sizeof(float);
            size_t texDataSize  = m_vertices.size() * 2 * sizeof(float);
            size_t idxDataSize  = m_indices.size() * sizeof(uint32_t);

            bufferData.reserve(posDataSize + normDataSize + texDataSize + idxDataSize);

            size_t posOffset = 0;
            for (const auto &vertex : m_vertices)
            {
                const uint8_t *posBytes =
                reinterpret_cast<const uint8_t *>(&vertex.position.x);
                bufferData.insert(bufferData.end(), posBytes, posBytes + 12);
            }

            size_t normOffset = bufferData.size();
            for (const auto &vertex : m_vertices)
            {
                const uint8_t *normBytes =
                reinterpret_cast<const uint8_t *>(&vertex.normal.x);
                bufferData.insert(bufferData.end(), normBytes, normBytes + 12);
            }

            size_t texOffset = bufferData.size();
            for (const auto &vertex : m_vertices)
            {
                float texCoords[2] = {vertex.texCoord.x, vertex.texCoord.y};
                const uint8_t *texBytes = reinterpret_cast<const uint8_t *>(texCoords);
                bufferData.insert(bufferData.end(), texBytes, texBytes + 8);
            }

            size_t         idxOffset = bufferData.size();
            const uint8_t *idxBytes =
            reinterpret_cast<const uint8_t *>(m_indices.data());
            bufferData.insert(bufferData.end(), idxBytes, idxBytes + idxDataSize);

            // Buffer views
            gltfJson["bufferViews"][0]["buffer"]     = 0;
            gltfJson["bufferViews"][0]["byteOffset"] = posOffset;
            gltfJson["bufferViews"][0]["byteLength"] = posDataSize;
            gltfJson["bufferViews"][0]["target"]     = GL_ARRAY_BUFFER;

            gltfJson["bufferViews"][1]["buffer"]     = 0;
            gltfJson["bufferViews"][1]["byteOffset"] = normOffset;
            gltfJson["bufferViews"][1]["byteLength"] = normDataSize;
            gltfJson["bufferViews"][1]["target"]     = GL_ARRAY_BUFFER;

            gltfJson["bufferViews"][2]["buffer"]     = 0;
            gltfJson["bufferViews"][2]["byteOffset"] = texOffset;
            gltfJson["bufferViews"][2]["byteLength"] = texDataSize;
            gltfJson["bufferViews"][2]["target"]     = GL_ARRAY_BUFFER;

            gltfJson["bufferViews"][3]["buffer"]     = 0;
            gltfJson["bufferViews"][3]["byteOffset"] = idxOffset;
            gltfJson["bufferViews"][3]["byteLength"] = idxDataSize;
            gltfJson["bufferViews"][3]["target"]     = GL_ELEMENT_ARRAY_BUFFER;

            // Accessors
            gltfJson["accessors"][0]["bufferView"]    = 0;
            gltfJson["accessors"][0]["componentType"] = GL_FLOAT;
            gltfJson["accessors"][0]["count"]         = m_vertices.size();
            gltfJson["accessors"][0]["type"]          = "VEC3";

            gltfJson["accessors"][1]["bufferView"]    = 1;
            gltfJson["accessors"][1]["componentType"] = GL_FLOAT;
            gltfJson["accessors"][1]["count"]         = m_vertices.size();
            gltfJson["accessors"][1]["type"]          = "VEC3";

            gltfJson["accessors"][2]["bufferView"]    = 2;
            gltfJson["accessors"][2]["componentType"] = GL_FLOAT;
            gltfJson["accessors"][2]["count"]         = m_vertices.size();
            gltfJson["accessors"][2]["type"]          = "VEC2";

            gltfJson["accessors"][3]["bufferView"]    = 3;
            gltfJson["accessors"][3]["componentType"] = GL_UNSIGNED_INT;
            gltfJson["accessors"][3]["count"]         = m_indices.size();
            gltfJson["accessors"][3]["type"]          = "SCALAR";

            // Single primitive
            gltfJson["meshes"][0]["primitives"][0]["attributes"]["POSITION"] = 0;
            gltfJson["meshes"][0]["primitives"][0]["attributes"]["NORMAL"] = 1;
            gltfJson["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"] = 2;
            gltfJson["meshes"][0]["primitives"][0]["indices"]  = 3;
            gltfJson["meshes"][0]["primitives"][0]["material"] = 0;
        }

        // Create buffer file
        std::filesystem::path gltfPath(path);
        std::string bufferFileName = gltfPath.stem().string() + ".bin";
        std::string bufferFilePath = gltfPath.parent_path().string() + "/" + bufferFileName;

        std::ofstream bufferFile(bufferFilePath, std::ios::binary);
        if (!bufferFile.is_open())
        {
            LOG_ERROR("Failed to create buffer file: %s", bufferFilePath.c_str());
            return Model::Status::FileNotFound;
        }
        bufferFile.write(reinterpret_cast<const char *>(bufferData.data()),
                         bufferData.size());
        bufferFile.close();

        // Buffer
        gltfJson["buffers"][0]["uri"]        = bufferFileName;
        gltfJson["buffers"][0]["byteLength"] = bufferData.size();

        // Images and textures (if any)
        bool hasTextures = false;
        for (const auto &material : m_materials)
        {
            if (!material.diffuseTexturePath.empty())
            {
                hasTextures                       = true;
                std::string           texturePath = material.diffuseTexturePath;
                std::filesystem::path texPath(texturePath);
                if (texPath.is_absolute())
                {
                    texturePath = texPath.filename().string();
                }
                gltfJson["images"][0]["uri"]      = texturePath;
                gltfJson["textures"][0]["source"] = 0;
                break;
            }
        }

        // Materials - save all materials with correct colors
        for (size_t i = 0; i < m_materials.size(); ++i)
        {
            const Material &material = m_materials[i];

            gltfJson["materials"][i]["name"] = material.name;
            gltfJson["materials"][i]["pbrMetallicRoughness"]["baseColorFacto"
                                                             "r"] = {
            material.diffuse.x, material.diffuse.y, material.diffuse.z, 1.0f};
            gltfJson["materials"][i]["pbrMetallicRoughness"]["metallicFactor"] =
            material.metallicFactor;
            gltfJson["materials"][i]["pbrMetallicRoughness"]["roughnessFacto"
                                                             "r"] = material.roughnessFactor;

            if (!material.diffuseTexturePath.empty() && hasTextures)
            {
                gltfJson["materials"][i]["pbrMetallicRoughness"]["baseColorText"
                                                                 "ure"]["inde"
                                                                        "x"] = 0;
                gltfJson["materials"][i]["pbrMetallicRoughness"]["baseColorText"
                                                                 "ure"]["texCoo"
                                                                        "rd"] = 0;
            }
        }

        // Create default material if none exist
        if (m_materials.empty())
        {
            gltfJson["materials"][0]["name"] = "DefaultMaterial";
            gltfJson["materials"][0]["pbrMetallicRoughness"]["baseColorFacto"
                                                             "r"] = {0.8, 0.8, 0.8, 1.0};
            gltfJson["materials"][0]["pbrMetallicRoughness"]["metallicFactor"] = 0.0;
            gltfJson["materials"][0]["pbrMetallicRoughness"]["roughnessFacto"
                                                             "r"] = 0.5;
        }

        // Mesh
        gltfJson["meshes"][0]["name"] = "Mesh";

        // Node
        gltfJson["nodes"][0]["name"] = "RootNode";
        gltfJson["nodes"][0]["mesh"] = 0;

        // Scene
        gltfJson["scenes"][0]["name"]  = "Scene";
        gltfJson["scenes"][0]["nodes"] = {0};
        gltfJson["scene"]              = 0;

        // Write glTF file
        std::ofstream gltfFile(path);
        if (!gltfFile.is_open())
        {
            LOG_ERROR("Failed to create glTF file: %s", path.c_str());
            return Model::Status::FileNotFound;
        }
        gltfFile << gltfJson.dump(2);
        gltfFile.close();

        size_t totalVertices =
        isMultiMaterial ? std::accumulate(m_materialVertices.begin(),
                                          m_materialVertices.end(),
                                          0UL,
                                          [](size_t sum, const auto &pair)
                                          { return sum + pair.second.size(); }) :
                          m_vertices.size();
        size_t totalIndices =
        isMultiMaterial ? std::accumulate(m_materialIndices.begin(),
                                          m_materialIndices.end(),
                                          0UL,
                                          [](size_t sum, const auto &pair)
                                          { return sum + pair.second.size(); }) :
                          m_indices.size();

        LOG_DEBUG("Successfully saved glTF model with %zu vertices, %zu "
                  "indices, %zu materials, %zu primitives",
                  totalVertices,
                  totalIndices,
                  m_materials.size(),
                  isMultiMaterial ? m_materials.size() : 1);
        return Model::Status::Success;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to save glTF model: %s", e.what());
        return Model::Status::InvalidFormat;
    }
}

void GLTFModel::setData(const std::vector<Mesh3DRenderer::Vertex> &vertices,
                        const std::vector<uint32_t>               &indices)
{
    LOG_DEBUG("Setting glTF model data with %zu vertices and %zu indices",
              vertices.size(),
              indices.size());

    m_vertices = vertices;
    m_indices  = indices;

    // Create default material
    createDefaultMaterial();

    if (!m_materials.empty())
    {
        std::string materialName         = m_materials[0].name;
        m_materialVertices[materialName] = vertices;
        m_materialIndices[materialName]  = indices;
    }

    m_loaded = true;
}

void GLTFModel::setDataWithMaterials(const std::vector<Model::MaterialGroup> &materialGroups)
{
    LOG_DEBUG("Setting glTF model data with %zu material groups", materialGroups.size());

    // Clear previous data
    m_vertices.clear();
    m_indices.clear();
    m_materials.clear();
    m_materialVertices.clear();
    m_materialIndices.clear();

    // Process each material group
    for (const auto &group : materialGroups)
    {
        // Convert OBJ material to glTF material
        Material gltfMaterial;
        gltfMaterial.name = group.material.name;
        gltfMaterial.baseColorFactor =
        glm::vec4(group.material.diffuse, group.material.alpha);
        gltfMaterial.diffuse            = group.material.diffuse;
        gltfMaterial.ambient            = group.material.ambient;
        gltfMaterial.specular           = group.material.specular;
        gltfMaterial.shininess          = group.material.shininess;
        gltfMaterial.diffuseTexturePath = group.material.diffuseTexture;
        gltfMaterial.normalTexturePath  = group.material.normalTexture;

        // Intelligent PBR property assignment based on material name and properties
        std::string materialNameLower = group.material.name;
        std::transform(materialNameLower.begin(),
                       materialNameLower.end(),
                       materialNameLower.begin(),
                       ::tolower);

        // Use the material properties directly from the source material
        gltfMaterial.metallicFactor =
        (group.material.metallic > 0.0f) ? group.material.metallic : 0.0f;
        gltfMaterial.roughnessFactor =
        (group.material.roughness > 0.0f) ? group.material.roughness : 0.5f;

        m_materials.push_back(gltfMaterial);

        // Store material vertices and indices
        m_materialVertices[group.materialName] = group.vertices;
        m_materialIndices[group.materialName]  = group.indices;

        // Add to combined vertex/index arrays
        size_t baseIndex = m_vertices.size();
        m_vertices.insert(m_vertices.end(),
                          group.vertices.begin(),
                          group.vertices.end());

        for (uint32_t index : group.indices)
        {
            m_indices.push_back(index + static_cast<uint32_t>(baseIndex));
        }
    }

    // Create default material if none were provided
    if (m_materials.empty())
    {
        createDefaultMaterial();
    }

    m_loaded = true;
    LOG_DEBUG("Set glTF model data with %zu total vertices, %zu total indices, "
              "and %zu materials",
              m_vertices.size(),
              m_indices.size(),
              m_materials.size());
}

std::vector<Model::MaterialGroup> GLTFModel::getMaterialGroups() const
{
    std::vector<Model::MaterialGroup> groups;

    for (const auto &[materialName, vertices] : m_materialVertices)
    {
        Model::MaterialGroup group;
        group.materialName = materialName;
        group.vertices     = vertices;

        // Get the indices for this material
        auto indicesIt = m_materialIndices.find(materialName);
        if (indicesIt != m_materialIndices.end())
        {
            group.indices = indicesIt->second;
        }

        // Find matching glTF material and convert to Model::Material format
        for (const auto &material : m_materials)
        {
            if (material.name == materialName)
            {
                // Convert glTF material to Model::Material format
                group.material.name           = material.name;
                group.material.ambient        = material.ambient;
                group.material.diffuse        = material.diffuse;
                group.material.specular       = material.specular;
                group.material.shininess      = material.shininess;
                group.material.alpha          = material.baseColorFactor.a;
                group.material.metallic       = material.metallicFactor;
                group.material.roughness      = material.roughnessFactor;
                group.material.diffuseTexture = material.diffuseTexturePath;
                group.material.normalTexture  = material.normalTexturePath;
                break;
            }
        }

        // Apply intelligent material enhancement
        applyMaterialIntelligence(group);

        groups.push_back(group);
    }

    return groups;
}

void GLTFModel::applyMaterialIntelligence(Model::MaterialGroup &group) const
{
    // Convert material name to lowercase for case-insensitive matching
    std::string materialNameLower = group.materialName;
    std::transform(materialNameLower.begin(),
                   materialNameLower.end(),
                   materialNameLower.begin(),
                   ::tolower);

    // Detect car paint materials
    bool isCarPaint = materialNameLower.find("paint") != std::string::npos ||
                      materialNameLower.find("body") != std::string::npos ||
                      materialNameLower.find("car_paint") != std::string::npos;

    if (isCarPaint && group.material.metallic < 0.8f)
    {
        // Car paint should be highly metallic with medium-low roughness
        group.material.metallic  = 0.9f;
        group.material.roughness = std::min(0.3f, group.material.roughness);
        LOG_DEBUG("Enhanced car paint material '%s' with metallic=%.2f, "
                  "roughness=%.2f",
                  group.materialName.c_str(),
                  group.material.metallic,
                  group.material.roughness);
    }

    // Detect headlight/light materials
    bool isHeadlight = materialNameLower.find("light") != std::string::npos ||
                       materialNameLower.find("lamp") != std::string::npos ||
                       materialNameLower.find("bulb") != std::string::npos;

    if (isHeadlight)
    {
        // Headlights should be bright white with some transparency
        // Set diffuse to bright white
        group.material.diffuse = glm::vec3(1.0f);
        // Make specular bright to simulate emission
        group.material.specular  = glm::vec3(1.0f);
        group.material.shininess = 100.0f;

        // Headlight glass should be slightly transparent
        if (group.material.alpha > 0.95f)
        {
            group.material.alpha = 0.9f; // Make slightly transparent
        }

        LOG_DEBUG("Enhanced headlight material '%s' with bright specular "
                  "properties",
                  group.materialName.c_str());
    }

    // Detect glass materials
    bool isGlass = materialNameLower.find("glass") != std::string::npos ||
                   materialNameLower.find("window") != std::string::npos ||
                   materialNameLower.find("windshield") != std::string::npos;

    if (isGlass)
    {
        // Glass material enhancement - using darker tint similar to Khronos glTF sample viewer
        // Set darker blue-tinted color for glass unless it already has a specific color
        if (glm::length(group.material.diffuse) < 0.3f ||
            (group.material.diffuse.r > 0.7f &&
             group.material.diffuse.g > 0.7f && group.material.diffuse.b > 0.7f))
        {
            // Darker blue/gray tint similar to the Khronos glTF sample viewer
            group.material.diffuse = glm::vec3(0.1f, 0.12f, 0.14f);
        }
        else
        {
            // If it has a color already, make it darker
            group.material.diffuse *= 0.3f;
        }

        // Ensure proper transparency - more transparent than before
        group.material.alpha = 0.15f;

        // Proper glass-like properties
        group.material.metallic = 0.0f; // Glass is not metallic (pure dielectric)
        group.material.roughness = 0.03f; // Very smooth surface (polished glass)

        LOG_DEBUG("Enhanced glass material '%s' with color=(%.2f, %.2f, %.2f), "
                  "alpha=%.2f, metallic=%.2f, roughness=%.2f",
                  group.materialName.c_str(),
                  group.material.diffuse.r,
                  group.material.diffuse.g,
                  group.material.diffuse.b,
                  group.material.alpha,
                  group.material.metallic,
                  group.material.roughness);
    }

    // Detect rubber/tire materials
    bool isRubber = materialNameLower.find("rubber") != std::string::npos ||
                    materialNameLower.find("tire") != std::string::npos ||
                    materialNameLower.find("tread") != std::string::npos;

    if (isRubber)
    {
        // Rubber should have low metallic and medium-high roughness
        group.material.metallic  = 0.0f;
        group.material.roughness = std::max(0.8f, group.material.roughness);
        LOG_DEBUG("Enhanced rubber material '%s' with metallic=%.2f, "
                  "roughness=%.2f",
                  group.materialName.c_str(),
                  group.material.metallic,
                  group.material.roughness);
    }
}

void GLTFModel::createDefaultMaterial()
{
    Material defaultMaterial;
    defaultMaterial.name            = "DefaultMaterial";
    defaultMaterial.baseColorFactor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    defaultMaterial.metallicFactor  = 0.0f;
    defaultMaterial.roughnessFactor = 0.5f;

    // Set legacy properties for compatibility
    defaultMaterial.diffuse = glm::vec3(defaultMaterial.baseColorFactor);
    defaultMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f); // Properly scaled ambient
    defaultMaterial.specular  = glm::vec3(0.0f, 0.0f, 0.0f);
    defaultMaterial.shininess = 0.0f;

    m_materials.push_back(defaultMaterial);
}

// Placeholder implementations for the remaining methods
// These will be implemented in the next part

bool GLTFModel::parseMaterials(const json &gltfJson)
{
    if (!gltfJson.contains("materials") || !gltfJson["materials"].is_array())
    {
        createDefaultMaterial();
        return true;
    }

    for (const auto &materialJson : gltfJson["materials"])
    {

        Material material;

        if (materialJson.contains("name") && materialJson["name"].is_string())
        {
            material.name = materialJson["name"].get<std::string>();
        }
        if (material.name.empty())
        {
            material.name = "Material_" + std::to_string(m_materials.size());
        }

        // Parse alpha mode and factor
        float alphaValue = 1.0f; // Default to fully opaque
        if (materialJson.contains("alphaMode") && materialJson["alphaMode"].is_string())
        {
            std::string alphaMode = materialJson["alphaMode"].get<std::string>();
            if (alphaMode == "BLEND")
            {
                alphaValue = material.baseColorFactor.a; // Will be possibly overridden by baseColorFactor
            }
            else if (alphaMode == "MASK")
            {
                // For masked mode, look for alphaCutoff
                if (materialJson.contains("alphaCutoff") &&
                    materialJson["alphaCutoff"].is_number())
                {
                    // Get alphaCutoff value and use it to set material properties
                    // float alphaCutoff = materialJson["alphaCutoff"].get<float>();
                    alphaValue = 1.0f; // Treat as opaque for now, cutoff handled in shaders
                    // Note: alphaCutoff will be used in shaders for alpha testing
                }
                else
                {
                    alphaValue = 1.0f;
                }
            }
            else
            { // OPAQUE
                alphaValue = 1.0f;
            }
        }
        else
        {
            alphaValue = 1.0f; // Default to opaque
        }
        // Store alpha in the baseColorFactor.a
        material.baseColorFactor.a = alphaValue;

        // Parse PBR metallic roughness
        if (materialJson.contains("pbrMetallicRoughness") &&
            materialJson["pbrMetallicRoughness"].is_object())
        {
            const auto &pbr = materialJson["pbrMetallicRoughness"];

            // Base color factor
            if (pbr.contains("baseColorFactor") && pbr["baseColorFactor"].is_array())
            {
                const auto &bcf = pbr["baseColorFactor"];
                if (bcf.size() >= 4)
                {
                    material.baseColorFactor = glm::vec4(bcf[0].get<float>(),
                                                         bcf[1].get<float>(),
                                                         bcf[2].get<float>(),
                                                         bcf[3].get<float>());
                    // Alpha is stored in baseColorFactor.a
                }
            }

            // Metallic and roughness factors
            if (pbr.contains("metallicFactor") && pbr["metallicFactor"].is_number())
            {
                material.metallicFactor = pbr["metallicFactor"].get<float>();
            }
            if (pbr.contains("roughnessFactor") && pbr["roughnessFactor"].is_number())
            {
                material.roughnessFactor = pbr["roughnessFactor"].get<float>();
            }

            // Base color texture
            if (pbr.contains("baseColorTexture") && pbr["baseColorTexture"].is_object())
            {
                const auto &baseColorTexture = pbr["baseColorTexture"];
                if (baseColorTexture.contains("index") &&
                    baseColorTexture["index"].is_number())
                {
                    material.baseColorTexture.index =
                    baseColorTexture["index"].get<size_t>();

                    // Resolve texture to image URI
                    if (material.baseColorTexture.index < m_textures.size())
                    {
                        const Texture &texture =
                        m_textures[material.baseColorTexture.index];
                        if (texture.source < m_images.size())
                        {
                            const Image &image = m_images[texture.source];
                            // Store the full path including base directory
                            material.diffuseTexturePath =
                            m_baseDirectory + image.uri;
                            LOG_DEBUG("Resolved base color texture: %s",
                                      material.diffuseTexturePath.c_str());
                        }
                    }
                }
                if (baseColorTexture.contains("texCoord") &&
                    baseColorTexture["texCoord"].is_number())
                {
                    material.baseColorTexture.texCoord =
                    baseColorTexture["texCoord"].get<size_t>();
                }

                // Check for KHR_texture_transform extension
                parseTextureTransform(baseColorTexture, material.baseColorTexture, material);
            }

            // Metallic-roughness texture
            if (pbr.contains("metallicRoughnessTexture") &&
                pbr["metallicRoughnessTexture"].is_object())
            {
                const auto &metallicRoughnessTexture =
                pbr["metallicRoughnessTexture"];
                if (metallicRoughnessTexture.contains("index") &&
                    metallicRoughnessTexture["index"].is_number())
                {
                    material.metallicRoughnessTexture.index =
                    metallicRoughnessTexture["index"].get<size_t>();

                    // Resolve texture to image URI
                    if (material.metallicRoughnessTexture.index < m_textures.size())
                    {
                        const Texture &texture =
                        m_textures[material.metallicRoughnessTexture.index];
                        if (texture.source < m_images.size())
                        {
                            const Image &image = m_images[texture.source];
                            // Store the full path including base directory
                            material.metallicRoughnessTexturePath =
                            m_baseDirectory + image.uri;
                            LOG_DEBUG("Resolved metallic-roughness texture: %s",
                                      material.metallicRoughnessTexturePath.c_str());
                        }
                    }
                }
                if (metallicRoughnessTexture.contains("texCoord") &&
                    metallicRoughnessTexture["texCoord"].is_number())
                {
                    material.metallicRoughnessTexture.texCoord =
                    metallicRoughnessTexture["texCoord"].get<size_t>();
                }

                // Check for KHR_texture_transform extension
                parseTextureTransform(metallicRoughnessTexture,
                                      material.metallicRoughnessTexture,
                                      material);
            }
        }

        // Parse normal texture
        if (materialJson.contains("normalTexture") &&
            materialJson["normalTexture"].is_object())
        {
            const auto &normalTextureJson = materialJson["normalTexture"];
            if (normalTextureJson.contains("index") &&
                normalTextureJson["index"].is_number())
            {
                material.normalTexture.index = normalTextureJson["index"].get<size_t>();

                // Resolve texture to image URI
                if (material.normalTexture.index < m_textures.size())
                {
                    const Texture &texture = m_textures[material.normalTexture.index];
                    if (texture.source < m_images.size())
                    {
                        const Image &image = m_images[texture.source];
                        // Store the full path including base directory
                        material.normalTexturePath = m_baseDirectory + image.uri;
                        LOG_DEBUG("Resolved normal texture: %s",
                                  material.normalTexturePath.c_str());
                    }
                }
            }
            if (normalTextureJson.contains("texCoord") &&
                normalTextureJson["texCoord"].is_number())
            {
                material.normalTexture.texCoord =
                normalTextureJson["texCoord"].get<size_t>();
            }

            // Check for KHR_texture_transform extension
            parseTextureTransform(normalTextureJson, material.normalTexture, material);
        }

        // Parse occlusion texture
        if (materialJson.contains("occlusionTexture") &&
            materialJson["occlusionTexture"].is_object())
        {
            const auto &occlusionTextureJson = materialJson["occlusionTexture"];
            if (occlusionTextureJson.contains("index") &&
                occlusionTextureJson["index"].is_number())
            {
                material.occlusionTexture.index =
                occlusionTextureJson["index"].get<size_t>();
            }
            if (occlusionTextureJson.contains("texCoord") &&
                occlusionTextureJson["texCoord"].is_number())
            {
                material.occlusionTexture.texCoord =
                occlusionTextureJson["texCoord"].get<size_t>();
            }

            // Check for KHR_texture_transform extension
            parseTextureTransform(occlusionTextureJson, material.occlusionTexture, material);
        }

        // Parse emissive properties
        if (materialJson.contains("emissiveFactor") &&
            materialJson["emissiveFactor"].is_array())
        {
            const auto &ef = materialJson["emissiveFactor"];
            if (ef.size() >= 3)
            {
                material.emissiveFactor =
                glm::vec3(ef[0].get<float>(), ef[1].get<float>(), ef[2].get<float>());
            }
        }

        if (materialJson.contains("emissiveTexture") &&
            materialJson["emissiveTexture"].is_object())
        {
            const auto &emissiveTextureJson = materialJson["emissiveTexture"];
            if (emissiveTextureJson.contains("index") &&
                emissiveTextureJson["index"].is_number())
            {
                material.emissiveTexture.index =
                emissiveTextureJson["index"].get<size_t>();
            }
            if (emissiveTextureJson.contains("texCoord") &&
                emissiveTextureJson["texCoord"].is_number())
            {
                material.emissiveTexture.texCoord =
                emissiveTextureJson["texCoord"].get<size_t>();
            }

            // Check for KHR_texture_transform extension
            parseTextureTransform(emissiveTextureJson, material.emissiveTexture, material);
        }

        // Convert PBR to legacy properties for compatibility
        material.diffuse = glm::vec3(material.baseColorFactor);
        material.ambient = material.diffuse * 0.2f; // Scale ambient properly

        // Convert metallic/roughness to specular approximation
        float metallic     = material.metallicFactor;
        float roughness    = material.roughnessFactor;
        material.specular  = glm::vec3(metallic * (1.0f - roughness));
        material.shininess = (1.0f - roughness) * 128.0f;

        // Add emissive contribution to ambient for basic rendering
        material.ambient += material.emissiveFactor;

        // Use special handling for material types based on name
        std::string materialNameLower = material.name;
        std::transform(materialNameLower.begin(),
                       materialNameLower.end(),
                       materialNameLower.begin(),
                       ::tolower);

        // Glass materials detection

        // If not already set by the model, adjust properties for glass
        // Glass: low metallic, very low roughness, possible transparency
        // Use values exactly as specified in the material
        // No special material type detection

        m_materials.push_back(material);
        LOG_DEBUG("Parsed material: %s (metallic=%.2f, roughness=%.2f, "
                  "alpha=%.2f)",
                  material.name.c_str(),
                  material.metallicFactor,
                  material.roughnessFactor,
                  material.baseColorFactor.a);
    }

    // Create default material if none found
    if (m_materials.empty())
    {
        createDefaultMaterial();
    }

    return true;
}

bool GLTFModel::parseMeshes(const json &gltfJson)
{
    if (!gltfJson.contains("meshes") || !gltfJson["meshes"].is_array())
    {
        return false;
    }

    for (const auto &meshJson : gltfJson["meshes"])
    {
        Mesh mesh;

        if (meshJson.contains("name") && meshJson["name"].is_string())
        {
            mesh.name = meshJson["name"].get<std::string>();
        }
        if (mesh.name.empty())
        {
            mesh.name = "Mesh_" + std::to_string(m_meshes.size());
        }

        // Parse primitives
        if (meshJson.contains("primitives") && meshJson["primitives"].is_array())
        {
            for (const auto &primJson : meshJson["primitives"])
            {
                Primitive primitive;

                // Parse attributes
                if (primJson.contains("attributes") && primJson["attributes"].is_object())
                {
                    const auto &attrs = primJson["attributes"];
                    for (auto it = attrs.begin(); it != attrs.end(); ++it)
                    {
                        if (it.value().is_number())
                        {
                            primitive.attributes[it.key()] = it.value().get<size_t>();
                        }
                    }
                }

                // Parse indices accessor
                if (primJson.contains("indices") && primJson["indices"].is_number())
                {
                    primitive.indices = primJson["indices"].get<size_t>();
                }

                // Parse material index
                if (primJson.contains("material") && primJson["material"].is_number())
                {
                    primitive.material = primJson["material"].get<size_t>();
                }

                // Parse primitive mode
                if (primJson.contains("mode") && primJson["mode"].is_number())
                {
                    primitive.mode =
                    static_cast<PrimitiveMode>(primJson["mode"].get<uint32_t>());
                }

                mesh.primitives.push_back(primitive);
                LOG_DEBUG("Parsed primitive: %zu attributes, material=%zu, "
                          "mode=%d",
                          primitive.attributes.size(),
                          primitive.material,
                          static_cast<int>(primitive.mode));
            }
        }

        m_meshes.push_back(mesh);
        LOG_DEBUG("Parsed mesh: %s with %zu primitives",
                  mesh.name.c_str(),
                  mesh.primitives.size());
    }

    return true;
}

bool GLTFModel::parseNodes(const json &gltfJson)
{
    if (!gltfJson.contains("nodes") || !gltfJson["nodes"].is_array())
    {
        return false;
    }

    for (const auto &nodeJson : gltfJson["nodes"])
    {
        Node node;

        if (nodeJson.contains("name") && nodeJson["name"].is_string())
        {
            node.name = nodeJson["name"].get<std::string>();
        }
        if (node.name.empty())
        {
            node.name = "Node_" + std::to_string(m_nodes.size());
        }

        if (nodeJson.contains("mesh") && nodeJson["mesh"].is_number())
        {
            node.mesh = nodeJson["mesh"].get<size_t>();
        }

        // Parse node transformation
        if (nodeJson.contains("matrix") && nodeJson["matrix"].is_array() &&
            nodeJson["matrix"].size() == 16)
        {
            // Direct matrix specification (column-major order)
            const auto &matrixJson = nodeJson["matrix"];
            node.matrix            = glm::mat4(matrixJson[0].get<float>(),
                                    matrixJson[1].get<float>(),
                                    matrixJson[2].get<float>(),
                                    matrixJson[3].get<float>(),
                                    matrixJson[4].get<float>(),
                                    matrixJson[5].get<float>(),
                                    matrixJson[6].get<float>(),
                                    matrixJson[7].get<float>(),
                                    matrixJson[8].get<float>(),
                                    matrixJson[9].get<float>(),
                                    matrixJson[10].get<float>(),
                                    matrixJson[11].get<float>(),
                                    matrixJson[12].get<float>(),
                                    matrixJson[13].get<float>(),
                                    matrixJson[14].get<float>(),
                                    matrixJson[15].get<float>());
            LOG_DEBUG("Node %s uses matrix transform", node.name.c_str());
        }
        else
        {
            // TRS (Translation, Rotation, Scale) properties
            if (nodeJson.contains("translation") && nodeJson["translation"].is_array() &&
                nodeJson["translation"].size() == 3)
            {
                const auto &translationJson = nodeJson["translation"];
                node.translation = glm::vec3(translationJson[0].get<float>(),
                                             translationJson[1].get<float>(),
                                             translationJson[2].get<float>());
                LOG_DEBUG("Node %s has translation [%.2f, %.2f, %.2f]",
                          node.name.c_str(),
                          node.translation.x,
                          node.translation.y,
                          node.translation.z);
            }

            if (nodeJson.contains("rotation") && nodeJson["rotation"].is_array() &&
                nodeJson["rotation"].size() == 4)
            {
                const auto &rotationJson = nodeJson["rotation"];
                // Note: glTF uses [x,y,z,w] quaternion format
                node.rotation = glm::vec4(rotationJson[0].get<float>(),
                                          rotationJson[1].get<float>(),
                                          rotationJson[2].get<float>(),
                                          rotationJson[3].get<float>());
                LOG_DEBUG("Node %s has rotation quaternion [%.2f, %.2f, %.2f, "
                          "%.2f]",
                          node.name.c_str(),
                          node.rotation.x,
                          node.rotation.y,
                          node.rotation.z,
                          node.rotation.w);
            }

            if (nodeJson.contains("scale") && nodeJson["scale"].is_array() &&
                nodeJson["scale"].size() == 3)
            {
                const auto &scaleJson = nodeJson["scale"];
                node.scale            = glm::vec3(scaleJson[0].get<float>(),
                                       scaleJson[1].get<float>(),
                                       scaleJson[2].get<float>());
                LOG_DEBUG("Node %s has scale [%.2f, %.2f, %.2f]",
                          node.name.c_str(),
                          node.scale.x,
                          node.scale.y,
                          node.scale.z);
            }
        }

        // Parse children
        if (nodeJson.contains("children") && nodeJson["children"].is_array())
        {
            for (const auto &childIndex : nodeJson["children"])
            {
                if (childIndex.is_number())
                {
                    node.children.push_back(childIndex.get<size_t>());
                }
            }
            LOG_DEBUG("Node %s has %zu children",
                      node.name.c_str(),
                      node.children.size());
        }

        m_nodes.push_back(node);
        LOG_DEBUG("Parsed node: %s", node.name.c_str());
    }

    return true;
}

bool GLTFModel::parseScenes(const json &gltfJson)
{
    if (!gltfJson.contains("scenes") || !gltfJson["scenes"].is_array())
    {
        return false;
    }

    for (const auto &sceneJson : gltfJson["scenes"])
    {
        Scene scene;

        if (sceneJson.contains("name") && sceneJson["name"].is_string())
        {
            scene.name = sceneJson["name"].get<std::string>();
        }
        if (scene.name.empty())
        {
            scene.name = "Scene_" + std::to_string(m_scenes.size());
        }

        if (sceneJson.contains("nodes") && sceneJson["nodes"].is_array())
        {
            for (const auto &nodeIdx : sceneJson["nodes"])
            {
                if (nodeIdx.is_number())
                {
                    scene.nodes.push_back(nodeIdx.get<size_t>());
                }
            }
        }

        m_scenes.push_back(scene);
        LOG_DEBUG("Parsed scene: %s with %zu root nodes",
                  scene.name.c_str(),
                  scene.nodes.size());
    }

    return true;
}

bool GLTFModel::parseImages(const json &gltfJson)
{
    if (!gltfJson.contains("images") || !gltfJson["images"].is_array())
    {
        return true; // Images are optional
    }

    for (const auto &imageJson : gltfJson["images"])
    {
        Image image;

        if (imageJson.contains("uri") && imageJson["uri"].is_string())
        {
            image.uri = imageJson["uri"].get<std::string>();
        }

        m_images.push_back(image);
        LOG_DEBUG("Parsed image: uri='%s'", image.uri.c_str());
    }

    return true;
}

bool GLTFModel::parseTextures(const json &gltfJson)
{
    if (!gltfJson.contains("textures") || !gltfJson["textures"].is_array())
    {
        return true; // Textures are optional
    }

    for (const auto &textureJson : gltfJson["textures"])
    {
        Texture texture;

        if (textureJson.contains("source") && textureJson["source"].is_number())
        {
            texture.source = textureJson["source"].get<size_t>();
        }

        m_textures.push_back(texture);
        LOG_DEBUG("Parsed texture: source=%zu", texture.source);
    }

    return true;
}

void GLTFModel::processScene(size_t sceneIndex)
{
    if (sceneIndex >= m_scenes.size())
    {
        LOG_ERROR("Invalid scene index: %zu", sceneIndex);
        createPlaceholderGeometry();
        return;
    }

    const Scene &scene = m_scenes[sceneIndex];
    LOG_DEBUG("Processing scene: %s", scene.name.c_str());

    // Clear previous vertex data
    m_vertices.clear();
    m_indices.clear();
    m_materialVertices.clear();
    m_materialIndices.clear();

    // Process each root node in the scene
    for (size_t nodeIndex : scene.nodes)
    {
        processNode(nodeIndex, glm::mat4(1.0f));
    }

    // Combine all material groups into main vertex/index arrays
    for (const auto &[materialName, vertices] : m_materialVertices)
    {
        size_t baseIndex = m_vertices.size();

        // Add vertices
        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

        // Add indices with offset
        auto indicesIt = m_materialIndices.find(materialName);
        if (indicesIt != m_materialIndices.end())
        {
            for (uint32_t index : indicesIt->second)
            {
                m_indices.push_back(index + static_cast<uint32_t>(baseIndex));
            }
        }
    }

    LOG_DEBUG("Processed scene with %zu total vertices and %zu total indices",
              m_vertices.size(),
              m_indices.size());
}

void GLTFModel::processNode(size_t nodeIndex, const glm::mat4 &parentTransform)
{
    if (nodeIndex >= m_nodes.size())
    {
        LOG_ERROR("Invalid node index: %zu", nodeIndex);
        return;
    }

    const Node &node = m_nodes[nodeIndex];

    // Calculate node's local transform
    glm::mat4 localTransform;
    // Check if a matrix is directly specified in the node
    if (node.matrix != glm::mat4(1.0f))
    { // Compare with identity matrix to see if it's set
        // The matrix in glTF is stored in column-major order
        // GLM matrices are also column-major, so we can use the matrix directly
        localTransform = node.matrix;
    }
    else
    {
        // Otherwise build the transform from TRS components
        // Order is: T * R * S (apply scale first, then rotation, then translation)

        // Scale matrix
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), node.scale);

        // Rotation matrix from quaternion (glTF quaternions are in XYZW order)
        glm::quat quaternion(node.rotation.w,
                             node.rotation.x,
                             node.rotation.y,
                             node.rotation.z);
        glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);

        // Translation matrix
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), node.translation);

        // Combined transformation: T * R * S
        localTransform = translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Combine with parent transform
    glm::mat4 nodeTransform = parentTransform * localTransform;

    LOG_DEBUG("Processing node: %s", node.name.c_str());

    // Process mesh if this node has one
    if (node.mesh != SIZE_MAX && node.mesh < m_meshes.size())
    {
        const Mesh &mesh = m_meshes[node.mesh];

        for (const auto &primitive : mesh.primitives)
        {
            processPrimitive(primitive, node.name, nodeTransform);
        }
    }

    // Recursively process child nodes
    for (size_t childIndex : node.children)
    {
        processNode(childIndex, nodeTransform);
    }
}

bool GLTFModel::parseTextureTransform(const json &textureJson, TextureInfo &texInfo, Material &material)
{
    // Parse KHR_texture_transform extension if present
    if (textureJson.contains("extensions") && textureJson["extensions"].is_object())
    {
        const auto &extensions = textureJson["extensions"];

        if (extensions.contains("KHR_texture_transform") &&
            extensions["KHR_texture_transform"].is_object())
        {
            const auto &transformExt = extensions["KHR_texture_transform"];

            // Parse offset (default: [0,0])
            if (transformExt.contains("offset") && transformExt["offset"].is_array() &&
                transformExt["offset"].size() >= 2)
            {
                material.texOffset.x = transformExt["offset"][0].get<float>();
                material.texOffset.y = transformExt["offset"][1].get<float>();
            }

            // Parse scale (default: [1,1])
            if (transformExt.contains("scale") && transformExt["scale"].is_array() &&
                transformExt["scale"].size() >= 2)
            {
                material.texScale.x = transformExt["scale"][0].get<float>();
                material.texScale.y = transformExt["scale"][1].get<float>();
            }

            // Parse rotation (default: 0)
            if (transformExt.contains("rotation") && transformExt["rotation"].is_number())
            {
                material.texRotation = transformExt["rotation"].get<float>();
            }

            // Parse texCoord override (default: use the texCoord from the texture)
            if (transformExt.contains("texCoord") && transformExt["texCoord"].is_number())
            {
                texInfo.texCoord = transformExt["texCoord"].get<size_t>();
            }

            LOG_DEBUG("Parsed KHR_texture_transform: offset=[%.2f, %.2f], "
                      "scale=[%.2f, %.2f], rotation=%.2f",
                      material.texOffset.x,
                      material.texOffset.y,
                      material.texScale.x,
                      material.texScale.y,
                      material.texRotation);
            return true;
        }
    }

    return false;
}

void GLTFModel::processPrimitive(const Primitive   &primitive,
                                 const std::string &meshName,
                                 const glm::mat4   &transform)
{
    // Get material name
    std::string     materialName = "DefaultMaterial";
    const Material *materialPtr  = nullptr;

    if (primitive.material != SIZE_MAX && primitive.material < m_materials.size())
    {
        materialName = m_materials[primitive.material].name;
        materialPtr  = &m_materials[primitive.material];
    }

    // Get position data
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end())
    {
        LOG_ERROR("Primitive missing POSITION attribute");
        return;
    }

    std::vector<float> positions = getAccessorData<float>(posIt->second);
    if (positions.size() % 3 != 0)
    {
        LOG_ERROR("Invalid position data size: %zu", positions.size());
        return;
    }

    size_t vertexCount = positions.size() / 3;

    // Get normal data (optional)
    std::vector<float> normals;
    auto               normalIt = primitive.attributes.find("NORMAL");
    if (normalIt != primitive.attributes.end())
    {
        normals = getAccessorData<float>(normalIt->second);
        if (normals.size() != positions.size())
        {
            LOG_WARN(
            "Normal count doesn't match position count, ignoring normals");
            normals.clear();
        }
    }

    // Get texture coordinate data (optional)
    // Try multiple texture coordinate attributes (TEXCOORD_0, TEXCOORD_1, etc.)
    std::vector<float> texCoords;


    // Handle other attributes (tangents, colors, etc.)
    // Tangents could be vec3 or vec4 (with w component)

    // Determine the texCoord index to use based on material's texture
    // This is important for materials that use different UV sets for different textures
    int texCoordIndex = 0; // Default to TEXCOORD_0

    // If we have a material with a normal texture that specifies a texCoord index, use that
    if (materialPtr && materialPtr->normalTexture.index != SIZE_MAX)
    {
        texCoordIndex = static_cast<int>(materialPtr->normalTexture.texCoord);
        LOG_DEBUG("Using texCoord index %d from material's normal texture", texCoordIndex);
    }

    // Try loading the specified texCoord index first
    auto texAttrName = "TEXCOORD_" + std::to_string(texCoordIndex);
    auto texCoordIt  = primitive.attributes.find(texAttrName);
    if (texCoordIt != primitive.attributes.end())
    {
        texCoords = getAccessorData<float>(texCoordIt->second);
        if (texCoords.size() == vertexCount * 2)
        {
            LOG_DEBUG("Using texture coordinates from %s", texAttrName.c_str());
        }
        else
        {
            LOG_WARN("TexCoord count doesn't match vertex count in %s, falling "
                     "back to available sets",
                     texAttrName.c_str());
            texCoords.clear();
        }
    }

    // If we still don't have valid texture coordinates, try all available sets
    if (texCoords.empty())
    {
        for (int texCoordIdx = 0; texCoordIdx < 2; texCoordIdx++)
        { // Support up to 2 texture coordinate sets
            auto texAttrName = "TEXCOORD_" + std::to_string(texCoordIdx);
            auto texCoordIt  = primitive.attributes.find(texAttrName);

            if (texCoordIt != primitive.attributes.end())
            {
                texCoords = getAccessorData<float>(texCoordIt->second);
                if (texCoords.size() == vertexCount * 2)
                {
                    LOG_DEBUG("Falling back to texture coordinates from %s",
                              texAttrName.c_str());
                    break;
                }
                else
                {
                    LOG_WARN("TexCoord count doesn't match vertex count in %s, "
                             "trying next set",
                             texAttrName.c_str());
                    texCoords.clear();
                }
            }
        }
    }

    // Handle other attributes (tangents, colors, etc.)
    std::vector<float> tangents;
    auto               tangentIt = primitive.attributes.find("TANGENT");
    if (tangentIt != primitive.attributes.end())
    {
        tangents = getAccessorData<float>(tangentIt->second);
        if (tangents.size() != positions.size() &&
            tangents.size() != positions.size() * 4 / 3)
        {
            // Tangents could be vec3 or vec4 (with w component)
            LOG_WARN("Tangent count doesn't match, ignoring tangents");
            tangents.clear();
        }
    }

    // Create vertices
    std::vector<Mesh3DRenderer::Vertex> vertices;
    vertices.reserve(vertexCount);

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (size_t i = 0; i < vertexCount; ++i)
    {
        Mesh3DRenderer::Vertex vertex;

        // Position
        glm::vec4 pos(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2], 1.0f);
        // Transform position by the node's transform matrix
        // This applies the correct hierarchy of transformations from the glTF scene
        pos             = transform * pos;
        vertex.position = glm::vec3(pos);

        // Normal
        if (!normals.empty())
        {
            glm::vec3 normal(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
            // Transform normal by the inverse transpose of the model matrix
            // This ensures normals are transformed correctly even with non-uniform scaling
            vertex.normal = glm::normalize(normalMatrix * normal);
        }
        else
        {
            // When no normals are provided, use a default normal
            // Since we're in a right-handed system with Y up, default normal points up
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up normal
            /* Removed problematic flat normal calculation
            if (false)
{
                // Removed flat normal calculation code
            }
            */
        }

        // Texture coordinates
        if (!texCoords.empty())
        {
            float u = texCoords[i * 2];
            float v = texCoords[i * 2 + 1];

            // According to glTF spec, texture coordinates origin is at the top-left corner
            // in normalized UV space (0,0) = top-left, (1,1) = bottom-right

            // We need to check if texture coords need to be flipped depending on the renderer
            // conventions. In OpenGL texcoords are (0,0) = bottom-left, (1,1) = top-right

            // By default in glTF, we don't need to flip, but we respect user settings

            // If we have a flag for explicit V coordinate flipping
            // In OpenGL texcoords are (0,0) = bottom-left, (1,1) = top-right

            // Handle V coordinate properly according to glTF spec 2.0:
            // "When image data is defined in a right-handed coordinate system with
            // the origin at the top left, the data can be used directly; in that case,
            // values increase from top to bottom and from left to right."

            // Following the glTF specification strictly:
            // In glTF, texture coordinates have origin at top-left (0,0 is top-left)
            // OpenGL texture coordinates have origin at bottom-left (0,0 is bottom-left)
            // So we need to flip the V coordinate by default: v_opengl = 1.0 - v_gltf

            // Apply texture transforms from KHR_texture_transform extension if available
            if (materialPtr)
            {
                // Use material's texture transform if available
                // This would normally come from KHR_texture_transform extension
                float     rotation = materialPtr->texRotation;
                glm::vec2 scale    = materialPtr->texScale;
                glm::vec2 offset   = materialPtr->texOffset;

                if (rotation != 0.0f || scale != glm::vec2(1.0f) || offset != glm::vec2(0.0f))
                {
                    // Apply texture coordinate transformations in the correct order
                    // 1. Scale around origin
                    u *= scale.x;
                    v *= scale.y;

                    // 2. Rotate around origin
                    if (rotation != 0.0f)
                    {
                        float sinR = sin(rotation);
                        float cosR = cos(rotation);
                        float newU = u * cosR - v * sinR;
                        float newV = u * sinR + v * cosR;
                        u          = newU;
                        v          = newV;
                    }

                    // 3. Translate by offset
                    u += offset.x;
                    v += offset.y;
                }
            }

            // Apply V-flipping according to glTF spec standard behavior
            // Only flip if we're not already flipping the image data
            if (!m_flipImageData)
            {
                v = 1.0f - v;
            }

            // Apply user-specified texture coordinate flipping if enabled
            if (m_flipTexCoords)
            {

                // If image data was flipped during loading (e.g., for PNG/JPG loading)
                // and we're using that same texture, we need to adjust texture coords
                // If the image was flipped, we need to flip the V coord to match

                v = 1.0f - v;
            }

            vertex.texCoord = glm::vec2(u, v);

            // Log texture coordinate information for debugging
            if (i == 0)
            { // Log only for first vertex to avoid spam
                LOG_DEBUG("Texture coordinates for first vertex: u=%.4f, "
                          "v=%.4f (flipImageData=%s, flipTexCoords=%s)",
                          u,
                          v,
                          m_flipImageData ? "true" : "false",
                          m_flipTexCoords ? "true" : "false");
            }
        }
        else
        {
            // No texture coordinates available, use defaults
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Get indices
    std::vector<uint32_t> indices;
    if (primitive.indices != SIZE_MAX)
    {
        indices = getAccessorData<uint32_t>(primitive.indices);
    }
    else
    {
        // Generate indices for non-indexed geometry
        indices.reserve(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i)
        {
            indices.push_back(static_cast<uint32_t>(i));
        }
    }

    // Add to material group
    size_t baseIndex = m_materialVertices[materialName].size();

    m_materialVertices[materialName].insert(m_materialVertices[materialName].end(),
                                            vertices.begin(),
                                            vertices.end());

    // Adjust indices with base offset
    for (uint32_t index : indices)
    {
        m_materialIndices[materialName].push_back(index + static_cast<uint32_t>(baseIndex));
    }

    LOG_DEBUG("Processed primitive: %zu vertices, %zu indices, material=%s",
              vertices.size(),
              indices.size(),
              materialName.c_str());
}

void GLTFModel::createPlaceholderGeometry()
{
    // Create a simple cube for testing
    std::vector<Mesh3DRenderer::Vertex> vertices = {
    // Front face
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

    // Back face
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}};

    std::vector<uint32_t> indices = {// Front face
                                     0,
                                     1,
                                     2,
                                     2,
                                     3,
                                     0,
                                     // Back face
                                     4,
                                     5,
                                     6,
                                     6,
                                     7,
                                     4,
                                     // Left face
                                     4,
                                     0,
                                     3,
                                     3,
                                     5,
                                     4,
                                     // Right face
                                     1,
                                     7,
                                     6,
                                     6,
                                     2,
                                     1,
                                     // Top face
                                     3,
                                     2,
                                     6,
                                     6,
                                     5,
                                     3,
                                     // Bottom face
                                     4,
                                     7,
                                     1,
                                     1,
                                     0,
                                     4};

    m_vertices = vertices;
    m_indices  = indices;

    // Assign to default material group
    if (!m_materials.empty())
    {
        std::string materialName         = m_materials[0].name;
        m_materialVertices[materialName] = vertices;
        m_materialIndices[materialName]  = indices;
    }
}

} // namespace ti::graphics
