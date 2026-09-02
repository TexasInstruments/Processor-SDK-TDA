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

#include "render.h"

#ifdef __cplusplus
extern "C" {
#endif

int load_texture(GLuint tex, int width, int height, int textureType, void* data)
{
    GLenum target = GL_TEXTURE_2D;
    GLint param = GL_NEAREST;

    if ((textureType == GL_RGB) || (textureType == GL_RGBA)) {
        target = GL_TEXTURE_2D;
        param = GL_NEAREST;

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            textureType,
            width,
            height,
            0,
            textureType,
            GL_UNSIGNED_BYTE,//textureFormat,
            data
            );
        GL_CHECK(glTexImage2D);
    } else {
        printf("Incorrect texture type %x\n", textureType);
        return -1;
    }

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, param); //GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, param); //GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL_CHECK(glTexParameteri);
    return 0;
}

int load_texture_from_raw_file(GLuint tex, uint32_t width, uint32_t height, int textureType, const char* filename, uint32_t offset)
{
    int ret;
    void* data;
    FILE* fp;
    int dataread;
    int numbytes = 0;
    char file[SRVMAXPATHLENGTH];
    //open and load raw file

    char *file_path = get_file_path();
    if(file_path == NULL)
    {
	    file_path = (char *)"./";
    }

    switch (textureType) {
        case GL_RGBA:
            numbytes = 4 * width * height;
            break;

        case GL_RGB:
            numbytes = 3 * width * height;
            break;


        default:
            printf("Invalid texture type %d\n", textureType);
            return -1;
    }
    snprintf(file, SRVMAXPATHLENGTH, "%s/%s/%s", file_path, "psdkra/srv", filename);
    fp = fopen(file, "rb");
    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    data = malloc(numbytes);
    if(data == NULL)
    {
            fclose(fp);
	    printf("Error allocating memory for texture read: %s\n", filename);
	    return -1;
    }
    fseek(fp, offset, SEEK_CUR);
    dataread = fread(data, 1, numbytes, fp);
    fclose(fp);
    if(dataread != numbytes) {
        free(data);
        printf("Error in file size != width*height\n");
        return -1; //TODO: reinstate this
    }

    ret = load_texture(tex, width, height, textureType, data);

    free(data);

    return ret;
}

uint32_t u32(uint8_t b[4]){
  uint32_t u;
  u = b[3];
  u = (u  << 8) + b[2];
  u = (u  << 8) + b[1];
  u = (u  << 8) + b[0];
  return u;
}

uint16_t u16(uint8_t b[2]){
  uint16_t u;
  u = b[1];
  u = (u  << 8) + b[0];
  return u;
}

int load_texture_bmp(GLuint tex, const char *filename)
{
	FILE *fp;
	uint8_t header[54];
	uint32_t width, height, offset;
	uint16_t bpp;
	int type;
	int dataread;
        char file[SRVMAXPATHLENGTH];

	char *file_path = get_file_path();
	if(file_path == NULL)
	{
		file_path = (char *)"./";
	}

        snprintf(file, SRVMAXPATHLENGTH, "%s/%s/%s", file_path, "psdkra/srv", filename);
        fp = fopen(file, "rb");
	if(!fp)
	{
		fprintf(stderr, "cannot open bmp file: %s\n", file);
		return -1;
	}
	dataread = fread(header, sizeof(uint8_t), 54, fp);
        if (dataread != 54)
        {
            printf("Incorrect Bytes read from file\n");
		fclose(fp);
		printf("Incorrect Bytes read from file\n");
		return -1;
        }

	offset = u32(&header[10]);
	width  = u32(&header[18]);
	height = u32(&header[22]);
	bpp    = u16(&header[28]);

	fclose(fp);

	switch(bpp)
	{
		case 24:
			type = GL_RGB;
			break;
		case 32:
			type = GL_RGBA;
			break;
		default:
			printf("Invalid number of bits per pixel: %d in %s\n", bpp, filename);
			return -1;
	}

	return(load_texture_from_raw_file(tex, width, height, type, filename, offset));
}

#if 0
void get_bmp_info(const char *filename, uint32_t *width, uint32_t *height, uint32_t *bpp)
{
	FILE *fp;
	uint8_t header[54];
	int dataread;

	fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("Cannot open file: %s\n", filename);
		return;
	}

	dataread = fread(header, sizeof(uint8_t), 54, fp);
	if (dataread != 54)
	{
		fclose(fp);
		printf("Incorrect Bytes read from file\n");
		return;
	}

	*width  = *(uint32_t *)(&header[18]);
	*height = *(uint32_t *)(&header[22]);
	*bpp    = *(uint32_t *)(&header[28]);

	fclose(fp);
}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */


