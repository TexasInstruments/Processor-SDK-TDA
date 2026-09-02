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
#include <math.h>

GLuint vertexPositionAttribLoc; //3 panes+1 car
GLuint vertexTexCoord1AttribLoc; //4 panes+1 car
GLuint vertexTexCoord2AttribLoc; //4 panes+1 car
GLuint vertexIndexImage1AttribLoc; //4 panes+1 car
GLuint vertexIndexImage2AttribLoc; //4 panes+1 car
GLuint blendAttribLoc; //blend of LUT
GLuint uiProgramObject;

GLint samplerLocation0;
GLint samplerLocation1;
GLint samplerLocation2;
GLint samplerLocation3;
GLint projMatrixLocation;
GLint mvMatrixLocation;
GLint uniform_select;
#ifndef STATIC_COORDINATES
GLint uniform_rangex;
GLint uniform_rangey;
GLint uniform_texturex;
GLint uniform_texturey;
#endif

int shader_output_select = 0;
GLenum render_mode = GL_TRIANGLE_STRIP;

extern bool srv_render_to_file;

extern glm::mat4 mMVP_bowl[];

extern int32_t srv_offset_x_left;
extern int32_t srv_offset_x_right;
extern int32_t srv_offset_y_front;
extern int32_t srv_offset_y_back;
srv_bowl_reshape_t srv_bowl_reshape;
static unsigned int cam_width;
static unsigned int cam_height;

//Mesh splitting logic
#define MAX_VBO_MESH_SPLIT 8
static GLuint vboId[MAX_VBO_MESH_SPLIT*3];

static void * prevLUT=(void *)0xdead;

#define MAX_INDEX_BUFFERS 2

typedef struct {
	unsigned int *buffer;
	unsigned int length;
} t_index_buffer;

t_index_buffer index_buffers[MAX_INDEX_BUFFERS];
unsigned int active_index_buffer = 1;

#ifdef STANDALONE
unsigned int active_image_set = 0;
unsigned int srv_image_offset = 0;
#endif

bool index_buffer_changed = 0;
static bool lut_updated = false;
static bool lut_external = true;
static void *lut_staging;

//Shaders for surround view
static const char srv_vert_shader_lut[] =
" attribute vec3 aVertexPosition;\n "
" attribute vec2 aTextureCoord1;\n "
" attribute vec2 aTextureCoord2;\n "
" attribute vec2 blendVals;\n "
" uniform mat4 uMVMatrix;\n "
" varying vec2 outNormTexture;\n "
" varying vec2 outNormTexture1;\n "
" varying vec2 outBlendVals;\n "
" varying float outFloatChannelX;\n "
" varying float outFloatChannelY;\n "
" varying float outFloatChannelZ;\n "
#ifdef STATIC_COORDINATES
" float uRangeX = 440.0; \n "
" float uRangeY = 540.0; \n "
" float uTextureX = 20480.0; \n "
" float uTextureY = 11520.0;\n "
#else
" uniform float uRangeX; \n "
" uniform float uRangeY; \n "
" uniform float uTextureX; \n "
" uniform float uTextureY;\n "
#endif
" void main(void) {\n "
"     gl_Position = uMVMatrix * vec4(aVertexPosition.x, aVertexPosition.y, aVertexPosition.z, 1.0);\n "
"     outFloatChannelX = aVertexPosition.x/(uRangeX * 2.0);\n"
"     outFloatChannelY = aVertexPosition.y/(uRangeY * 2.0);\n"
"     outFloatChannelZ = aVertexPosition.z/450.0f;\n"
"     outNormTexture.x = aTextureCoord1.t/uTextureX;\n"
"     outNormTexture.y = aTextureCoord1.s/uTextureY;\n"
"     outNormTexture1.x = aTextureCoord2.t/uTextureX;\n"
"     outNormTexture1.y = aTextureCoord2.s/uTextureY;\n"
"     outBlendVals = blendVals;\n"
" }\n"
;

static const char srv_frag_shader_lut[] =
#ifndef STANDALONE
" #extension GL_OES_EGL_image_external : require \n"
" precision mediump float;\n "
#endif
#ifdef GL_ES
//" precision mediump float;\n "
#endif
#ifndef STANDALONE
" uniform samplerExternalOES uSampler[2];\n "
#else
" uniform sampler2D uSampler[2];\n "
#endif

" uniform int select;\n "
" varying vec2 outNormTexture;\n "
" varying vec2 outNormTexture1;\n "
" varying vec2 outBlendVals;\n "
" varying float outFloatChannelX;\n "
" varying float outFloatChannelY;\n "
" varying float outFloatChannelZ;\n "
" vec4 iFragColor1; \n "
" vec4 iFragColor2; \n "
" vec4 outColor; \n "
" vec4 packFloatToVec4i(const float value)\n"
" {\n"
"	  const vec4 bitSh = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);\n"
"     const vec4 bitMsk = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);\n"
"     vec4 res = fract(value * bitSh);\n"
"     res -= res.xxyz * bitMsk;\n"
"     return res;\n"
" }\n"
" float unpackFloatFromVec4i(const vec4 value)\n"
" {\n"
"     const vec4 bitSh = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);\n"
"     return(dot(value, bitSh));\n"
" }\n"
" void main(){\n"
"     iFragColor1  = texture2D(uSampler[0], outNormTexture);\n "
"     iFragColor2  = texture2D(uSampler[1], outNormTexture1);\n "
"     if (select == 0) \n"
"     { \n"
"     	   outColor = packFloatToVec4i(outFloatChannelX);\n "
"     } \n"
"     else if (select == 1) \n"
"     { \n"
"     	   outColor = packFloatToVec4i(outFloatChannelY);\n "
"     } \n"
"     else if (select == 2) \n"
"     { \n"
"     	   outColor = packFloatToVec4i(outFloatChannelZ);\n "
"     } \n"
"     else \n"
"     { \n"
"     	   outColor = iFragColor1;\n "
"     	   outColor = (outBlendVals.x)*iFragColor1 + (outBlendVals.y)*iFragColor2;\n "
"     } \n"
"     gl_FragColor.rgba = outColor.abgr;\n "
" }\n"
;

static const char srv_vert_shader[] =
" attribute vec3 aVertexPosition;\n "
" attribute vec2 aTextureCoord1;\n "
" attribute vec2 aTextureCoord2;\n "
" attribute vec2 blendVals;\n "
" uniform mat4 uMVMatrix;\n "
" varying vec2 outNormTexture;\n "
" varying vec2 outNormTexture1;\n "
" varying vec2 outBlendVals;\n "
#ifdef STATIC_COORDINATES
" float uRangeX = 440.0; \n "
" float uRangeY = 540.0; \n "
" float uTextureX = 20480.0; \n "
" float uTextureY = 11520.0;\n "
#else
" uniform float uRangeX; \n "
" uniform float uRangeY; \n "
" uniform float uTextureX; \n "
" uniform float uTextureY;\n "
#endif

" void main(void) {\n "
"     gl_Position = uMVMatrix * vec4(aVertexPosition.x, aVertexPosition.y, aVertexPosition.z, 1.0);\n "
"     outNormTexture.x = aTextureCoord1.t/uTextureX;\n"
"     outNormTexture.y = aTextureCoord1.s/uTextureY;\n"
"     outNormTexture1.x = aTextureCoord2.t/uTextureX;\n"
"     outNormTexture1.y = aTextureCoord2.s/uTextureY;\n"
"     outBlendVals = blendVals;\n"
" }\n"
;

static const char srv_frag_shader[] =
#ifndef STANDALONE
" #extension GL_OES_EGL_image_external : require \n"
#endif
#ifdef GL_ES
" precision mediump float;\n "
#endif
#ifndef STANDALONE
" uniform samplerExternalOES uSampler[2];\n "
#else
" uniform sampler2D uSampler[2];\n "
#endif

" uniform int select;\n "
" varying vec2 outNormTexture;\n "
" varying vec2 outNormTexture1;\n "
" varying vec2 outBlendVals;\n "
" vec4 iFragColor1; \n "
" vec4 iFragColor2; \n "
" void main(){\n"
"     iFragColor1  = texture2D(uSampler[0], outNormTexture);\n "
"     iFragColor2  = texture2D(uSampler[1], outNormTexture1);\n "
#if defined (QNX)
"     gl_FragColor.rgba = ((outBlendVals.x)*iFragColor1 + (outBlendVals.y)*iFragColor2).bgra;\n "
#else
"     gl_FragColor = (outBlendVals.x)*iFragColor1 + (outBlendVals.y)*iFragColor2;\n "
#endif
" }\n"
;

void generate_indices(t_index_buffer *index_buffer, int xlength, int ylength, int gap)
{
	unsigned int *buffer = index_buffer->buffer;
	int x, y, k=0;
	for (y=0; y<ylength-gap; y+=gap)
	{
		if(y>0)
			buffer[k++]=(unsigned int) (y*xlength);
		for (x=0; x<xlength; x+=gap)
		{
			buffer[k++]=(unsigned int) (y*xlength + x);
			buffer[k++]=(unsigned int) ((y+gap)*xlength + x);
		}
		if(y < ylength - 1 - gap)
			buffer[k++]=(unsigned int) ((y+gap)*xlength + (xlength -1));
	}
	index_buffer->length = k;
}

void srv_bowl_inc()
{
	if(lut_external == true)
	{
		printf("Operation not supported for external LUT\n");
		return;
	}
	switch(srv_bowl_reshape)
	{
		case left:
			srv_offset_x_left -= 50;
			break;
		case right:
			srv_offset_x_right += 50;
			break;
		case front:
			srv_offset_y_front -= 50;
			break;
		case back:
			srv_offset_y_back += 50;
			break;
		default:
			break;
	}
	srv_generate_lut(POINTS_WIDTH, POINTS_HEIGHT,
			cam_width,
			cam_height,
			POINTS_SUBX,
			POINTS_SUBY,
			(srv_lut_t *)lut_staging);
	lut_updated = true;
}

void srv_bowl_dec()
{
	if(lut_external == true)
	{
		printf("Operation not supported for external LUT\n");
		return;
	}
	switch(srv_bowl_reshape)
	{
		case left:
			srv_offset_x_left += 50;
			break;
		case right:
			srv_offset_x_right -= 50;
			break;
		case front:
			srv_offset_y_front += 50;
			break;
		case back:
			srv_offset_y_back -= 50;
			break;
		default:
			break;
	}
	srv_generate_lut(POINTS_WIDTH, POINTS_HEIGHT,
			cam_width,
			cam_height,
			POINTS_SUBX,
			POINTS_SUBY,
			(srv_lut_t *)lut_staging);
	lut_updated = true;
}

#ifdef STANDALONE
typedef struct _srv_bowl_offset_t
{
	int l, r, f, b;
} srv_bowl_offset_t;

static srv_bowl_offset_t srv_bowl_offsets[SRV_NUM_IMAGE_SETS] =
{
	{l: -400, r: 400, f: -400, b: 400},
	{l: -200, r: 400, f: -400, b: 400},
	{l: -200, r: 200, f: -400, b: 400},
	{l: -200, r: 200, f: -200, b: 400}
};

static void srv_set_bowl_offsets(unsigned int image_set)
{
	srv_offset_x_left = srv_bowl_offsets[image_set].l;
	srv_offset_x_right = srv_bowl_offsets[image_set].r;
	srv_offset_y_front = srv_bowl_offsets[image_set].f;
	srv_offset_y_back = srv_bowl_offsets[image_set].b;
}

void srv_bowl_update()
{
	if(lut_external == true)
	{
		printf("Operation not supported for external LUT\n");
		return;
	}

	srv_set_bowl_offsets(active_image_set);

	srv_generate_lut(POINTS_WIDTH, POINTS_HEIGHT,
			cam_width,
			cam_height,
			POINTS_SUBX,
			POINTS_SUBY,
			(srv_lut_t *)lut_staging);
	lut_updated = true;
	srv_image_offset = active_image_set;
}
#endif

int srv_setup(render_state_t *pObj)
{
	if(pObj->LUT3D == NULL)
	{
		// Generate the LUT if it wasn't passed
		pObj->LUT3D = malloc(sizeof(int16_t) * POINTS_WIDTH/POINTS_SUBX * POINTS_HEIGHT/POINTS_SUBY * 9 * 2);
		lut_staging = malloc(sizeof(int16_t) * POINTS_WIDTH/POINTS_SUBX * POINTS_HEIGHT/POINTS_SUBY * 9 * 2);
		cam_width = pObj->cam_width;
		cam_height = pObj->cam_height;
#ifdef STANDALONE
		srv_image_offset = active_image_set;
		srv_set_bowl_offsets(active_image_set);
#endif
		srv_generate_lut(POINTS_WIDTH, POINTS_HEIGHT,
				pObj->cam_width,
				pObj->cam_height,
				POINTS_SUBX,
				POINTS_SUBY,
				(srv_lut_t *)pObj->LUT3D);
		lut_external = false;
	}

#ifndef SRV_INTERLEAVED_BLEND
	if(pObj->blendLUT3D == NULL)
	{
		// Generate the BlendLUT if it wasn't passed
		pObj->blendLUT3D = malloc(sizeof(int16_t) * POINTS_WIDTH/POINTS_SUBX * POINTS_HEIGHT/POINTS_SUBY * 9 * 2);
		srv_generate_blend_lut(POINTS_WIDTH, POINTS_HEIGHT,
				POINTS_SUBX,
				POINTS_SUBY,
				(srv_blend_lut_t *)pObj->blendLUT3D);
	}
#endif
	if(srv_render_to_file == true)
	{
		uiProgramObject = render_createProgram(
				srv_vert_shader_lut,
				srv_frag_shader_lut
				);
	}
	else
	{
#if 1
		uiProgramObject = renderutils_createProgram(
				srv_vert_shader,
				srv_frag_shader
				);
#else
		uiProgramObject = renderutils_loadAndCreateProgram("srv_vert.vsh", "srv_frag.fsh");
#endif
	}
	if (uiProgramObject==0)
	{
		return -1;
	}

	glUseProgram(uiProgramObject);

#ifndef STANDALONE
	appEglCheckGlError("glUseProgram");
#endif
	//locate sampler uniforms
	uniform_select = glGetUniformLocation(uiProgramObject, "select");
	GL_CHECK(glGetAttribLocation);
	samplerLocation0 = glGetUniformLocation(uiProgramObject, "uSampler[0]");
	glUniform1i(samplerLocation0, 0);
	GL_CHECK(glUniform1i);
	samplerLocation1 = glGetUniformLocation(uiProgramObject, "uSampler[1]");
	glUniform1i(samplerLocation1, 1);
	GL_CHECK(glUniform1i);
	mvMatrixLocation = glGetUniformLocation(uiProgramObject, "uMVMatrix");
	GL_CHECK(glGetAttribLocation);

#ifndef STATIC_COORDINATES
	uniform_rangex = glGetUniformLocation(uiProgramObject, "uRangeX");
	GL_CHECK(glGetUniformLocation);
	uniform_rangey = glGetUniformLocation(uiProgramObject, "uRangeY");
	GL_CHECK(glGetUniformLocation);
	uniform_texturex = glGetUniformLocation(uiProgramObject, "uTextureX");
	GL_CHECK(glGetUniformLocation);
	uniform_texturey= glGetUniformLocation(uiProgramObject, "uTextureY");
	GL_CHECK(glGetUniformLocation);
	glUniform1f(uniform_rangex, (GLfloat)float(POINTS_WIDTH/2));
	GL_CHECK(glUniform1f);
	glUniform1f(uniform_rangey, (GLfloat)float(POINTS_HEIGHT/2));
	GL_CHECK(glUniform1f);
	glUniform1f(uniform_texturex, (GLfloat)float(pObj->cam_width * 16));
	GL_CHECK(glUniform1f);
	glUniform1f(uniform_texturey, (GLfloat)float(pObj->cam_height * 16));
	GL_CHECK(glUniform1f);
#endif

	vertexPositionAttribLoc = glGetAttribLocation(uiProgramObject, "aVertexPosition");
	GL_CHECK(glGetAttribLocation);
	blendAttribLoc = glGetAttribLocation(uiProgramObject, "blendVals");
	GL_CHECK(glGetAttribLocation);
	vertexTexCoord1AttribLoc = glGetAttribLocation(uiProgramObject, "aTextureCoord1");
	GL_CHECK(glGetAttribLocation);
	vertexTexCoord2AttribLoc = glGetAttribLocation(uiProgramObject, "aTextureCoord2");
	GL_CHECK(glGetAttribLocation);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#ifndef STANDALONE
	appEglCheckGlError("glClearColor");
#endif

	glDisable(GL_DEPTH_TEST);
#ifdef ENABLE_GLOBAL_BLENDING
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	//Generate indices
	for (int i = 0; i < MAX_INDEX_BUFFERS; i++)
	{
		index_buffers[i].buffer = (unsigned int *)malloc(QUADRANT_WIDTH * QUADRANT_HEIGHT * 3 * sizeof(unsigned int));
		generate_indices((t_index_buffer *)&index_buffers[i], QUADRANT_WIDTH, QUADRANT_HEIGHT, pow(2,i));
	}

	// Generate named buffers for indices and vertices
	glGenBuffers(QUADRANTS*3, vboId);

	return 0;
}

//Vertices init for surround view (VBO approach)
static int surroundview_init_vertices_vbo(render_state_t *pObj, GLuint vertexId, GLuint indexId, GLuint blendId,
		void* vertexBuff, void* indexBuff, void * blendBuff,
		int vertexBuffSize, int indexBuffSize, int blendBuffSize
		)
{
	//upload the vertex and texture and image index interleaved array
	//Bowl LUT - Interleaved data (5 data)
	glBindBuffer(GL_ARRAY_BUFFER, vertexId);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffSize, vertexBuff, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionAttribLoc, 3, GL_VERTEX_ENUM, GL_FALSE, sizeof(srv_lut_t), 0);
	glVertexAttribPointer(vertexTexCoord1AttribLoc, 2, GL_TEXCOORD_ENUM, GL_FALSE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, u1)));
	glVertexAttribPointer(vertexTexCoord2AttribLoc, 2, GL_TEXCOORD_ENUM, GL_FALSE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, u2)));
#ifdef SRV_INTERLEAVED_BLEND
	glVertexAttribPointer(blendAttribLoc, 2, GL_BLEND_ENUM, GL_TRUE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, blend1)));
#endif
	GL_CHECK(glVertexAttribPointer);

#ifndef SRV_INTERLEAVED_BLEND
	glBindBuffer(GL_ARRAY_BUFFER, blendId);
	glBufferData(GL_ARRAY_BUFFER, blendBuffSize, blendBuff, GL_STATIC_DRAW);
	glVertexAttribPointer(blendAttribLoc, 2, GL_BLEND_ENUM, GL_TRUE, sizeof(_srv_blend_lut_t), 0);
	GL_CHECK(glVertexAttribPointer);
#endif

	//Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffSize, indexBuff, GL_STATIC_DRAW);
	GL_CHECK(glBufferData);

	//Enable for the rendering
	glEnableVertexAttribArray(vertexPositionAttribLoc);
	glEnableVertexAttribArray(vertexTexCoord1AttribLoc);
	glEnableVertexAttribArray(vertexTexCoord2AttribLoc);
	glEnableVertexAttribArray(blendAttribLoc);

	return 0;
}

void surroundview_init_vertices_vbo_wrap(render_state_t *pObj)
{
	int i;
	int vertexoffset = 0;
	void *lut_temp;
	int blendoffset = 0;

	if(lut_updated == true)
	{
		lut_temp = pObj->LUT3D;
		pObj->LUT3D = lut_staging;
		lut_staging = lut_temp;
	}

	for(i = 0;i < QUADRANTS;i ++)
	{
		vertexoffset = i * (sizeof(srv_lut_t)*QUADRANT_WIDTH*QUADRANT_HEIGHT);
		blendoffset = i * (sizeof(srv_blend_lut_t)*QUADRANT_WIDTH*QUADRANT_HEIGHT);
		surroundview_init_vertices_vbo(
				pObj,
				vboId[i*3], vboId[i*3+1], vboId[i*3+2],
				(char*)pObj->LUT3D + vertexoffset,
				(char*)(index_buffers[active_index_buffer].buffer),
				(char*)pObj->blendLUT3D + blendoffset,
				sizeof(srv_lut_t)*QUADRANT_WIDTH*QUADRANT_HEIGHT,
				sizeof(int)*(index_buffers[active_index_buffer].length),
				sizeof(srv_blend_lut_t)*QUADRANT_WIDTH*QUADRANT_HEIGHT
				);
	}

	index_buffer_changed = false;
	lut_updated = false;
}

void onscreen_mesh_state_restore_program_textures_attribs(render_state_t *pObj, GLuint *texYuv, int tex1, int tex2, int viewport_id)
{
	//set the program we need
	glUseProgram(uiProgramObject);

	glUniform1i(samplerLocation0, 0);
	glActiveTexture(GL_TEXTURE0);

#ifndef STANDALONE
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv[tex1]);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
	glBindTexture(GL_TEXTURE_2D, texYuv[tex1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
	GL_CHECK(glBindTexture);

	glUniform1i(samplerLocation1, 1);
	glActiveTexture(GL_TEXTURE1);

#ifndef STANDALONE
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv[tex2]);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
	glBindTexture(GL_TEXTURE_2D, texYuv[tex2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
	GL_CHECK(glBindTexture);

	//Enable the attributes
	glEnableVertexAttribArray(vertexPositionAttribLoc);
	glEnableVertexAttribArray(vertexTexCoord1AttribLoc);
	glEnableVertexAttribArray(vertexTexCoord2AttribLoc);
	glEnableVertexAttribArray(blendAttribLoc);

	glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, &mMVP_bowl[viewport_id][0][0]);
	GL_CHECK(glUniformMatrix4fv);

	glUniform1i(uniform_select, shader_output_select);

}

void onscreen_mesh_state_restore_vbo(render_state_t *pObj,
		GLuint vertexId, GLuint indexId, GLuint blendId)
{

	//restore the vertices and indices
	glBindBuffer(GL_ARRAY_BUFFER, vertexId);
	glVertexAttribPointer(vertexPositionAttribLoc, 3, GL_VERTEX_ENUM, GL_FALSE, sizeof(srv_lut_t), 0);
	glVertexAttribPointer(vertexTexCoord1AttribLoc, 2, GL_TEXCOORD_ENUM, GL_FALSE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, u1)));
	glVertexAttribPointer(vertexTexCoord2AttribLoc, 2, GL_TEXCOORD_ENUM, GL_FALSE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, u2)));
#ifdef SRV_INTERLEAVED_BLEND
	glVertexAttribPointer(blendAttribLoc, 2, GL_BLEND_ENUM, GL_TRUE, sizeof(srv_lut_t), (GLvoid*)(offsetof(srv_lut_t, blend1)));
#else
	glBindBuffer(GL_ARRAY_BUFFER, blendId);
	glVertexAttribPointer(blendAttribLoc, 2, GL_BLEND_ENUM, GL_TRUE, sizeof(_srv_blend_lut_t), 0);
#endif
	//Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
}

void srv_draw(render_state_t *pObj, GLuint *texYuv, int viewport_id)
{
	int i;
	GLuint *tex = texYuv;
	if(prevLUT != pObj->LUT3D || index_buffer_changed == true || lut_updated == true)
	{
		prevLUT = pObj->LUT3D;
		surroundview_init_vertices_vbo_wrap(pObj);
	}

#ifdef STANDALONE
	tex = &texYuv[4 * srv_image_offset];
#endif
	//First setup the program once
	glUseProgram(uiProgramObject);
	//then change the meshes and draw
	for(i = 0;i < QUADRANTS;i ++)
	{
		onscreen_mesh_state_restore_program_textures_attribs(
				pObj, tex, (0+i)%4, (3+i)%4, viewport_id);
		onscreen_mesh_state_restore_vbo(
				pObj, vboId[i*3], vboId[i*3+1], vboId[i*3+2]);
		GL_CHECK(onscreen_mesh_state_restore_vbo);
		glDrawElements(render_mode, index_buffers[active_index_buffer].length, GL_UNSIGNED_INT,  0);
		GL_CHECK(glDrawElements);
	}
}
