#ifndef _APP_CAMERA_PIPELINE_SETUP
#define _APP_CAMERA_PIPELINE_SETUP

#include <app_sensor_module.hpp>
#include <app_capture_module.hpp>
#include <app_viss_module.hpp>
#include <app_aewb_module.hpp>
#include <app_ldc_module.hpp>
#include <app_img_mosaic_module.hpp>
#include <string>

/*! \brief Config values needed to wire up the shared Sensor -> Capture ->
 *  VISS -> AEWB -> LDC camera front-end, factored out of the (byte-identical)
 *  block that app_tidl_cam and app_tidl_seg_cam both carried. */
struct AppCameraPipelineCfg {
    std::string sensor_name;
    vx_int32    sensor_index;
    vx_int32    usecase_option;
    vx_uint32   num_cameras_enabled;
    vx_uint32   ch_mask;
    vx_int32    enable_ldc;
    vx_uint32   is_interactive;
    vx_int32    enable_error_detection;
    vx_int32    en_out_capture_write;
    std::string output_file_path;
    vx_int32    en_out_viss_write;
    vx_int32    en_out_ldc_write;
    vx_int32    bufq_depth;
};

/*! \brief Configure Sensor/Capture/VISS/AEWB/LDC from \a cfg and query the
 *  sensor. Returns the status of querrySensor() — VX_SUCCESS on success, the
 *  sensor-query failure status otherwise. Caller logs its own demo-tagged
 *  error message on failure. */
vx_status app_setup_camera_pipeline_modules(SensorObj &sensorObj, CaptureObj &captureObj,
                                            VISSObj &vissObj, AEWBObj &aewbObj, LDCObj &ldcObj,
                                            const AppCameraPipelineCfg &cfg);

/*! \brief Lay out ImgMosaic windows for 1/2/4-camera configurations
 *  (full screen / horizontal split / 2x2 grid). Shared by all camera demos —
 *  sets params.windows[]/num_windows/clear_count on \a imgMosaicObj using
 *  its own out_width/out_height. Caller must set out_width/out_height,
 *  num_inputs, and bufq_depth first. */
void app_layout_img_mosaic_windows(ImgMosaicObj &imgMosaicObj, vx_int32 num_cameras,
                                   vx_int32 clear_count);

#endif /* _APP_CAMERA_PIPELINE_SETUP */
