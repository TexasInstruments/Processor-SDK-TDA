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

#ifndef GRAPHICS_SHADER_MANAGER_H_
#define GRAPHICS_SHADER_MANAGER_H_

// C++ includes
#include <string>
#include <unordered_map>
#include <vector>

// Graphics library includes
#include <GLES3/gl32.h>

// Local includes
#include <utils/include/logger.h>

namespace ti::graphics
{

/**
 * @brief Class for managing shader programs
 *
 * This class handles shader compilation, linking, and management.
 * It provides a centralized way to create and manage shader programs,
 * avoiding code duplication across different renderers.
 */
/**
 * @brief Structure to hold shader compilation constants
 */
struct ShaderConstant
{
    /**
     * @brief Name of the constant (e.g., "USE_LIGHTING")
     */
    std::string name{};

    /**
     * @brief Value of the constant (e.g., "1" or undefined)
     */
    std::string value{};

    /**
     * @brief Whether to define or undefine the constant
     */
    bool defined{true};

    ShaderConstant(const std::string& n, const std::string& v = "", bool d = true)
        : name(n), value(v), defined(d)
    {
    }
};

/**
 * @brief Class for managing shader programs
 *
 * This class handles shader compilation, linking, and management.
 * It provides a centralized way to create and manage shader programs,
 * with support for compilation constants and conditional compilation.
 */
class ShaderManager
{
  public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the shader manager instance
     */
    static ShaderManager& getInstance();

    /**
     * @brief Create a graphics shader program from source strings
     * @param name Unique identifier for the shader program
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @param geometrySource Optional geometry shader source code
     * @param tessControlSource Optional tessellation control shader source
     * code
     * @param tessEvalSource Optional tessellation evaluation shader source
     * code
     * @return Program ID if successful, 0 if failed
     */
    GLuint createGraphicsProgramFromSource(const std::string& name, const std::string& vertexSource,
                                           const std::string& fragmentSource,
                                           const std::string& geometrySource    = "",
                                           const std::string& tessControlSource = "",
                                           const std::string& tessEvalSource    = "");

    /**
     * @brief Create a graphics shader program from files
     * @param name Unique identifier for the shader program
     * @param vertexPath Path to vertex shader file
     * @param fragmentPath Path to fragment shader file
     * @param geometryPath Optional path to geometry shader file
     * @param tessControlPath Optional path to tessellation control shader file
     * @param tessEvalPath Optional path to tessellation evaluation shader file
     * @return Program ID if successful, 0 if failed
     */
    GLuint createGraphicsProgramFromFiles(const std::string& name, const std::string& vertexPath,
                                          const std::string& fragmentPath,
                                          const std::string& geometryPath    = "",
                                          const std::string& tessControlPath = "",
                                          const std::string& tessEvalPath    = "");

    /**
     * @brief Create a compute shader program from source
     * @param name Unique identifier for the shader program
     * @param computeSource Compute shader source code
     * @return Program ID if successful, 0 if failed
     */
    GLuint createComputeProgramFromSource(const std::string& name,
                                          const std::string& computeSource);

    /**
     * @brief Create a compute shader program from file
     * @param name Unique identifier for the shader program
     * @param computePath Path to compute shader file
     * @return Program ID if successful, 0 if failed
     */
    GLuint createComputeProgramFromFile(const std::string& name, const std::string& computePath);

    /**
     * @brief Get an existing shader program
     * @param name Shader program identifier
     * @return Program ID if found, 0 if not found
     */
    GLuint getProgram(const std::string& name) const;

    /**
     * @brief Delete a shader program
     * @param name Shader program identifier
     */
    void deleteProgram(const std::string& name);

    /**
     * @brief Clean up all shader programs
     */
    void cleanup();

    /**
     * @brief Set global shader constants
     * @param constants Vector of constants to apply to all shader compilations
     */
    void setGlobalConstants(const std::vector<ShaderConstant>& constants);

    /**
     * @brief Add a global shader constant
     * @param constant Constant to add to global constants
     */
    void addGlobalConstant(const ShaderConstant& constant);

    /**
     * @brief Clear all global shader constants
     */
    void clearGlobalConstants();

    /**
     * @brief Create a graphics program with specific constants
     * @param name Program name
     * @param vertexSource Vertex shader source
     * @param fragmentSource Fragment shader source
     * @param constants Shader-specific constants
     * @param geometrySource Optional geometry shader source
     * @param tessControlSource Optional tessellation control shader source
     * @param tessEvalSource Optional tessellation evaluation shader source
     * @return Program ID if successful, 0 if failed
     */
    GLuint createGraphicsProgramWithConstants(
        const std::string& name, const std::string& vertexSource, const std::string& fragmentSource,
        const std::vector<ShaderConstant>& constants, const std::string& geometrySource = "",
        const std::string& tessControlSource = "", const std::string& tessEvalSource = "");

    /**
     * @brief Create a compute program with specific constants
     * @param name Program name
     * @param computeSource Compute shader source
     * @param constants Shader-specific constants
     * @return Program ID if successful, 0 if failed
     */
    GLuint createComputeProgramWithConstants(const std::string&                 name,
                                             const std::string&                 computeSource,
                                             const std::vector<ShaderConstant>& constants);
  private:
    ShaderManager()
    {
    }

    ~ShaderManager();

    // Prevent copying
    ShaderManager(const ShaderManager&)            = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    /**
     * @brief Compile a shader
     * @param type Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
     * @param source Shader source code
     * @param typeName String name of shader type for logging
     * @return Shader ID if successful, 0 if failed
     */
    GLuint compileShader(GLenum type, const std::string& source, const char* typeName);

    /**
     * @brief Link shaders into a program
     * @param shaders Vector of shader IDs to link
     * @return Program ID if successful, 0 if failed
     */
    GLuint linkProgram(const std::vector<GLuint>& shaders);

    /**
     * @brief Check for shader compilation errors
     * @param shader Shader ID
     * @param type Shader type string for logging
     * @return true if compilation successful
     */
    bool checkShaderErrors(GLuint shader, const char* type);

    /**
     * @brief Check for program linking errors
     * @param program Program ID
     * @return true if linking successful
     */
    bool checkProgramErrors(GLuint program);

    /**
     * @brief Load shader source from file
     * @param path Path to shader file
     * @return Shader source code if successful, empty string if failed
     */
    std::string loadShaderFile(const std::string& path);

    /**
     * @brief Process source code with compilation constants
     * @param source Original shader source
     * @param constants Constants to apply
     * @return Processed source code with constants injected
     */
    std::string processShaderConstants(const std::string&                 source,
                                       const std::vector<ShaderConstant>& constants);

    /**
     * @brief Shader programs
     */
    std::unordered_map<std::string, GLuint> m_programs{};

    /**
     * @brief Global compilation constants
     */
    std::vector<ShaderConstant> m_globalConstants{};
};

} // namespace ti::graphics

#endif // GRAPHICS_SHADER_MANAGER_H_
