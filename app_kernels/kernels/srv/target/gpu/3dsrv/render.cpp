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

#include <utils/include/resources.h>

#include "render.h"
#include "car.h"
#include "srv_views.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#ifdef _WIN32
#include <Windows.h>
HANDLE hThread;
DWORD   dwThreadId;
#else
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

pthread_t scan_thread;
int scan_thread_data;

//Variables needed for FPS calculation
struct timeval tv;
unsigned long prev_time_usecs = 0;
unsigned long cur_time_usecs, delta_usecs;
float fps;
unsigned int frame_count=0;
#endif

/**View port definitions**/
#define MAX_VIEWPORTS 2
typedef struct _srv_viewport_t
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	bool animate;
} srv_viewport_t;

srv_viewport_t srv_viewports[] = {
	{
		x : 0,
		y : 0,
		width : 1920,
		height: 1080,
		animate: true,
	},
#if 0
	{
		x : 960,
		y : 0,
		width : 960,
		height: 1080,
		animate: false,
	}
#endif
};
int num_viewports = 0;

#ifdef SRV_USE_JOYSTICK
#include <fcntl.h>
#include <linux/input.h>
#define SRV_JOYSTICK_DEVICE_NAME "/dev/input/by-id/usb-Logitech_Logitech_Extreme_3D-event-joystick"
#endif

srv_coords_t srv_coords_vp[MAX_VIEWPORTS];
extern srv_coords_t srv_coords[];
extern int num_srv_views;

extern srv_bowl_reshape_t srv_bowl_reshape;
bool srv_bowl_reshape_dir;
int srv_param_select = 0;
bool srv_param_car = true;
bool srv_param_bowl = true;
int srv_param_view1 = 0;
int srv_param_view2 = 1;
int srv_param_step = 0;
bool srv_render_to_file = false;
bool srv_exit = false;
#ifndef STANDALONE
bool srv_print_stats = false;
#else
bool srv_print_stats = true;
#endif
FILE *fx, *fy, *fz, *fc;

#ifdef STANDALONE
extern unsigned int active_image_set;
#endif

// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
glm::mat4 mProjection[MAX_VIEWPORTS];
// Camera matrix
glm::mat4 mView[MAX_VIEWPORTS];
// Model matrix : an identity matrix (model will be at the origin)
glm::mat4 mModel_bowl[MAX_VIEWPORTS];  // Changes for each model !
// Our ModelViewProjection : multiplication of our 3 matrices
glm::mat4 mMVP_bowl[MAX_VIEWPORTS];
glm::mat4 mMVP_car[MAX_VIEWPORTS];

glm::vec3 vCameraDir[MAX_VIEWPORTS];
extern int shader_output_select;
extern GLint uniform_select;
extern unsigned int active_index_buffer;
extern bool index_buffer_changed;
extern GLenum render_mode;
#if 0//def STANDALONE
void System_eglCheckEglError(const char* op, EGLBoolean returnVal) {
	EGLint error;

	if (returnVal != EGL_TRUE) {
		printf(" EGL: %s() returned %d\n", op, returnVal);
	}

	for (error = eglGetError(); error != EGL_SUCCESS; error = eglGetError()) {
		printf(" EGL: after %s() eglError (0x%x)\n", op, error);
	}
}
#endif

char *get_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

void System_eglCheckGlError(const char* op) {
   GLint error;

   for (error = glGetError(); error; error = glGetError()) {
       printf("GL: after %s() glError (0x%x)\n", op, error);
   }
}

#define RENDER_PRINT(args...) if(srv_print_stats) printf(args)

#ifndef _WIN32
static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    printf("Terminal setting flags:0x%x", oldt.c_lflag);
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}
#endif

enum e_mode {
	cam,
	target,
	angle
} mode;
#if 0
float camx = 0.0;
float camy = 0.0;
float camz = 640.0f;
float targetx = 0.0;
float targety = 0.0;
float targetz = 0.0;
float anglex = 0.0f;
float angley = 0.0f;
float anglez = 0.0f;
#endif
float cam_delta = 20.0f;
float target_delta = 5.0f;
float angle_delta = 0.5f;
float cam_delta_inc = 20.0f;
float target_delta_inc = 500.0f;
float angle_delta_inc = 5000.1f;
bool bRenderCar = true;
bool bRenderSRV = true;
bool joystick_enabled = false;
static int current_viewport = 0;

static float *delta = &cam_delta;
static float *delta_inc = &cam_delta_inc;
static float *x, *y, *z;

#define COORD_TRANSITION(vp, x, i, j) \
  srv_coords_vp[vp].x = srv_coords_vp[vp].x + j * ((srv_coords[i].x - srv_coords_vp[vp].x)/num_iterations)

#define SET_COORDS(vp, x, i) \
  srv_coords_vp[vp].x = srv_coords[i].x

#define MODE_CAM(s) do { \
	mode = cam; \
	delta = &cam_delta; \
	delta_inc = &cam_delta_inc; \
	x = &s.camx; \
	y = &s.camy; \
	z = &s.camz; \
	} while(0)

#define MODE_TARGET(s) do { \
	mode = target; \
	delta = &target_delta; \
	delta_inc = &target_delta_inc; \
	x = &s.targetx; \
	y = &s.targety; \
	z = &s.targetz; \
	} while(0)

#define MODE_ANGLE(s) do { \
	mode = angle; \
	delta = &angle_delta; \
	delta_inc = &angle_delta_inc; \
	x = &s.anglex; \
	y = &s.angley; \
	z = &s.anglez; \
	} while(0)

static int gap = 50;
int num_iterations = 30;
static int current_index[MAX_VIEWPORTS];
static int step[MAX_VIEWPORTS]  = {0};
bool animatengap[MAX_VIEWPORTS] = {false};

void set_coords(int viewport_id, int index)
{
	SET_COORDS(viewport_id, camx, index);
	SET_COORDS(viewport_id, camy, index);
	SET_COORDS(viewport_id, camz, index);
	SET_COORDS(viewport_id, targetx, index);
	SET_COORDS(viewport_id, targety, index);
	SET_COORDS(viewport_id, targetz, index);
	SET_COORDS(viewport_id, anglex, index);
	SET_COORDS(viewport_id, angley, index);
	SET_COORDS(viewport_id, anglez, index);
	render_updateView();
}

void set_coords_transition(int viewport_id, int index1, int index2, int step)
{
	SET_COORDS(viewport_id, camx, index1);
	SET_COORDS(viewport_id, camy, index1);
	SET_COORDS(viewport_id, camz, index1);
	SET_COORDS(viewport_id, targetx, index1);
	SET_COORDS(viewport_id, targety, index1);
	SET_COORDS(viewport_id, targetz, index1);
	SET_COORDS(viewport_id, anglex, index1);
	SET_COORDS(viewport_id, angley, index1);
	SET_COORDS(viewport_id, anglez, index1);
	COORD_TRANSITION(viewport_id, camx, index2, step);
	COORD_TRANSITION(viewport_id, camy, index2, step);
	COORD_TRANSITION(viewport_id, camz, index2, step);
	COORD_TRANSITION(viewport_id, targetx, index2, step);
	COORD_TRANSITION(viewport_id, targety, index2, step);
	COORD_TRANSITION(viewport_id, targetz, index2, step);
	COORD_TRANSITION(viewport_id, anglex, index2, step);
	COORD_TRANSITION(viewport_id, angley, index2, step);
	COORD_TRANSITION(viewport_id, anglez, index2, step);
	render_updateView();
}

void animate_to_coords(int viewport_id, int index, unsigned int usec_between_frames)
{
	int j;
	for(j = 0; j < num_iterations; j++)
	{
		COORD_TRANSITION(viewport_id, camx, index, j);
		COORD_TRANSITION(viewport_id, camy, index, j);
		COORD_TRANSITION(viewport_id, camz, index, j);
		COORD_TRANSITION(viewport_id, targetx, index, j);
		COORD_TRANSITION(viewport_id, targety, index, j);
		COORD_TRANSITION(viewport_id, targetz, index, j);
		COORD_TRANSITION(viewport_id, anglex, index, j);
		COORD_TRANSITION(viewport_id, angley, index, j);
		COORD_TRANSITION(viewport_id, anglez, index, j);
		render_updateView();
		//car_x = srv_coords[i].car_x + j * ((srv_coords[(i+1)%num_srv_views].car_x - srv_coords[i].car_x)/num_iterations);
#ifdef _WIN32
		Sleep(usec_between_frames/1000);
#else
		usleep(usec_between_frames);
#endif
	}
}
#ifdef SRV_USE_JOYSTICK
void *scan_thread_function(void *ptr)
{
	int i, fd, rd;
	char input;
	struct input_event ev[64];
	int version;

	MODE_CAM(srv_coords_vp[0]);

	if((fd = open(SRV_JOYSTICK_DEVICE_NAME, O_RDONLY)) < 0)
	{
		printf("Unable to open joystick device\n");
		return NULL;
	}
	if(ioctl(fd, EVIOCGVERSION, &version))
	{
		printf("Cannot get event io version\m");
		return NULL;
	}

	printf("Event driver version: %d\n", version);


	while(1)
	{
		rd = read(fd, ev, sizeof(struct input_event) * 64);
		if (rd < (int) sizeof(struct input_event))
		{
			printf("Invalid read from joystick driver\n");
			return NULL;
		}

		for (i = 0; i < rd/sizeof(struct input_event); i++)
		{
			if(ev[i].type == EV_KEY)
			{
				switch(ev[i].code)
				{
				case 288:
					if(ev[i].value == 1)
					{
						MODE_TARGET(srv_coords_vp[0]);
					}
					else
					{
						MODE_CAM(srv_coords_vp[0]);
					}
					break;
				case 289:
					if(ev[i].value == 1)
					{
						MODE_ANGLE(srv_coords_vp[0]);
					}
					else
					{
						MODE_CAM(srv_coords_vp[0]);
					}
					break;
				case 290:
					if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_BS_LEFT, 6000);
					}
					break;
				case 291:
					if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_BS_RIGHT, 6000);
					}
					break;
				case 292:
					if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_ZOOMED_OUT, 6000);
					}
					break;
				case 293:
					if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_ZOOMED_IN, 6000);
					}
					break;
				case 294:
					camx = srv_coords[0].camx;
					camy = srv_coords[0].camy;
					camz = srv_coords[0].camz;
					targetx = srv_coords[0].targetx;
					targety = srv_coords[0].targety;
					targetz = srv_coords[0].targetz;
					anglex = srv_coords[0].anglex;
					angley = srv_coords[0].angley;
					anglez = srv_coords[0].anglez;
					break;
				case 296:
					if(ev[i].value == 1)
						bRenderSRV = !bRenderSRV;
					break;
				case 297:
					if(ev[i].value == 1)
						bRenderCar = !bRenderCar;
					break;
#if 0
				case 298:
					break;
				case 299:
					break;
#endif
				default:
					break;
				}
			}
			if(ev[i].type == EV_ABS)
			{
				switch(ev[i].code)
				{
				case 0: /* X axis */
					if(joystick_enabled)
					{
						if(abs(ev[i].value - 512) > 50)
							*x += (ev[i].value - 512)/(*delta_inc);
					}
					break;
				case 1: /* X axis */
					if(joystick_enabled)
					{
						if(abs(ev[i].value - 512) > 50)
							*y += (ev[i].value - 512)/(*delta_inc);
					}
					break;
				case 5: /* Z axis */
					if(joystick_enabled)
					{
						if(abs(ev[i].value - 128) > 20)
							*z += (ev[i].value - 128)/(*delta_inc);
					}
					break;
				case 6: /* Enable joystick */
					if(ev[i].value == 0)
					{
						joystick_enabled = true;
					}
					else
					{
						joystick_enabled = false;
					}
					break;
				case 16:
					if(ev[i].value == -1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_LEFT, 6000);
					}
					else if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_RIGHT, 6000);
					}
					break;
				case 17:
					if(ev[i].value == -1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_FRONT, 6000);
					}
					else if(ev[i].value == 1)
					{
						animate = false;
						animate_to_coords(SRV_VIEW_BACK, 6000);
					}

					break;
				default:
					break;
				}
			}
		}
#if 0
		switch(input)
		{
		case 'i':
			if(*delta < 0)
				*delta = -*delta;
			break;
		case 'd':
			if(*delta > 0)
				*delta = -*delta;
			break;
		case 'x':
				*x += *delta;
			break;
		case 'y':
				*y += *delta;
			break;
		case 'z':
				*z += *delta;
			break;
		case 'm':
			if(*delta >=0)
				*delta += *delta_inc;
			else
				*delta -= *delta_inc;
			printf("Delta: %f\n", *delta);
			break;
		case 'l' :
			if(*delta >=0)
				*delta -= *delta_inc;
			else
				*delta += *delta_inc;
			break;
		case 'g' :
			if(active_index_buffer == 0)
				active_index_buffer = 1;
			else
				active_index_buffer = 0;
			index_buffer_changed = true;
			break;
		case '1':
			animate = false;
			animate_to_coords(0, 6000);
			break;
		case '2':
			animate = false;
			animate_to_coords(1, 6000);
			break;
		case '3':
			animate = false;
			animate_to_coords(2, 6000);
			break;
		case 'j':
			animate = true;
			break;
		case 'k':
			animate = false;
			break;
		case 'q':
			camx = srv_coords[0].camx;
			camy = srv_coords[0].camy;
			camz = srv_coords[0].camz;
			targetx = srv_coords[0].targetx;
			targety = srv_coords[0].targety;
			targetz = srv_coords[0].targetz;
			anglex = srv_coords[0].anglex;
			angley = srv_coords[0].angley;
			anglez = srv_coords[0].anglez;
			break;
		case 'n':
			if(render_mode == GL_LINE_STRIP)
				render_mode = GL_TRIANGLE_STRIP;
			else
				render_mode = GL_LINE_STRIP;
			break;
		default:
			break;
		}
#endif
		render_updateView();
		RENDER_PRINT("%f, %f, %f, %f, %f, %f, %f, %f, %f\n",
				camx,
				camy,
				camz,
				targetx,
				targety,
				targetz,
				anglex,
				angley,
				anglez);
	}
}
#else
void render_process_keys(char input)
{
	switch(input)
	{
	case 'c':
		MODE_CAM(srv_coords_vp[current_viewport]);
		break;
	case 't':
		MODE_TARGET(srv_coords_vp[current_viewport]);
		break;
	case 'a':
		MODE_ANGLE(srv_coords_vp[current_viewport]);
		break;
	case 'b':
		srv_param_bowl = !srv_param_bowl;
		break;
	case 'v':
		srv_param_car = !srv_param_car;
		break;
	case 'i':
		if(*delta < 0)
			*delta = -*delta;
		break;
	case 'd':
		if(*delta > 0)
			*delta = -*delta;
		break;
	case 'x':
		*x += *delta;
		break;
	case 'y':
		*y += *delta;
		break;
	case 'z':
		*z += *delta;
		break;
	case 'm':
		if(*delta >=0)
			*delta += *delta_inc;
		else
			*delta -= *delta_inc;
		RENDER_PRINT("Delta: %f\n", *delta);
		break;
	case 'l' :
		if(*delta >=0)
			*delta -= *delta_inc;
		else
			*delta += *delta_inc;
		break;
	case 'g' :
		if(active_index_buffer == 0)
			active_index_buffer = 1;
		else
			active_index_buffer = 0;
		index_buffer_changed = true;
		break;
	case '<':
		current_viewport = (current_viewport - 1)%num_viewports;
		break;
	case '>':
		current_viewport = (current_viewport + 1)%num_viewports;
		break;
	case '1':
		srv_viewports[current_viewport].animate = false;
		animate_to_coords(current_viewport, 0, 6000);
		break;
	case '2':
		srv_viewports[current_viewport].animate = false;
		animate_to_coords(current_viewport, 1, 6000);
		break;
	case '3':
		srv_viewports[current_viewport].animate = false;
		animate_to_coords(current_viewport, 2, 6000);
		break;
	case 'j':
		srv_viewports[current_viewport].animate = true;
		break;
	case 'k':
		srv_viewports[current_viewport].animate = false;
		break;
	case 'q':
		srv_exit = true;
		break;
	case 'n':
		if(render_mode == GL_LINE_STRIP)
			render_mode = GL_TRIANGLE_STRIP;
		else
			render_mode = GL_LINE_STRIP;
		break;
	case 'w':
		car_change();
		break;
	case 'L':
		srv_bowl_reshape = left;
		if(srv_bowl_reshape_dir)
			srv_bowl_inc();
		else
			srv_bowl_dec();
		break;
	case 'R':
		srv_bowl_reshape = right;
		if(srv_bowl_reshape_dir)
			srv_bowl_inc();
		else
			srv_bowl_dec();
		break;
	case 'F':
		srv_bowl_reshape = front;
		if(srv_bowl_reshape_dir)
			srv_bowl_inc();
		else
			srv_bowl_dec();
		break;
	case 'B':
		srv_bowl_reshape = back;
		if(srv_bowl_reshape_dir)
			srv_bowl_inc();
		else
			srv_bowl_dec();
		break;
	case '.':
		srv_bowl_reshape_dir = true;
		break;
	case ',':
		srv_bowl_reshape_dir = false;
		break;
#ifdef STANDALONE
	case 'N':
		active_image_set = (active_image_set + 1) % SRV_NUM_IMAGE_SETS;
		srv_bowl_update();
		break;
	case 'P':
		active_image_set = (active_image_set - 1) % SRV_NUM_IMAGE_SETS;
		srv_bowl_update();
		break;
#endif
	default:
		break;
	}
	render_updateView();
	RENDER_PRINT("%f, %f, %f, %f, %f, %f, %f, %f, %f\n",
			srv_coords_vp[current_viewport].camx,
			srv_coords_vp[current_viewport].camy,
			srv_coords_vp[current_viewport].camz,
			srv_coords_vp[current_viewport].targetx,
			srv_coords_vp[current_viewport].targety,
			srv_coords_vp[current_viewport].targetz,
			srv_coords_vp[current_viewport].anglex,
			srv_coords_vp[current_viewport].angley,
			srv_coords_vp[current_viewport].anglez);
}
#ifdef _WIN32
DWORD WINAPI scan_thread_function(LPVOID lpParam)
#else
void *scan_thread_function(void *ptr)
#endif
{
	char input;
#ifndef _WIN32
	disable_waiting_for_enter();
#endif

	while(1)
	{
		input = getchar();
		render_process_keys(input);
	}
}
#endif
#define degreesToRadians(x) x*(3.141592f/180.0f)

void  render_updateView()
{
	for(int i = 0; i < num_viewports; i++)
	{
		glm::vec3 camera_position = glm::vec3(srv_coords_vp[i].camx, srv_coords_vp[i].camy, srv_coords_vp[i].camz);
		glm::vec3 camera_target = glm::vec3(srv_coords_vp[i].targetx,srv_coords_vp[i].targety,srv_coords_vp[i].targetz);
		mProjection[i] = glm::perspective(degreesToRadians(40), (float)srv_viewports[i].width/ (float)srv_viewports[i].height, 1.0f, 5000.0f);
		mView[i]       = glm::lookAt(
				camera_position,
				camera_target,
				glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
				);
		mView[i] = glm::rotate(mView[i], srv_coords_vp[i].anglex, glm::vec3(1.0, 0.0, 0.0));
		mView[i] = glm::rotate(mView[i], srv_coords_vp[i].angley, glm::vec3(0.0, 1.0, 0.0));
		mView[i] = glm::rotate(mView[i], srv_coords_vp[i].anglez, glm::vec3(0.0, 0.0, 1.0));
		mModel_bowl[i]     = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, (80.0f/540.0f)));  // Changes for each model !
		//mModel_car = glm::translate(mModel_car, glm::vec3(0.0, 0.0, 800.0));
		mMVP_bowl[i]       = mProjection[i] * mView[i] * mModel_bowl[i];
//		vCameraDir[i] = glm::normalize(camera_position - camera_target);
		vCameraDir[i] = glm::vec3(glm::normalize(mView[i] * glm::vec4(camera_position, 1.0)));	
		car_updateView(i);
	}
}


GLuint render_loadShader(GLenum shaderType, const char* pSource) {
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
                   D_PRINTF(" GL: Could not compile shader %d:\n%s\n",
                       shaderType, buf);
                   free(buf);
               }
           } else {
               D_PRINTF(" GL: Guessing at GL_INFO_LOG_LENGTH size\n");
               char* buf = (char*) malloc(0x1000);
               if (buf) {
                   glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                   D_PRINTF(" GL: Could not compile shader %d:\n%s\n",
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

GLuint render_createProgram(const char* pVertexSource, const char* pFragmentSource) {
   GLuint vertexShader = render_loadShader(GL_VERTEX_SHADER, pVertexSource);
   if (!vertexShader) {
       return 0;
   }

   GLuint pixelShader = render_loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
   if (!pixelShader) {
       glDeleteShader(vertexShader);
       return 0;
   }

   GLuint program = glCreateProgram();
   if (program) {
       glAttachShader(program, vertexShader);
       System_eglCheckGlError("glAttachShader");
       glAttachShader(program, pixelShader);
       System_eglCheckGlError("glAttachShader");
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
                   D_PRINTF(" GL: Could not link program:\n%s\n", buf);
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

int render_setup(render_state_t *pObj)
{
    srv_setup(pObj);

    //Initialize views
    srv_views_init(pObj);

    const auto &path = get_file_path() + std::string("/psdkra/srv");
    ti::utils::setResourcePath(path);

    //STEP2 - initialise the vertices
    car_init(pObj->screen_width, pObj->screen_height);
    GL_CHECK(car_init_vertices_vbo);

    //STEP3 - initialise the individual views
    //screen1_init_vbo();
    GL_CHECK(screen1_init_vbo);

    num_viewports = sizeof(srv_viewports)/sizeof(srv_viewport_t);

    for (int i = 0; i < num_viewports; i++)
    {
	    current_index[i] = (1+i)%num_srv_views;
	    set_coords(i, current_index[i]);
    }
 
    //boxes_init();
    render_updateView();

    // Default mode for key/joystick input
    MODE_CAM(srv_coords_vp[0]);

    //cull
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

#if defined(STANDALONE) || defined(SRV_USE_JOYSTICK)
#ifdef _WIN32
	hThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size
		scan_thread_function,       // thread function name
		NULL,          // argument to thread function
		0,                      // use default creation flags
		&dwThreadId);   // returns the thread identifier


								// Check the return value for success.
								// If CreateThread fails, terminate execution.
								// This will automatically clean up threads and memory.

	if (hThread == NULL)
	{
		printf("CreateThread failed");
		ExitProcess(3);
	}
#else
    pthread_create(&scan_thread, NULL, scan_thread_function, (void *)&scan_thread_data);
#endif
#endif
    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

static int views_change(srv_coords_t original_views, srv_coords_t incoming_views)
{
    int retVal = 0;

    if ( (original_views.camx    != incoming_views.camx) ||
         (original_views.camy    != incoming_views.camy) ||
         (original_views.camz    != incoming_views.camz) ||
         (original_views.targetx != incoming_views.targetx) ||
         (original_views.targety != incoming_views.targety) ||
         (original_views.targetz != incoming_views.targetz) ||
         (original_views.anglex  != incoming_views.anglex) ||
         (original_views.angley  != incoming_views.angley) ||
         (original_views.anglez  != incoming_views.anglez) )
    {
        retVal = 1;
    }

    return retVal;
}

#ifdef STANDALONE
void render_renderFrame(render_state_t *pObj, System_EglObj *pEglObj, GLuint *texYuv)
#else
void render_renderFrame(render_state_t *pObj, void *pEglObj, GLuint *texYuv)
#endif
{
#if 1
        int view_change_needed = 0;
        int num_srv_views;
        uint32_t nlines = 0;

        num_srv_views = pObj->num_srv_views;

        for (nlines = 0; nlines < pObj->num_srv_views; nlines++)
        {
            if (views_change(srv_coords[nlines], pObj->srv_views[nlines]))
            {
                srv_coords[nlines] = pObj->srv_views[nlines];
                view_change_needed = 1;
            }
        }

        if (1 == view_change_needed)
        {
            render_updateView();
        }

	if(srv_render_to_file == true)
		set_coords_transition(0, srv_param_view1, srv_param_view2, srv_param_step);

#ifndef _WIN32
	if(prev_time_usecs == 0)
	{
		gettimeofday(&tv, NULL);
		prev_time_usecs = 1000000 * tv.tv_sec + tv.tv_usec;
	}
#endif

	shader_output_select = srv_param_select;
	glClear(GL_COLOR_BUFFER_BIT);
	{
		for(int i = 0; i < num_viewports; i++)
		{
			if(srv_viewports[i].animate == true)
			{
				if(animatengap[i] == false)
				{
					/* stay here for gap number of frames;
					 *
					 */
					step[i]++;
					if(step[i] >= gap)
					{
						step[i] = 0;
						animatengap[i] = true;
					}
				}
				else
				{
					unsigned int next_index=(current_index[i] + 1)%num_srv_views;
					COORD_TRANSITION(i, camx, next_index, step[i]);
					COORD_TRANSITION(i, camy, next_index, step[i]);
					COORD_TRANSITION(i, camz, next_index, step[i]);
					COORD_TRANSITION(i, targetx, next_index, step[i]);
					COORD_TRANSITION(i, targety, next_index, step[i]);
					COORD_TRANSITION(i, targetz, next_index, step[i]);
					COORD_TRANSITION(i, anglex, next_index, step[i]);
					COORD_TRANSITION(i, angley, next_index, step[i]);
					COORD_TRANSITION(i, anglez, next_index, step[i]);
					render_updateView();
					step[i]++;
					if(step[i] >= num_iterations)
					{
						step[i] = 0;
						animatengap[i] = false;
						current_index[i] = next_index;
					}
				}
			}

			glViewport(srv_viewports[i].x,
					srv_viewports[i].y,
					srv_viewports[i].width,
					srv_viewports[i].height);

			if(srv_param_bowl)
			{
				srv_draw(pObj, texYuv, i);
			}
			if(srv_param_car)
				car_draw(i);
		}
		//boxes_draw((ObjectBox *)pObj->BoxLUT, (Pose3D_f *)pObj->BoxPose3D, texYuv);
	}

#if ENABLE_SGX_RENDERED_PREVIEW
	// Draw the other panes
	{
		glViewport(0, 1080-(200+440*1),520,440);
		//screen1_draw_vbo(texYuv[0]);
	}
	{
		glViewport(0, 1080-(200+440*2),520,440);
		//screen1_draw_vbo(texYuv[1]);
	}
	{
		glViewport(520+880, 1080-(200+440*1),520,440);
		//screen1_draw_vbo(texYuv[2]);
	}
	{
		glViewport(520+880, 1080-(200+440*2),520,440);
		//screen1_draw_vbo(texYuv[3]);
	}
#endif

#ifndef _WIN32
	if(frame_count < 100)
	{
		frame_count++;
	}
	else
	{
		frame_count = 0;
		gettimeofday(&tv, NULL);
		cur_time_usecs = 1000000 * tv.tv_sec + tv.tv_usec;
		delta_usecs = cur_time_usecs - prev_time_usecs;
		fps = 100.0f/((float)delta_usecs/1000000.0f);
		prev_time_usecs = cur_time_usecs;
		RENDER_PRINT("%f fps\n", fps);
	}
#endif
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
void render_ndcToScreen(int *xscr, int *yscr, float xndc, float yndc)
{
	*xscr = (int)round(srv_viewports[0].x + (srv_viewports[0].width * (xndc + 1.0)/2));
	*yscr = (int)round(srv_viewports[0].y + (srv_viewports[0].height * (1.0 - yndc)/2));
}

void render_worldToNdc(float *xndc, float *yndc, GLint x, GLint y, GLint z)
{
	glm::vec4 vTransformedOrigin = mMVP_bowl[0] * glm::vec4(x, y, z, 1);
	*xndc = vTransformedOrigin.x/vTransformedOrigin.z;
	*yndc = vTransformedOrigin.y/vTransformedOrigin.z;
}

void render_worldToScreen(int *xscr, int *yscr, GLint x, GLint y, GLint z)
{
	float xndc, yndc;
	render_worldToNdc(&xndc, &yndc, x, y, z);
	render_ndcToScreen(xscr, yscr, xndc, yndc);
}

void render_lineIntercepts(intercepts *intcepts,
		int x1, int y1, int x2, int y2,
		int xleft, int xright, int ytop, int ybottom)
{
	float m;
	if(x2 != x1)
	{
		m = (float)(y2 - y1)/(float)(x2 - x1);
	}
	else
	{
		m = FLT_MAX;
	}

	intcepts->l.x = xleft;
	intcepts->r.x = xright;
	if(x2 != x1)
	{
		intcepts->l.y = (int)(m * (xleft - x1) + y1);
		intcepts->r.y = (int)(m * (xright - x1) + y1);
	}
	else
	{
		intcepts->l.y = -INT_MAX ;
		intcepts->r.y = INT_MAX;
	}

	intcepts->t.y = ytop;
	intcepts->b.y = ybottom;

	if (m != 0)
	{
		intcepts->t.x = (int)((ytop - y1)/m + x1);
		intcepts->b.x = (int)((ybottom - y1)/m + x1);
	}
	else
	{
		intcepts->t.x = -INT_MAX;
		intcepts->b.x = INT_MAX;
	}
}

void render_getIntercepts(intercepts *worldx, intercepts *worldy,
		int xleft, int xright, int ytop, int ybottom)
{
	int x1, y1, x2, y2;

	//X-axis intercepts
	render_worldToScreen(&x1, &y1, -500, 0, 0);
	render_worldToScreen(&x2, &y2, 500, 0, 0);

	render_lineIntercepts(worldx, x1, y1, x2, y2,
			xleft, xright, ytop, ybottom);

	//Y-axis intercepts
	render_worldToScreen(&x1, &y1, 0, 500, 0);
	render_worldToScreen(&x2, &y2, 0, -500, 0);

	render_lineIntercepts(worldy, x1, y1, x2, y2,
			xleft, xright, ytop, ybottom);

}
