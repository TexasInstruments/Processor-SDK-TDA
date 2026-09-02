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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include "srv_views.h"

using namespace std;

srv_coords_t srv_coords[] = {
//		{0.000000, 0.000000, 180.072601, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000},    //TDA3x-1 Feb 23
//		{0.000000, 88.167992, 171.995667, 0.000000, 25.990564, 33.987663, -1.499456, 0.000000, 0.000000},//TDA3x-2 Feb 23
//		{0.000000, 86.860039, 139.732086, 0.000000, 42.522865, 33.483463, -1.500000, 0.000000, 0.000000},//TDA3x-3 Feb 23
        {0.000000, 0.000000, 640.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}, //zoomed out
        {0.000000, 0.000000, 440.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}, //zoomed in
        {0.000000, 0.000000, 240.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}, //zoomed in
        {0.000000, 88.200000, 192.000000, 0.000000, 26.000000, 34.000000, -1.500000, 0.000000, 0.000000}, //Front
        {0.000000, 86.859566, 139.720367, 0.000000, 42.528866, 33.483280, -1.500000, 0.000000, 0.000000}, //Zoomed in front
        {0.000000, 86.838882, 138.913574, 0.000000, 30.598755, 33.475307, -1.500000, 0.000000, 0.000000}, //Zoomed in front
        {0.000000, 0.000000, 440.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}, //zoomed out
        {0.000000, 88.200000, 192.000000, -1.000000, 26.000000, 34.000000, -1.500000, 0.000000, -3.1416},//Back
        {-0.000000, 200.000000, 220.000000, -7.000000, 63.000000, 0.000000, -1.500000, -0.000000, -1.570169}, //Left
        {-0.000000, 200.000000, 220.000000, -7.000000, 63.000000, 0.000000, -1.500000, 0.000000, 1.570169}, //Right
        {-0.000000, -29.049999, 440.000000, -67.374092, -39.541992, 0.000000, 0.000000, -0.000000, 0.000000}, //Left blindspot
        {-0.000000, -29.049999, 440.000000, 67.374092, -39.541992, 0.000000, 0.000000, -0.000000, 0.000000}, //Right blindspot 
        {0.000000, 0.000000, 300.000000, 0.000000, 60.000000, 0.000000, -1.000000, 0.000000, 3.100000}, //zoomed out
        {0.000000, 0.000000, 380.000000, 0.000000, 60.000000, 0.000000, -1.000000, 0.000000, 3.100000}, //zoomed out
        {0.000000, 0.000000, 440.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}, //zoomed in
        {0.000000, 190.300049, 150.000000, -1.000000, 80.000000, 35.000000, -1.000000, 0.000000, 0.000000}, // FIle DUMP 0
        {0.000000, 190.300049, 150.000000, -1.000000, 80.000000, 35.000000, -1.750000, 0.000000, 0.000000}, // File DUMP 1
        {0.000000, 200.000000, 240.000000, -1.000000, 80.000000, 35.000000, -1.500000, 0.000000, 0.000000}, //Front
        {-15.149982, 149.549988, 257.200226, 3.312001, 38.720036, 35.000000, -1.484400, 0.008200, -3.228336},//Back
        {-0.000000, 197.199997, 240.000000, -1.000000, 66.135956, 35.000000, -1.521800, -0.000000, -1.565569}, //Left
        {-0.000000, 200.000000, 240.000000, -1.000000, 68.776001, 35.000000, -1.500000, 0.000000, 1.570169}, //Right
        {-0.000000, -29.049999, 440.000000, -67.374092, -39.541992, 0.000000, 0.000000, -0.000000, 0.000000}, //Left blindspot
        {-0.000000, -29.049999, 440.000000, 67.374092, -39.541992, 0.000000, 0.000000, -0.000000, 0.000000}, //Right blindspot
        {-15.149982, 149.549988, 257.200226, 3.312001, 38.720036, 35.000000, -1.484400, 0.008200, -3.228336},//Back
        {-0.000000, 197.199997, 240.000000, -1.000000, 66.135956, 35.000000, -1.521800, -0.000000, -1.565569}, //Left
        {-0.000000, 200.000000, 240.000000, -1.000000, 68.776001, 35.000000, -1.500000, 0.000000, 1.570169}, //Right
        {0.000000, 230.300049, 240.000000, -1.000000, 80.000000, 35.000000, -1.500000, 0.000000, 0.000000}, //
        {0.000000, 250.300049, 240.000000, -1.000000, 80.000000, 35.000000, -1.500000, 0.000000, 0.000000}
};

int num_srv_views;

void srv_views_init(render_state_t *pObj)
{
        ifstream file("srv_views.txt");
        uint32_t nlines = 0;
        string line;

        if (pObj->num_srv_views > 0)
        {
	    num_srv_views = pObj->num_srv_views;

            for (nlines = 0; nlines < pObj->num_srv_views; nlines++)
            {
                srv_coords[nlines] = pObj->srv_views[nlines];
            }
        }
        else
        {
            num_srv_views = (int)(sizeof(srv_coords)/sizeof(srv_coords_t));

            if(!file.is_open())
            {
                printf("3DSRV: Cannot open srv_views.txt. Using default views\n");
                return;
            }

            while (!file.eof() && (nlines < MAX_SRV_VIEWS))
            {
                getline(file, line);
                sscanf(line.c_str(), "%f, %f, %f, %f, %f, %f, %f, %f, %f",
                                     &(srv_coords[nlines].camx),
                                     &(srv_coords[nlines].camy),
                                     &(srv_coords[nlines].camz),
                                     &(srv_coords[nlines].anglex),
                                     &(srv_coords[nlines].angley),
                                     &(srv_coords[nlines].anglez),
                                     &(srv_coords[nlines].targetx),
                                     &(srv_coords[nlines].targety),
                                     &(srv_coords[nlines].targetz));
                nlines++;
            }
            num_srv_views = nlines;
            file.close();
        }
}
