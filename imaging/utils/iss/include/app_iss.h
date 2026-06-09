/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

 /*******************************************************************************
 *  Function Definition
 *******************************************************************************
 */
/**
 *******************************************************************************
 *
 * \brief   Sensor Creation
 *
 *******************************************************************************
*/
#ifndef APP_UTILS_ISS_H_
#define APP_UTILS_ISS_H_

#include <sensor_drv/include/iss_sensors.h>
#include <algos/dcc/include/dcc_defs.h>
#include <itt_server_remote/include/itt_srvr_remote.h>
#include <TI/hwa_viss_srvr_remote.h>

#define MAX_FOLDER_NAME_LEN (128)
#define MAX_FILE_NAME_LEN (512)
#define DCC_ROOT "/opt/vision_apps/dcc"
#define MAX_FOLDER_NAME_LEN (128)

/**
 * \defgroup group_vision_apps_utils_iss ISS
 *
 * \brief This section contains APIs for ISS
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

int32_t appIssInit(void);
int32_t appIssDeInit(void);

int32_t appEnumerateImageSensor(char * sensor_name_list[], uint8_t  * num_sensors_found);
int32_t appQueryImageSensor(char* sensor_name, IssSensor_CreateParams* pSensorCreatePrms);
int32_t appStartImageSensor(char* sensor_name, uint32_t channel_mask);
int32_t appStopImageSensor(char* sensor_name, uint32_t channel_mask);
int32_t appInitImageSensor(char* sensor_name, uint32_t featuresEnabled, uint32_t channel_mask);
int32_t appDeInitImageSensor(char* sensor_name);
int32_t appIssGetDCCSizeVISS(const char * sensor_name, uint32_t wdr_mode);
int32_t appIssGetDCCBuffVISS(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes);
int32_t appIssGetDCCSize2A(const char * sensor_name, uint32_t wdr_mode);
int32_t appIssGetDCCBuff2A(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes);
int32_t appControlImageSensor(char* sensor_name);
int32_t appIssGetResizeParams(uint16_t in_width, uint16_t in_height, uint16_t tgt_width, uint16_t tgt_height, uint16_t * out_width, uint16_t * out_height);
int32_t appDetectImageSensor(uint8_t *sensor_id_list, uint8_t *num_sensors_found, uint32_t channel_mask);

int32_t appIssGetDCCSizeLDC(const char * sensor_name, uint32_t wdr_mode);
int32_t appIssGetDCCBuffLDC(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes);
int32_t get_dcc_dir_size(char* dcc_folder_path);
int32_t get_dcc_dir_data(char* dcc_folder_path, uint8_t * dcc_buf);

#if defined(LINUX)
uint8_t is_viss_plugin(uint32_t plugin_id);
uint8_t is_aewb_plugin(uint32_t plugin_id);
uint8_t is_ldc_plugin(uint32_t plugin_id);

int32_t appDccUpdateNode(uint8_t * dcc_buf, int32_t num_bytes, vx_node node, uint32_t replicate_nodex_idx, vx_context context);
int32_t appDccUpdatefromFS(char* sensor_name, uint8_t wdr_mode, 
                        vx_node node_viss, uint32_t viss_node_index,
                        vx_node node_aewb, uint32_t aewb_node_index, 
                        vx_node node_ldc, uint32_t ldc_node_index,
                        vx_context context);

int32_t appSplitVpacDcc(uint8_t *dcc_buf_in, uint32_t prmSize,
                        uint8_t **dcc_buf_viss, uint32_t *viss_bytes,
                        uint8_t **dcc_buf_aewb, uint32_t *aewb_bytes,
                        uint8_t **dcc_buf_ldc,  uint32_t *ldc_bytes);

int32_t appUpdateVpacDcc(uint8_t *dcc_buf, uint32_t prmSize, vx_context context, 
                        vx_node viss_node, uint32_t viss_node_index,
                        vx_node aewb_node, uint32_t aewb_node_index, 
                        vx_node ldc_node, uint32_t ldc_node_index);


#endif /*defined(LINUX)*/

#endif /* APP_UTILS_ISS_H_ */

