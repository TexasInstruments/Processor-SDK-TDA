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
#ifdef BUILD_CAPTURE

#include <VX/vx.h>
#include <TI/tivx.h>
#include <VX/vx_khr_safe_casts.h>
#include <TI/video_io_kernels.h>
#include <TI/video_io_capture.h>
#include <TI/video_io_display.h>
#include "test_engine/test.h"
#include <TI/tivx_config.h>
#include <string.h>
#include "tivx_utils_file_rd_wr.h"
#include <TI/tivx_task.h>
#include "math.h"
#include <limits.h>
#include "test_tiovx/test_tiovx.h"
#include "tivx_utils_file_rd_wr.h"
#include "tivx_video_io_host_priv.h"
#include "tivx_kernel_capture.h"
#include <vx_internal.h>

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include "test_video_io_common.h"
#include <utils/iss/include/app_iss.h>

#define MAX_NUM_BUF         (8u)

/* Camera sensor used by the tests. Selected by name rather than by position:
 * the enumerated list depends on which drivers the imaging build includes,
 * so an index is not stable across builds. */
#define SENSOR_NAME         SENSOR_SONY_IMX390_UB953_D3

/* Number of camera channels to use. Supported values: 1 or 4. */
#define NUM_CHANNELS        (1U)
#define CAPT_INST_ID        (0U)

#define CAPTURE_NODE_ERROR  (1U)

#define CHANNEL_SWITCH_FRAME_COUNT          (150u)

#define STREAMING_START_OFF  0
#define STREAMING_START_ON   1

#define ERROR_GENERATION_OFF  0
#define ERROR_GENERATION_ON   1

#define DISABLE_CH_0              0
#define DISABLE_CH_1              1
#define DISABLE_CH_2              2
#define DISABLE_CH_3              3
#define DISABLE_CH_2_3            4
#define DISABLE_ALL_CH            5
#define ENABLE_ALL_CH             6
#define DISABLE_REENABLE_ALL_CH   7

#define CAPTURE_TEST_FRAME_RATE   32000

#define LARGE_IMAGE_WIDTH         64000U  
#define LARGE_IMAGE_HEIGHT        64000U

/*
 * SENSOR_CHANNEL_MASK selects which FPD-Link RX ports the tests use. Bit N set
 * means a camera is connected to port N, and the number of bits set must equal
 * NUM_CHANNELS.
 *
 * Set it to match the ports the cameras are actually wired to. Everything else
 * follows from it: the mask is passed to appInitImageSensor()/appStartImageSensor()
 * and captureGetChVcNum() derives the capture node's virtual channel numbers
 * from it, so moving a camera to a different port is a one-line change here.
 *
 * Only ports 0-3 are supported. The deserializer these ports belong to feeds one
 * CSI-RX instance, and apart from testCaptureTwoCsiRxInstances every test
 * configures a single instance (numInst = 1, instId[0] = 0). Ports 4 and above
 * live on a further deserializer feeding a different CSI-RX instance; using them
 * would additionally require the virtual channel to be taken modulo 4 and
 * chInstMap/numInst/instId to be set per channel.
 */
#if defined(SOC_J722S)

/* Test Farm configuration:
 *  - NUM_CHANNELS = 1: single camera; the bit set below is its port.
 *  - NUM_CHANNELS = 4: not currently available in the Test Farm. If a local
 *    four-camera setup is used, set the bits for the connected ports.
 */
    #if (NUM_CHANNELS == 1U)
        #define SENSOR_CHANNEL_MASK    (1U << 2U)
    #elif (NUM_CHANNELS == 4U)
        #define SENSOR_CHANNEL_MASK    (0xFU)
    #else
        #error "Unsupported NUM_CHANNELS configuration"
    #endif

#else

/* Default configuration:
 *  - NUM_CHANNELS = 1: single camera; the bit set below is its port.
 *  - NUM_CHANNELS = 4: cameras connected to ports 0-3.
 */
    #if (NUM_CHANNELS == 1U)
        #define SENSOR_CHANNEL_MASK    (1U << 0U)
    #elif (NUM_CHANNELS == 4U)
        #define SENSOR_CHANNEL_MASK    (0xFU)
    #else
        #error "Unsupported NUM_CHANNELS configuration"
    #endif

#endif

/**
 * Returns the CSI-2 virtual channel carrying the ch_idx-th camera enabled in
 * SENSOR_CHANNEL_MASK.
 *
 * The deserializer forwards each FPD-Link RX port on the virtual channel
 * matching the port number: port 0 -> VC0 ... port 3 -> VC3. The capture node
 * must therefore be programmed with the physical port number, not with a
 * sequential channel index - otherwise CSI-RX waits on a virtual channel that
 * never carries data and the graph never completes a frame.
 */
static uint32_t captureGetChVcNum(uint32_t ch_idx)
{
    uint32_t port;
    uint32_t enabled_count = 0U;
    uint32_t vc_num = 0U;

    for (port = 0U; port < 32U; port++)
    {
        if (0U != (SENSOR_CHANNEL_MASK & (1U << port)))
        {
            if (enabled_count == ch_idx)
            {
                vc_num = port;
                break;
            }
            enabled_count++;
        }
    }

    return vc_num;
}

/* The UB9xxx RAW12 / UB96x UYVY test patterns are generated inside the
 * deserializer itself, not by a camera on an FPD-Link port. Their sensor driver
 * declares numChan = 1, so they only accept channel 0 and always arrive on
 * virtual channel 0 - they are independent of SENSOR_CHANNEL_MASK. */
#define TESTPATTERN_CHANNEL_MASK   (1U << 0U)
#define TESTPATTERN_VC_NUM         (0U)

/**
 * Returns the entry of sensor_list matching sensor_name, or NULL.
 *
 * The enumerated list depends on which sensor drivers the imaging build
 * includes, so a sensor's index is not stable across builds. Looking the
 * sensor up by name avoids silently selecting a different one - picking the
 * UYVY pattern in place of the RAW12 pattern, for instance, leaves the capture
 * node configured for a format the sensor never sends, and the graph waits for
 * a frame that never arrives.
 */
static char *captureFindSensorByName(char *sensor_list[], vx_uint8 num_sensors, const char *sensor_name)
{
    char *found = NULL;
    vx_uint8 idx;

    for (idx = 0U; idx < num_sensors; idx++)
    {
        if (0 == strncmp(sensor_list[idx], sensor_name, ISS_SENSORS_MAX_NAME))
        {
            found = sensor_list[idx];
            break;
        }
    }

    return found;
}

/* Golden CRC-32 value expected for frames captured as VX_TYPE_IMAGE */
static const vx_uint32 golden_crc_img = 0xCC38620A;
/* Golden CRC-32 value expected for frames captured as TIVX_TYPE_RAW_IMAGE */
static const vx_uint32 golden_crc_raw_img = 0x5B70FB09;

/**
 * Updates a CRC-32 value using the supplied data buffer.
 *
 * The calculation uses the reflected CRC-32 polynomial 0xEDB88320.
 * CRC initialization and the final XOR operation are handled by the
 * caller.
 */
static vx_uint32 captureCrc32Update(vx_uint32 crc, const vx_uint8 *data, vx_size num_bytes)
{
    vx_size byte_idx;
    vx_uint32 bit_idx;

    for (byte_idx = 0U; byte_idx < num_bytes; byte_idx++)
    {
        crc ^= (vx_uint32)data[byte_idx];

        for (bit_idx = 0U; bit_idx < 8U; bit_idx++)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xEDB88320U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

/**
 * Calculates CRC-32 over the valid pixels of a VX_DF_IMAGE_U16 image.
 *
 * stride_y is used to locate each row and stride_x is used to locate
 * each pixel. Only the two bytes belonging to each U16 pixel are added
 * to the CRC. Row and per-pixel padding are excluded.
 */
static vx_uint32 captureCalculateImageCrcU16(const void *data_ptr, vx_uint32 width, vx_uint32 height, const vx_imagepatch_addressing_t *image_addr)
{
    const vx_uint8 *base_ptr = (const vx_uint8 *)data_ptr;
    vx_uint32 crc = 0xFFFFFFFFU;

    for (vx_uint32 y = 0U; y < height; y++)
    {
        const vx_uint8 *row_ptr = base_ptr + ((vx_size)y * (vx_size)image_addr->stride_y);

        for (vx_uint32 x = 0U; x < width; x++)
        {
            const vx_uint8 *pixel_ptr = row_ptr + ((vx_size)x * (vx_size)image_addr->stride_x);

            crc = captureCrc32Update(crc, pixel_ptr, sizeof(vx_uint16));
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

/**
 * Calculates CRC-32 over the valid pixel bytes of a RAW image exposure.
 *
 * The number of bytes included in the CRC depends on the RAW pixel
 * container. Padding between consecutive rows is excluded.
 */
static vx_uint32 captureCalculateRawImageCrc(const void *data_ptr, vx_uint32 width, vx_uint32 height, const vx_imagepatch_addressing_t *image_addr, vx_uint32 pixel_container)
{
    const vx_uint8 *base_ptr = (const vx_uint8 *)data_ptr;
    vx_uint32 crc = 0xFFFFFFFFU;
    vx_size valid_row_bytes = 0U;
    vx_uint32 y;

    if (pixel_container == (vx_uint32)TIVX_RAW_IMAGE_16_BIT)
    {
        valid_row_bytes = (vx_size)width * sizeof(vx_uint16);
    }
    else if (pixel_container == (vx_uint32)TIVX_RAW_IMAGE_8_BIT)
    {
        valid_row_bytes = (vx_size)width * sizeof(vx_uint8);
    }
    else if (pixel_container == (vx_uint32)TIVX_RAW_IMAGE_P12_BIT)
    {
        /*
         * Packed RAW12 stores two pixels in three bytes.
         * The expression also rounds up for an odd image width.
         */
        valid_row_bytes = (((vx_size)width * 3U) + 1U) / 2U;
    }
    else
    {
        /*
         * Unsupported RAW pixel container. A zero result cannot be
         * confused with a valid calculation without caller validation.
         */
        return 0U;
    }

    for (y = 0U; y < height; y++)
    {
        const vx_uint8 *row_ptr = base_ptr + ((vx_size)y * (vx_size)image_addr->stride_y);

        crc = captureCrc32Update(crc, row_ptr, valid_row_bytes);
    }

    return crc ^ 0xFFFFFFFFU;
}

static const vx_char user_data_object_name[] = "tivx_capture_params_t";

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static uint32_t initSensorParams(uint32_t sensor_features_supported)
{
    uint32_t sensor_features_enabled = 0;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
    }else
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
    }

    return sensor_features_enabled;
}

TESTCASE(tivxVideoIOCapture, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int loop_cnt;
    int raw_capture;
} Arg_Capture;

#define CAPTURE_PARAMETERS \
    CT_GENERATE_PARAMETERS("capture", ARG, 5, 1), \
    CT_GENERATE_PARAMETERS("capture", ARG, 5, 0)

/**
 * Test raw image capture with parameterized configurations.
 *
 * Tests capture with different parameter sets including various frame counts
 * and configuration options to ensure robust operation across scenarios.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRawImageCapture, Arg_Capture, CAPTURE_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;

    /* Data objects for graph */
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_raw_image raw_image = 0;
    vx_image image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;
    vx_bool done;
    vx_object_array dequeue_capture_array;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = arg_->loop_cnt;

    /* Init for test case */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
    }

    /* Creating objects for graph */
    {
        if (0 == arg_->raw_capture)
        {
            ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            if (0 == arg_->raw_capture)
            {
                ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
            else
            {
                ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.numInst = 1U;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = captureGetChVcNum(loop_id);
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);
    }

    /* Pipelining and graph verification */
    {
        /* input @ node index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, n0, 1);

        /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }


    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));/*Mask for 4 cameras*/

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

#if (NUM_CHANNELS > 1)
        uint64_t timestamp0 = 0, timestamp1 = 0;

        if (0 == arg_->raw_capture)
        {
            tivx_raw_image element0, element1;

            ASSERT_VX_OBJECT(element0 = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

            ASSERT_VX_OBJECT(element1 = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, 1), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

            /* The 30 FPS sensor parameters of IMX390 is around 32.4 ms latency in capture */
            ASSERT(timestamp > (prev_timestamp+CAPTURE_TEST_FRAME_RATE));

            prev_timestamp = timestamp;

            VX_CALL(tivxReleaseRawImage(&element0));
            VX_CALL(tivxReleaseRawImage(&element1));
        }
        else
        {
            vx_image element0, element1;

            ASSERT_VX_OBJECT(element0 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

            ASSERT_VX_OBJECT(element1 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 1), VX_TYPE_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

            /* The 30 FPS sensor parameters of IMX390 is around 32.4 ms latency in capture */
            /* TIOVX-995: Currently is around 32.5ms in broadcast mode */
            ASSERT(timestamp > (prev_timestamp+CAPTURE_TEST_FRAME_RATE));

            prev_timestamp = timestamp;

            VX_CALL(vxReleaseImage(&element0));
            VX_CALL(vxReleaseImage(&element1));
        }
#endif // #if (NUM_CHANNELS > 1)

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(n0, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    if (0 == arg_->raw_capture)
    {
        VX_CALL(tivxReleaseRawImage(&raw_image));
    }
    else
    {
        VX_CALL(vxReleaseImage(&image));
    }

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxVideoIOUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

typedef struct {
    const char* name;
    int loop_cnt;
    int start_sensors;
    int generate_error;
    int camera_disable;
    int measure_perf;
} Arg_CaptureTimeout;

#if (NUM_CHANNELS == 4U)
#define CAPTURE_TIMEOUT_CHANNEL_SPECIFIC \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_0, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_1, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_2, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_3, 0),
#else
#define CAPTURE_TIMEOUT_CHANNEL_SPECIFIC
#endif

#define CAPTURE_TIMEOUT_PARAMETERS \
    CAPTURE_TIMEOUT_CHANNEL_SPECIFIC \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_OFF, 0, 0)

/**
 * Test raw image capture with timeout scenarios.
 *
 * Tests capture behavior under timeout conditions including channel-specific
 * timeouts, streaming control, and error frame generation to ensure proper
 * timeout handling and error recovery.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRawImageCaptureTimeout, Arg_CaptureTimeout, CAPTURE_TIMEOUT_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;

    /* Data objects for graph */
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_raw_image raw_image = 0, black_raw_image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;
    vx_object_array dequeue_capture_array;
    vx_bool done;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Error frame params */
    vx_event_t event;
    vx_bool is_invalid;

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = arg_->loop_cnt;

    /* Init for test case */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
    }

    /* Creating objects for graph */
    {
        ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.timeout        = 90;
        local_capture_config.timeoutInitial = 500;
        local_capture_config.numInst = 1U;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = captureGetChVcNum(loop_id);
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_ERROR, 0, CAPTURE_NODE_ERROR));

    }

    /* Pipelining and graph verification */
    {
        /* input @ node index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, n0, 1);

        /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }

    /* Allocating black frame */
    {
        ASSERT_VX_OBJECT(black_raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* Initialize to black_frame black */

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(n0, (vx_reference)black_raw_image));

        VX_CALL(vxQueryReference((vx_reference)black_raw_image, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

        ASSERT(is_invalid==vx_true_e);
    }

    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));/*Mask for 4 cameras*/

    exe_time = tivxPlatformGetTimeInUsecs();

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
    }
    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    

    /* In the event of error generation, disable sensor as indicated by test case arguments */
    if (ERROR_GENERATION_ON == arg_->generate_error)
    {
        VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK)); /*Disabling camera 4 */
        if (DISABLE_CH_0 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xE)); // re-enabling all but 0th camera
        }
        else if (DISABLE_CH_1 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xD)); // re-enabling all but 1st camera
        }
        else if (DISABLE_CH_2 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xB)); // re-enabling all but 2nd camera
        }
        else if (DISABLE_CH_3 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, (1<<(3))-1)); // re-enabling all but 3rd camera
        }

        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<loop_cnt; loop_id++)
        {
            uint32_t num_refs;
            vx_object_array out_capture_frames;
            tivx_raw_image image_element;

            /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
             * and prints an error message saying that there are no events in the queue */
            if (VX_SUCCESS == vxWaitEvent(context, &event, vx_true_e))
            {
                /* Verifying that the capture node error event is hit */
                ASSERT(event.app_value==CAPTURE_NODE_ERROR);
            }

            /* Get output reference, waits until a reference is available */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

            if (loop_id > num_buf)
            {
                ASSERT_VX_OBJECT(image_element = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, arg_->camera_disable), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(tivxReleaseRawImage(&image_element));
            }

            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
        }

        vxWaitGraph(graph);

        
    }

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            tivx_raw_image image_element;
            vx_bool is_invalid;

            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));

            if (ERROR_GENERATION_ON == arg_->generate_error)
            {
                ASSERT_VX_OBJECT(image_element = (tivx_raw_image) vxGetObjectArrayItem(dequeue_capture_array, arg_->camera_disable), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(tivxReleaseRawImage(&image_element));
            }
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(n0, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        #if !defined(SOC_J722S)
        if (ERROR_GENERATION_ON == arg_->generate_error)
        {
            for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
            {
                uint8_t bitmask = 1<<(chIdx);
                if (chIdx == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)!=0U);
                }

            }
        }
        #endif
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(tivxReleaseRawImage(&black_raw_image));
    VX_CALL(tivxReleaseRawImage(&raw_image));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxVideoIOUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Test case 1: Start with all cameras streaming then turn off camera 0
 * Test case 2: Start with all cameras streaming then turn off camera 1
 * Test case 3: Start with all cameras streaming then turn off camera 2
 * Test case 4: Start with all cameras streaming then turn off camera 3
 * Test case 5: Start with all cameras streaming then turn off cameras 2 and 3
 * Test case 6: Start with all cameras streaming then turn off all cameras
 * Test case 7: Start with all cameras not streaming then turn on all cameras
 * Test case 8: Start with all cameras streaming then turn off all cameras then turn back on all cameras
 * Test case 9: Start with all cameras on and don't generate any errors
 */
    #if (NUM_CHANNELS == 4U)
#define CAPTURE_DISPLAY_CHANNEL_SPECIFIC \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON, DISABLE_CH_0, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON, DISABLE_CH_1, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON, DISABLE_CH_2, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON, DISABLE_CH_3, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON, ERROR_GENERATION_ON, DISABLE_CH_2_3, 0),
#else
#define CAPTURE_DISPLAY_CHANNEL_SPECIFIC
#endif

#define CAPTURE_DISPLAY_PARAMETERS \
    CAPTURE_DISPLAY_CHANNEL_SPECIFIC \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_ALL_CH, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_OFF, ERROR_GENERATION_ON,  ENABLE_ALL_CH,  0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_REENABLE_ALL_CH, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 10, STREAMING_START_ON,  ERROR_GENERATION_OFF, 0, 0)

/**
 * Test raw image capture with display and streaming scenarios.
 *
 * Tests capture with various streaming control and error generation modes
 * including channel enable/disable sequences to validate display-compatible
 * operation and error handling.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRawImageCaptureDisplay, Arg_CaptureTimeout, CAPTURE_DISPLAY_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node captureNode, displayNode;

    /* Data objects for graph */
    vx_image image, sample_image, black_frame, black_frame_invalid_params;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    vx_user_data_object display_param_obj;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;
    tivx_display_params_t display_params;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;
    vx_object_array dequeue_capture_array;
    vx_bool done;

    /* Display command objects */
    tivx_display_select_channel_params_t channel_prms;
    vx_user_data_object switch_ch_obj;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Error frame params */
    vx_event_t event;
    vx_bool is_invalid;

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = arg_->loop_cnt;

    /* Init for test case */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
    }

    /* Creating objects for capture */
    {
        ASSERT_VX_OBJECT(image = vxCreateImage(context, 1936, 1096, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.timeout        = 90;
        local_capture_config.timeoutInitial = 500;
        local_capture_config.numInst = 1U;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = captureGetChVcNum(loop_id);
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(captureNode = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)captureNode, VX_EVENT_NODE_ERROR, 0, CAPTURE_NODE_ERROR));

    }

    /* Creating objects for display */
    {
        /* Display initialization */
        memset(&display_params, 0, sizeof(tivx_display_params_t));
        display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params.pipeId=0;
        display_params.outWidth=1920;
        display_params.outHeight=1080;
        display_params.posX=0;
        display_params.posY=0;

        ASSERT_VX_OBJECT(display_param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &display_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        sample_image = (vx_image) vxGetObjectArrayItem(capture_frames[0], 0);

#ifdef BUILD_DISPLAY
        ASSERT_VX_OBJECT(displayNode = tivxDisplayNode(graph, display_param_obj, sample_image), VX_TYPE_NODE);
#endif

        VX_CALL(vxReleaseImage(&sample_image));
        /* Create User Data object for channel switching */
        channel_prms.active_channel_id = 0;
        ASSERT_VX_OBJECT(switch_ch_obj = vxCreateUserDataObject(context,
            "tivx_display_select_channel_params_t",
            sizeof(tivx_display_select_channel_params_t), &channel_prms),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        refs[0] = (vx_reference)switch_ch_obj;
    }

    /* Pipelining and graph verification */
    {
        /* input @ node index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, captureNode, 1);

        /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxSetNodeTarget(captureNode, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
#ifdef BUILD_DISPLAY
        VX_CALL(vxSetNodeTarget(displayNode, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1));
#endif
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }


    /* Allocating black frame */
    {
        vx_pixel_value_t init_val;

        init_val.U16  = 0U;

        ASSERT_VX_OBJECT(black_frame = vxCreateUniformImage(context, 1936, 1096, VX_DF_IMAGE_U16, &init_val), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(black_frame_invalid_params = vxCreateImage(context, 1920, 1096, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        /* Initialize to black_frame black */

        /* Note: sending image w/ incorrect params to validate error checking */
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxCaptureRegisterErrorFrame(captureNode, (vx_reference)black_frame_invalid_params));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(captureNode, (vx_reference)black_frame));

        VX_CALL(vxQueryReference((vx_reference)black_frame, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

        ASSERT(is_invalid==vx_true_e);
    }

    /* initializing sensor */
    if (STREAMING_START_ON == arg_->start_sensors)
    {
        VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));/*Mask for 4 cameras*/
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
        {
            channel_prms.active_channel_id =
                (channel_prms.active_channel_id + 1) % num_capture_channels;
            VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                sizeof(tivx_display_select_channel_params_t),
                &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(tivxNodeSendCommand(displayNode, 0,
                TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
        }
#if (NUM_CHANNELS > 1)
        if (STREAMING_START_ON == arg_->start_sensors)
        {
            vx_image element0, element1;
            uint64_t timestamp0 = 0, timestamp1 = 0;

            ASSERT_VX_OBJECT(element0 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

            ASSERT_VX_OBJECT(element1 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 1), VX_TYPE_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxReleaseImage(&element0));
            VX_CALL(vxReleaseImage(&element1));
        }
#endif // #if (NUM_CHANNELS > 1)
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

        /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
         * and prints an error message saying that there are no events in the queue */
        vxWaitEvent(context, &event, vx_true_e);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
    

    /* In the event of error generation, disable sensor as indicated by test case arguments */
    if (ERROR_GENERATION_ON == arg_->generate_error)
    {
        if (STREAMING_START_ON == arg_->start_sensors)
        {
            VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK)); /*Disabling camera 4 */
            if (DISABLE_CH_0 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xE)); // re-enabling all but 0th camera
            }
            else if (DISABLE_CH_1 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xD)); // re-enabling all but 1st camera
            }
            else if (DISABLE_CH_2 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xB)); // re-enabling all but 2nd camera
            }
            else if (DISABLE_CH_3 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, (1<<(3))-1)); // re-enabling all but 3rd camera
            }
            else if (DISABLE_CH_2_3 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xC)); // re-enabling all 0th and 1st camera
            }
        }
        else
        {
            VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));/*Mask for 4 cameras*/
        }

        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<loop_cnt; loop_id++)
        {
            uint32_t num_refs;
            vx_object_array out_capture_frames;

            /* Get output reference, waits until a reference is available */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

            if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
            {
                channel_prms.active_channel_id =
                    (channel_prms.active_channel_id + 1) % num_capture_channels;
                VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                    sizeof(tivx_display_select_channel_params_t),
                    &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(tivxNodeSendCommand(displayNode, 0,
                    TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
            }

            if ( (loop_id > num_buf) &&
                 (ERROR_GENERATION_ON == arg_->generate_error) &&
                 (arg_->camera_disable < 4) )
            {
                vx_image image_element;
                ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(out_capture_frames, arg_->camera_disable), VX_TYPE_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(vxReleaseImage(&image_element));
            }

            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

            /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
             * and prints an error message saying that there are no events in the queue */
            vxWaitEvent(context, &event, vx_true_e);
        }

        if (DISABLE_REENABLE_ALL_CH == arg_->camera_disable)
        {
            /* ensure all graph processing is complete */
            vxWaitGraph(graph);

            VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));/*Mask for 4 cameras*/

            /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
            for(loop_id=0; loop_id<loop_cnt; loop_id++)
            {
                uint32_t num_refs;
                vx_object_array out_capture_frames;

                /* Get output reference, waits until a reference is available */
                vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

                if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
                {
                    channel_prms.active_channel_id =
                        (channel_prms.active_channel_id + 1) % num_capture_channels;
                    VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                        sizeof(tivx_display_select_channel_params_t),
                        &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    VX_CALL(tivxNodeSendCommand(displayNode, 0,
                        TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
                }

                /* Ensuring that the capture frame is valid after reenabling sensor */
                if (loop_id > num_buf)
                {
                    vx_image image_element;
                    ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

                    VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                    ASSERT(is_invalid==vx_false_e);

                    VX_CALL(vxReleaseImage(&image_element));
                }

                vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

                /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
                 * and prints an error message saying that there are no events in the queue */
                vxWaitEvent(context, &event, vx_true_e);
            }
        }
    }

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));

            if ( (ERROR_GENERATION_ON == arg_->generate_error) &&
                 (arg_->camera_disable < 4) )
            {
                vx_image image_element;
                ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(dequeue_capture_array, arg_->camera_disable), VX_TYPE_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(vxReleaseImage(&image_element));
            }
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(captureNode, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        #if !defined(SOC_J722S)
        if (ERROR_GENERATION_ON == arg_->generate_error)
        {
            for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
            {
                uint8_t bitmask = 1<<(chIdx);
                if (chIdx == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else if ((DISABLE_CH_2_3 == arg_->camera_disable) &&
                         ((chIdx == 0) || (chIdx == 1)))
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else if (DISABLE_ALL_CH == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)!=0U);
                }
            }
        }
        #endif
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    VX_CALL(vxReleaseImage(&black_frame_invalid_params));
    VX_CALL(vxReleaseNode(&displayNode));
    VX_CALL(vxReleaseNode(&captureNode));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseImage(&black_frame));
    VX_CALL(vxReleaseImage(&image));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));
    VX_CALL(vxReleaseUserDataObject(&display_param_obj));
    VX_CALL(vxReleaseUserDataObject(&switch_ch_obj));

    tivxVideoIOUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

typedef struct {
    const char* name;
    vx_enum format;
    vx_uint32 width;
    vx_uint32 height;
    vx_enum ref_type;
} Arg_CaptureErrorFrame;

#define ERROR_FRAME_PARAMETERS \
    CT_GENERATE_PARAMETERS("U16",         ARG, VX_DF_IMAGE_U16,  1280, 720,  VX_TYPE_IMAGE), \
    CT_GENERATE_PARAMETERS("UYVY",        ARG, VX_DF_IMAGE_UYVY, 640,  480,  VX_TYPE_IMAGE), \
    CT_GENERATE_PARAMETERS("RAW",         ARG, 0,                1920, 1080, TIVX_TYPE_RAW_IMAGE)

/**
 * Test error frame registration with various formats and dimensions.
 *
 * Tests registration of error frames across different image formats (U16, UYVY)
 * and raw image types with various resolutions to ensure proper format validation.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRegisterErrorFrame, Arg_CaptureErrorFrame, ERROR_FRAME_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_reference ref = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);

    if (arg_->ref_type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
    {
        tivx_raw_image_create_params_t raw_prms;

        memset(&raw_prms, 0, sizeof(raw_prms));
        
        raw_prms.width = arg_->width;
        raw_prms.height = arg_->height;
        raw_prms.num_exposures = 1U;
        raw_prms.line_interleaved = vx_false_e;
        raw_prms.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_prms.format[0].msb = 11U;

        tivx_raw_image raw_template;
        ASSERT_VX_OBJECT(raw_template = tivxCreateRawImage(context, &raw_prms), TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)raw_template, 1), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(tivxReleaseRawImage(&raw_template));

        tivx_raw_image raw_img;
        ASSERT_VX_OBJECT(raw_img = tivxCreateRawImage(context, &raw_prms), TIVX_TYPE_RAW_IMAGE);
        ref = vxCastRefFromRawImage(raw_img);
    }
    else
    {
        vx_image img_template;
        ASSERT_VX_OBJECT(img_template = vxCreateImage(context, arg_->width, arg_->height, arg_->format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)img_template, 1), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(vxReleaseImage(&img_template));
        
        vx_image img;
        ASSERT_VX_OBJECT(img = vxCreateImage(context, arg_->width, arg_->height, arg_->format), VX_TYPE_IMAGE);
        ref = vxCastRefFromImage(img);
    }

    ASSERT_VX_OBJECT(ref, arg_->ref_type);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, ref));

    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

#define INVALID_FORMAT \
    CT_GENERATE_PARAMETERS("NV12_Invalid", ARG, VX_DF_IMAGE_NV12, 1920, 1080, VX_ERROR_INVALID_FORMAT), \
    CT_GENERATE_PARAMETERS("U8_Invalid",   ARG, VX_DF_IMAGE_U8,   640,  480,  VX_ERROR_INVALID_FORMAT)

/**
 * Test error frame registration rejection for invalid formats.
 *
 * Validates that error frame registration correctly rejects unsupported image
 * formats (NV12, U8) and returns appropriate error status.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRegisterErrorFrameInvalid, Arg_CaptureErrorFrame, INVALID_FORMAT)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_image img_template = NULL;
    vx_reference ref = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);
    
    ASSERT_VX_OBJECT(img_template = vxCreateImage(context, arg_->width, arg_->height, arg_->format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)img_template, 1), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&img_template));

    vx_image img = vxCreateImage(context, arg_->width, arg_->height, arg_->format);
    ASSERT_VX_OBJECT(img, VX_TYPE_IMAGE);
    ref = vxCastRefFromImage(img);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, ref));
    
    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

#define INVALID_DIM_PARAMETERS \
    CT_GENERATE_PARAMETERS("IMAGE8K", ARG, VX_DF_IMAGE_U16, 8192, 8192, VX_TYPE_IMAGE), \
    CT_GENERATE_PARAMETERS("RAW8K",   ARG, 0,               8192, 8192, TIVX_TYPE_RAW_IMAGE)

/**
 * Test error frame registration rejection for invalid dimensions.
 *
 * Validates that error frame registration correctly rejects frames with
 * dimensions exceeding supported limits (8K resolution).
 */
TEST_WITH_ARG(tivxVideoIOCapture, testRegisterErrorFrameInvalidDimensions, Arg_CaptureErrorFrame, INVALID_DIM_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_image small_image = NULL;
    vx_image huge_image = NULL;
    vx_reference ref = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);
    
    if (arg_->ref_type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
    {
        tivx_raw_image_create_params_t raw_prms;

        memset(&raw_prms, 0, sizeof(raw_prms));
        raw_prms.width = 128;
        raw_prms.height = 128;
        raw_prms.num_exposures = 1U;
        raw_prms.line_interleaved = vx_false_e;
        raw_prms.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_prms.format[0].msb = 11;
        
        tivx_raw_image raw_template;
        ASSERT_VX_OBJECT(raw_template = tivxCreateRawImage(context, &raw_prms), TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)raw_template, 1), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(tivxReleaseRawImage(&raw_template));
        
        tivx_raw_image raw_img;
        raw_prms.width = arg_->width; 
        raw_prms.height = arg_->height;
        ASSERT_VX_OBJECT(raw_img = tivxCreateRawImage(context, &raw_prms), TIVX_TYPE_RAW_IMAGE);
        ref = vxCastRefFromRawImage(raw_img);
    }
    else
    {
        vx_image img_template;
        ASSERT_VX_OBJECT(img_template = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)img_template, 1), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(vxReleaseImage(&img_template));
        
        vx_image img;
        ASSERT_VX_OBJECT(img = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ref = vxCastRefFromImage(img);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);
    
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, ref));
    
    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test error handling when registering error frame with incorrect type.
 *
 * Validates that the capture node rejects error frame registration when
 * the frame type doesn't match expected types (VX_TYPE_IMAGE, VX_TYPE_RAW_IMAGE).
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameTypeMismatch)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_image standard_img = NULL;
    tivx_raw_image raw_img = NULL;
    vx_reference ref = NULL;
    tivx_capture_params_t params;
    tivx_raw_image_create_params_t raw_params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);
    
    ASSERT_VX_OBJECT(standard_img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)standard_img, 1), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&standard_img));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);

    raw_params.width = 640U;
    raw_params.height = 480U;
    raw_params.num_exposures = 1U;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11U;

    ASSERT_VX_OBJECT(raw_img = tivxCreateRawImage(context, &raw_params), TIVX_TYPE_RAW_IMAGE);
    ref = vxCastRefFromRawImage(raw_img);
    
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, ref));
    
    VX_CALL(tivxReleaseRawImage(&raw_img));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test error handling when registering error frame with incompatible format.
 *
 * Validates that the capture node rejects error frame registration when
 * the image/raw image format doesn't match the configured capture format.
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameFormatMismatch)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_image node_img = NULL;
    vx_image error_img = NULL;
    vx_reference ref = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(node_img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)node_img, 1), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&node_img));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(error_img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ref = vxCastRefFromImage(error_img);
    
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, ref));
    
    VX_CALL(vxReleaseImage(&error_img));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test error handling when registering error frame with unsupported object type.
 *
 * Validates that the capture node rejects error frame registration when
 * the object type is not a supported reference type (e.g., user data objects).
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameUnsupportedObjectType)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_user_data_object config = NULL;
    tivx_capture_params_t params;
    vx_int32 val = 0;
    vx_scalar scalar  = NULL;
    vx_node node = NULL;
    vx_object_array obj_arr = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create a valid input for Capture Node */
    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);

    /* 
     * Create an invalid output for Capture Node
     *
     * Create a scalar object and use it as the object-array template
     * Create an object array containing scalar objects (unsupported type).
     */
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_INT32, &val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)scalar, 1), VX_TYPE_OBJECT_ARRAY);

    /* 
     * Create a Capture node using the unsupported output object type.
     * The graph is intentionally not verified because this test targets
     * tivxCaptureRegisterErrorFrame() validation.
     */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);

    /* Verify that error-frame registration rejects the unsupported reference object type. */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, (vx_reference)scalar));
    
    /* Releasing */
    vxReleaseScalar(&scalar);
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test error handling when error frame query operation fails.
 *
 * Validates that the capture node handles failures in querying error frame
 * properties and maintains proper error state.
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameQueryFailure)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array output_arr = NULL;
    vx_image img = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    tivx_capture_params_init(&params);
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), VX_TYPE_USER_DATA_OBJECT);
    
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_arr = vxCreateObjectArray(context, (vx_reference)img, 1), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&img));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, output_arr), VX_TYPE_NODE);

    vx_int32 dummy_var = 0;
    vx_reference safe_ref = (vx_reference)&dummy_var;
    
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, safe_ref));
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseObjectArray(&output_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Verifies error-frame registration when the Capture node output parameter
 * has not been configured.
 *
 * The test creates the Capture node manually using vxCreateGenericNode()
 * instead of tivxCaptureNode(). Only the required input parameter is
 * assigned, while the output object-array parameter
 * (TIVX_KERNEL_CAPTURE_OUTPUT_IDX) is intentionally left unset.
 *
 * Graph verification is intentionally skipped because it would reject the
 * incomplete node configuration before tivxCaptureRegisterErrorFrame() is
 * executed.
 *
 * tivxCaptureValidateAllocFrame() retrieves the Capture node
 * output parameter, but since no reference is associated with it, 
 * the invalid-reference error path executes and prints:
 *
 *     "Invalid reference"
 *
 * This test specifically validates the host-side error handling for an
 * incompletely configured Capture node and covers the defensive code path
 * that cannot be reached through the standard tivxCaptureNode() API.
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameNull)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_image image = NULL;
    
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create a valid input for Capture Node - User Data Object of a single capture params structure */
    vx_user_data_object config;
    tivx_capture_params_t params;
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /*
     * Create a valid image so that the failure is caused by the missing node
     * output parameter, rather than by an invalid image reference.
     */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    /*
     * Create the Capture node manually. This avoids tivxCaptureNode(), which
     * rejects NULL required output parameter during node construction.
     */
    ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, TIVX_KERNEL_CAPTURE_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    /*
     * Configure only the Capture input parameter. The output parameter at
     * TIVX_KERNEL_CAPTURE_OUTPUT_IDX is intentionally left unset.
     */
    VX_CALL(vxSetParameterByIndex(node, TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX, vxCastRefFromUserDataObject(config)));

    /*
     * Do not call vxVerifyGraph(). The test needs the node to remain in the
     * intentionally incomplete state.
     * 
     * The failure is intentionally caused by the Capture node configuration, not
     * by the supplied error-frame image. The image reference is valid, but the
     * Capture node has no output object-array associated with parameter
     * TIVX_KERNEL_CAPTURE_OUTPUT_IDX. Therefore,
     * tivxCaptureValidateAllocFrame() fails before any validation of the supplied
     * error-frame reference is performed.
     */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, vxCastRefFromImage(image)));
    
    /* Release all objects created by the test. */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test error handling when registering error frame on NULL node.
 *
 * Validates that error frame registration fails gracefully when
 * attempted on an invalid or NULL capture node.
 */
TEST(tivxVideoIOCapture, testRegisterErrorFrameNullNode)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_reference ref = NULL; 

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 640U, 480U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ref = vxCastRefFromImage(img);
    
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(NULL, ref));
    
    VX_CALL(vxReleaseImage(&img));
}

/**
 * Test allocation failure during error frame registration for standard images.
 *
 * This test verifies that tivxCaptureRegisterErrorFrame() correctly handles
 * image buffer allocation failures.
 *
 * A deliberately oversized VX_DF_IMAGE_U16 image is created so that the
 * required image buffer exceeds the available shared memory. The image object
 * itself is created successfully because vxCreateImage() allocates only the
 * image reference and metadata. The actual pixel buffer allocation is deferred
 * until the image is mapped inside tivxCaptureAllocFrame().
 *
 * During tivxCaptureRegisterErrorFrame(), the following call sequence is
 * executed:
 *
 *     tivxCaptureRegisterErrorFrame()
 *         -> tivxCaptureAllocFrame()
 *             -> vxMapImagePatch()
 *                 -> ownCopyAndMapCheckParams()
 *                     -> ownAllocImageBuffer()
 *                         -> tivxMemBufferAlloc()
 *
 * The oversized allocation request causes tivxMemBufferAlloc() to fail and
 * return VX_ERROR_NO_MEMORY. The failure propagates through
 * ownAllocImageBuffer() and ownCopyAndMapCheckParams() back to
 * tivxCaptureAllocFrame().
 *
 * The test verifies that the allocation failure is correctly detected and
 * propagated to the caller instead of allowing error frame registration to
 * succeed.
 */
TEST(tivxVideoIOCapture, testAllocFrameImageFailure)
{
    /* Graph objects */
    vx_node node;
    vx_graph graph;
    vx_object_array obj_arr;
    vx_user_data_object config;
    vx_image image = 0;
    tivx_capture_params_t params;
    vx_context context = context_->vx_context_;
 
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
    /* 
     * Create output for Capture Node - Object array output which has been created from an vx_image
     *
     * The dimensions of an oversized image are intentionally
     * much larger than any realistic camera frame so that memory
     * allocation performed later during tivxCaptureRegisterErrorFrame()
     * is expected to fail.
     */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, LARGE_IMAGE_WIDTH, LARGE_IMAGE_HEIGHT, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
 
    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
 
    /*
     * Registration is expected to fail because the oversized image
     * cannot be allocated when tivxCaptureAllocFrame() maps the image.
     */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, vxCastRefFromImage(image)));
 
    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test allocation failure during error frame registration for RAW images.
 *
 * This test verifies that tivxCaptureRegisterErrorFrame() correctly reports
 * an error when the framework cannot allocate memory for the supplied RAW
 * image.
 *
 * A RAW image with intentionally oversized dimensions is created so that
 * buffer allocation is expected to fail when the image is mapped for the
 * first time inside tivxCaptureAllocFrame(). The test ensures that the
 * allocation failure is detected and propagated back to the caller instead
 * of allowing the registration to succeed.
 * 
 * During tivxCaptureRegisterErrorFrame(), the following call sequence is
 * executed:
 *
 *     tivxCaptureRegisterErrorFrame()
 *         -> tivxCaptureAllocFrame()
 *             -> tivxMapRawImagePatch()
 *                 -> ownCopyAndMapCheckParams()
 *                     -> ownAllocRawImageBuffer()
 *                         -> tivxMemBufferAlloc()
 *
 * The requested buffer size exceeds the available shared memory, causing
 * tivxMemBufferAlloc() to return VX_ERROR_NO_MEMORY. The error propagates
 * back through the call chain, allowing this test to verify the error
 * handling path in tivxCaptureAllocFrame() and
 * tivxCaptureRegisterErrorFrame().
 */
TEST(tivxVideoIOCapture, testAllocFrameRawImageFailure)
{
    /* Graph objects */
    vx_node node;
    vx_graph graph;
    vx_object_array obj_arr;
    vx_user_data_object config;
    tivx_raw_image raw_image = 0;
    tivx_capture_params_t params;
    tivx_raw_image_create_params_t raw_params;
    vx_context context = context_->vx_context_;
 
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
    /* 
     * Create output for Capture Node - Object array output which has been created from an tivx_raw_image
     *
     * The dimensions of an oversized RAW image are intentionally
     * much larger than any realistic camera frame so that memory
     * allocation performed later during tivxCaptureRegisterErrorFrame()
     * is expected to fail.
     */
    memset(&raw_params, 0, sizeof(raw_params));
    raw_params.width  = LARGE_IMAGE_WIDTH;
    raw_params.height = LARGE_IMAGE_HEIGHT;
    raw_params.num_exposures = 1U;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9U;
    /* Bayer pattern */
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after  = 0;
    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)raw_image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
 
    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
 
    /*
     * Registration is expected to fail because the oversized RAW image
     * cannot be allocated when tivxCaptureAllocFrame() maps the image.
     */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, vxCastRefFromRawImage(raw_image)));
 
    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    tivxReleaseRawImage(&raw_image);
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test kernel registration and unregistration lifecycle.
 *
 * Validates that capture kernels can be properly added to and removed from
 * the OpenVX context.
 */
TEST(tivxVideoIOCapture, testtivxAddAndRemoveKernelCapture)
{
    vx_context context = context_->vx_context_;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxAddKernelCapture(context));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxRemoveKernelCapture(context));
}

/**
 * Test error handling when adding capture kernel with NULL context.
 *
 * Validates that capture kernel registration fails safely when passed
 * an invalid or NULL OpenVX context.
 */
TEST(tivxVideoIOCapture, testtivxAddKernelCaptureNullContext)
{
    ASSERT_NE_VX_STATUS(VX_SUCCESS, tivxAddKernelCapture(NULL));
}

typedef struct { 
    const char* name; 
    const char* obj_name; 
    vx_enum     ref_type;
    vx_size     obj_size;
} Arg_AddCaptureValidate;

#define INVALID_INPUT_OUTPUT \
    CT_GENERATE_PARAMETERS("InputName",  ARG, "wrong_name",            VX_TYPE_IMAGE,  sizeof(tivx_capture_params_t)), \
    CT_GENERATE_PARAMETERS("InputSize",  ARG, "tivx_capture_params_t", VX_TYPE_IMAGE,  sizeof(tivx_capture_params_t) + 1U), \
    CT_GENERATE_PARAMETERS("OutputType", ARG, "tivx_capture_params_t", VX_TYPE_SCALAR, sizeof(tivx_capture_params_t))

/**
 * Verifies Capture host-kernel validation for invalid input and output
 * parameter configurations.
 *
 * The test manually creates a Capture node and supplies intentionally invalid
 * references before calling vxVerifyGraph(). During graph verification,
 * tivxAddKernelCaptureValidate() checks the Capture configuration object and
 * the element type of the output object array.
 *
 * The parameterized test covers the following cases:
 *
 * "InputName": The input user data object has the expected size but an
 * incorrect object name. tivxAddKernelCaptureValidate() rejects it because
 * the input must be named "tivx_capture_params_t".
 *
 * "InputSize": The input user data object has the expected name but an
 * incorrect payload size. tivxAddKernelCaptureValidate() rejects it because
 * the object size must exactly match sizeof(tivx_capture_params_t).
 *
 * "OutputType": The output object array contains VX_TYPE_SCALAR elements
 * instead of VX_TYPE_IMAGE or TIVX_TYPE_RAW_IMAGE elements.
 * tivxAddKernelCaptureValidate() detects the unsupported element type and
 * rejects the output configuration.
 *
 * In every case, validation returns an error, causing vxVerifyGraph() to fail
 * as expected. This test validates parameter checking during graph
 * verification; it does not execute the Capture graph.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testtivxAddKernelCaptureValidateInputOutputError, Arg_AddCaptureValidate, INVALID_INPUT_OUTPUT)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object input = NULL;
    vx_object_array output = NULL;
    vx_reference ref = NULL;
    vx_kernel kernel = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /*
     * Retrieve the Capture kernel and manually create a Capture node.
     *
     * Using vxCreateGenericNode() allows the test to assign intentionally invalid
     * input and output references (Capture node params) before graph verification.
     */
    kernel = vxGetKernelByName(context, TIVX_KERNEL_CAPTURE_NAME);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    /*
     * Create the Capture configuration object using the parameterized name and
     * size. Depending on the test case, either the object name or its payload size
     * is intentionally invalid.
     */
    ASSERT_VX_OBJECT(input = vxCreateUserDataObject(context, arg_->obj_name, arg_->obj_size, NULL), VX_TYPE_USER_DATA_OBJECT);

    /*
     * Create the exemplar used for the output object array.
     *
     * The InputName and InputSize cases use a valid VX_TYPE_IMAGE exemplar so
     * that only the input configuration is invalid. The OutputType case uses a
     * VX_TYPE_SCALAR exemplar to force output element-type validation to fail.
     */
    if (arg_->ref_type == VX_TYPE_IMAGE)
    {
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }
    else
    {
        vx_uint32 val = 0;
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &val), VX_TYPE_SCALAR);
    }
    ASSERT_VX_OBJECT(output = vxCreateObjectArray(context, ref, 1), VX_TYPE_OBJECT_ARRAY);

    /*
     * Assign the test references to the Capture node. Parameter index 0 is the
     * Capture configuration input, while parameter index 1 is the output object
     * array.
     */
    vxSetParameterByIndex(node, 0, (vx_reference)input);
    vxSetParameterByIndex(node, 1, (vx_reference)output);

    /*
     * Graph verification is expected to fail in tivxAddKernelCaptureValidate()
     * because each parameter set contains one intentionally invalid Capture
     * input or output property.
     */
    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseUserDataObject(&input));
    VX_CALL(vxReleaseObjectArray(&output));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    tivxVideoIOUnLoadKernels(context);
}

typedef struct {
    const char* test_name;
    vx_bool set_input;
} Arg_CaptureNullParams;

#define NULL_PARAM_CASES \
    CT_GENERATE_PARAMETERS("InputNull",  ARG, vx_false_e), \
    CT_GENERATE_PARAMETERS("OutputNull", ARG, vx_true_e)

/**
 * Verifies Capture host-kernel validation when a required node parameter is
 * not configured.
 *
 * The test manually creates a Capture node and intentionally leaves one of
 * its required parameters without an associated reference before calling
 * vxVerifyGraph().
 *
 * The "InputNull" case assigns a valid output object array but leaves the
 * Capture configuration input unset. The "OutputNull" case assigns a valid
 * configuration input but leaves the output object array unset.
 *
 * tivxAddKernelCaptureValidate() detects the missing required reference,
 * reports "One or more REQUIRED parameters are set to NULL", and causes graph
 * verification to fail with VX_ERROR_INVALID_PARAMETERS.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testtivxAddKernelCaptureValidateNullParams, Arg_CaptureNullParams, NULL_PARAM_CASES)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_user_data_object config = NULL;
    vx_image image = NULL;
    vx_object_array output = NULL;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create a valid Capture configuration input. */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create a valid Capture output object array. */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateObjectArray(context, vxCastRefFromImage(image), NUM_CHANNELS),VX_TYPE_OBJECT_ARRAY);

    /*
     * Retrieve the Capture kernel and manually create a Capture node so that
     * one required parameter can intentionally remain unconfigured.
     */
    ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, TIVX_KERNEL_CAPTURE_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel),VX_TYPE_NODE);

    if ((vx_bool)vx_true_e == arg_->set_input)
    {
        /*
         * Configure only the valid Capture input. The output parameter is
         * intentionally left unset to exercise the OutputNull case.
         */
        VX_CALL(vxSetParameterByIndex(node, TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX, vxCastRefFromUserDataObject(config)));
    }
    else
    {
        /*
         * Configure only the valid Capture output. The input parameter is
         * intentionally left unset to exercise the InputNull case.
         */
        VX_CALL(vxSetParameterByIndex(node, TIVX_KERNEL_CAPTURE_OUTPUT_IDX, vxCastRefFromObjectArray(output)));
    }

    /*
     * Graph verification must fail because one required Capture node
     * parameter has no associated reference.
     */
    ASSERT_NE_VX_STATUS(VX_SUCCESS,vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseObjectArray(&output));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseUserDataObject(&config));
    VX_CALL(vxReleaseGraph(&graph));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test capture parameters initialization with NULL pointer.
 *
 * Validates that the init function handles NULL parameter pointer
 * without crashing or causing memory issues.
 */
TEST(tivxVideoIOCapture, testtivxcaptureparamsinitNULL)
{
    tivx_capture_params_init(NULL);
}

/**
 * Verifies that graph verification fails when the Capture node is configured
 * with more output channels than supported by the target kernel.
 *
 * During graph verification phase, tivxCaptureCreate() reads the number
 * of Capture channels, detects that it exceeds TIVX_CAPTURE_MAX_CH
 * and reports the corresponding Capture error - VX_ERROR_INVALID_PARAMETERS.
 * 
 * As a result, vxVerifyGraph() is expected to return VX_FAILURE.
 */
TEST(tivxVideoIOCapture, testCaptureInvalidNumberOfChannels)
{
    /* Graph objects */
    vx_node node;
    vx_graph graph;
    vx_image image = 0;
    vx_object_array obj_arr;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_context context = context_->vx_context_;

    /* Init for test case */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating objects for graph */
    {
        /* Create input for Capture Node - User Data Object of a single capture params structure */
        tivx_capture_params_init(&params);
        params.numInst = 1U;
        /* 
         * Maximum number of channels supported in the capture node is TIVX_CAPTURE_MAX_CH 
         * and by exceeding it, the VX_ERROR_INVALID_PARAMETERS will be envoked by
         * tivxCaptureCreate(), during verification phase.
         */
        params.numCh   = TIVX_CAPTURE_MAX_CH + 1;
        for (uint32_t instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            params.instId[instIdx] = instIdx;
            params.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            params.instCfg[instIdx].numDataLanes         = 4U;
            for (uint32_t loop_id = 0U; loop_id < params.instCfg[0U].numDataLanes ; loop_id++)
            {
                params.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
        }
        ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create output for Capture Node - Object array output which has been created from an vx_image */
        ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, TIVX_CAPTURE_MAX_CH + 1), VX_TYPE_OBJECT_ARRAY);

        /* Creates a camera Capture Node */
        ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        ASSERT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));
    }

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

typedef struct
{
    const char* name;
    vx_enum format;
} Arg_CaptureImageFormat;

#define CAPTURE_SUPPORTED_IMAGE_FORMATS \
    CT_GENERATE_PARAMETERS("RGBX", ARG, VX_DF_IMAGE_RGBX),   \
    CT_GENERATE_PARAMETERS("BGRX", ARG, TIVX_DF_IMAGE_BGRX), \
    CT_GENERATE_PARAMETERS("U16",  ARG, VX_DF_IMAGE_U16),    \
    CT_GENERATE_PARAMETERS("UYVY", ARG, VX_DF_IMAGE_UYVY),   \
    CT_GENERATE_PARAMETERS("YUYV", ARG, VX_DF_IMAGE_YUYV)

/**
 * Verifies that the Capture node accepts all supported VX_TYPE_IMAGE formats.
 *
 * The test creates and verifies a Capture graph for each supported image
 * format: RGBX, BGRX, U16, UYVY, and YUYV.
 *
 * During graph verification, the Capture host kernel validates the output image
 * format, while the target kernel initializes the corresponding Capture and
 * CSI/FVID2 configuration. Graph verification is expected to succeed for every
 * format listed in CAPTURE_SUPPORTED_IMAGE_FORMATS.
 *
 * This test validates format compatibility during graph verification. It does
 * not execute the graph or validate captured image data.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCaptureSupportedImageFormats, Arg_CaptureImageFormat, CAPTURE_SUPPORTED_IMAGE_FORMATS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array obj_arr;
    vx_user_data_object config;
    vx_image image = 0;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, arg_->format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Verifies handling of the Capture print-statistics control command.
 *
 * The test creates and verifies a Capture graph, then sends the
 * TIVX_CAPTURE_PRINT_STATISTICS command to the Capture target kernel.
 *
 * The command is handled by tivxCaptureControl(), which invokes
 * tivxCapturePrintStatus() for each active Capture instance. The target
 * retrieves the current CSIRX instance statistics through FVID2 and prints
 * them to the console.
 *
 * The test verifies that the command is accepted and processed successfully.
 * It does not execute the graph or validate the numerical values of the
 * reported statistics.
 */
TEST(tivxVideoIOCapture, testCapturePrintStatisticsCommand)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /*
    * Send the print-statistics control command to the Capture target kernel.
    * The command is expected to retrieve and print the current CSIRX instance
    * statistics successfully.
    */
    tivxNodeSendCommand(node, 0, TIVX_CAPTURE_PRINT_STATISTICS, NULL, 0);

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

typedef struct {
    const char* name;
    vx_bool is_null;
} Arg_CaptureGetStatisticsInvalidParameter;

#define CAPTURE_GET_STATISTICS_INVALID_PARAMETERS \
    CT_GENERATE_PARAMETERS("invalid_size", ARG, vx_false_e),   \
    CT_GENERATE_PARAMETERS("null_reference", ARG, vx_true_e)

/**
 * Verifies that the Capture get-statistics command rejects invalid parameters.
 *
 * The test covers two invalid parameter configurations:
 *
 * - A user data object whose size matches tivx_capture_params_t instead of
 *   the required tivx_capture_statistics_t. In this case,
 *   tivxCaptureGetStatistics() detects the invalid object size and returns
 *   VX_ERROR_INVALID_PARAMETERS.
 *
 * - A NULL statistics reference. In this case, tivxCaptureControl() detects
 *   that no user data object was provided and returns VX_FAILURE.
 *
 * The test verifies that TIVX_CAPTURE_GET_STATISTICS does not succeed when
 * the supplied statistics object is missing or has an invalid size.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCaptureGetStatisticsInvalidParameters, Arg_CaptureGetStatisticsInvalidParameter, CAPTURE_GET_STATISTICS_INVALID_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config, capture_stats_obj;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;
    vx_reference refs[1];

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    if (vx_false_e == arg_->is_null)
    {
        /**
         * For getting capture statistics, user data object should be created
         * as tivx_capture_statistics_t. But using unexpected structure  like
         * tivx_capture_params_t will cause detection of  the invalid object
         * size and return VX_ERROR_INVALID_PARAMETERS.
         */
        capture_stats_obj = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0, TIVX_CAPTURE_GET_STATISTICS, refs, 1u));
    }
    else
    {
        /**
         * For  getting capture  statistics, user data object  should be created on
         * tivx_capture_statistics_t.  
         *
         * In case User data object is NULL, tivxCaptureControl() detects
         * that no user data object was provided and returns VX_FAILURE.
         * User data object was NULL.
         */
        refs[0] = NULL;
        ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0, TIVX_CAPTURE_GET_STATISTICS, refs, 1u));
    }

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    if (vx_false_e == arg_->is_null)
    {
        VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));
    }
    tivxVideoIOUnLoadKernels(context);
}

typedef struct {
    const char* name;
    vx_bool is_null;
} Arg_CaptureRegisterErrorFrameParams;

#define CAPTURE_REGISTER_ERROR_FRAME_PARAMETERS \
    CT_GENERATE_PARAMETERS("valid",   ARG, vx_false_e),   \
    CT_GENERATE_PARAMETERS("nullptr", ARG, vx_true_e)

/**
 * Test capture error frame registration with parameterized scenarios.
 *
 * Tests error frame registration with valid frames and NULL references to
 * ensure proper error handling and timeout behavior when error frames are present.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCaptureRegisterErrorFrame, Arg_CaptureRegisterErrorFrameParams, CAPTURE_REGISTER_ERROR_FRAME_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;
    vx_reference refs[1];

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    params.timeout = 0;
    params.timeoutInitial = 0;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    if (vx_true_e == arg_->is_null)
    {
        refs[0] = NULL;
        /* Provided reference was NULL for Capture register error frame. Returned status is a failure. */
        ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0, TIVX_CAPTURE_REGISTER_ERROR_FRAME, refs, 1u));
    }
    else
    {
        /* Error frame has been sent and can use error timeout (Error timeout is only used if this error frame is sent). */
        refs[0] = (vx_reference)config;

        /* 
         * Calling tivxNodeSendCommand with TIVX_CAPTURE_REGISTER_ERROR_FRAME in order to set enableErrorFrameTimeout flag.
         * enableErrorFrameTimeout is a flag indicating if error frame has been sent and can use error timeout.
         * Error timeout is only used if this error frame is sent.
         */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node, 0, TIVX_CAPTURE_REGISTER_ERROR_FRAME, refs, 1u));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

        /*
         * Attempt to register the error frame again. The Capture target rejects
         * the request because an error frame has already been registered.
         */
        ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0, TIVX_CAPTURE_REGISTER_ERROR_FRAME, refs, 1u));
    }

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Verifies that the Capture target kernel rejects an unsupported control
 * command.
 *
 * The test first creates and verifies a valid Capture graph so that the
 * target kernel instance is initialized. It then sends an unsupported command
 * ID through tivxNodeSendCommand().
 *
 * In tivxCaptureControl(), the command does not match any supported Capture
 * control command and enters the default switch branch. The target kernel
 * prints "Invalid Command Id" and returns VX_FAILURE, which is propagated back
 * to the caller.
 */
TEST(tivxVideoIOCapture, testCaptureControlInvalidCommand)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /* 
     * Use a command ID that does not match any supported Capture control command
     * to force the default branch in tivxCaptureControl().
     *
     * Valid Capture Node Control Commands are
     *   TIVX_CAPTURE_PRINT_STATISTICS
     *   TIVX_CAPTURE_GET_STATISTICS
     *   TIVX_CAPTURE_REGISTER_ERROR_FRAME
     * 
     * Using invalid Capture Control Command will result in VX_FAILURE return status.
     */
    uint32_t invalid_command = 0x90000000U;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0, invalid_command, NULL, 0));

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Verifies that the Capture target rejects an unsupported CSI Rx instance ID.
 *
 * The test configures one Capture instance with an unsupported instance ID,
 * while mapping the Capture channel to CSI Rx instance 0. During graph
 * verification, tivxCaptureMapInstId() maps the unsupported instance ID to
 * 0xFFFFU. tivxCaptureGetDrvInstIndex() cannot match this value with the
 * instance requested by the channel, causing tivxCaptureSetCreateParams() and
 * vxVerifyGraph() to return failure.
 *
 * This test covers the default branch in tivxCaptureMapInstId().
 */
TEST(tivxVideoIOCapture, testCaptureInvalidInstanceId)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    /*
     * Initialize the Capture parameters and configure one unsupported CSI Rx
     * instance. The value 0xFFFFFFFFU does not match any supported instance ID
     * handled by tivxCaptureMapInstId().
     */
    params.numInst = 1U;
    params.numCh   = 1U;
    params.instId[0U] = 0xFFFFFFFF;
    for (uint32_t chIdx = 0U; chIdx < params.numCh; chIdx++)
    {
        params.chInstMap[chIdx] = 0U;
    }

    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    /*
     * Graph verification must fail because the configured instance ID cannot
     * be matched with the CSI Rx instance requested by the Capture channel.
     */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

typedef struct {
    const char* name;
    uint32_t pixel_container;   /*!< \brief Pixel Container, see \ref tivx_raw_image_pixel_container_e */
    uint32_t msb;               /*!< \brief Most significant bit in pixel container */
} Arg_CaptureRawImageParams;

/**
 * Reports whether an MSB value is supported for a given RAW pixel container.
 *
 * Mirrors the container/MSB combinations that the Capture target kernel maps
 * to a CSI-2 data type: RAW10/12/14/16 for a 16-bit container, RAW6/7/8 for an
 * 8-bit container, and RAW12 for a P12 packed container. Any other combination
 * yields CAPTURE_IN_CSI_DT_INVALID and must fail graph verification.
 */
static vx_bool captureIsValidMsbForContainer(uint32_t pixel_container, uint32_t msb)
{
    vx_bool is_valid = vx_false_e;

    if (TIVX_RAW_IMAGE_16_BIT == pixel_container)
    {
        if ((9U == msb) || (11U == msb) || (13U == msb) || (15U == msb))
        {
            is_valid = vx_true_e;
        }
    }
    else if (TIVX_RAW_IMAGE_8_BIT == pixel_container)
    {
        if ((5U == msb) || (6U == msb) || (7U == msb))
        {
            is_valid = vx_true_e;
        }
    }
    else if (TIVX_RAW_IMAGE_P12_BIT == pixel_container)
    {
        if (11U == msb)
        {
            is_valid = vx_true_e;
        }
    }
    else
    {
        /* Unsupported container type */
    }
    
    return is_valid;
}

#define INVALID_MSB 0
#define INVALID_PIXEL_CONTAINER 0

#define CAPTURE_RAW_IMAGE_PARAMETERS \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW10", ARG, TIVX_RAW_IMAGE_16_BIT, 9),               \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW12", ARG, TIVX_RAW_IMAGE_16_BIT, 11),              \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW14", ARG, TIVX_RAW_IMAGE_16_BIT, 13),              \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW16", ARG, TIVX_RAW_IMAGE_16_BIT, 15),              \
    CT_GENERATE_PARAMETERS("INVALID",             ARG, TIVX_RAW_IMAGE_16_BIT, INVALID_MSB),     \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW6",  ARG, TIVX_RAW_IMAGE_8_BIT, 5),                \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW7",  ARG, TIVX_RAW_IMAGE_8_BIT, 6),                \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW8",  ARG, TIVX_RAW_IMAGE_8_BIT, 7),                \
    CT_GENERATE_PARAMETERS("INVALID",             ARG, TIVX_RAW_IMAGE_8_BIT, INVALID_MSB),      \
    CT_GENERATE_PARAMETERS("FVID2_CSI2_DF_RAW12", ARG, TIVX_RAW_IMAGE_P12_BIT, 11),             \
    CT_GENERATE_PARAMETERS("INVALID_P12_MSB",     ARG, TIVX_RAW_IMAGE_P12_BIT, INVALID_MSB),    \
    CT_GENERATE_PARAMETERS("INVALID",             ARG, INVALID_PIXEL_CONTAINER, INVALID_MSB)

/**
 * Verifies Capture graph validation for supported and unsupported RAW image
 * data-type configurations.
 *
 * The test creates a Capture graph using different RAW image pixel containers
 * and most-significant-bit values. 
 * Supported combinations are expected to produce a valid CSI-2 data type and
 * allow vxVerifyGraph() to succeed. Unsupported combinations return
 * CAPTURE_IN_CSI_DT_INVALID, causing tivxCaptureSetCreateParams() to report
 * "Un-supported Capture Data-type" and graph verification to fail.
 *
 * This test validates RAW image format configuration during graph
 * verification. It does not execute the graph or validate captured image
 * data.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCaptureSupportedRawImageFormats, Arg_CaptureRawImageParams, CAPTURE_RAW_IMAGE_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_object_array obj_arr;
    tivx_raw_image raw_image = 0;
    vx_node node;
    vx_user_data_object config;
    tivx_capture_params_t params;
    tivx_raw_image_create_params_t raw_params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an tivx_raw_image */
    memset(&raw_params, 0, sizeof(raw_params));
    raw_params.width  = 1920U;
    raw_params.height = 1080U;
    raw_params.num_exposures = 1U;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = arg_->pixel_container;
    raw_params.format[0].msb = arg_->msb;
    raw_params.meta_height_before = 0U;
    raw_params.meta_height_after  = 0U;
    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)raw_image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Create Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    if (captureIsValidMsbForContainer(arg_->pixel_container, arg_->msb))
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }
    else
    {
        /* Un-supported Capture Data-type */
        ASSERT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));
    }

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    tivxReleaseRawImage(&raw_image);
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

#if (TIVX_CAPTURE_MAX_INST >= 2U)
typedef struct {
    const char* name;
    vx_bool use_invalid_instance_configuration;
} Arg_CsirxInstanceParams;

#define CSIRX_PARAMETERS \
    CT_GENERATE_PARAMETERS("valid_instance", ARG, vx_false_e), \
    CT_GENERATE_PARAMETERS("missing_instance", ARG, vx_true_e)

/**
 * Verifies CSI Rx instance mapping during Capture graph verification.
 *
 * The test maps every Capture channel to CSI Rx instance 1 and covers two
 * configurations.
 *
 * In the valid configuration, all supported CSI Rx instances are enabled.
 * During target-kernel initialization, tivxCaptureGetDrvInstIndex() searches
 * the configured instance list, finds instance 1, and graph verification
 * succeeds.
 *
 * In the invalid configuration, params.numInst is set to zero while the
 * Capture channels still request CSI Rx instance 1. Since no instances are
 * configured, tivxCaptureGetDrvInstIndex() cannot resolve the requested
 * instance. tivxCaptureSetCreateParams() detects that the returned instance
 * index is greater than or equal to prms->numOfInstUsed, prints
 * "Wrong Instance ID provided", and causes vxVerifyGraph() to fail.
 *
 * This test validates CSI Rx instance mapping and error handling during graph
 * verification. It does not execute the graph or perform frame capture.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCaptureCsiRxInstanceMapping, Arg_CsirxInstanceParams, CSIRX_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    /*
     * Map every Capture channel to CSI Rx instance 1. In the valid scenario,
     * tivxCaptureGetDrvInstIndex() must iterate through the configured instance
     * list before finding the requested instance.
     */
    for (uint32_t i = 0U; i < NUM_CHANNELS; i++)
    {
        params.chInstMap[i] = TIVX_CAPTURE_INST_ID_1;
    }
    if (vx_false_e == arg_->use_invalid_instance_configuration)
    {    
        /*
         * Configure all supported CSI Rx instances so that instance 1 can be
         * resolved successfully during Capture target initialization.
         */
        params.numInst = TIVX_CAPTURE_MAX_INST;
    }
    else if (vx_true_e == arg_->use_invalid_instance_configuration)
    {    
        /*
         * Configure zero CSI Rx instances while all channels still request
         * instance 1. This forces the "Wrong Instance ID provided" failure in
         * tivxCaptureSetCreateParams().
         */
        params.numInst = 0;
    }
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Create Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    if (vx_false_e == arg_-> use_invalid_instance_configuration)
    { 
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }
    else if (vx_true_e == arg_-> use_invalid_instance_configuration)
    {
        ASSERT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));
    }

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}
#endif

/**
 * Verifies simultaneous frame capture from two CSI Rx instances.
 *
 * The test initializes sensors connected to CSI Rx instance 0 and CSI Rx
 * instance 1, then creates a Capture node with two output channels. Channel 0
 * is mapped to virtual channel 0 on CSI Rx instance 0, while channel 1 is
 * mapped to virtual channel 0 on CSI Rx instance 1.
 *
 * The graph is configured for queue-based automatic scheduling with multiple
 * output buffers. After the pipeline is primed, completed object arrays are
 * dequeued and returned to the graph so that the buffers can be continuously
 * reused for frame capture.
 *
 * Successful graph verification and buffer processing confirm that the
 * Capture target can configure and operate two CSI Rx driver instances within
 * a single Capture node.
 *
 * This test requires sensors to be available on both configured hardware
 * ports. If the second sensor is unavailable, the dual-instance scenario is
 * not exercised.
 */
TEST(tivxVideoIOCapture, testCaptureTwoCsiRxInstances)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node  node  = NULL;

    /* Data objects for graph */
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    vx_image image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = 2U;
    uint32_t buf_id, loop_id, loop_cnt = 5U, num_buf = 3U, instIdx, out_num_refs;
    vx_bool done;
    vx_object_array dequeue_capture_array;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    char  availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];

    vx_uint8 num_sensors_found;
    IssSensor_CreateParams sensorParams;
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Init for test case */
    {
        tivxVideoIOLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
        tivx_clr_debug_zone(VX_ZONE_INFO);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    }

    /* Sensor Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for (uint8_t i = 0; i < ISS_SENSORS_MAX_SUPPORTED_SENSOR; i++)
        {
            sensor_list[i] = availableSensorNames[i];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);
        
        memset(&sensorParams, 0, sizeof(sensorParams));

        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));
        
        sensor_features_supported = sensorParams.sensorInfo.features;
        sensor_features_enabled   = initSensorParams(sensor_features_supported);

        /*
         * Initialize sensors connected to hardware channels SENSOR_CHANNEL_MASK and channel 4.
         * The sensor API expects a channel bitmask, so (1U << 4U) selects the fourth 
         * channel.
         */
        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));

        int32_t init_status;
        init_status = appInitImageSensor(sensor_name, sensor_features_enabled, 1U << 4U);
        if (init_status != 0)
        {
            printf("Sensor not available on port 4, skipping initialization\n");
            num_capture_channels = 1U;
        }
    }

    /* Creating objects for graph */
    {
        ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
        }

        /*
         * Configuration for dual-instance capture.
         * Map each capture channel to a specific hardware CSI instance.
         */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.numInst = 2U;       // Enable two hardware CSI instances
        local_capture_config.numCh   = num_capture_channels;
        for (instIdx = 0U ; instIdx < local_capture_config.numInst; instIdx++)
        {
            local_capture_config.instId[instIdx]                       = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
        }
        // Map Channel 0 to the virtual channel of the SENSOR_CHANNEL_MASK port on CSI Instance 0
        local_capture_config.chVcNum[0]   = captureGetChVcNum(0U);
        local_capture_config.chInstMap[0] = 0;
        // Map Channel 1 to Virtual Channel 0 on CSI Instance 1
        local_capture_config.chVcNum[1]   = 0;
        local_capture_config.chInstMap[1] = 1U;

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);
    }

    /* Pipelining and graph verification */
    {
        add_graph_parameter_by_node_index(graph, node, 1);

        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size        = num_buf;
        graph_parameters_queue_params_list[0].refs_list             = (vx_reference*)&capture_frames[0];

        /* 
        * Set the graph to AUTO schedule mode. 
        * In this mode, the graph execution is triggered automatically 
        * as soon as a new buffer (reference) is enqueued into the parameter queue.
        */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }

    /* Starting two sensors */
    VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));
    {
        int32_t start_status = appStartImageSensor(sensor_name, 1U << 4U);
        if(start_status != 0)
        {
            printf("Sensor not available on port 4, skipping start\n");
        }
    }

    /* Enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id = 0; buf_id < num_buf-1; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* After pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    /*
    * Continuously recycle Capture output buffers.
    *
    * vxGraphParameterDequeueDoneRef() blocks until the graph returns an object
    * array containing newly captured frames. The same object array is then
    * returned to the ready queue with vxGraphParameterEnqueueReadyRef(), allowing
    * it to be reused for subsequent captures.
    *
    * The additional num_buf iterations exercise the initially primed pipeline
    * together with the requested loop_cnt steady-state capture iterations.
    */
    for(loop_id = 0; loop_id < (loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));
    {
        int32_t stop_status = appStopImageSensor(sensor_name, 1U<<4U);
        if(stop_status != 0)
        {
            printf("Sensor not available on port 4, skipping stop\n");
        }
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));

    for(buf_id = 0; buf_id < num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    tivxVideoIOUnLoadKernels(context);
    appDeInitImageSensor(sensor_name);
    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/**
 * Exercises Capture processing when no error frame is registered.
 *
 * The test configures finite timeout and timeoutInitial values, but does not
 * register an error frame through tivxCaptureRegisterErrorFrame(). Therefore,
 * enableErrorFrameTimeout remains disabled in the Capture target kernel.
 *
 * During graph processing, the Capture node executes the
 * no-error-frame path in tivxCaptureSetTimeout(). The warning message is not
 * explicitly verified by this test.
 *
 * A physical image sensor is initialized and started before vxProcessGraph()
 * so that a frame is eventually received and the infinite event wait can
 * complete successfully.
 *
 * This test exercises the no-error-frame path and verifies successful graph
 * processing without a registered error frame. It does not verify finite
 * timeout expiration, inactive-channel handling, or error-frame replacement behavior.
 */
TEST(tivxVideoIOCapture, testErrorFrameTimeout)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = 0;
    vx_node node = NULL;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    char  availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];

    vx_uint8 num_sensors_found;
    IssSensor_CreateParams sensorParams;
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* 
     * Sensor Initialization 
     *  
     * Initialize a real image sensor. A running sensor is required to
     * eventually generate a frame and release the blocking event
     * wait inside tivxCaptureTimeout().
     */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for (uint8_t i = 0; i < ISS_SENSORS_MAX_SUPPORTED_SENSOR; i++)
        {
            sensor_list[i] = availableSensorNames[i];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);
        
        memset(&sensorParams, 0, sizeof(sensorParams));

        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));
        
        sensor_features_supported = sensorParams.sensorInfo.features;
        sensor_features_enabled   = initSensorParams(sensor_features_supported);

        /*
        * Initialize the sensor connected to hardware channel 0. The value 1U is a
        * channel bitmask with bit 0 set.
        */
        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
    }

    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;

    /* Map each capture channel onto the virtual channel of its FPD-Link port. */
    for (uint32_t ch_idx = 0U; ch_idx < NUM_CHANNELS; ch_idx++)
    {
        params.chVcNum[ch_idx] = captureGetChVcNum(ch_idx);
    }
    
    /* 
     * Setting finite timeout and timeoutInitial (!VX_TIMEOUT_WAIT_FOREVER) in order to force
     * printing warning message in case of defaulting to waiting forever. 
     *
     * Configure finite timeout values, but intentionally do not register an error frame.
     * 
     * Since tivxCaptureRegisterErrorFrame() is never called,
     * enableErrorFrameTimeout remains disabled. During graph processing,
     * this exercises the no-error-frame path in tivxCaptureSetTimeout().
     */
    params.timeout        = 1000U;
    params.timeoutInitial = 1000U;

    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /*
     * Start the sensor before processing the graph. Without an incoming frame,
     * vxProcessGraph() could remain blocked because the Capture target has fallen
     * back to VX_TIMEOUT_WAIT_FOREVER.
     */
    VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    /*
     * Process the graph to execute tivxCaptureSetTimeout(). The running sensor
     * provides a frame so that the VX_TIMEOUT_WAIT_FOREVER event wait completes.
     */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    appDeInitImageSensor(sensor_name);
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}

/**
 * Test capture kernel resource management across repeated create/process/delete cycles.
 *
 * Validates that the capture kernel correctly releases all resources on deletion
 * by repeatedly creating, processing, and deleting the capture node without
 * resource leak indications.
 */
TEST(tivxVideoIOCapture, testCaptureCreateProcessDelete)
{
    vx_context context = context_->vx_context_;
    uint32_t loop_id, loop_cnt = 10U;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    uint32_t num_capture_channels = NUM_CHANNELS;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    for (loop_id = 0; loop_id < loop_cnt; loop_id++)
    {
        vx_graph graph = NULL;
        vx_node node = NULL;
        vx_user_data_object config = NULL;
        vx_object_array obj_arr = NULL;
        vx_image image = NULL;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        /* Create input for Capture Node - User Data Object of a single capture params structure */
        tivx_capture_params_t params;
        tivx_capture_params_init(&params);
        params.numInst = 1U;
        params.numCh   = NUM_CHANNELS;

        /* Map each capture channel onto the virtual channel of its FPD-Link port. */
        for (uint32_t ch_idx = 0U; ch_idx < NUM_CHANNELS; ch_idx++)
        {
            params.chVcNum[ch_idx] = captureGetChVcNum(ch_idx);
        }
        ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        
        /* Sensor Initialization */
        {
            memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
            for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
            {
                sensor_list[count] = availableSensorNames[count];
            }
            VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

            sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
            ASSERT(NULL != sensor_name);

            memset(&sensorParams, 0, sizeof(sensorParams));
            VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

            sensor_features_supported = sensorParams.sensorInfo.features;

            sensor_features_enabled = initSensorParams(sensor_features_supported);

            VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
        }

        /* Create output for Capture Node - Object array output which has been created from an vx_image */
        ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        /* Creates a camera Capture Node */
        ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

        VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&image));
        appDeInitImageSensor(sensor_name);
        VX_CALL(vxReleaseObjectArray(&obj_arr));
        VX_CALL(vxReleaseUserDataObject(&config));
    }

    tivxVideoIOUnLoadKernels(context);
}

/**
 * Verifies captured frame content using CRC-based golden-reference
 * validation.
 *
 * The test configures the UB9xxx RAW12 test-pattern sensor and creates
 * a Capture graph with either raw-image or U16 image output, depending
 * on the test argument.
 *
 * The first channel frame is mapped to host memory. CRC is calculated
 * over valid image data, excluding any padding between image rows.
 *
 * Initial pipeline frames are skipped to allow the Capture  stream to
 * stabilize. The remaining CRC values are compared against the golden
 * CRC corresponding to the selected output type.
 */
TEST_WITH_ARG(tivxVideoIOCapture, testCompareCapturedFrameToGoldenReference, Arg_Capture, CAPTURE_PARAMETERS)
{
    /* Capture graph and node objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;

    /* Capture configuration input and output data objects */
    vx_object_array capture_output_frames[MAX_NUM_BUF];
    vx_user_data_object capture_input_config;
    tivx_capture_params_t params;
    tivx_raw_image raw_image = 0;
    vx_image image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt = arg_->loop_cnt, num_buf = 3, instIdx, out_num_refs;
    vx_object_array dequeue_capture_array;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Init for test case */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Sensor Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, UB9XX_RAW_TESTPAT);
        ASSERT(NULL != sensor_name);

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, TESTPATTERN_CHANNEL_MASK));
    }

    /* Creating input and output objects for Capture Node */
    {
        /* Allocate output refs, multiple refs created to allow pipelining of graph */
        if (0 == arg_->raw_capture)
        {
            ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            if (0 == arg_->raw_capture)
            {
                ASSERT_VX_OBJECT(capture_output_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
            else
            {
                ASSERT_VX_OBJECT(capture_output_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
        }

        /* Allocate input ref */
        tivx_capture_params_init(&params);
        params.numInst = 1U;
        params.numCh   = num_capture_channels;
        uint32_t chIdx = 0U;

        for (instIdx = 0U ; instIdx < params.numInst; instIdx++)
        {
            params.instId[instIdx] = instIdx;
            params.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            params.instCfg[instIdx].numDataLanes         = 4U;

            for (loop_id = 0U; loop_id < params.instCfg[0U].numDataLanes ; loop_id++)
            {
                params.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                params.chVcNum[chIdx]   = TESTPATTERN_VC_NUM;
                params.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }
        ASSERT_VX_OBJECT(capture_input_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, capture_input_config, capture_output_frames[0]), VX_TYPE_NODE);
    }

    /* Pipelining and graph verification */
    {
        add_graph_parameter_by_node_index(graph, node, 1);

        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_output_frames[0];

        /* 
         * Set the graph to AUTO schedule mode. 
         * In this mode, the graph execution is triggered automatically 
         * as soon as a new buffer (reference) is enqueued into the parameter queue.
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }

    /* Starting sensor */
    VX_CALL(appStartImageSensor(sensor_name, TESTPATTERN_CHANNEL_MASK));

    /* Enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_output_frames[buf_id], 1);
    }

    /* After pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_output_frames[buf_id], 1);

    /*
     * Continuously recycle Capture output buffer.
     *
     * vxGraphParameterDequeueDoneRef blocks until the graph returns an object
     * array containing newly captured frame. The same object array is then
     * returned to the ready queue with vxGraphParameterEnqueueReadyRef,
     * allowing it to be reused for subsequent captures.
     *
     * The additional num_buf iterations exercise initially primed pipeline
     * together with the requested loop_cnt steady-state capture iterations.
     */
    for(loop_id = 0; loop_id < (loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array capture_output_array;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_output_array, 1, &num_refs);

        if (0 == arg_->raw_capture)
        {
            tivx_raw_image capture_output_frame;

            /* Retrieves the reference to the image in location index of the ObjectArray */
            ASSERT_VX_OBJECT(capture_output_frame = (tivx_raw_image) vxGetObjectArrayItem(capture_output_array, 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

            /*
             * Map the entire image buffer to host memory to ensure
             * the capture frame buffer is allocated and accessible. 
             */
            vx_map_id map_id;
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            tivx_raw_image_format_t raw_format[TIVX_RAW_IMAGE_MAX_EXPOSURES];
            
            VX_CALL(tivxQueryRawImage((tivx_raw_image)capture_output_frame, TIVX_RAW_IMAGE_WIDTH,  &img_width,  sizeof(vx_uint32)));
            VX_CALL(tivxQueryRawImage((tivx_raw_image)capture_output_frame, TIVX_RAW_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32)));
            VX_CALL(tivxQueryRawImage((tivx_raw_image)capture_output_frame, TIVX_RAW_IMAGE_FORMAT, raw_format,  sizeof(raw_format)));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x   = img_width;
            rect.end_y   = img_height;

            vx_status status = tivxMapRawImagePatch((tivx_raw_image)capture_output_frame, &rect, 0U, &map_id, &image_addr, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER);

            /* CRC */
            if ((vx_status)VX_SUCCESS == status)
            {
                /*
                 * Calculate CRC over img_width x img_height valid pixels.
                 */
                vx_uint32 calculated_crc = captureCalculateRawImageCrc(data_ptr, img_width, img_height, &image_addr, raw_format[0U].pixel_container);

                tivxUnmapRawImagePatch((tivx_raw_image)capture_output_frame, map_id);

                /* Compare calculated CRC with golden CRC */
                if (loop_id >= num_buf)
                {
                    ASSERT_EQ_INT(golden_crc_raw_img, calculated_crc);

                    tivx_set_debug_zone(VX_ZONE_INFO);
                    VX_PRINT(VX_ZONE_INFO, "\nRaw frame %u has CRC: 0x%08X\n", (unsigned int)loop_id, (unsigned int)calculated_crc);
                    tivx_clr_debug_zone(VX_ZONE_INFO);
                }
            }
            /* CRC */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate capture frame\n");
            }

            VX_CALL(tivxReleaseRawImage(&capture_output_frame));
        }
        else
        {
            vx_image capture_output_frame;

            /* Retrieves the reference to the image in location index of the ObjectArray */
            ASSERT_VX_OBJECT(capture_output_frame = (vx_image) vxGetObjectArrayItem(capture_output_array, 0), VX_TYPE_IMAGE);

            /*
             * Map the entire image buffer to host memory to ensure
             * the capture frame buffer is allocated and accessible. 
             */
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;

            vxQueryImage((vx_image)capture_output_frame, VX_IMAGE_WIDTH,  &img_width,  sizeof(vx_uint32));
            vxQueryImage((vx_image)capture_output_frame, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x   = img_width;
            rect.end_y   = img_height;

            vx_status status = vxMapImagePatch((vx_image)capture_output_frame, &rect, 0, &map_id, &image_addr, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

            /* CRC */
            if ((vx_status)VX_SUCCESS == status)
            {
                /*
                 * Calculate CRC over img_width x img_height valid U16 pixels.
                 * Padding bytes introduced by stride_y are not included.
                 */
                vx_uint32 calculated_crc = captureCalculateImageCrcU16(data_ptr, img_width, img_height, &image_addr);

                vxUnmapImagePatch(capture_output_frame, map_id);

                /* Compare calculated CRC with golden CRC */
                if (loop_id >= num_buf)
                {
                    ASSERT_EQ_INT(golden_crc_img, calculated_crc);

                    tivx_set_debug_zone(VX_ZONE_INFO);
                    VX_PRINT(VX_ZONE_INFO, "\nFrame %u has CRC: 0x%08X\n", (unsigned int)loop_id, (unsigned int)calculated_crc);
                    tivx_clr_debug_zone(VX_ZONE_INFO);
                }
            }
            /* CRC */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate capture frame\n");
            }

            VX_CALL(vxReleaseImage(&capture_output_frame));
        }
        
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_output_array, 1);
    }

    /* Ensure all graph processing is complete */
    vxWaitGraph(graph);

    /* Dequeue all buffers */
    vx_bool done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    VX_CALL(appStopImageSensor(sensor_name, TESTPATTERN_CHANNEL_MASK));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    if (0 == arg_->raw_capture)
    {
        VX_CALL(tivxReleaseRawImage(&raw_image));
    }
    else
    {
        VX_CALL(vxReleaseImage(&image));
    }
    for(buf_id = 0; buf_id < num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_output_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_input_config));
    tivxVideoIOUnLoadKernels(context);
    appDeInitImageSensor(sensor_name);
}

TESTCASE(tivxVideoIOCaptureAddition, CT_VXContext, ct_setup_vx_context, 0)

/**
 * Verifies handling of an object descriptor allocation failure during
 * Capture error-frame registration.
 *
 * The Capture graph is created and verified before the remaining entries in
 * the shared object descriptor table are reserved by the test. The subsequent
 * TIVX_CAPTURE_REGISTER_ERROR_FRAME command reaches
 * tivxCaptureAllocErrorDesc(), where ownObjDescAlloc() is expected to return
 * NULL because no free descriptor entry remains.
 *
 * The test verifies that the allocation failure is detected and propagated
 * to the caller as VX_FAILURE.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureErrorDescriptorAllocationFailure)
{
    /* OpenVX context provided by the test framework. */
    vx_context context = context_->vx_context_;

    /* Objects required to construct the Capture graph. */
    vx_graph graph;
    vx_user_data_object config;
    tivx_capture_params_t params;
    vx_object_array obj_arr;
    vx_image image = NULL;
    vx_node node = NULL;
    vx_reference refs[1];

    /*
     * Internal descriptors used to reserve all entries that remain available
     * after the Capture graph has been verified.
     */
    tivx_obj_desc_t *reserved_obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};

    uint32_t num_reserved_obj_desc = 0U;
    uint32_t desc_idx;
    vx_status command_status;
    vx_status free_status;

    /* Load the Video I/O kernels and register their cleanup callback. */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    /* Create the graph containing the Capture node. */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create the Capture configuration. */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create the Capture output image and object array. */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Create the Capture node and assign it to the Capture target. */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    /*
     * Verify the graph before exhausting the descriptor table so that all
     * descriptors required for creating the Capture instance are available.
     */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /*
     * Reserve every remaining entry in the shared object descriptor table.
     * ownObjDescAlloc() returns NULL when no free entry remains.
     */
    while (num_reserved_obj_desc < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST)
    {
        reserved_obj_desc[num_reserved_obj_desc] = ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, NULL);

        if (NULL == reserved_obj_desc[num_reserved_obj_desc])
        {
            break;
        }

        num_reserved_obj_desc++;
    }

    /*
     * The loop must stop because descriptor allocation failed, rather than
     * because the local storage array limit was reached.
     */
    ASSERT(num_reserved_obj_desc < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST);

    refs[0] = (vx_reference)image;

    /*
     * Error-frame registration must fail because
     * tivxCaptureAllocErrorDesc() cannot allocate an internal descriptor.
     */
    command_status = tivxNodeSendCommand(node, 0U, TIVX_CAPTURE_REGISTER_ERROR_FRAME, refs, 1U);

    /*
     * Release all descriptors reserved by the test before checking the
     * command result, ensuring that a failed assertion does not leave the
     * shared descriptor table exhausted.
     */
    for (desc_idx = 0U; desc_idx < num_reserved_obj_desc; desc_idx++)
    {
        free_status = ownObjDescFree(&reserved_obj_desc[desc_idx]);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, free_status);
    }

    /* Verify propagation of the target-side allocation failure. */
    ASSERT_EQ_VX_STATUS( VX_FAILURE, command_status);

    /* Release all OpenVX resources created by the test. */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));

    tivxVideoIOUnLoadKernels(context);
}

/**
 * Exercises a Capture node that owns two CSI Rx instances while every channel is
 * mapped to instance 0, leaving the second instance created and started but idle.
 *
 * tivx_capture_params_init() assigns instId[loopCnt] = loopCnt and maps every
 * channel to instance 0 (chInstMap[cnt] = 0U), so setting numInst = 2 is
 * sufficient to obtain instance 0 serving all NUM_CHANNELS channels and instance
 * 1 serving none. In the Capture target kernel this makes prms->numOfInstUsed
 * equal 2 while prms->instParams[1].numCh stays 0, which deterministically
 * exercises the following second-instance paths:
 *
 *  - tivxCaptureGetChannelIndices() and tivxCaptureGetNodeChannelNum() are
 *    called with instId = 1, so their instance loops execute one non-matching
 *    iteration before the instIdx == instId break.
 *  - tivxCaptureEnqueueFrameToDriver() obtains startChIdx == endChIdx for
 *    instance 1, so its per-channel loop performs no iterations,
 *    frmList->numFrames stays 0, and the false outcome of the
 *    "frmList->numFrames > 0U" check is taken without calling Fvid2_queue().
 *  - tivxCaptureDequeueFrameFromDriver() queries instance 1, which is running
 *    but owns no channel and therefore has no frame available, so Csirx returns
 *    FVID2_EAGAIN and the FVID2_EAGAIN check in the final else block takes its
 *    false outcome without accumulating an error.
 *  - tivxCaptureStart(), tivxCapturePrintStatus() and tivxCaptureDelete()
 *    iterate over two Capture instances instead of one.
 *
 * testCaptureTwoCsiRxInstances() reaches an equivalent configuration only when
 * no sensor is present on port 4, because it maps channel 1 to instance 1 and
 * falls back to a single channel otherwise. This test produces the idle-instance
 * configuration unconditionally.
 *
 * A physical sensor is initialized and started on instance 0 so that
 * vxProcessGraph() receives a frame and completes. A single vxProcessGraph()
 * call covers the pipe-up invocations of tivxCaptureProcess() followed by the
 * steady-state invocation, so the idle-instance enqueue path is executed for
 * both node states and the idle-instance dequeue path for the steady state.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureIdleCsiRxInstance)
{
#if (TIVX_CAPTURE_MAX_INST >= 2U)
    /* OpenVX context provided by the test framework. */
    vx_context context = context_->vx_context_;

    /* Objects required to construct the Capture graph. */
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_user_data_object config = NULL;
    vx_object_array obj_arr = NULL;
    vx_image image = NULL;
    tivx_capture_params_t params;

    /* Sensor objects. */
    char *sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    vx_uint8 num_sensors_found, count;
    IssSensor_CreateParams sensorParams;
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Load the Video I/O kernels and register their cleanup callback. */
    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /*
     * Request two CSI Rx instances. Every channel keeps the chInstMap value of 0
     * assigned by tivx_capture_params_init(), so instance 1 is created and
     * started without owning any channel.
     */
    tivx_capture_params_init(&params);
    params.numInst = 2U;
    params.numCh   = NUM_CHANNELS;

    /* Map each capture channel onto the virtual channel of its FPD-Link port. */
    for (uint32_t ch_idx = 0U; ch_idx < NUM_CHANNELS; ch_idx++)
    {
        params.chVcNum[ch_idx] = captureGetChVcNum(ch_idx);
    }
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Sensor initialization for the channels served by instance 0. */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for (count = 0; count < ISS_SENSORS_MAX_SUPPORTED_SENSOR; count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, SENSOR_NAME);
        ASSERT(NULL != sensor_name);

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;
        sensor_features_enabled   = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, SENSOR_CHANNEL_MASK));
    }

    /* Create output for Capture Node - Object array output created from a vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    /* tivxCaptureCreate() must succeed for both Capture instances. */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(appStartImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    VX_CALL(appStopImageSensor(sensor_name, SENSOR_CHANNEL_MASK));

    /* Releasing the node runs tivxCaptureDelete() for both Capture instances. */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    appDeInitImageSensor(sensor_name);
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
#endif
}

#define CAPTURE_UNSUPPORTED_IMAGE_FORMATS \
    CT_GENERATE_PARAMETERS("RGB",  ARG, VX_DF_IMAGE_RGB),  \
    CT_GENERATE_PARAMETERS("NV12", ARG, VX_DF_IMAGE_NV12), \
    CT_GENERATE_PARAMETERS("NV21", ARG, VX_DF_IMAGE_NV21)

/**
 * Verifies that the Capture node rejects unsupported VX_TYPE_IMAGE formats.
 *
 * During graph verification, the Capture host kernel validates the output image
 * format and does not accept unsupported image types (VX_DF_IMAGE_RGB, VX_DF_IMAGE_NV12, VX_DF_IMAGE_NV21, ...)
 * and returns VX_ERROR_INVALID_PARAMETERS during vxVerifyGraph().
 *
 * This test validates format compatibility during graph verification. It does
 * not execute the graph.
 */
TEST_WITH_ARG(tivxVideoIOCaptureAddition, testCaptureUnsupportedImageFormats, Arg_CaptureImageFormat, CAPTURE_UNSUPPORTED_IMAGE_FORMATS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array obj_arr;
    vx_user_data_object config;
    vx_image image = 0;
    tivx_capture_params_t params;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Create input for Capture Node - User Data Object of a single capture params structure */
    tivx_capture_params_init(&params);
    params.numInst = 1U;
    params.numCh   = NUM_CHANNELS;
    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Create output for Capture Node - Object array output which has been created from an vx_image */
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920U, 1080U, arg_->format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);

    /* Creates a camera Capture Node */
    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, obj_arr), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    /* Expected to fail because image format is invalid */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    /* Releasing */
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleaseUserDataObject(&config));
    tivxVideoIOUnLoadKernels(context);
}



#define APP_EVENT_CAPTURE_NODE_ERROR    (1U)

typedef enum
{
    TIVX_TEST_CAPTURE_ERR_SCENARIO_SHORT_FRAME = 0,
    TIVX_TEST_CAPTURE_ERR_SCENARIO_LONG_FRAME,
    TIVX_TEST_CAPTURE_ERR_SCENARIO_HEADER_ECC,
    TIVX_TEST_CAPTURE_ERR_SCENARIO_ASF
} tivx_test_capture_err_scenario_e;

/**
 * Shared implementation for the capture node error event handling tests.
 *
 * Creates a capture graph, injects one specific fault (per 'scenario') and
 * monitors for VX_EVENT_NODE_ERROR events from the capture node, decoding any
 * errors that occur (CSI instance-specific error information).
 *
 * Scenario notes:
 * - SHORT_FRAME: raw_params.height is nudged +32 vs the sensor's real
 *   resolution, so the configured frame is shorter than what actually
 *   arrives -- TIVX_CAPTURE_ERR_SHORT_FRAME expected.
 * - LONG_FRAME: raw_params.height is nudged -32 vs the sensor's real
 *   resolution, so the configured frame is longer than what actually
 *   arrives -- TIVX_CAPTURE_ERR_LONG_FRAME expected. Confirmed end-to-end
 *   on hardware (this is what led to finding and fixing the UDMA TR
 *   short/long-packet info-bit extraction bug in csirx_drvUdma.c).
 * - HEADER_ECC: numDataLanes is set to 3 while the sensor's serdes keeps
 *   driving all 4 physical lanes, corrupting CSI2 packet headers -- confirmed
 *   to produce TIVX_CAPTURE_ERR_HEADER_ECC end-to-end.
 * - ASF: uses the TIVX_CAPTURE_INJECT_ASF_ERR node command (a self-contained
 *   software fault injection via IOCTL_CSIRX_TRIG_ASF_EVENT, not a physical
 *   fault) to trigger an ASF ESM event.
 */
static void testCaptureErrorScenarioRun(vx_context context, tivx_test_capture_err_scenario_e scenario)
{
    vx_graph graph;
    vx_node n0;

    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_raw_image raw_image = 0;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = 1U;
    uint32_t buf_id, loop_id, num_buf, instIdx, chIdx, out_num_refs;
    vx_bool done;
    vx_object_array dequeue_capture_array;
    uint32_t real_frame_count = 0U;
    uint32_t poll_without_frame_count = 0U;
    vx_bool asf_error_injected = vx_false_e;
    vx_bool expected_error_seen = vx_false_e;
    uint16_t expected_error_bit;

    if (TIVX_TEST_CAPTURE_ERR_SCENARIO_SHORT_FRAME == scenario)
    {
        expected_error_bit = TIVX_CAPTURE_ERR_SHORT_FRAME;
    }
    else if (TIVX_TEST_CAPTURE_ERR_SCENARIO_LONG_FRAME == scenario)
    {
        expected_error_bit = TIVX_CAPTURE_ERR_LONG_FRAME;
    }
    else if (TIVX_TEST_CAPTURE_ERR_SCENARIO_HEADER_ECC == scenario)
    {
        expected_error_bit = TIVX_CAPTURE_ERR_HEADER_ECC;
    }
    else
    {
        expected_error_bit = (TIVX_CAPTURE_ERR_ASF_TRANS_TIMEOUT | TIVX_CAPTURE_ERR_ASF_CSR_PARITY | TIVX_CAPTURE_ERR_ASF_DAP_PARITY);
    }

    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    num_buf = 3;

    tivxVideoIOLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_video_io_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
    for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
    {
        sensor_list[count] = availableSensorNames[count];
    }
    VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

    sensor_name = captureFindSensorByName(sensor_list, num_sensors_found, UB9XX_RAW_TESTPAT);
    ASSERT(NULL != sensor_name);

    memset(&sensorParams, 0, sizeof(sensorParams));
    VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

    sensor_features_supported = sensorParams.sensorInfo.features;
    sensor_features_enabled = initSensorParams(sensor_features_supported);

    VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, TESTPATTERN_CHANNEL_MASK));

    if (TIVX_TEST_CAPTURE_ERR_SCENARIO_SHORT_FRAME == scenario)
    {
        /* Configured frame shorter than what the sensor actually sends ->
         * the buffer's row budget runs out before the real frame ends. */
        sensorParams.sensorInfo.raw_params.height = sensorParams.sensorInfo.raw_params.height + 32;
    }
    else if (TIVX_TEST_CAPTURE_ERR_SCENARIO_LONG_FRAME == scenario)
    {
        /* Configured frame longer than what the sensor actually sends ->
         * the real frame ends before the buffer's row budget runs out. */
        sensorParams.sensorInfo.raw_params.height = sensorParams.sensorInfo.raw_params.height - 32;
    }

    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
    }

    tivx_capture_params_init(&local_capture_config);
    local_capture_config.timeout                         = 90;
    local_capture_config.timeoutInitial                  = 500;
    local_capture_config.numInst                         = 1U;
    local_capture_config.numCh                           = num_capture_channels; // 2
    local_capture_config.instId[0]                       = 0;
    local_capture_config.instCfg[0].enableCsiv2p0Support = (uint32_t)vx_true_e;
    local_capture_config.instCfg[0].numDataLanes         =
        (TIVX_TEST_CAPTURE_ERR_SCENARIO_HEADER_ECC == scenario) ? 3U : 4U;

    for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
    {
        local_capture_config.instCfg[0].dataLanesMap[loop_id] = (loop_id + 1u);
    }

    local_capture_config.chVcNum[0]   = TESTPATTERN_VC_NUM;
    local_capture_config.chInstMap[0] = 0;

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    VX_CALL(vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_ERROR, 0, APP_EVENT_CAPTURE_NODE_ERROR));

    add_graph_parameter_by_node_index(graph, n0, 1);

    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    tivx_raw_image error_frame;
    ASSERT_VX_OBJECT(error_frame = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(n0, (vx_reference)error_frame));

    VX_CALL(appStartImageSensor(sensor_name, TESTPATTERN_CHANNEL_MASK));

    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    for(loop_id=0; (real_frame_count<(40U+num_buf)) && (poll_without_frame_count<400U); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
        if ((TIVX_TEST_CAPTURE_ERR_SCENARIO_ASF == scenario) &&
            (vx_false_e == asf_error_injected) && (real_frame_count >= 10U))
        {
            tivxNodeSendCommand(n0, 0, TIVX_CAPTURE_INJECT_ASF_ERR, NULL, 0);
            asf_error_injected = vx_true_e;
        }
#endif

        vx_event_t event;
        vx_status event_status = vxWaitEvent(context, &event, vx_true_e);
        if ((vx_status)VX_SUCCESS == event_status)
        {
            if (((vx_enum)VX_EVENT_NODE_ERROR == event.type) &&
                (APP_EVENT_CAPTURE_NODE_ERROR == event.app_value))
            {
                if (n0 == event.event_info.node_error.node)
                {
                    tivx_capture_error_per_channel_t* local_error_info;
                    local_error_info = (tivx_capture_error_per_channel_t*)event.event_info.node_error.error_info;
                    if (event.event_info.node_error.status == -1)
                    {
                        /* Indices [0, num_capture_channels) are per-channel,
                         * frame-correlated errors (e.g. SHORT_FRAME); index
                         * [num_capture_channels] holds errors that cannot be
                         * correlated to a channel or frame (e.g. HEADER_ECC).
                         * Check both since this same scenario runner handles
                         * both error kinds. */
                        uint32_t err_idx;
                        VX_PRINT(VX_ZONE_ERROR,"Capture node error detected\n");
                        VX_PRINT(VX_ZONE_ERROR,"Node error status: %d\n",
                                event.event_info.node_error.status);
                        for (err_idx = 0U; err_idx <= num_capture_channels; err_idx++)
                        {
                            if (0U != local_error_info[err_idx].error_bitfield)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "RECEIVED error_bitfield[%u]: %u\n", loop_id, local_error_info[err_idx].error_bitfield);
                                VX_PRINT(VX_ZONE_ERROR, "RECEIVED timestamp[%u]: 0x%x\n", loop_id, local_error_info[err_idx].timestamp);

                                if (0U != (local_error_info[err_idx].error_bitfield & expected_error_bit))
                                {
                                    expected_error_seen = vx_true_e;
                                }
                            }
                        }
                    }
                }
            }
        }

        num_refs = 0U;
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &num_refs));

        if (num_refs > 0U)
        {
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);
        }

        if (num_refs > 0U)
        {
            poll_without_frame_count = 0U;
            real_frame_count++;
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
        }
        else
        {
            /* Nothing ready yet -- pace the poll instead of busy-spinning at
             * CPU speed, and bound how long we retry in case frames stop
             * arriving entirely (see the outer for-loop condition). */
            poll_without_frame_count++;
            tivxTaskWaitMsecs(5U);
        }
    }

    vxWaitGraph(graph);

    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    ASSERT(vx_true_e == expected_error_seen);

    VX_CALL(appStopImageSensor(sensor_name, TESTPATTERN_CHANNEL_MASK));


    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(tivxReleaseRawImage(&raw_image));
    VX_CALL(tivxReleaseRawImage(&error_frame));

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));

    tivxVideoIOUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);
}

/**
 * Test capture node error event handling: TIVX_CAPTURE_ERR_SHORT_FRAME.
 * See testCaptureErrorScenarioRun() for how the fault is injected.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureErrorShortFrame)
{
    testCaptureErrorScenarioRun(context_->vx_context_, TIVX_TEST_CAPTURE_ERR_SCENARIO_SHORT_FRAME);
}

/**
 * Test capture node error event handling: TIVX_CAPTURE_ERR_LONG_FRAME.
 * Confirmed end-to-end on hardware. See testCaptureErrorScenarioRun() for how
 * the fault is injected.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureErrorLongFrame)
{
    testCaptureErrorScenarioRun(context_->vx_context_, TIVX_TEST_CAPTURE_ERR_SCENARIO_LONG_FRAME);
}

/**
 * Test capture node error event handling: TIVX_CAPTURE_ERR_HEADER_ECC.
 * Confirmed end-to-end on hardware. See testCaptureErrorScenarioRun() for how
 * the fault is injected.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureErrorHeaderEcc)
{
    testCaptureErrorScenarioRun(context_->vx_context_, TIVX_TEST_CAPTURE_ERR_SCENARIO_HEADER_ECC);
}

#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
/**
 * Test capture node error event handling: ASF ESM error injection.
 * Confirmed end-to-end on hardware, standalone. See
 * testCaptureErrorScenarioRun() for how the fault is injected.
 */
TEST(tivxVideoIOCaptureAddition, testCaptureErrorAsf)
{
    testCaptureErrorScenarioRun(context_->vx_context_, TIVX_TEST_CAPTURE_ERR_SCENARIO_ASF);
}
#endif

#if defined(BUILD_DISPLAY)
TESTCASE_TESTS(tivxVideoIOCapture,
               testRawImageCapture,
               testRawImageCaptureTimeout,
               testRawImageCaptureDisplay,
               testRegisterErrorFrame,
               testRegisterErrorFrameInvalid,
               testRegisterErrorFrameInvalidDimensions,
               testRegisterErrorFrameTypeMismatch,
               testRegisterErrorFrameFormatMismatch,
               testRegisterErrorFrameUnsupportedObjectType,
               testRegisterErrorFrameQueryFailure,
               testRegisterErrorFrameNull,
               testRegisterErrorFrameNullNode,
               testAllocFrameImageFailure,
               testAllocFrameRawImageFailure,
               testtivxAddAndRemoveKernelCapture,
               testtivxAddKernelCaptureNullContext,
               testtivxAddKernelCaptureValidateInputOutputError,
               testtivxAddKernelCaptureValidateNullParams,
               testtivxcaptureparamsinitNULL,
               testCaptureInvalidNumberOfChannels,
               testCaptureSupportedImageFormats,
               testCapturePrintStatisticsCommand,
               testCaptureGetStatisticsInvalidParameters,
               testCaptureRegisterErrorFrame,
               testCaptureControlInvalidCommand,
               testCaptureInvalidInstanceId,
               testCaptureSupportedRawImageFormats,
#if (TIVX_CAPTURE_MAX_INST >= 2U)
               testCaptureCsiRxInstanceMapping,
#endif
               testCaptureTwoCsiRxInstances,
               testErrorFrameTimeout,
               testCaptureCreateProcessDelete,
               testCompareCapturedFrameToGoldenReference)

TESTCASE_TESTS(tivxVideoIOCaptureAddition,
                testCaptureErrorDescriptorAllocationFailure,
                testCaptureIdleCsiRxInstance,
                testCaptureUnsupportedImageFormats,
                testCaptureErrorShortFrame,
                testCaptureErrorLongFrame,
                testCaptureErrorHeaderEcc
#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
                ,testCaptureErrorAsf
#endif
                )
#else
TESTCASE_TESTS(tivxVideoIOCapture,
               testRawImageCapture,
               testRawImageCaptureTimeout,
               testRegisterErrorFrame,
               testRegisterErrorFrameInvalid,
               testRegisterErrorFrameInvalidDimensions,
               testRegisterErrorFrameTypeMismatch,
               testRegisterErrorFrameFormatMismatch,
               testRegisterErrorFrameUnsupportedObjectType,
               testRegisterErrorFrameQueryFailure,
               testRegisterErrorFrameNull,
               testRegisterErrorFrameNullNode,
               testAllocFrameImageFailure,
               testAllocFrameRawImageFailure,
               testtivxAddAndRemoveKernelCapture,
               testtivxAddKernelCaptureNullContext,
               testtivxAddKernelCaptureValidateInputOutputError,
               testtivxAddKernelCaptureValidateNullParams,
               testtivxcaptureparamsinitNULL,
               testCaptureInvalidNumberOfChannels,
               testCaptureSupportedImageFormats,
               testCapturePrintStatisticsCommand,
               testCaptureGetStatisticsInvalidParameters,
               testCaptureRegisterErrorFrame,
               testCaptureControlInvalidCommand,
               testCaptureInvalidInstanceId,
               testCaptureSupportedRawImageFormats,
#if (TIVX_CAPTURE_MAX_INST >= 2U)
               testCaptureCsiRxInstanceMapping,
#endif
               testCaptureTwoCsiRxInstances,
               testErrorFrameTimeout,
               testCaptureCreateProcessDelete,
               testCompareCapturedFrameToGoldenReference)

TESTCASE_TESTS(tivxVideoIOCaptureAddition,
                testCaptureErrorDescriptorAllocationFailure,
                testCaptureIdleCsiRxInstance,
                testCaptureUnsupportedImageFormats,
                testCaptureErrorShortFrame,
                testCaptureErrorLongFrame,
                testCaptureErrorHeaderEcc
#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
                ,testCaptureErrorAsf
#endif
                )
#endif
#endif /* BUILD_CAPTURE */