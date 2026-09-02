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

#ifndef GRAPHICS_MODEL_FACTORY_H_
#define GRAPHICS_MODEL_FACTORY_H_

#include <graphics/include/Model.h>
#include <memory>
#include <string>

namespace ti::graphics
{

/**
 * @brief Factory class for creating model instances
 *
 * This class handles the creation of appropriate model instances
 * based on file format. It currently only supports GLTF/GLB format.
 */
class ModelFactory
{
  public:
    /**
     * @brief Create a model instance for the given file format
     * @param format File format extension (e.g., "gltf")
     * @return Unique pointer to model instance, nullptr if format unknown
     */
    static std::unique_ptr<Model> create(const std::string& format);

    /**
     * @brief Create a model instance and load from file
     * @param path Path to model file
     * @return Unique pointer to model instance, nullptr if format unknown
     *
     * This method determines the format from the file extension
     * and creates an appropriate model instance.
     */
    static std::unique_ptr<Model> loadFromFile(const std::string& path);
  private:
    /**
     * @brief Get file extension from path
     * @param path File path
     * @return Lowercase extension without dot
     */
    static std::string getFileExtension(const std::string& path);
};

} // namespace ti::graphics

#endif // GRAPHICS_MODEL_FACTORY_H_
