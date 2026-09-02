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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <graphics/include/GLError.h>
#include <graphics/include/ShaderManager.h>

namespace ti::graphics
{

ShaderManager &ShaderManager::getInstance()
{
    static ShaderManager instance;
    return instance;
}

ShaderManager::~ShaderManager()
{
    cleanup();
}

void ShaderManager::setGlobalConstants(const std::vector<ShaderConstant> &constants)
{
    m_globalConstants = constants;
}

void ShaderManager::addGlobalConstant(const ShaderConstant &constant)
{
    m_globalConstants.push_back(constant);
}

void ShaderManager::clearGlobalConstants()
{
    m_globalConstants.clear();
}

std::string ShaderManager::processShaderConstants(const std::string &source,
                                                  const std::vector<ShaderConstant> &constants)
{
    std::stringstream processed;

    // Add version directive if present in the original source
    size_t versionPos = source.find("#version");
    if (versionPos != std::string::npos)
    {
        size_t newlinePos = source.find('\n', versionPos);
        if (newlinePos != std::string::npos)
        {
            processed << source.substr(0, newlinePos + 1);
            processed << "\n";
        }
    }

    // Add global constants
    for (const auto &constant : m_globalConstants)
    {
        if (constant.defined)
        {
            if (constant.value.empty())
            {
                processed << "#define " << constant.name << "\n";
            }
            else
            {
                processed << "#define " << constant.name << " " << constant.value << "\n";
            }
        }
        else
        {
            processed << "#undef " << constant.name << "\n";
        }
    }

    // Add shader-specific constants
    for (const auto &constant : constants)
    {
        if (constant.defined)
        {
            if (constant.value.empty())
            {
                processed << "#define " << constant.name << "\n";
            }
            else
            {
                processed << "#define " << constant.name << " " << constant.value << "\n";
            }
        }
        else
        {
            processed << "#undef " << constant.name << "\n";
        }
    }

    // Add the rest of the shader source
    if (versionPos != std::string::npos)
    {
        size_t newlinePos = source.find('\n', versionPos);
        if (newlinePos != std::string::npos)
        {
            processed << source.substr(newlinePos + 1);
        }
    }
    else
    {
        processed << source;
    }

    return processed.str();
}

GLuint
ShaderManager::createGraphicsProgramWithConstants(const std::string &name,
                                                  const std::string &vertexSource,
                                                  const std::string &fragmentSource,
                                                  const std::vector<ShaderConstant> &constants,
                                                  const std::string &geometrySource,
                                                  const std::string &tessControlSource,
                                                  const std::string &tessEvalSource)
{
    GLuint      program                    = 0;
    std::string processedVertexSource      = "";
    std::string processedFragmentSource    = "";
    std::string processedGeometrySource    = "";
    std::string processedTessControlSource = "";
    std::string processedTessEvalSource    = "";

    processedVertexSource   = processShaderConstants(vertexSource, constants);
    processedFragmentSource = processShaderConstants(fragmentSource, constants);

    if (!geometrySource.empty())
    {
        processedGeometrySource = processShaderConstants(geometrySource, constants);
    }

    if (!tessControlSource.empty())
    {
        processedTessControlSource = processShaderConstants(tessControlSource, constants);
    }

    if (!tessEvalSource.empty())
    {
        processedTessEvalSource = processShaderConstants(tessEvalSource, constants);
    }

    program = createGraphicsProgramFromSource(name,
                                              processedVertexSource,
                                              processedFragmentSource,
                                              processedGeometrySource,
                                              processedTessControlSource,
                                              processedTessEvalSource);

    return program;
}

GLuint ShaderManager::createComputeProgramWithConstants(const std::string &name,
                                                        const std::string &computeSource,
                                                        const std::vector<ShaderConstant> &constants)
{
    GLuint      program                = 0;
    std::string processedComputeSource = "";

    processedComputeSource = processShaderConstants(computeSource, constants);
    program = createComputeProgramFromSource(name, processedComputeSource);

    return program;
}

GLuint
ShaderManager::createGraphicsProgramFromSource(const std::string &name,
                                               const std::string &vertexSource,
                                               const std::string &fragmentSource,
                                               const std::string &geometrySource,
                                               const std::string &tessControlSource,
                                               const std::string &tessEvalSource)
{
    GLuint              program           = 0;
    auto                it                = m_programs.find(name);
    std::vector<GLuint> shaders           = {};
    GLuint              vertexShader      = 0;
    GLuint              fragmentShader    = 0;
    GLuint              geometryShader    = 0;
    GLuint              tessControlShader = 0;
    GLuint              tessEvalShader    = 0;
    bool                success           = true;

    if (it != m_programs.end())
    {
        LOG_WARN("Shader program '%s' already exists, returning existing "
                 "program\n",
                 name.c_str());
        program = it->second;
    }
    else
    {
        vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, "vertex");
        if (!vertexShader)
        {
            LOG_ERROR("Failed to compile vertex shader for program '%s'\n", name.c_str());
            success = false;
        }

        if (success)
        {
            shaders.push_back(vertexShader);
            fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "fragment");
            if (!fragmentShader)
            {
                LOG_ERROR("Failed to compile fragment shader for "
                          "program '%s'\n",
                          name.c_str());
                success = false;
            }
        }

        if (success)
        {
            shaders.push_back(fragmentShader);

            if (!geometrySource.empty())
            {
                geometryShader =
                compileShader(GL_GEOMETRY_SHADER, geometrySource, "geometry");
                if (!geometryShader)
                {
                    LOG_ERROR("Failed to compile geometry shader for "
                              "program '%s'\n",
                              name.c_str());
                    success = false;
                }
                else
                {
                    shaders.push_back(geometryShader);
                }
            }
        }

        if (success && !tessControlSource.empty())
        {
            tessControlShader =
            compileShader(GL_TESS_CONTROL_SHADER, tessControlSource, "tessellation control");
            if (!tessControlShader)
            {
                LOG_ERROR("Failed to compile tessellation control shader "
                          "for program '%s'\n",
                          name.c_str());
                success = false;
            }
            else
            {
                shaders.push_back(tessControlShader);
            }
        }

        if (success && !tessEvalSource.empty())
        {
            tessEvalShader =
            compileShader(GL_TESS_EVALUATION_SHADER, tessEvalSource, "tessellation evaluation");
            if (!tessEvalShader)
            {
                LOG_ERROR("Failed to compile tessellation evaluation "
                          "shader for program '%s'\n",
                          name.c_str());
                success = false;
            }
            else
            {
                shaders.push_back(tessEvalShader);
            }
        }

        if (success)
        {
            program = linkProgram(shaders);
            if (!program)
            {
                LOG_ERROR("Failed to link shader program '%s'\n", name.c_str());
            }
            else
            {
                m_programs[name] = program;
                LOG_DEBUG("Created shader program '%s' (ID: %u)\n", name.c_str(), program);
            }
        }

        for (GLuint shader : shaders)
        {
            glDeleteShader(shader);
        }
    }

    return program;
}

GLuint ShaderManager::createGraphicsProgramFromFiles(const std::string &name,
                                                     const std::string &vertexPath,
                                                     const std::string &fragmentPath,
                                                     const std::string &geometryPath,
                                                     const std::string &tessControlPath,
                                                     const std::string &tessEvalPath)
{
    // Load required shader sources
    std::string vertexSource = loadShaderFile(vertexPath);
    if (vertexSource.empty())
    {
        LOG_ERROR("Failed to load vertex shader file '%s'\n", vertexPath.c_str());
        return 0;
    }

    std::string fragmentSource = loadShaderFile(fragmentPath);
    if (fragmentSource.empty())
    {
        LOG_ERROR("Failed to load fragment shader file '%s'\n", fragmentPath.c_str());
        return 0;
    }

    // Load optional shader sources
    std::string geometrySource;
    if (!geometryPath.empty())
    {
        geometrySource = loadShaderFile(geometryPath);
        if (geometrySource.empty())
        {
            LOG_ERROR("Failed to load geometry shader file '%s'\n", geometryPath.c_str());
            return 0;
        }
    }

    std::string tessControlSource;
    if (!tessControlPath.empty())
    {
        tessControlSource = loadShaderFile(tessControlPath);
        if (tessControlSource.empty())
        {
            LOG_ERROR("Failed to load tessellation control shader file '%s'\n",
                      tessControlPath.c_str());
            return 0;
        }
    }

    std::string tessEvalSource;
    if (!tessEvalPath.empty())
    {
        tessEvalSource = loadShaderFile(tessEvalPath);
        if (tessEvalSource.empty())
        {
            LOG_ERROR("Failed to load tessellation evaluation shader file "
                      "'%s'\n",
                      tessEvalPath.c_str());
            return 0;
        }
    }

    return createGraphicsProgramFromSource(
    name, vertexSource, fragmentSource, geometrySource, tessControlSource, tessEvalSource);
}

GLuint ShaderManager::createComputeProgramFromSource(const std::string &name,
                                                     const std::string &computeSource)
{
    // Check if program already exists
    auto it = m_programs.find(name);
    if (it != m_programs.end())
    {
        LOG_WARN("Shader program '%s' already exists, returning existing "
                 "program\n",
                 name.c_str());
        return it->second;
    }

    // Compile compute shader
    GLuint computeShader = compileShader(GL_COMPUTE_SHADER, computeSource, "compute");
    if (!computeShader)
    {
        LOG_ERROR("Failed to compile compute shader for program '%s'\n", name.c_str());
        return 0;
    }

    // Link program
    GLuint program = linkProgram({computeShader});

    // Clean up shader
    glDeleteShader(computeShader);

    if (!program)
    {
        LOG_ERROR("Failed to link compute shader program '%s'\n", name.c_str());
        return 0;
    }

    // Store program
    m_programs[name] = program;
    LOG_DEBUG("Created compute shader program '%s' (ID: %u)\n", name.c_str(), program);

    return program;
}

GLuint ShaderManager::createComputeProgramFromFile(const std::string &name,
                                                   const std::string &computePath)
{
    std::string computeSource = loadShaderFile(computePath);
    if (computeSource.empty())
    {
        LOG_ERROR("Failed to load compute shader file '%s'\n", computePath.c_str());
        return 0;
    }

    return createComputeProgramFromSource(name, computeSource);
}

GLuint ShaderManager::getProgram(const std::string &name) const
{
    auto it = m_programs.find(name);
    if (it != m_programs.end())
    {
        return it->second;
    }
    LOG_WARN("Shader program '%s' not found\n", name.c_str());
    return 0;
}

void ShaderManager::deleteProgram(const std::string &name)
{
    auto it = m_programs.find(name);
    if (it != m_programs.end())
    {
        glDeleteProgram(it->second);
        CHECK_GL_ERRORS();
        m_programs.erase(it);
        LOG_DEBUG("Deleted shader program '%s'\n", name.c_str());
    }
}

void ShaderManager::cleanup()
{
    for (const auto &pair : m_programs)
    {
        glDeleteProgram(pair.second);
        CHECK_GL_ERRORS();
    }
    m_programs.clear();
    LOG_DEBUG("Cleaned up all shader programs");
}

GLuint ShaderManager::compileShader(GLenum type, const std::string &source, const char *typeName)
{
    GLuint shader = glCreateShader(type);
    if (!shader || !CHECK_GL_ERRORS())
    {
        LOG_ERROR("Failed to create shader object\n");
        return 0;
    }

    const char *sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    CHECK_GL_ERRORS();

    glCompileShader(shader);
    CHECK_GL_ERRORS();

    if (!checkShaderErrors(shader, typeName))
    {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint ShaderManager::linkProgram(const std::vector<GLuint> &shaders)
{
    GLuint program = glCreateProgram();
    if (!program || !CHECK_GL_ERRORS())
    {
        LOG_ERROR("Failed to create program object\n");
        return 0;
    }

    // Attach all shaders
    for (GLuint shader : shaders)
    {
        glAttachShader(program, shader);
        CHECK_GL_ERRORS();
    }

    glLinkProgram(program);
    CHECK_GL_ERRORS();

    if (!checkProgramErrors(program))
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

bool ShaderManager::checkShaderErrors(GLuint shader, const char *type)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());

        LOG_ERROR("Failed to compile %s shader:\n%s\n", type, log.data());
        return false;
    }

    return true;
}

bool ShaderManager::checkProgramErrors(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());

        LOG_ERROR("Failed to link shader program:\n%s", log.data());
        return false;
    }

    return true;
}

std::string ShaderManager::loadShaderFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open shader file '%s'\n", path.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace ti::graphics
