#include "app_camera_pipeline_setup.h"
#include <string.h>

vx_status app_setup_camera_pipeline_modules(SensorObj &sensorObj, CaptureObj &captureObj,
                                            VISSObj &vissObj, AEWBObj &aewbObj, LDCObj &ldcObj,
                                            const AppCameraPipelineCfg &cfg)
{
    /* Configure sensor module */
    strncpy(sensorObj.sensor_name, cfg.sensor_name.c_str(), ISS_SENSORS_MAX_NAME - 1);
    sensorObj.sensor_index = cfg.sensor_index;
    sensorObj.usecase_option = cfg.usecase_option;
    sensorObj.num_cameras_enabled = cfg.num_cameras_enabled;
    sensorObj.ch_mask = cfg.ch_mask;
    sensorObj.enable_ldc = cfg.enable_ldc;
    sensorObj.is_interactive = cfg.is_interactive;

    /* Query sensor (interactive or from config) */
    vx_status status = sensorObj.querrySensor();
    if (status != VX_SUCCESS)
    {
        return status;
    }

    /* Configure capture module */
    captureObj.enable_error_detection = cfg.enable_error_detection;
    captureObj.en_out_capture_write = cfg.en_out_capture_write;
    strncpy(captureObj.output_file_path, cfg.output_file_path.c_str(), APP_MAX_FILE_PATH - 1);
    captureObj.bufq_depth = cfg.bufq_depth;

    /* Configure VISS module (sensor info is auto-fetched in initModule) */
    vissObj.num_cameras_enabled = sensorObj.num_cameras_enabled;
    vissObj.en_out_viss_write = cfg.en_out_viss_write;
    strncpy(vissObj.output_file_path, cfg.output_file_path.c_str(), APP_MAX_FILE_PATH - 1);
    vissObj.bufq_depth = cfg.bufq_depth;
    strncpy(vissObj.target, TIVX_TARGET_VPAC_VISS1, sizeof(vissObj.target) - 1);

    /* Configure AEWB module (sensor info is auto-fetched in initModule) */
    aewbObj.num_cameras_enabled = sensorObj.num_cameras_enabled;
    aewbObj.bufq_depth = cfg.bufq_depth;

    /* Configure LDC module */
    ldcObj.num_cameras_enabled = sensorObj.num_cameras_enabled;
    ldcObj.en_out_ldc_write = cfg.en_out_ldc_write;
    strncpy(ldcObj.output_file_path, cfg.output_file_path.c_str(), APP_MAX_FILE_PATH - 1);
    strncpy(ldcObj.target, TIVX_TARGET_VPAC_LDC1, sizeof(ldcObj.target) - 1);

    return VX_SUCCESS;
}

void app_layout_img_mosaic_windows(ImgMosaicObj &imgMosaicObj, vx_int32 num_cameras,
                                   vx_int32 clear_count)
{
    /* Single camera: full screen */
    if (num_cameras == 1)
    {
        imgMosaicObj.params.windows[0].startX  = 0;
        imgMosaicObj.params.windows[0].startY  = 0;
        imgMosaicObj.params.windows[0].width   = imgMosaicObj.out_width;
        imgMosaicObj.params.windows[0].height  = imgMosaicObj.out_height;
        imgMosaicObj.params.windows[0].input_select   = 0;
        imgMosaicObj.params.windows[0].channel_select = 0;
    }
    /* Two cameras: split screen horizontally */
    else if (num_cameras == 2)
    {
        for (vx_int32 idx = 0; idx < 2; idx++)
        {
            imgMosaicObj.params.windows[idx].startX  = idx * (imgMosaicObj.out_width / 2);
            imgMosaicObj.params.windows[idx].startY  = 0;
            imgMosaicObj.params.windows[idx].width   = imgMosaicObj.out_width / 2;
            imgMosaicObj.params.windows[idx].height  = imgMosaicObj.out_height;
            imgMosaicObj.params.windows[idx].input_select   = 0;
            imgMosaicObj.params.windows[idx].channel_select = idx;
        }
    }
    /* Three or more cameras: 2x2 grid (odd counts leave the last cell empty) */
    else if (num_cameras >= 3)
    {
        for (vx_int32 idx = 0; idx < num_cameras; idx++)
        {
            imgMosaicObj.params.windows[idx].startX  = (idx % 2) * (imgMosaicObj.out_width / 2);
            imgMosaicObj.params.windows[idx].startY  = (idx / 2) * (imgMosaicObj.out_height / 2);
            imgMosaicObj.params.windows[idx].width   = imgMosaicObj.out_width / 2;
            imgMosaicObj.params.windows[idx].height  = imgMosaicObj.out_height / 2;
            imgMosaicObj.params.windows[idx].input_select   = 0;
            imgMosaicObj.params.windows[idx].channel_select = idx;
        }
    }
    imgMosaicObj.params.num_windows = num_cameras;
    imgMosaicObj.params.clear_count = clear_count;
}
