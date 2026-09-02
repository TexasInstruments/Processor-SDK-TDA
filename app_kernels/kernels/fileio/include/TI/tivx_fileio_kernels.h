#ifndef TIVX_FILEIO_KERNELS_H_
#define TIVX_FILEIO_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_vision_apps_kernels_filio TIVX Kernels for File I/O
 *
 * \brief This section documents the kernels defined for File I/O
 *
 * \ingroup group_vision_apps_kernels
 */
/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief OpenVX module name
 * \ingroup group_vision_apps_kernels_fileio
 */
#define TIVX_MODULE_NAME_FILEIO    "fileio"

/*! \brief Kernel Name: Write vx_array
 *  \see group_vision_apps_kernels_fileio
 */
#define TIVX_KERNEL_WRITE_ARRAY_NAME     "com.ti.fileio.write.vx_array"

/*! \brief Kernel Name: Write vx_image
 *  \see group_vision_apps_kernels_fileio
 */
#define TIVX_KERNEL_WRITE_IMAGE_NAME     "com.ti.fileio.write.vx_image"

/*! \brief Kernel Name: Write tivx_raw_image
 *  \see group_vision_apps_kernels_fileio
 */
#define TIVX_KERNEL_WRITE_RAW_IMAGE_NAME     "com.ti.fileio.write.tivx_raw_image"

/*! \brief Kernel Name: Write vx_tensor
 *  \see group_vision_apps_kernels_fileio
 */
#define TIVX_KERNEL_WRITE_TENSOR_NAME     "com.ti.fileio.write.vx_tensor"

/*! \brief Kernel Name: Write vx_user_data_object
 *  \see group_vision_apps_kernels_fileio
 */
#define TIVX_KERNEL_WRITE_USER_DATA_OBJECT_NAME     "com.ti.fileio.write.vx_user_data_object"

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_fileio
 */
/* Length of the filepath string */
#define TIVX_FILEIO_FILE_PATH_LENGTH    (512U)

/* Length of the fileprefix string */
#define TIVX_FILEIO_FILE_PREFIX_LENGTH  (256U)

/* Command to write file to memory */
#define TIVX_FILEIO_CMD_SET_FILE_WRITE  (50)

/*********************************
 *      Functions
 *********************************/

/*!
 * \brief Used for the Application to load the fileio kernels into the context.
 * \ingroup group_vision_apps_kernels_fileio
 */
void tivxFileIOLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the fileio kernels from the context.
 * \ingroup group_vision_apps_kernels_fileio
 */
void tivxFileIOUnLoadKernels(vx_context context);

/*!
 * \brief Function to register FileIO Kernels on the arm Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterFileIOTargetMpuKernels(void);

/*!
 * \brief Function to un-register FileIO Kernels on the arm Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterFileIOTargetMpuKernels(void);

typedef struct
{
    vx_uint32 start_frame;
    vx_uint32 num_frames;
    vx_uint32 num_skip;

} tivxFileIOWriteCmd;

#ifdef __cplusplus
}
#endif

#endif /* TIVX_FILEIO_KERNELS_H_ */
