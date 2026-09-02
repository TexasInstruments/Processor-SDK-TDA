#ifndef __CORE_GENERATE_GPULUT_H__
#define __CORE_GENERATE_GPULUT_H__ 



#define POW_2_CALMAT_R_SHIFT_INV    1.0/(1<<CALMAT_R_SHIFT)
#define POW_2_CALMAT_T_SHIFT_INV    1.0/(1<<CALMAT_T_SHIFT)

/* Inline division (SP) */
#define cmn_DIVSP(a,b) {  \
  float  TWO = 2.0;       \
  float  X;               \
  X = _rcpsp((b));        \
  X = X*(TWO - (b)*X);    \
  X = X*(TWO - (b)*X);    \
  X = (a)*X;              \
  return (X);             \
}

typedef struct
{
    float    *buf_GLUT3d_undist_ptr;
    uint32_t buf_GLUT3d_undist_size;
} tivxGenerateGpulutParams;

#endif

void svGenerate_3D_GPULUT(svGpuLutGen_t *sv, svLdcLut_t  *ldclut, tivxGenerateGpulutParams *prms, svACCalmatStruct_t* calmat_scaled, float *lut3dxyz, uint16_t *out_gpulut, int32_t *lut_count);
