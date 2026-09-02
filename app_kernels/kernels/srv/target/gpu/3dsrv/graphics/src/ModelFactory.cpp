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
#include <filesystem>

#include <graphics/include/GLTFModel.h>
#include <graphics/include/ModelFactory.h>
#include <utils/include/logger.h>

namespace ti::graphics
{

std::unique_ptr<Model> ModelFactory::create(const std::string &format)
{
    LOG_DEBUG("Creating model instance for format '%s'", format.c_str());

    // Convert format to lowercase for case-insensitive comparison
    std::string lowerFormat = format;
    std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);

    // Create GLTF model instance
    if (lowerFormat == "gltf" || lowerFormat == "glb")
    {
        LOG_DEBUG("Creating glTF model instance");
        return std::make_unique<GLTFModel>();
    }

    // Unknown format - only GLTF is supported
    LOG_ERROR("Unsupported model format: %s (only GLTF/GLB is supported)",
              format.c_str());
    return nullptr;
}

std::unique_ptr<Model> ModelFactory::loadFromFile(const std::string &path)
{
    LOG_DEBUG("Loading model from file: %s", path.c_str());

    // Get file extension
    std::string ext = getFileExtension(path);
    if (ext.empty())
    {
        LOG_ERROR("Failed to determine file extension: %s", path.c_str());
        return nullptr;
    }
    LOG_DEBUG("Detected file format: %s", ext.c_str());

    // Create model instance
    auto model = create(ext);
    if (!model)
    {
        LOG_ERROR("Failed to create model instance for format: %s", ext.c_str());
        return nullptr;
    }

    // Load the model
    if (model->load(path) != Model::Status::Success)
    {
        LOG_ERROR("Failed to load model: %s (%s)",
                  path.c_str(),
                  model->getError().c_str());
        return nullptr;
    }

    LOG_DEBUG("Successfully loaded model from %s", path.c_str());
    return model;
}

std::string ModelFactory::getFileExtension(const std::string &path)
{
    LOG_DEBUG("Extracting file extension from: %s", path.c_str());

    std::filesystem::path fsPath(path);
    std::string           ext = fsPath.extension().string();

    // Remove dot and convert to lowercase
    if (!ext.empty() && ext[0] == '.')
    {
        ext = ext.substr(1);
    }
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext.empty())
    {
        LOG_WARN("No file extension found in path: %s", path.c_str());
    }
    else
    {
        LOG_DEBUG("Extracted extension: %s", ext.c_str());
    }

    return ext;
}

} // namespace ti::graphics
