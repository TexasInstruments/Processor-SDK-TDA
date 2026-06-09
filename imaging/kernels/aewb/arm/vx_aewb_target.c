/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include <math.h>

#include "TI/tivx.h"
#include "TI/hwa_vpac_viss.h"
#include "TI/j7_imaging_aewb.h"
#include "tivx_imaging_kernels_priv.h"
#include "tivx_kernel_aewb.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI_aaa_awb.h"
#include "TI_aaa_ae.h"
#include "AWB_config_hardware.h"
#include "idcc.h"
#include <app_ipc.h>
#include <app_remote_service.h>

#include <iss_sensors.h>
#include <itt_srvr_remote.h>
#include <TI/hwa_viss_srvr_remote.h>
#if defined(FREERTOS)
extern void vPortTaskUsesFPU(void);
#endif
static vx_status AWB_TI_create(awbprm_t *p_awb_params, awb_calc_data_t* p_calib);
static vx_status AWB_TI_parse_H3a_buf(uint8_t * pH3a_out, awbprm_t *p_awb_params,
        h3a_aewb_paxel_data_t * awb_h3a_res, h3a_aewb_paxel_data_t * p_h3a_merge);
static vx_status AWB_TI_process(
        h3a_aewb_paxel_data_t * awb_h3a_res,
        awbprm_t              * p_awb_params,
        tivx_ae_awb_params_t  * aewb_prev,
        tivx_ae_awb_params_t  * aewb_result,
        uint8_t               * scratch_mem,
        uint8_t                 sensor_pre_gain);

static void map_sensor_ae_dynparams(IssAeDynamicParams * p_ae_dynPrms, tiae_exp_prog_t * p_ae_exp_prg);
#ifndef x86_64
static int32_t sendAewbToViss(uint32_t cpu_id, uint32_t channel_id, tivx_ae_awb_params_t *pAewbPrms);
#endif

typedef struct
{
    vx_uint16 dcc_id;
    dcc_parser_input_params_t * dcc_input_params;
    dcc_parser_output_params_t * dcc_output_params;
    awbprm_t * p_awb_params;
    tiae_prm_t * p_ae_params;
    h3a_aewb_paxel_data_t      * p_h3a_merge;
    uint32_t      frame_count;
    int32_t       sensor_pre_wb_gain;
    tivx_ae_awb_params_t ae_awb_result_prev;
    h3a_aewb_paxel_data_t * awb_h3a_res;
    uint8_t * awb_scratch_memory;
} tivxAEWBParams;

static tivx_target_kernel vx_aewb_target_kernel = NULL;

static vx_status VX_CALLBACK tivxAewbProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxAewbCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxAewbDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxAewbControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params,
       void *priv_arg);

#ifndef x86_64

extern AlgItt_IssAewb2AControlParams aewbCtrlPrms[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
extern tivx_mutex                 itt_aewb_lock[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
#endif //x86_64

static vx_status AE_TI_process(h3a_aewb_paxel_data_t *awb_h3a_res, int32_t h3a_data_x, int32_t h3a_data_y,
    tiae_prm_t *h, tivx_ae_awb_params_t *aewb_prev, tivx_ae_awb_params_t *aewb_result)
    {
        vx_status                  status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(!awb_h3a_res)
    {
        VX_PRINT(VX_ZONE_ERROR, "awb_h3a_res is NULL");
        status = (vx_status)VX_FAILURE;
    }
    if(!(aewb_prev))
    {
        VX_PRINT(VX_ZONE_ERROR, "aewb_prev is NULL");
        status = (vx_status)VX_FAILURE;
    }
    if(!aewb_result)
    {
        VX_PRINT(VX_ZONE_ERROR, "aewb_result is NULL");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    if ((vx_status)VX_SUCCESS == status)
    {
        //tiae_exp_t exp1 = h->prev_ae;
        tiae_exp_t ae_exposure;
        aewb_result->ae_converged = (uint32_t)TI_AE_do(h, awb_h3a_res, h3a_data_x, h3a_data_y, NULL, 256, 256, 256, &ae_exposure);

        //float adj = (float)exp2.aperture_size / exp1.aperture_size
        //            * exp2.exposure_time  / exp1.exposure_time
        //            * exp2.analog_gain    / exp1.analog_gain
        //            * exp2.digital_gain   / exp1.digital_gain;
        //printf(">>> AE debug kernel: adj = %4f, locked = %d, lock_cnt = %d\n", adj, h->locked, h->lock_cnt);
        //printf(">>> AE debug kernel: T1 = %d, T2 = %d\n", exp1.exposure_time, exp2.exposure_time);

        aewb_result->analog_gain   = (uint32_t)ae_exposure.analog_gain;
        aewb_result->digital_gain  = (uint32_t)ae_exposure.digital_gain;
        aewb_result->exposure_time = (uint32_t)ae_exposure.exposure_time;
        aewb_result->ae_valid = 1U;
    }

    return status;
}

// Gang TODO: this only works with j7 h3a c model; MUST change to work with j7 H/W
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static uint8_t * decode_h3a_header.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
static uint8_t * decode_h3a_header(uint8_t * h3a_buf, uint16_t *n_row, uint16_t *n_col, uint16_t * n_pix)
{
    // the header is added by H3A c model
    uint16_t * header = (uint16_t*) h3a_buf;
    *n_row = header[2];
    *n_col = header[3];
    uint16_t winh = ((header[1] + header[5]) - (uint16_t)1) / header[5];
    uint16_t winv = ((header[0] + header[4]) - (uint16_t)1) / header[4];
    *n_pix = winh * winv;
    VX_PRINT(VX_ZONE_INFO, "H3A config: winH=%d, winW=%d, winVC=%d, winHC=%d, incV=%d, incH=%d\n",
            header[0], header[1], header[2], header[3], header[4], header[5]);

    return &h3a_buf[12];
}

static uint8_t * decode_h3a_header_dcc(uint8_t * h3a_buf, iss_ipipe_h3a_aewb * h3a_dcc_cfg,
                                                            uint16_t *n_row, uint16_t *n_col, uint16_t * n_pix)
{
    uint16_t winh;
    uint16_t winv;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(0U == h3a_dcc_cfg->h_skip)
    {
        VX_PRINT(VX_ZONE_WARNING, "Warning : h_skip = 0. Setting to 1 to avoid divide by zero error \n");
        h3a_dcc_cfg->h_skip = 1;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(0U == h3a_dcc_cfg->v_skip)
    {
        VX_PRINT(VX_ZONE_WARNING, "Warning : v_skip = 0. Setting to 1 to avoid divide by zero error \n");
        h3a_dcc_cfg->v_skip = 1;
    }
    /* LDRA_JUSTIFY_END */

    winh = (uint16_t)(((uint16_t)h3a_dcc_cfg->h_size + (uint16_t)h3a_dcc_cfg->h_skip) - (uint16_t)1)/(uint16_t)h3a_dcc_cfg->h_skip;
    winv = (uint16_t)(((uint16_t)h3a_dcc_cfg->v_size + (uint16_t)h3a_dcc_cfg->v_skip) - (uint16_t)1)/(uint16_t)h3a_dcc_cfg->v_skip;

    *n_row = h3a_dcc_cfg->v_count;
    *n_col = h3a_dcc_cfg->h_count;
    *n_pix = winh * winv;
    VX_PRINT(VX_ZONE_INFO, "decode_h3a_header_dcc : num_rows = %d\n", *n_row);
    VX_PRINT(VX_ZONE_INFO, "decode_h3a_header_dcc : num_cols = %d\n", *n_col);
    VX_PRINT(VX_ZONE_INFO, "decode_h3a_header_dcc : num_pix = %d\n", *n_pix);

    return h3a_buf;
}

#ifndef x86_64

/*
    2A node does not know sensor name, I2C address or any other parameters
    It recieves channel ID from OpenVX graph and computes exp, gain params
    The results are send to sensor driver f/w along with channel ID
    It is the responsibility of sensor driver f/w to apply these results to
    the correct sensor

    This needs to generic even if 2A node and sensor driver are executing on
    different cores and Operating Systems

    2A node will send a Control Command to sensor driver f/w. Payload format is

    ----ISS_SENSORS_MAX_NAME bytes sensor name prefilled to "dummy". Ignored by sensor driver
    ----32-bit channel ID
    ----32-bit Command ID = SETEXP_PARAMS
    ----aePrms
*/

static int32_t sendExpGainToSensor(tivx_aewb_config_t * aewb_config, IssSensor_ExposureParams *pAePrms)
{
    int32_t status = -1;
    uint8_t  cmdPrm[CMD_PARAM_SIZE];
    uint8_t * ptr8 = (uint8_t * )cmdPrm;
    uint32_t chId = (uint32_t)aewb_config->channel_id;
    char dummy_name[ISS_SENSORS_MAX_NAME] = "dummy";
    IMAGE_SENSOR_CTRLCMD ctrlCmd = (IMAGE_SENSOR_CTRLCMD)IMAGE_SENSOR_CTRLCMD_SETEXPGAIN;

    memcpy((void *)ptr8, (const void *)dummy_name, ISS_SENSORS_MAX_NAME);
    ptr8 += ISS_SENSORS_MAX_NAME;

    memcpy((void *)ptr8, (const void *)&chId, sizeof(uint32_t));
    ptr8 += sizeof(uint32_t);

    memcpy((void *)ptr8, (const void *)&ctrlCmd, sizeof(IMAGE_SENSOR_CTRLCMD));
    ptr8 += sizeof(IMAGE_SENSOR_CTRLCMD);

    memcpy((void *)ptr8, (const void *)pAePrms, sizeof(IssSensor_ExposureParams));
    VX_PRINT(VX_ZONE_INFO, "Sending command to sensor \n");

    #if (defined(SOC_AM62A))
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU1_0;
    #else
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU2_0;
    #endif

    status = appRemoteServiceRun(
        remote_cpu_id,
        IMAGE_SENSOR_REMOTE_SERVICE_NAME,
        (uint32_t)IM_SENSOR_CMD_CTL,
        (void*)cmdPrm,
        CMD_PARAM_SIZE,
        APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK
    );
    return status;
}

/*
    2A node will send a Control Command to sensor driver f/w. Payload format is

    ----ISS_SENSORS_MAX_NAME bytes sensor name prefilled to "dummy". Ignored by sensor driver
    ----32-bit channel ID
    ----32-bit Command ID = GETEXPPRG
    ----Sensor driver will copy IssAeDynamicParams in the command buffer at the location after command ID

    Must be careful that worst case size of IssAeDynamicParams does not exceed the available bytes in command buffer
*/
static int32_t getExpPrgFromSensor(uint32_t channel_id, IssAeDynamicParams * p_AEdynPrms)
{
    int32_t status = -1;
    uint8_t  cmdPrm[CMD_PARAM_SIZE];
    uint8_t * ptr8 = (uint8_t * )cmdPrm;
    char dummy_name[ISS_SENSORS_MAX_NAME] = "dummy";
    IMAGE_SENSOR_CTRLCMD ctrlCmd = (IMAGE_SENSOR_CTRLCMD)IMAGE_SENSOR_CTRLCMD_GETEXPPRG;

    memcpy((void *)ptr8, (const void *)dummy_name, ISS_SENSORS_MAX_NAME);
    ptr8 += ISS_SENSORS_MAX_NAME;

    memcpy((void *)ptr8, (const void *)&channel_id, sizeof(uint32_t));
    ptr8 += sizeof(uint32_t);

    memcpy((void *)ptr8, (const void *)&ctrlCmd, sizeof(IMAGE_SENSOR_CTRLCMD));
    ptr8 += sizeof(IMAGE_SENSOR_CTRLCMD);

    VX_PRINT(VX_ZONE_INFO, "Sending control command to sensor \n");

    #if (defined(SOC_AM62A))
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU1_0;
    #else
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU2_0;
    #endif

    status = appRemoteServiceRun(
        remote_cpu_id,
        IMAGE_SENSOR_REMOTE_SERVICE_NAME,
        (uint32_t)IM_SENSOR_CMD_CTL,
        (void*)cmdPrm,
        CMD_PARAM_SIZE,
        0
    );
    memcpy((void *)p_AEdynPrms, (const void *)ptr8, sizeof(IssAeDynamicParams));
    return status;
}

/*
    2A node will send a Control Command to sensor driver f/w. Payload format is

    ----ISS_SENSORS_MAX_NAME bytes sensor name prefilled to "dummy". Ignored by sensor driver
    ----32-bit channel ID
    ----32-bit Command ID = GETWBPRG
    ----Sensor driver will copy IssAwbDynamicParams in the command buffer at the location after command ID

*/
static int32_t getWbPrgFromSensor(uint32_t channel_id, IssAwbDynamicParams * p_AWBdynPrms)
{
    int32_t status = -1;
    uint8_t  cmdPrm[CMD_PARAM_SIZE];
    uint8_t * ptr8 = (uint8_t * )cmdPrm;
    char dummy_name[ISS_SENSORS_MAX_NAME] = "dummy";
    IMAGE_SENSOR_CTRLCMD ctrlCmd = (IMAGE_SENSOR_CTRLCMD)IMAGE_SENSOR_CTRLCMD_GETWBCFG;

    memcpy((void *)ptr8, (const void *)dummy_name, ISS_SENSORS_MAX_NAME);
    ptr8 += ISS_SENSORS_MAX_NAME;

    memcpy((void *)ptr8, (const void *)&channel_id, sizeof(uint32_t));
    ptr8 += sizeof(uint32_t);

    memcpy((void *)ptr8, (const void *)&ctrlCmd, sizeof(IMAGE_SENSOR_CTRLCMD));
    ptr8 += sizeof(IMAGE_SENSOR_CTRLCMD);

    #if (defined(SOC_AM62A))
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU1_0;
    #else
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU2_0;
    #endif

    status = appRemoteServiceRun(
        remote_cpu_id,
        IMAGE_SENSOR_REMOTE_SERVICE_NAME,
        (uint32_t)IM_SENSOR_CMD_CTL,
        (void*)cmdPrm,
        CMD_PARAM_SIZE,
        0
    );
    memcpy((void *)p_AWBdynPrms, (const void *)ptr8, sizeof(IssAwbDynamicParams));
    return status;
}

/*
    2A node does not know sensor name, I2C address or any other parameters
    It recieves channel ID from OpenVX graph and computes exp, gain params
    The results are send to sensor driver f/w along with channel ID
    It is the responsibility of sensor driver f/w to apply these results to
    the correct sensor

    This needs to generic even if 2A node and sensor driver are executing on
    different cores and Operating Systems

    2A node will send a Control Command to sensor driver f/w. Payload format is

    ----ISS_SENSORS_MAX_NAME bytes sensor name prefilled to "dummy". Ignored by sensor driver
    ----32-bit channel ID
    ----32-bit Command ID = IMAGE_SENSOR_CTRLCMD_SETWBGAIN
    ----aePrms
*/
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static int32_t sendWbGainToSensor.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
static int32_t sendWbGainToSensor(tivx_aewb_config_t * aewb_config, IssSensor_WhiteBalanceParams *pAwbPrms)
{
    int32_t status = -1;
    uint8_t  cmdPrm[CMD_PARAM_SIZE];
    uint8_t * ptr8 = (uint8_t * )cmdPrm;
    uint32_t chId = (uint32_t)aewb_config->channel_id;
    char dummy_name[ISS_SENSORS_MAX_NAME] = "dummy";
    IMAGE_SENSOR_CTRLCMD ctrlCmd = (IMAGE_SENSOR_CTRLCMD)IMAGE_SENSOR_CTRLCMD_SETWBGAIN;

    memcpy((void *)ptr8, (const void *)dummy_name, ISS_SENSORS_MAX_NAME);
    ptr8 += ISS_SENSORS_MAX_NAME;

    memcpy((void *)ptr8, (const void *)&chId, sizeof(uint32_t));
    ptr8 += sizeof(uint32_t);

    memcpy((void *)ptr8, (const void *)&ctrlCmd, sizeof(IMAGE_SENSOR_CTRLCMD));
    ptr8 += sizeof(IMAGE_SENSOR_CTRLCMD);

    memcpy((void *)ptr8, (const void *)pAwbPrms, sizeof(IssSensor_WhiteBalanceParams));

    #if (defined(SOC_AM62A))
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU1_0;
    #else
        uint32_t remote_cpu_id = APP_IPC_CPU_MCU2_0;
    #endif

    status = appRemoteServiceRun(
        remote_cpu_id,
        IMAGE_SENSOR_REMOTE_SERVICE_NAME,
        (uint32_t)IM_SENSOR_CMD_CTL,
        (void*)cmdPrm,
        CMD_PARAM_SIZE,
        APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK
    );
    return status;
}

/*

    2A node will send a Control Command to VISS node. Payload format is

    ----32-bit channel ID
    ----32-bit Command ID = VISS_CMD_SET_2A_PARAMS
    ----awbPrms
*/
static uint8_t  vissCmdPrm[VISS_CMD_PARAM_SIZE];
static int32_t sendAewbToViss(uint32_t cpu_id, uint32_t channel_id, tivx_ae_awb_params_t *pAewbPrms)
{
    int32_t status = -1;
    uint8_t * ptr8 = (uint8_t * )vissCmdPrm;
    uint32_t chId = channel_id;

    memcpy((void *)ptr8, (const void *)&chId, sizeof(uint32_t));
    ptr8 += sizeof(uint32_t);

    memcpy((void *)ptr8, (const void *)pAewbPrms, sizeof(tivx_ae_awb_params_t));

    status = appRemoteServiceRun(
        cpu_id,
        VISS_SERVER_REMOTE_SERVICE_NAME,
        (uint32_t)VISS_CMD_SET_2A_PARAMS,
        (void*)vissCmdPrm,
        VISS_CMD_PARAM_SIZE,
        APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK
    );

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(0!=status)
    {
        VX_PRINT(VX_ZONE_ERROR, "VISS_CMD_SET_2A_PARAMS returned 0x%x \n", status);
    }
    /* LDRA_JUSTIFY_END */
    return status;
}
#else //x86_64

extern tivx_ae_awb_params_t g_ae_awb_result;

IssAeDynamicParams g_ae_dynPrms =
{
    .targetBrightnessRange = {
        .min = 30,
        .max = 45
    },
    .targetBrightness = 35,
    .threshold = 1,
    .exposureTimeStepSize = 1,
    .enableBlc = 0,
    .numAeDynParams = 0,
    .exposureTimeRange = {
        {
            .min = 100,
            .max = 40000
        },
        {
            .min = 40000,
            .max = 40000
        }
    },
    .analogGainRange = {
        {
            .min = 1024,
            .max = 1024
        },
        {
            .min = 1024,
            .max = 15872
        }
    },
    .digitalGainRange = {
        {
            .min = 256,
            .max = 256
        },
        {
            .min = 256,
            .max = 256
        }
    }
};
#endif //x86_64

static vx_status VX_CALLBACK tivxAewbProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    int32_t to_ret = 0;
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_user_data_object_t *h3a_aew_af_desc;
    tivx_obj_desc_user_data_object_t *ae_awb_result_desc;
    tivxAEWBParams *nodePrms = NULL;
#ifndef x86_64
    vx_bool_e test_mode = vx_false_e;
    tivx_h3a_data_t *pHh3aData;
    uint32_t viss_cpu_id = 0;
    uint32_t viss_channel_id = 0;
#endif

    uint32_t node_param_size;
    uint8_t skipAWB = 0;
    uint8_t skipAE = 0;

#ifndef x86_64
    uint8_t rIndex = 0xFF;
    uint8_t grIndex = 0xFF;
    uint8_t gbIndex = 0xFF;
    uint8_t bIndex  = 0xFF;
#endif
    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &node_param_size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms = (tivxAEWBParams *)temp_kernel_context;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (nodePrms == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "nodePrms is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    #if defined (FREERTOS)
    vPortTaskUsesFPU();
    #endif
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */

    if ((vx_status)VX_SUCCESS == status)
    {
        if(NULL == nodePrms->p_awb_params)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxAewbProcess : Failed to get kernel instance \n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* All ok */
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxAewbProcess : Failed to get kernel instance \n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ( (num_params != TIVX_KERNEL_AEWB_MAX_PARAMS)
            || (NULL == obj_desc[TIVX_KERNEL_AEWB_CONFIGURATION_IDX])
            || (NULL == obj_desc[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX])
            || (NULL == obj_desc[TIVX_KERNEL_AEWB_AE_AWB_RESULT_IDX])
        )
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "tivxAewbProcess : Incorrect params : num_params =%d\n", num_params);
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_AEWB_CONFIGURATION_IDX];
        histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_AEWB_HISTOGRAM_IDX];
        h3a_aew_af_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX];
        ae_awb_result_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_AEWB_AE_AWB_RESULT_IDX];

        /*
           In RAW frame inject mode, VISS node indicates that H3A data is invalid.
           In this scenario, image sensor is not connected. Therefore no physical
           communication should be attempted with the image sensor
        */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((uint32_t)tivxFlagIsBitSet(obj_desc[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID) != (uint32_t)0U)
        {
            VX_PRINT(VX_ZONE_INFO, "tivxAewbProcess : H3A output maybe invalid.\n");
#ifndef x86_64
            test_mode = vx_true_e;
#endif
        }
        /* LDRA_JUSTIFY_END */
            void *configuration_target_ptr;
            void *histogram_target_ptr;
            tivx_h3a_data_t *h3a_aew_af_target_ptr;
            tivx_ae_awb_params_t *ae_awb_result_prev_target_ptr;
            tivx_ae_awb_params_t *ae_awb_result_target_ptr;
            tivx_aewb_config_t * aewb_config;

            configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
               configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
               (vx_enum)VX_READ_ONLY));

            aewb_config = (tivx_aewb_config_t *)configuration_target_ptr;
            ae_awb_result_prev_target_ptr = &(nodePrms->ae_awb_result_prev);

#ifndef x86_64
            vx_status lock_status;
            rIndex  = (uint8_t)nodePrms->p_awb_params->sen_awb_calc_data->red_index;
            grIndex = (uint8_t)nodePrms->p_awb_params->sen_awb_calc_data->green1_index;
            gbIndex = (uint8_t)nodePrms->p_awb_params->sen_awb_calc_data->green2_index;
            bIndex  = (uint8_t)nodePrms->p_awb_params->sen_awb_calc_data->blue_index;

            lock_status = tivxMutexLock(itt_aewb_lock[aewb_config->channel_id]);
            tivxCheckStatus(&status, lock_status);
            nodePrms->p_awb_params->mode = (uint16_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.awbMode;
            nodePrms->p_ae_params->mode = (uint16_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.aeMode;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
            <justification end> */
            if ((vx_bool)vx_true_e == (vx_bool)test_mode)
            {
                nodePrms->p_ae_params->mode = (uint16_t)ALGORITHMS_ISS_AE_DISABLED;
                nodePrms->sensor_pre_wb_gain = 0;
            }
            /* LDRA_JUSTIFY_END */

            if(((int32_t)(-1)==nodePrms->sensor_pre_wb_gain) && ((int32_t)ALGORITHMS_ISS_AWB_AUTO == (int32_t)nodePrms->p_awb_params->mode))
            {
                IssAwbDynamicParams awb_dynPrms;
                status = getWbPrgFromSensor(aewb_config->channel_id, &awb_dynPrms);
                nodePrms->sensor_pre_wb_gain = 0;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)0 == status)
                /* LDRA_JUSTIFY_END */
                {
                    nodePrms->sensor_pre_wb_gain = (int32_t)awb_dynPrms.sensor_pre_gain;
                }
            }
#else
            nodePrms->p_awb_params->mode = aewb_config->awb_mode;
            nodePrms->p_ae_params->mode = aewb_config->ae_mode;
#endif

            ae_awb_result_target_ptr = tivxMemShared2TargetPtr(&ae_awb_result_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(ae_awb_result_target_ptr,
               ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
               (vx_enum)VX_WRITE_ONLY));

            if((int32_t)ALGORITHMS_ISS_AWB_AUTO == (int32_t)nodePrms->p_awb_params->mode)
            {
                if(0U != (((uint32_t)nodePrms->frame_count + (uint32_t)aewb_config->channel_id) % ((uint32_t)aewb_config->awb_num_skip_frames+1U)))
                {
                    skipAWB = 1;
                    ae_awb_result_target_ptr->awb_valid = 0;

                    ae_awb_result_target_ptr->wb_gains[0]       = ae_awb_result_prev_target_ptr->wb_gains[0];
                    ae_awb_result_target_ptr->wb_gains[1]       = ae_awb_result_prev_target_ptr->wb_gains[1];
                    ae_awb_result_target_ptr->wb_gains[2]       = ae_awb_result_prev_target_ptr->wb_gains[2];
                    ae_awb_result_target_ptr->wb_gains[3]       = ae_awb_result_prev_target_ptr->wb_gains[3];
                    ae_awb_result_target_ptr->wb_offsets[0]     = ae_awb_result_prev_target_ptr->wb_offsets[0];
                    ae_awb_result_target_ptr->wb_offsets[1]     = ae_awb_result_prev_target_ptr->wb_offsets[1];
                    ae_awb_result_target_ptr->wb_offsets[2]     = ae_awb_result_prev_target_ptr->wb_offsets[2];
                    ae_awb_result_target_ptr->wb_offsets[3]     = ae_awb_result_prev_target_ptr->wb_offsets[3];
                    ae_awb_result_target_ptr->color_temperature = ae_awb_result_prev_target_ptr->color_temperature;
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
            <justification end> */
            else if((int32_t)ALGORITHMS_ISS_AWB_MANUAL == (int32_t)nodePrms->p_awb_params->mode)
            {
#ifndef x86_64
                ae_awb_result_target_ptr->wb_gains[rIndex] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.rGain;
                ae_awb_result_target_ptr->wb_gains[grIndex] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.gGain;
                ae_awb_result_target_ptr->wb_gains[gbIndex] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.gGain;
                ae_awb_result_target_ptr->wb_gains[bIndex] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.bGain;
                ae_awb_result_target_ptr->color_temperature = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.colorTemp;
                ae_awb_result_target_ptr->awb_valid = 1;
#endif
            }
            else
            {
                /* No action required for other AWB modes */
            }
            /* LDRA_JUSTIFY_END */

            if((int32_t)ALGORITHMS_ISS_AE_AUTO == (int32_t)nodePrms->p_ae_params->mode)
            {
                if(0U != (((uint32_t)nodePrms->frame_count + (uint32_t)aewb_config->channel_id) % ((uint32_t)aewb_config->ae_num_skip_frames+1U)))
                {
                    skipAE = 1;
                    ae_awb_result_target_ptr->ae_valid = 0;
                    ae_awb_result_target_ptr->analog_gain       = ae_awb_result_prev_target_ptr->analog_gain;
                    ae_awb_result_target_ptr->digital_gain      = ae_awb_result_prev_target_ptr->digital_gain;
                    ae_awb_result_target_ptr->exposure_time     = ae_awb_result_prev_target_ptr->exposure_time;
                    ae_awb_result_target_ptr->ae_converged      = ae_awb_result_prev_target_ptr->ae_converged;
                }
            }
            else if((int32_t)ALGORITHMS_ISS_AE_MANUAL == (int32_t)nodePrms->p_ae_params->mode)
            {
#ifndef x86_64
                int32_t max_n = nodePrms->p_ae_params->exposure_program.num_ranges - 1;
                int32_t max_analog_gain = nodePrms->p_ae_params->exposure_program.analog_gain_range[max_n].max;
                int32_t min_analog_gain = nodePrms->p_ae_params->exposure_program.analog_gain_range[0].min;
                int32_t max_exp_time = nodePrms->p_ae_params->exposure_program.exposure_time_range[max_n].max;
                int32_t min_exp_time = nodePrms->p_ae_params->exposure_program.exposure_time_range[0].min;
                int32_t max_digital_gain = nodePrms->p_ae_params->exposure_program.digital_gain_range[max_n].max;
                int32_t min_digital_gain = nodePrms->p_ae_params->exposure_program.digital_gain_range[0].min;

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
                <justification end> */
                if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain > max_analog_gain)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain = (uint32_t)max_analog_gain;
                }else if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain < min_analog_gain)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain = (uint32_t)min_analog_gain;
                }
                else
                {
                    /* Analog gain is within valid range */
                }

                if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime > max_exp_time)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime = (uint32_t)max_exp_time;
                }else if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime < min_exp_time)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime = (uint32_t)min_exp_time;
                }
                else
                {
                    /* Exposure time is within valid range */
                }

                if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain > max_digital_gain)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain = (uint32_t)max_digital_gain;
                }else if((int32_t)aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain < min_digital_gain)
                {
                    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain = (uint32_t)min_digital_gain;
                }
                else
                {
                    /* Digital gain is within valid range */
                }
                /* LDRA_JUSTIFY_END */

                ae_awb_result_target_ptr->analog_gain   = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain;
                ae_awb_result_target_ptr->exposure_time = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime;
                ae_awb_result_target_ptr->digital_gain = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain;
#endif
            }
            else
            {
                /* No action required for other AE modes */
            }

            nodePrms->frame_count++;

            if((1U==skipAE) && (1U==skipAWB))
            {
                /*AE and AWB are both skipped. No need for H3A parsing or any other processing */
                tivxCheckStatus(&status, tivxMemBufferUnmap(ae_awb_result_target_ptr,
                   ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));

#ifndef x86_64
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if((vx_status)VX_SUCCESS == lock_status)
                /* LDRA_JUSTIFY_END */
                {
                    (void)tivxMutexUnlock(itt_aewb_lock[aewb_config->channel_id]);
                }
#endif
                VX_PRINT(VX_ZONE_INFO, "AE and AWB are skipped. tivxAewbProcess returning SUCCESS\n");
            }
            else
            {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if( histogram_desc != NULL)
            /* LDRA_JUSTIFY_END */
            {
                histogram_target_ptr = tivxMemShared2TargetPtr(&histogram_desc->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(histogram_target_ptr,
                   histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                   (vx_enum)VX_READ_ONLY));
            }

            /* kernel processing function start */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
            An else block can be added to catch and print the error status in a future release.
            <justification end> */
            if( h3a_aew_af_desc != NULL)
            /* LDRA_JUSTIFY_END */
            {
                h3a_aew_af_target_ptr = tivxMemShared2TargetPtr(&h3a_aew_af_desc->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(h3a_aew_af_target_ptr, h3a_aew_af_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

#ifndef x86_64
                pHh3aData = (tivx_h3a_data_t*)h3a_aew_af_target_ptr;
                viss_cpu_id = pHh3aData->cpu_id;
                viss_channel_id = pHh3aData->channel_id;
#endif
                 /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == nodePrms->awb_h3a_res)
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxAewbProcess: Failed to allocate memory for awb_h3a_res\n");
#ifndef x86_64
                    if((vx_status)VX_SUCCESS == lock_status)
                    {
                        (void)tivxMutexUnlock(itt_aewb_lock[aewb_config->channel_id]);
                    }
#endif
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
                else
                {
                    status = (vx_status)VX_SUCCESS;
                }
                /* LDRA_JUSTIFY_END */

    /*
    H3A config should be read from DCC
    Header sent by VISS node is read but AWB params are overwritten by DCC values
    */
                uint8_t *h3a_buf = NULL;
                 /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
                <justification end> */
                if((vx_status)VX_SUCCESS == status)
                {
                if(NULL == nodePrms->dcc_output_params)
                {
                    h3a_buf = decode_h3a_header((uint8_t *)h3a_aew_af_target_ptr->data,
                                             &nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_y,
                                             &nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_x,
                                             &nodePrms->p_awb_params->ti_awb_data_in.frame_data.pix_in_pax);
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    h3a_buf = decode_h3a_header_dcc((uint8_t *)h3a_aew_af_target_ptr->data,
                                            &nodePrms->dcc_output_params->ipipeH3A_AEWBCfg,
                                            &nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_y,
                                            &nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_x,
                                            &nodePrms->p_awb_params->ti_awb_data_in.frame_data.pix_in_pax);
                }
                

                tivxCheckStatus(&status, AWB_TI_parse_H3a_buf(h3a_buf, nodePrms->p_awb_params, nodePrms->awb_h3a_res, nodePrms->p_h3a_merge));

                ae_awb_result_target_ptr->h3a_source_data = 0;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((vx_status)VX_SUCCESS == status) && (1U==skipAE))
                /* LDRA_JUSTIFY_END */
                {
                    ae_awb_result_target_ptr->ae_valid = 0;
                }
                else
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(((vx_status)VX_SUCCESS == status) && ((int32_t)ALGORITHMS_ISS_AE_AUTO == (int32_t)nodePrms->p_ae_params->mode))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start> 
                        Rationale: The test framework and test apps cannot reach this portion.
                        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                        Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
                        <justification end> */
                        if(0U == (uint32_t)nodePrms->p_ae_params->exposure_program.num_ranges)
                        /* LDRA_JUSTIFY_END */
                        {
                            /*AE Exposure program not initialized. get it from sensor driver and re-initialize AE*/
                            IssAeDynamicParams ae_dynPrms;
                            tiae_exp_prog_t ae_exp_prg;
#ifndef x86_64
                            status = getExpPrgFromSensor(aewb_config->channel_id, &ae_dynPrms);
#else
                            memcpy(&ae_dynPrms, &g_ae_dynPrms, sizeof(IssAeDynamicParams));
                            status = (vx_status)VX_SUCCESS;
#endif
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start> 
                            Rationale: The test framework and test apps cannot reach this portion.
                            The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if((vx_status)VX_SUCCESS == status)
                            {
                                map_sensor_ae_dynparams(&ae_dynPrms, &ae_exp_prg);
                                TI_AE_init(nodePrms->p_ae_params, &ae_exp_prg);
                            }
                            else
                            /* LDRA_JUSTIFY_END */
                            {
                                ae_awb_result_target_ptr->ae_valid = 0;
                                nodePrms->p_ae_params->exposure_program.num_ranges = 0;
#ifndef x86_64
                                /* LDRA_JUSTIFY_START
                                <metric start> branch <metric end>
                                <justification start>
                                Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
                                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                                However, due to the stated rationale, this is not tested.
                                <justification end> */
                                if((vx_status)VX_SUCCESS == lock_status)
                                /* LDRA_JUSTIFY_END */
                                {
                                    (void)tivxMutexUnlock(itt_aewb_lock[aewb_config->channel_id]);
                                }
#endif
                                /* Set status to VX_SUCCESS and skip remaining AE processing */
                                status = (vx_status)VX_SUCCESS;
                                to_ret = 1;
                            }
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The test framework and test apps cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        /* Only process AE if previous operations succeeded and AE is valid */
                        if ( 0 == to_ret )
                        {
                            status = AE_TI_process(nodePrms->awb_h3a_res,
                                                (int32_t)nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_x,
                                                (int32_t)nodePrms->p_awb_params->ti_awb_data_in.frame_data.h3a_data_y,
                                                nodePrms->p_ae_params,
                                                ae_awb_result_prev_target_ptr,
                                                ae_awb_result_target_ptr);
                            if((vx_status)VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "AE_TI_process returned error 0x%x \n", status);
                                ae_awb_result_target_ptr->ae_valid = 0;
                            }
                        }
                        /* LDRA_JUSTIFY_END */
                    }

                    if( (0 == to_ret) && ((uint32_t)ALGORITHMS_ISS_AE_DISABLED != nodePrms->p_ae_params->mode) )
                    {
#ifndef x86_64
                        IssSensor_ExposureParams aePrms;
                        if(1U == ae_awb_result_target_ptr->ae_valid)
                        {
                            aePrms.chId = 0;
                            aePrms.analogGain[ISS_SENSOR_EXPOSURE_LONG] = ae_awb_result_target_ptr->analog_gain;
                            aePrms.analogGain[ISS_SENSOR_EXPOSURE_SHORT] = ae_awb_result_target_ptr->analog_gain;
                            aePrms.analogGain[ISS_SENSOR_EXPOSURE_VSHORT] = ae_awb_result_target_ptr->analog_gain;
                            aePrms.exposureTime[ISS_SENSOR_EXPOSURE_LONG] = ae_awb_result_target_ptr->exposure_time;
                            aePrms.exposureTime[ISS_SENSOR_EXPOSURE_SHORT] = ae_awb_result_target_ptr->exposure_time;
                            aePrms.exposureTime[ISS_SENSOR_EXPOSURE_VSHORT] = ae_awb_result_target_ptr->exposure_time;
                            aePrms.expRatio = 1U;/*Do not care*/
                            status = sendExpGainToSensor(aewb_config, &aePrms);
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start> 
                            Rationale: The test framework and test apps cannot reach this portion.
                            The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (0 != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to send AE results to image sensor \n");
                            }
                            /* LDRA_JUSTIFY_END */
                        }
#endif /*x86_64*/
                    }
                }

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
                <justification end> */
                if( (0 == to_ret)  && (1U==skipAWB))
                {
                    ae_awb_result_target_ptr->awb_valid = 0;
                }
                else if (0 == to_ret)
                /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start> 
                    Rationale: The test framework and test apps cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((int32_t)ALGORITHMS_ISS_AWB_AUTO == (int32_t)nodePrms->p_awb_params->mode)
                    {
                        status = AWB_TI_process(nodePrms->p_h3a_merge, nodePrms->p_awb_params, ae_awb_result_prev_target_ptr, ae_awb_result_target_ptr, nodePrms->awb_scratch_memory, (uint8_t)nodePrms->sensor_pre_wb_gain);

                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "AWB_TI_process returned error 0x%x \n", status);
                            ae_awb_result_target_ptr->awb_valid = 0;
                        }
                        
                        if (NULL != nodePrms->dcc_output_params)
                        {
                            if ((100U == nodePrms->dcc_output_params->awbCalbData.apply_rgb_adjust) && (0 == nodePrms->sensor_pre_wb_gain))
                            {
                                vx_float64 power = (vx_float64)nodePrms->dcc_output_params->awbCalbData.R_adjust / (vx_float64)100.0;
                                vx_float64 g0 = (vx_float64)ae_awb_result_target_ptr->wb_gains[0];
                                vx_float64 g1 = (vx_float64)ae_awb_result_target_ptr->wb_gains[1];
                                vx_float64 g2 = (vx_float64)ae_awb_result_target_ptr->wb_gains[2];
                                vx_float64 g3 = (vx_float64)ae_awb_result_target_ptr->wb_gains[3];
                                vx_float64 wb_gain0_double = ((pow((g0 / 512.0), power)) * 512.0) + 0.5;
                                vx_float64 wb_gain1_double = ((pow((g1 / 512.0), power)) * 512.0) + 0.5;
                                vx_float64 wb_gain2_double = ((pow((g2 / 512.0), power)) * 512.0) + 0.5;
                                vx_float64 wb_gain3_double = ((pow((g3 / 512.0), power)) * 512.0) + 0.5;
                                int32_t wb_gain0_signed = (int32_t)wb_gain0_double;
                                int32_t wb_gain1_signed = (int32_t)wb_gain1_double;
                                int32_t wb_gain2_signed = (int32_t)wb_gain2_double;
                                int32_t wb_gain3_signed = (int32_t)wb_gain3_double;
                                ae_awb_result_target_ptr->wb_gains[0] = (uint32_t)wb_gain0_signed;
                                ae_awb_result_target_ptr->wb_gains[1] = (uint32_t)wb_gain1_signed;
                                ae_awb_result_target_ptr->wb_gains[2] = (uint32_t)wb_gain2_signed;
                                ae_awb_result_target_ptr->wb_gains[3] = (uint32_t)wb_gain3_signed;
                            }
                        }

                        VX_PRINT(VX_ZONE_INFO, "AWB Gain = (%d, %d, %d, %d) \n",
                                ae_awb_result_target_ptr->wb_gains[0],
                                ae_awb_result_target_ptr->wb_gains[1],
                                ae_awb_result_target_ptr->wb_gains[2],
                                ae_awb_result_target_ptr->wb_gains[3]);
                    }
                    /* LDRA_JUSTIFY_END */
                }
                else
                {
                    /* Do Nothing */
                }
                }
                tivxCheckStatus(&status, tivxMemBufferUnmap(h3a_aew_af_target_ptr, h3a_aew_af_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }

            /* kernel processing function complete */

            tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
            configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if( histogram_desc != NULL)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(histogram_target_ptr,
                histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));
            }
            /* LDRA_JUSTIFY_END */

#ifndef x86_64
            if(1U == ae_awb_result_target_ptr->ae_valid)
            {
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.digitalGain = ae_awb_result_target_ptr->digital_gain;
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.analogGain = ae_awb_result_target_ptr->analog_gain;
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.expTime = ae_awb_result_target_ptr->exposure_time;
            }
            if(1U == ae_awb_result_target_ptr->awb_valid)
            {
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.rGain = ae_awb_result_target_ptr->wb_gains[rIndex];
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.gGain =
                    (ae_awb_result_target_ptr->wb_gains[grIndex] + ae_awb_result_target_ptr->wb_gains[gbIndex])/2U;
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.bGain = ae_awb_result_target_ptr->wb_gains[bIndex];
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.colorTemp = ae_awb_result_target_ptr->color_temperature;
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.awb_valid = ae_awb_result_target_ptr->awb_valid;
                aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.awb_converged = ae_awb_result_target_ptr->awb_converged;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((vx_status)VX_SUCCESS == lock_status)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxMutexUnlock(itt_aewb_lock[aewb_config->channel_id]);
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(1 == nodePrms->sensor_pre_wb_gain)
            {
                IssSensor_WhiteBalanceParams awbPrms;
                awbPrms.chId = aewb_config->channel_id;
                awbPrms.rGain[ISS_SENSOR_EXPOSURE_LONG] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.rGain;
                awbPrms.gGain[ISS_SENSOR_EXPOSURE_LONG] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.gGain;
                awbPrms.bGain[ISS_SENSOR_EXPOSURE_LONG] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.bGain;
                awbPrms.colorTemp[ISS_SENSOR_EXPOSURE_LONG] = aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.colorTemp;

                status = sendWbGainToSensor(aewb_config, &awbPrms);
                if (0 != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to send AWB results to image sensor \n");
                }
                
                memcpy(ae_awb_result_prev_target_ptr, ae_awb_result_target_ptr, sizeof(tivx_ae_awb_params_t));
                ae_awb_result_target_ptr->wb_gains[0] = 512;
                ae_awb_result_target_ptr->wb_gains[1] = 512;
                ae_awb_result_target_ptr->wb_gains[2] = 512;
                ae_awb_result_target_ptr->wb_gains[3] = 512;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
            <justification end> */
            if (viss_cpu_id < APP_IPC_CPU_MAX)
            /* LDRA_JUSTIFY_END */
            {
    #if defined(SOC_AM62A)
                if ((uint32_t)APP_IPC_CPU_MCU1_0 == viss_cpu_id)
    #else
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
                <justification end> */
                if ( ((uint32_t)APP_IPC_CPU_MCU2_0 == viss_cpu_id)
    #if (VPAC_COUNT > 1)
                    || ((uint32_t)APP_IPC_CPU_MCU4_0 == viss_cpu_id)
    #endif
                    )
                /* LDRA_JUSTIFY_END */
    #endif
                {
                    status = sendAewbToViss(viss_cpu_id, viss_channel_id, ae_awb_result_target_ptr);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The test framework and test apps cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (0 != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to send AWB results to VISS node \n");
                    }
                    /* LDRA_JUSTIFY_END */
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "The CPU ID for VISS from H3A does not have a VISS node enabled, skipping sending AEWB results to VISS\n");
                }
                /* LDRA_JUSTIFY_END */
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            Effect on this unit:
            <justification end> */
            else if (viss_cpu_id==(uint32_t)APP_IPC_CPU_INVALID)
            {
                /* Skipping this case given that this could be a signal from the user that the AEWB is not wanted to be sent */
            }
            else
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "The VISS CPU ID from H3A is not a valid CPU, skipping sending AEWB results to VISS\n");
            }
            /* LDRA_JUSTIFY_END */
#else
            memcpy(&g_ae_awb_result, ae_awb_result_target_ptr, sizeof(tivx_ae_awb_params_t));
#endif //x86_64

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
            <justification end> */
            if (0 == nodePrms->sensor_pre_wb_gain)
            {
                memcpy(ae_awb_result_prev_target_ptr, ae_awb_result_target_ptr, sizeof(tivx_ae_awb_params_t));
            }
            /* LDRA_JUSTIFY_END */

            tivxCheckStatus(&status, tivxMemBufferUnmap(ae_awb_result_target_ptr,
            ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            }
        VX_PRINT(VX_ZONE_INFO, "tivxAewbProcess returning %d\n", status);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxAewbProcess returning %d\n", status);
    }
    /* LDRA_JUSTIFY_END */
    return status;
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void map_sensor_ae_dynparams.* <function end>
<justification start>
Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
static void map_sensor_ae_dynparams(IssAeDynamicParams * p_ae_dynPrms, tiae_exp_prog_t * p_ae_exp_prg)
{
   uint8_t count;
   uint8_t num_dyn_params = (uint8_t)p_ae_dynPrms->numAeDynParams;

   p_ae_exp_prg->num_ranges = (int32_t)num_dyn_params;
   p_ae_exp_prg->target_brightness = (int32_t)p_ae_dynPrms->targetBrightness;
   p_ae_exp_prg->target_brightness_range.min = (int32_t)p_ae_dynPrms->targetBrightnessRange.min;
   p_ae_exp_prg->target_brightness_range.max = (int32_t)p_ae_dynPrms->targetBrightnessRange.max;
   p_ae_exp_prg->exposure_time_step_size = (int32_t)p_ae_dynPrms->exposureTimeStepSize;
   p_ae_exp_prg->enableBLC = p_ae_dynPrms->enableBlc;

   for(count = 0; count < num_dyn_params; count++)
   {
       p_ae_exp_prg->exposure_time_range[count].min = (int32_t)p_ae_dynPrms->exposureTimeRange[count].min;
       p_ae_exp_prg->exposure_time_range[count].max = (int32_t)p_ae_dynPrms->exposureTimeRange[count].max;

       p_ae_exp_prg->analog_gain_range[count].min = (int32_t)p_ae_dynPrms->analogGainRange[count].min;
       p_ae_exp_prg->analog_gain_range[count].max = (int32_t)p_ae_dynPrms->analogGainRange[count].max;

       p_ae_exp_prg->digital_gain_range[count].min = (int32_t)p_ae_dynPrms->digitalGainRange[count].min;
       p_ae_exp_prg->digital_gain_range[count].max = (int32_t)p_ae_dynPrms->digitalGainRange[count].max;

       p_ae_exp_prg->aperture_size_range[count].min = 1;
       p_ae_exp_prg->aperture_size_range[count].max = 1;
   }
}

static vx_status VX_CALLBACK tivxAewbCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status awbInitStatus;
    awb_calc_data_t * awb_calib;
    tivx_obj_desc_user_data_object_t *dcc_desc;
    vx_status dcc_status = (vx_status)VX_FAILURE;
    tivxAEWBParams *nodePrms = NULL;
    tivx_aewb_config_t *aewb_config;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    void *configuration_target_ptr;

    dcc_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_AEWB_AE_AWB_DCC_IDX];

    nodePrms = tivxMemAlloc((uint32_t)sizeof(tivxAEWBParams), (vx_enum)TIVX_MEM_EXTERNAL);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == nodePrms)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxMemAlloc returned NULL \n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }
    /* LDRA_JUSTIFY_END */

    /* Only continue with further allocations if initial allocation succeeded */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->dcc_input_params = (dcc_parser_input_params_t *)tivxMemAlloc((uint32_t)sizeof(dcc_parser_input_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
        nodePrms->dcc_output_params = (dcc_parser_output_params_t *)tivxMemAlloc((uint32_t)sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
        nodePrms->awb_scratch_memory = tivxMemAlloc((uint32_t)(sizeof(uint8_t) * (uint32_t)AWB_SCRATCH_MEM_SIZE), (vx_enum)TIVX_MEM_EXTERNAL);
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(((vx_status)VX_SUCCESS == status) && ((NULL == nodePrms->dcc_input_params) || (NULL == nodePrms->dcc_output_params) || (NULL == nodePrms->awb_scratch_memory)))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxMemAlloc returned NULL \n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }
    /* LDRA_JUSTIFY_END */

    /* Only proceed with configuration if previous operations succeeded */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_AEWB_CONFIGURATION_IDX];

        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    aewb_config = (tivx_aewb_config_t *)configuration_target_ptr;
    nodePrms->dcc_id = aewb_config->sensor_dcc_id;
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : DCC ID = %d \n", aewb_config->sensor_dcc_id);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : Image Format = %d \n", aewb_config->sensor_img_format);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : Phase = %d \n", aewb_config->sensor_img_phase);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : awb_mode = %d \n", aewb_config->awb_mode);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : ae_mode = %d \n", aewb_config->ae_mode);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : awb_num_skip_frames = %d \n", aewb_config->awb_num_skip_frames);
    VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : ae_num_skip_frames = %d \n", aewb_config->ae_num_skip_frames);
    nodePrms->frame_count = 0;

#ifndef x86_64
    nodePrms->sensor_pre_wb_gain = -1;
    nodePrms->ae_awb_result_prev.wb_gains[0] = 512;
    nodePrms->ae_awb_result_prev.wb_gains[1] = 512;
    nodePrms->ae_awb_result_prev.wb_gains[2] = 512;
    nodePrms->ae_awb_result_prev.wb_gains[3] = 512;

    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.aeMode = aewb_config->ae_mode;
    aewbCtrlPrms[aewb_config->channel_id].aewb2APrms.awbMode = aewb_config->awb_mode;

#else
    nodePrms->sensor_pre_wb_gain = 0;
#endif //x86_64

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != dcc_desc)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->dcc_input_params->dcc_buf_size = dcc_desc->mem_size;
        VX_PRINT(VX_ZONE_INFO, "tivxAewbCreate : DCC Numbytes = %d \n", nodePrms->dcc_input_params->dcc_buf_size);
        nodePrms->dcc_input_params->dcc_buf = tivxMemShared2TargetPtr(&dcc_desc->mem_ptr);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->dcc_input_params->dcc_buf)
        /* LDRA_JUSTIFY_END */
        {
            tivxCheckStatus(&status, tivxMemBufferMap(nodePrms->dcc_input_params->dcc_buf, dcc_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            nodePrms->dcc_input_params->analog_gain = 1000;
            nodePrms->dcc_input_params->cameraId = aewb_config->sensor_dcc_id;
            nodePrms->dcc_input_params->color_temparature = 5000;
            nodePrms->dcc_input_params->exposure_time = 33333;

            VX_PRINT(VX_ZONE_INFO, "Before dcc_update : awb_calb_data.radius = 0x%x \n", nodePrms->dcc_output_params->awbCalbData.radius);

            dcc_status = Dcc_Create(nodePrms->dcc_output_params, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(dcc_status != (vx_status)0)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxAewbCreate : Dcc_Create returned %d \n", dcc_status);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                dcc_status = dcc_update(nodePrms->dcc_input_params, nodePrms->dcc_output_params);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(dcc_status != (vx_status)0)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxAewbCreate : dcc_update returned %d \n", dcc_status);
            }
            /* LDRA_JUSTIFY_END */

            VX_PRINT(VX_ZONE_INFO, "After dcc_update : awb_calb_data.radius = 0x%x \n", nodePrms->dcc_output_params->awbCalbData.radius);
            tivxCheckStatus(&status, tivxMemBufferUnmap(nodePrms->dcc_input_params->dcc_buf, dcc_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "DCC buffer is NULL \n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_WARNING, "No DCC buffer passed. Disabling 2A \n");

    }
    /* LDRA_JUSTIFY_END */

    /* Only allocate AWB parameters if previous operations succeeded */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->p_awb_params = tivxMemAlloc((uint32_t)sizeof(awbprm_t), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == nodePrms->p_awb_params)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate AWB Param\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            nodePrms->p_awb_params->mode = (uint16_t)ALGORITHMS_ISS_AWB_DISABLED;
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Only allocate AE parameters if previous operations succeeded */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->p_ae_params = tivxMemAlloc((uint32_t)sizeof(tiae_prm_t), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == nodePrms->p_ae_params)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate AE Param\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            nodePrms->p_ae_params->mode = (uint16_t)ALGORITHMS_ISS_AE_DISABLED;
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Only allocate sensor calibration data if previous operations succeeded */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->p_awb_params->sen_awb_calc_data = tivxMemAlloc((uint32_t)sizeof(awb_calc_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == nodePrms->p_awb_params->sen_awb_calc_data)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate AWB sensor calibration\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Only allocate H3A merge data if previous operations succeeded */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->p_h3a_merge = tivxMemAlloc(((uint32_t)H3A_MAX_WINH)*((uint32_t)H3A_MAX_WINV)*sizeof(h3a_aewb_paxel_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == nodePrms->p_h3a_merge)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate AWB H3A merge data\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            memset(nodePrms->p_h3a_merge, 0x0, ((uint32_t)H3A_MAX_WINH)*((uint32_t)H3A_MAX_WINV)*sizeof(h3a_aewb_paxel_data_t));
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == dcc_status)
    /* LDRA_JUSTIFY_END */
    {
        awb_calib = &(nodePrms->dcc_output_params->awbCalbData);
        VX_PRINT(VX_ZONE_INFO, "DCC succeeded \n");
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        awb_calib = &ts1_5_awb_calc_data;
        VX_PRINT(VX_ZONE_INFO, "DCC failed. Using default AWB calibration \n");
    }
    /* LDRA_JUSTIFY_END */

    /* Only proceed with AE and AWB initialization if previous operations succeeded */
    if((vx_status)VX_SUCCESS == status)
    {
        TI_AE_init(nodePrms->p_ae_params, NULL);

        awbInitStatus = AWB_TI_create(nodePrms->p_awb_params, awb_calib);
    }
    else
    {
        awbInitStatus = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(((vx_status)VX_SUCCESS == status) && ((vx_status)VX_SUCCESS == awbInitStatus))
    /* LDRA_JUSTIFY_END */
    {
        nodePrms->awb_h3a_res = tivxMemAlloc(((uint32_t)H3A_MAX_WINH)*((uint32_t)H3A_MAX_WINV)*sizeof(h3a_aewb_paxel_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
        VX_PRINT(VX_ZONE_INFO, "AWB Initialization successful \n");
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        /* Only update status if it was previously successful */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        VX_PRINT(VX_ZONE_ERROR, "AWB Initialization returned %d \n", awbInitStatus);
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != nodePrms->awb_h3a_res)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxSetTargetKernelInstanceContext(kernel, nodePrms, (uint32_t)sizeof(tivxAEWBParams));
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                VX_PRINT(VX_ZONE_INFO, "AWB Initialization successful \n");
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxSetTargetKernelInstanceContext failed %d \n", status);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate H3A buffer \n");
        }
        /* LDRA_JUSTIFY_END */
        }

        /* Unmap configuration buffer at the end of configuration block */
        tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr, configuration_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    return status;
}

static vx_status VX_CALLBACK tivxAewbDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxAEWBParams* nodePrms = NULL;
    uint32_t size;
    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms = (tivxAEWBParams *)temp_kernel_context;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->dcc_input_params)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->dcc_input_params, (uint32_t)sizeof(dcc_parser_input_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free dcc_input_params\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->dcc_output_params)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->dcc_output_params, (uint32_t)sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free dcc_output_params\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->awb_scratch_memory)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->awb_scratch_memory, (uint32_t)(sizeof(uint8_t) * (uint32_t)AWB_SCRATCH_MEM_SIZE), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free awb_scratch_memory\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->p_h3a_merge)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->p_h3a_merge, ((uint32_t)H3A_MAX_WINH)*((uint32_t)H3A_MAX_WINV)*sizeof(h3a_aewb_paxel_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free p_h3a_merge\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->p_ae_params)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->p_ae_params, (uint32_t)sizeof(tiae_prm_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free p_ae_params\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->p_awb_params)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != nodePrms->p_awb_params->sen_awb_calc_data)
            /* LDRA_JUSTIFY_END */
            {
                free_status = tivxMemFree(nodePrms->p_awb_params->sen_awb_calc_data, (uint32_t)sizeof(awb_calc_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != free_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to free sen_awb_calc_data\n");
                }
                /* LDRA_JUSTIFY_END */
            }
            free_status = tivxMemFree(nodePrms->p_awb_params, (uint32_t)sizeof(awbprm_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free p_awb_params\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != nodePrms->awb_h3a_res)
        /* LDRA_JUSTIFY_END */
        {
            vx_status free_status = tivxMemFree(nodePrms->awb_h3a_res, ((uint32_t)H3A_MAX_WINH)*((uint32_t)H3A_MAX_WINV)*sizeof(h3a_aewb_paxel_data_t), (vx_enum)TIVX_MEM_EXTERNAL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != free_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free awb_h3a_res\n");
            }
            /* LDRA_JUSTIFY_END */
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxAewbDelete : Error in getting kernel instance \n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != nodePrms)
    /* LDRA_JUSTIFY_END */
    {
        vx_status free_status = tivxMemFree(nodePrms, (uint32_t)sizeof(tivxAEWBParams), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != free_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to free nodePrms\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static vx_status VX_CALLBACK tivxAewbControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params,
       void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxAEWBParams* nodePrms = NULL;
    uint32_t size;
    tivx_obj_desc_user_data_object_t * dcc_udo;
    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nodePrms = (tivxAEWBParams *)temp_kernel_context;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((uint32_t)ISS_CMD_SET_DCC_PARAMS == node_cmd_id)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((NULL != obj_desc) && (NULL != nodePrms))
        /* LDRA_JUSTIFY_END */
        {
            void *target_ptr;
            dcc_parser_input_params_t dcc_live_input_params;
            dcc_udo = (tivx_obj_desc_user_data_object_t *)(obj_desc[0U]);
            target_ptr = tivxMemShared2TargetPtr(&dcc_udo->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, dcc_udo->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            memcpy(&dcc_live_input_params, nodePrms->dcc_input_params, sizeof(dcc_parser_input_params_t));
            dcc_live_input_params.dcc_buf = (uint8_t *)target_ptr;
            dcc_live_input_params.dcc_buf_size = dcc_udo->mem_size;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                status = dcc_update(&dcc_live_input_params, nodePrms->dcc_output_params);
            }

            *(nodePrms->p_awb_params->sen_awb_calc_data) = nodePrms->dcc_output_params->awbCalbData;

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, dcc_udo->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "obj_desc is NULL \n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unknown node_cmd_id=0x%x \n", node_cmd_id);
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

void tivxAddTargetKernelAewb(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

#if defined(SOC_AM62A)
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
    {
        strncpy(target_name, TIVX_TARGET_MCU1_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
#else
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0 )
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_MCU2_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    #if !defined(SOC_J722S)
    else if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_1 )
    {
        strncpy(target_name, TIVX_TARGET_MCU2_1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
#endif
    else
    {
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        vx_aewb_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_AEWB_NAME,
                            target_name,
                            tivxAewbProcess,
                            tivxAewbCreate,
                            tivxAewbDelete,
                            tivxAewbControl,
                            NULL);
    }
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tivxRemoveTargetKernelAewb.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
void tivxRemoveTargetKernelAewb(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_aewb_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_aewb_target_kernel = NULL;
    }
}

static void parse_h3a_out(uint8_t h3a_buf[], int32_t n_col, int32_t n_row,
                          int32_t id_r, int32_t id_gr, int32_t id_gb, int32_t id_b,
                          h3a_aewb_paxel_data_t *h3a_data)
{
    uint8_t * cur_addr = h3a_buf;
    int32_t n_win = 0;
    int32_t j, i;

    for (j = 0; j < n_row; j++)
    {
        for (i = 0; i < n_col; i++)
        {
            uint16_t * pAewbWinData = (uint16_t *)cur_addr;

            uint32_t sum_gr = pAewbWinData[id_gr];
            uint32_t sum_gb = pAewbWinData[id_gb];
            uint16_t sum_g  = (uint16_t)((sum_gr + sum_gb + 1U) >> 1U);

            h3a_data[((j * n_col) + i)].green = sum_g;
            h3a_data[((j * n_col) + i)].red   = pAewbWinData[id_r];
            h3a_data[((j * n_col) + i)].blue  = pAewbWinData[id_b];

            cur_addr += sizeof(uint16_t) * 8U;
            n_win++;

            if ((n_win % 8) == 0)
            {
                cur_addr += sizeof(uint16_t) * 8U;
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
        <justification end> */
        if (((cur_addr - h3a_buf) % 32) == 16)
        {
            cur_addr += 16;
        }
        /* LDRA_JUSTIFY_END */
    }
}

static void h3a_merge(
    h3a_aewb_paxel_data_t *p_in,
    int32_t sz_h,
    int32_t sz_v,
    int32_t pix_in_pax,
    int32_t T_low,
    int32_t T_high,
    h3a_aewb_paxel_data_t *p_out)
{
    int32_t th1 = pix_in_pax * T_low;
    int32_t th2 = pix_in_pax * T_high;
    int32_t k;
    for (k = 0; k < ((sz_v * sz_h)); k++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
        <justification end> */
        if (((p_in[k].green >= (uint32_t)th1) && (p_in[k].green <= (uint32_t)th2)))
        /* LDRA_JUSTIFY_END */
        {
            p_out[k] = p_in[k];
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
        <justification end> */
        else
        {
            p_out[k].red   = (uint16_t)((p_out[k].red   * 7U) >> 3);
            p_out[k].green = (uint16_t)((p_out[k].green * 7U) >> 3);
            p_out[k].blue  = (uint16_t)((p_out[k].blue  * 7U) >> 3);
        }
        /* LDRA_JUSTIFY_END */
    }
}

static void h3a_normalize(h3a_aewb_paxel_data_t p_h3a[],
                          int32_t sz_h, int32_t sz_v, int32_t pix_in_pax)
{
    int32_t k;
    for (k = 0; k < ((sz_v * sz_h)); k++)
    {
        uint32_t r = (uint32_t)p_h3a[k].red;
        uint32_t g = (uint32_t)p_h3a[k].green;
        uint32_t b = (uint32_t)p_h3a[k].blue;
        p_h3a[k].red   = (uint16_t)((r + ((uint32_t)pix_in_pax >> 1U)) / (uint32_t)pix_in_pax);
        p_h3a[k].green = (uint16_t)((g + ((uint32_t)pix_in_pax >> 1U)) / (uint32_t)pix_in_pax);
        p_h3a[k].blue  = (uint16_t)((b + ((uint32_t)pix_in_pax >> 1U)) / (uint32_t)pix_in_pax);
    }
}

static vx_status AWB_TI_parse_H3a_buf(uint8_t * pH3a_out, awbprm_t * p_awb_params,
        h3a_aewb_paxel_data_t * awb_h3a_res, h3a_aewb_paxel_data_t * p_h3a_merge)
{
    int32_t h3a_n_col = (int32_t)p_awb_params->ti_awb_data_in.frame_data.h3a_data_x;
    int32_t h3a_n_row = (int32_t)p_awb_params->ti_awb_data_in.frame_data.h3a_data_y;
    int32_t h3a_n_pix = (int32_t)p_awb_params->ti_awb_data_in.frame_data.pix_in_pax;
    int32_t rIndex    = (int32_t)p_awb_params->sen_awb_calc_data->red_index;
    int32_t grIndex   = (int32_t)p_awb_params->sen_awb_calc_data->green1_index;
    int32_t gbIndex   = (int32_t)p_awb_params->sen_awb_calc_data->green2_index;
    int32_t bIndex    = (int32_t)p_awb_params->sen_awb_calc_data->blue_index;
    VX_PRINT(VX_ZONE_INFO, "H3A color: r=%d, gr=%d, gb=%d, b=%d \n", rIndex, grIndex, gbIndex, bIndex);

    parse_h3a_out(pH3a_out, h3a_n_col, h3a_n_row, rIndex, grIndex, gbIndex, bIndex, awb_h3a_res);

    // "0, 1023" below: no merge for 10-bit H3A without H3A switching
    h3a_merge(awb_h3a_res, h3a_n_col, h3a_n_row, h3a_n_pix, 0, 1023, p_h3a_merge);

    h3a_normalize(awb_h3a_res, h3a_n_col, h3a_n_row, h3a_n_pix);

    return (vx_status)VX_SUCCESS;
}

static vx_status AWB_TI_create(awbprm_t *p_awb_params, awb_calc_data_t* p_calib)
{
    vx_status status = (vx_status)VX_SUCCESS;
    TI_AWB_ERROR awbInitStatus;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((NULL == p_calib) || (NULL == p_awb_params))
    {
        VX_PRINT(VX_ZONE_ERROR, "AWB_TI_create Error : Invalid parameters \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        p_awb_params->init_done = 0;
        p_awb_params->mode = (uint16_t)AWB_WB_MODE_AUTO;
        p_awb_params->AWB_ScratchMemory = NULL;
        p_awb_params->manl_tmpr = 0;
        p_awb_params->sb_total_exp = 999999;

        *p_awb_params->sen_awb_calc_data = *p_calib;

        awbInitStatus = TI_AWB_init(p_awb_params);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
         if(TI_AWB_ERROR_OK == awbInitStatus)
        /* LDRA_JUSTIFY_END */
        {
            status = (vx_status)VX_SUCCESS;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ERROR : AWB Initialization returned %d \n", awbInitStatus);
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static uint16_t clipu16.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
static uint16_t clipu16(uint32_t x)
{
    uint16_t val = 0U;
    if (x > 65535U)
    {
        val = 65535U;
    }
    else
    {
        val = (uint16_t)x;
    }
    return val;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static vx_status AWB_TI_process.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
static vx_status AWB_TI_process(
        h3a_aewb_paxel_data_t * awb_h3a_res,
        awbprm_t              * p_awb_params,
        tivx_ae_awb_params_t  * aewb_prev,
        tivx_ae_awb_params_t  * aewb_result,
        uint8_t               * scratch_mem,
        uint8_t                 sensor_pre_gain)
{
    p_awb_params->ti_awb_data_in.frame_data.h3a_res = awb_h3a_res;
    p_awb_params->AWB_ScratchMemory = scratch_mem;
    p_awb_params->ti_awb_data_in.is_face.faces = NULL;      // Not supported
    vx_status status = (vx_status)VX_FAILURE;

    uint8_t rIndex  = (uint8_t)p_awb_params->sen_awb_calc_data->red_index;
    uint8_t grIndex = (uint8_t)p_awb_params->sen_awb_calc_data->green1_index;
    uint8_t gbIndex = (uint8_t)p_awb_params->sen_awb_calc_data->green2_index;
    uint8_t bIndex  = (uint8_t)p_awb_params->sen_awb_calc_data->blue_index;

    if (1U==sensor_pre_gain)
    {
        uint16_t paxelIndex = 0;
        h3a_aewb_paxel_data_t * h3a_paxel_data;
        awb_frame_data_t    * pFrame_data = &(p_awb_params->ti_awb_data_in.frame_data);
        uint32_t n_pax_unsigned = (uint32_t)pFrame_data->h3a_data_x * (uint32_t)pFrame_data->h3a_data_y;
        int32_t n_pax = (int32_t)n_pax_unsigned;
        uint32_t rGain_prev, bGain_prev, gGain_prev;

        rGain_prev = aewb_prev->wb_gains[rIndex];
        gGain_prev = (aewb_prev->wb_gains[grIndex]+aewb_prev->wb_gains[gbIndex])/2U;
        bGain_prev = aewb_prev->wb_gains[bIndex];
        if (((rGain_prev == 0U) || (gGain_prev == 0U)) || (bGain_prev == 0U))
        {
            rGain_prev = 512U;
            gGain_prev = 512U;
            bGain_prev = 512U;
        }

        rGain_prev = ( ((uint32_t)1U<<18U) + (rGain_prev>>1U) ) / rGain_prev;
        gGain_prev = ( ((uint32_t)1U<<18U) + (gGain_prev>>1U) ) / gGain_prev;
        bGain_prev = ( ((uint32_t)1U<<18U) + (bGain_prev>>1U) ) / bGain_prev;

        for (paxelIndex = 0; (int32_t)paxelIndex < n_pax; paxelIndex++)
        {
            h3a_paxel_data = &pFrame_data->h3a_res[paxelIndex];
            h3a_paxel_data->red   = clipu16(((((uint32_t)h3a_paxel_data->red   * rGain_prev) + 256U) >> 9U));
            h3a_paxel_data->green = clipu16(((((uint32_t)h3a_paxel_data->green * gGain_prev) + 256U) >> 9U));
            h3a_paxel_data->blue  = clipu16(((((uint32_t)h3a_paxel_data->blue  * bGain_prev) + 256U) >> 9U));
        }
    }

    awb_data_out_t  awb_data_out;
    TI_AWB_ERROR awbStatus = TI_AWB_ERROR_OK;
    awbStatus = TI_AWB_do(p_awb_params, &awb_data_out);
    
    if(TI_AWB_ERROR_OK == awbStatus)
    {
        TI_AWB_stab(p_awb_params, &awb_data_out);

        aewb_result->wb_gains[rIndex] = ((uint32_t)awb_data_out.gain_R << 1U);
        aewb_result->wb_gains[grIndex] = ((uint32_t)awb_data_out.gain_Gr << 1U);
        aewb_result->wb_gains[gbIndex] = ((uint32_t)awb_data_out.gain_Gb << 1U);
        aewb_result->wb_gains[bIndex] = ((uint32_t)awb_data_out.gain_B << 1U);

        aewb_result->wb_offsets[rIndex] = 0;
        aewb_result->wb_offsets[grIndex] = 0;
        aewb_result->wb_offsets[gbIndex] = 0;
        aewb_result->wb_offsets[bIndex] = 0;

        aewb_result->color_temperature = awb_data_out.color_temperature_estim;
        aewb_result->awb_valid = 1U;
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        aewb_result->awb_valid = 0U;
        VX_PRINT(VX_ZONE_ERROR, "TI_AWB_do Returned error %d \n", awbStatus);
        status = (vx_status)VX_FAILURE;
    }

    p_awb_params->AWB_ScratchMemory = NULL;
    return status;
}
