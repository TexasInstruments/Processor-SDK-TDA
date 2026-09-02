#ifndef __CORE_GENERATE_3D_BOWL_H__
#define __CORE_GENERATE_3D_BOWL_H__
/* Approximately equal to 75 cm from sensor */
#define MAX_BOWL 185
/* Using 100 w/ Shashank's maxZ change in the predefined view */
/* Approximately equal to 30 cm from sensor */
#define MIN_BOWL 100

#define FLAT_BOWL 400


void svGenerate_3D_Bowl(svGpuLutGen_t *sv,svGeometric_t *in_offset, float calmat_scaled[], float *GAlignLUT3D_XYZ );


#endif
