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

#ifndef RESOURCES_H_
#define RESOURCES_H_

#include <string>

/**
 * @brief Utility functions for the graphics engine
 */
namespace ti::utils
{

/**
 * @brief Set the base directory for resource files
 * @param path The path to the resources directory
 *
 * This function sets the base directory used for loading resources like
 * shaders, textures, and fonts. All resource paths will be resolved
 * relative to this directory.
 */
void setResourcePath(const std::string& path);

/**
 * @brief Get the current resource base directory
 * @return The path to the resources directory
 */
std::string getResourcePath();

/**
 * @brief Load a resource file into memory
 * @param path Path to the resource file (relative to resource directory)
 * @return The contents of the resource file as a string
 * @throw std::runtime_error if the file cannot be loaded
 *
 * This function loads the entire contents of a resource file into memory.
 * The path is resolved relative to the resource directory set by
 * setResourcePath().
 */
std::string loadResource(const std::string& path);

/**
 * @brief Check if a resource file exists
 * @param path Path to the resource file (relative to resource directory)
 * @return true if the file exists, false otherwise
 */
bool resourceExists(const std::string& path);

/**
 * @brief Get the absolute path to a resource file
 * @param path Path to the resource file (relative to resource directory)
 * @return The absolute path to the resource file
 *
 * This function combines the resource directory with the given path
 * to create an absolute path to the resource file.
 */
std::string getAbsoluteResourcePath(const std::string& path);

} // namespace ti::utils

#endif // RESOURCES_H_
