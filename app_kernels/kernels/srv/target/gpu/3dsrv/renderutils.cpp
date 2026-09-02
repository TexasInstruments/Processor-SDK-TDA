/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include "renderutils.h"

unsigned long getFileLength(FILE *file)
{
	int ret;

	if(!file) return 0;

	fseek(file, 0L, SEEK_END);
	ret = ftell(file);
	fseek(file, 0L, SEEK_SET);

	return ret;
}

int renderutils_loadShader(char* filename, GLchar** shader_source)
{
	FILE *file;
	unsigned long len;
	size_t readlen;
	int err;
	file = fopen(filename, "r");
	if(!file)
	{
		ru_perror("Unable to open file: %s\n", filename);
		err = -ENOENT;
		goto exit;
	}

	len = getFileLength(file);

	if (len <= 0)
	{
		ru_perror("Shader file empty: %s\n", filename);
		err = -ENOENT;
		goto close_file;
	} 

	*shader_source = (GLchar *) new char[len+1];
	if (*shader_source == 0)
	{
		ru_perror("Unable to allocate memory for shader: %s\n", filename);
		err = -ENOMEM;   // can't reserve memory
		goto close_file;
	}

	// len isn't always strlen cause some characters are stripped in ascii read...
	// it is important to 0-terminate the real length later, len is just max possible value... 
	(*shader_source)[len] = (char)0;

	readlen = fread(*shader_source, 1, len, file);

	if(readlen <= 0)
	{
		ru_perror("Unable to read shader source from file: %s. fread returns: %d\n",
				filename,
				(int)readlen);
		err = -EBADF;
		delete[] *shader_source;
		goto close_file;
	}

	(*shader_source)[readlen] = (char)0;  // 0-terminate it at the correct position
	err = 0;

close_file:
	fclose(file);
exit:
	return err; // No Error
}


int renderutils_unloadShader(GLchar** shader_source)
{
	if (*shader_source != 0)
		delete[] *shader_source;
	*shader_source = 0;
	return 0;
}

GLuint renderutils_compileShader(GLenum shaderType, const GLchar* pSource) {
   GLuint shader = glCreateShader(shaderType);
   if (shader) {
       glShaderSource(shader, 1, &pSource, NULL);
       glCompileShader(shader);
       GLint compiled = 0;
       glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
       if (!compiled) {
           GLint infoLen = 0;
           glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
           if (infoLen) {
               char* buf = (char*) malloc(infoLen);
               if (buf) {
                   glGetShaderInfoLog(shader, infoLen, NULL, buf);
                   ru_perror(" GL: Could not compile shader %d:\n%s\n",
                       shaderType, buf);
                   free(buf);
               }
           } else {
               char* buf = (char*) malloc(0x1000);
               if (buf) {
                   glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                   ru_perror(" GL: Could not compile shader %d:\n%s\n",
                   shaderType, buf);
                   free(buf);
               }
           }
           glDeleteShader(shader);
           shader = 0;
       }
   }
   return shader;
}

GLuint renderutils_createProgram(const GLchar* pVertexSource, const GLchar* pFragmentSource)
{
   GLuint vertexShader = renderutils_compileShader(GL_VERTEX_SHADER, pVertexSource);
   if (!vertexShader) {
       return 0;
   }

   GLuint pixelShader = renderutils_compileShader(GL_FRAGMENT_SHADER, pFragmentSource);
   if (!pixelShader) {
       glDeleteShader(vertexShader);
       return 0;
   }

   GLuint program = glCreateProgram();
   if (program) {
       glAttachShader(program, vertexShader);
//       ru_checkGlError("glAttachShader");
       glAttachShader(program, pixelShader);
//       ru_checkGlError("glAttachShader");
       glLinkProgram(program);
       GLint linkStatus = GL_FALSE;
       glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
       if (linkStatus != GL_TRUE) {
           GLint bufLength = 0;
           glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
           if (bufLength) {
               char* buf = (char*) malloc(bufLength);
               if (buf) {
                   glGetProgramInfoLog(program, bufLength, NULL, buf);
                   ru_perror(" GL: Could not link program:\n%s\n", buf);
                   free(buf);
               }
           }
           glDeleteProgram(program);
           program = 0;
       }
   }

   glDeleteShader(vertexShader);
   glDeleteShader(pixelShader);

   return program;
}


GLuint renderutils_loadAndCreateProgram(char* vshfile, char* fshfile)
{
    GLchar *vshader, *fshader;
    GLuint ret;
    if (renderutils_loadShader(vshfile, &vshader) < 0)
    {
	    renderutils_unloadShader(&vshader);
	    ru_perror("Cannot load vertex shader: %s\n", vshfile);
		return 0;
    }
    if (renderutils_loadShader(fshfile, &fshader) < 0)
    {
        ru_perror("Cannot load fragment shader: %s\n", fshfile);
	    renderutils_unloadShader(&vshader);
	    renderutils_unloadShader(&fshader);
	    ru_perror("Cannot load fragment shader: %s\n", fshfile);
		return 0;
    }
    ret = renderutils_createProgram(vshader, fshader);

    renderutils_unloadShader(&vshader);
    renderutils_unloadShader(&fshader);

    return ret;
}






