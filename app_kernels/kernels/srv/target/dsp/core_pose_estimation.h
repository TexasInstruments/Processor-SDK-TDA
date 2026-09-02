#ifndef __CORE_POSE_ESTIMATION_H__
#define __CORE_POSE_ESTIMATION_H__
/* Structs /funcs for Pose estimation */

/* Some memory allocations have to be experimentally increased */
/* There were memory overflows otherwise                       */
#define SCALING (2)
#define SCALING_2 (1)
//#define DEBUG_2
//#define DEBUG_1

typedef struct
{

    CameraIntrinsicParams     *buf_cip_ptr[MAX_INPUT_CAMERAS];
    uint32_t buf_cip_size;

    Point2D_f     *buf_chartPoints_ptr[MAX_INPUT_CAMERAS];
    uint32_t buf_chartPoints_size;

    Point2D_f    *buf_cornerPoints_ptr[MAX_INPUT_CAMERAS];
    uint32_t buf_cornerPoints_size;

    Point2D_f     *buf_baseChartPoints_ptr[4];
    uint32_t       buf_baseChartPoints_size;

    int32_t       *buf_inCornerPoints_ptr;
    uint32_t       buf_inCornerPoints_size;

    Matrix3D_f    *buf_H_cg_ptr;
    uint32_t       buf_H_cg_size;

    Matrix3D_f    *buf_R_cg_ptr;
    uint32_t       buf_R_cg_size;

    Matrix3D_f     *buf_R_gc_ptr;
    uint32_t        buf_R_gc_size;

    Point3D_f     *buf_t_cg_ptr;
    uint32_t       buf_t_cg_size;

    Point3D_f     *buf_t_gc_ptr;
    uint32_t       buf_t_gc_size;

    Point2D_f    *buf_normCornerPoint_ptr;
    uint32_t      buf_normCornerPoint_size;

    Point2D_f    *buf_points1norm_ptr;
    uint32_t      buf_points1norm_size;

    Point2D_f    *buf_points2norm_ptr;
    uint32_t      buf_points2norm_size;

    Flouble      *buf_xvec_ptr;
    uint32_t      buf_xvec_size;

    Flouble      *buf_fvec_ptr;
    uint32_t      buf_fvec_size;

    Flouble      *buf_bvec_ptr;
    uint32_t      buf_bvec_size;

    Flouble      *buf_tempvec_ptr;
    uint32_t      buf_tempvec_size;

    Flouble      *buf_deltavec_ptr;
    uint32_t      buf_deltavec_size;

    Flouble      *buf_Jacob_ptr;
    uint32_t      buf_Jacob_size;

    Flouble      *buf_JacobT_ptr;
    uint32_t      buf_JacobT_size;

    Flouble      *buf_gammaIdentity_ptr;
    uint32_t      buf_gammaIdentity_size;

    Flouble      *buf_A_ptr;
    uint32_t      buf_A_size;

    Flouble      *buf_U_ptr;
    uint32_t      buf_U_size;

    Flouble      *buf_U1_ptr;
    uint32_t      buf_U1_size;

    Flouble      *buf_V_ptr;
    uint32_t      buf_V_size;



    Flouble      *buf_Diag_ptr;
    uint32_t      buf_Diag_size;

    Flouble      *buf_superDiag_ptr;
    uint32_t      buf_superDiag_size;

    Flouble      *buf_AInv_ptr;
    uint32_t      buf_AInv_size;

    Flouble      *buf_Q_ptr;
    uint32_t      buf_Q_size;

    Flouble      *buf_R_ptr;
    uint32_t      buf_R_size;

    Flouble      *buf_yvec_ptr;
    uint32_t      buf_yvec_size;

    Flouble      *buf_h_ptr;
    uint32_t      buf_h_size;

    Flouble      *buf_R_DLT_ptr;
    uint32_t      buf_R_DLT_size;

    Flouble      *buf_t_DLT_ptr;
    uint32_t      buf_t_DLT_size;

    svACDetectStructFourCameraCorner_t  *buf_in_corners_ptr; 
    uint32_t      buf_in_corners_size;


   /* Non Pointer declarations */

    Point2D_f chartShift_tl;
    Point2D_f chartShift_tr;
    Point2D_f chartShift_bl;
    Point2D_f chartShift_br;   
 // DLT
    Matrix3D_f T1, T1Inv, T2, T2Inv, H, Hnorm, Htemp; 
    Matrix3D_f  Rt;  // temporary matrix
    int8_t  detectedPointNum[MAX_INPUT_CAMERAS];
    uint8_t bLeftChartOnly[MAX_INPUT_CAMERAS];
    Matrix3D_f P, Pnew;

} tivxPoseEstimationParams;

#ifdef DEBUG_2
 void print_matrix3D_f (Matrix3D_f *H_pt);
 void print_point2D_f (Point2D_f pt[NUM_CHART_CORNERS]);
#endif
int32_t svPoseEstimate(svPoseEstimation_t *sv, svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms,  svACDetectStructFourCameraCorner_t *inCornerPointsStruct, int8_t* inChartPos, int32_t* outcalmat,  uint8_t* bLeftChart, int8_t pEstimateMode);
 void constructCalmat(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms,int32_t* outcalmat);
 void initChartPoints(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, int8_t* inChartPos);
 void copyCornerPoints(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms,  int32_t * inCornerPoints);
 void svSetFrameCentersPEstimate(svPoseEstimation_t* sv,svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms );
 void initializePECameraIntrinsics(CameraIntrinsicParams* cip);
 void svExtrinsicPoseEstimation(svPoseEstimation_t *sv,svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms,
                svACDetectStructFourCameraCorner_t *inCornerPointsStruct, 
                int16_t vIdx,  Point2D_f *chartPoints, Point2D_f *cornerPoints, Point2D_f *normCornerPoints, uint16_t focalLength,
                uint16_t distCentX, uint16_t distCentY);
 void getPtArrFromIndex(Point2D_f* inPoints1, Point2D_f* inPoints2, Point2D_f* outPoints1, Point2D_f* outPoints2, uint32_t *idxArray, int16_t numPoints);
 void estimateRtBetweenChartAndCamera(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, int16_t vIdx, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv);
 void estimateRtBetweenChartAndCamera_ransac(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms,  int16_t vIdx, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv);
 void estimateRt_LM(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Flouble* R, Flouble* t, Matrix3D_f* P, Matrix3D_f* Pnew, int16_t numPoints);
 void getX(svPoseEstimation_t *sv, Point2D_f* points, Flouble* xvec, Flouble* lambda, int16_t numPoints);
 void computeJ(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* lambda, Flouble* J, Flouble* JT, int16_t Nrows, int16_t Ncols, int16_t numPoints);
 Flouble computeCost(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* xvec, Flouble* fvec, Flouble* lambda, int16_t numPoints);
 void computeF(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* fvec, Flouble* lambda, int16_t numPoints);
 Flouble vecNorm(Flouble * vector, int16_t length, uint8_t bSqrt);
 Flouble vecSum(Flouble * vector, int16_t length);
 void crossProduct(Flouble *v1, Flouble* v2, Flouble* vout);
 void transposeMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM, Flouble *outM);
 void multiplyMatrix(int16_t Nrows, int16_t Ncols, int16_t Ncols2, Flouble *inM1, Flouble *inM2, Flouble *outM);
 void addMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM1, Flouble *inM2, Flouble *outM);
 void subtractMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM1, Flouble *inM2, Flouble *outM);
 void initMatrix(int16_t Nrows, int16_t Ncols, Flouble* outM, uint8_t identity);
 void muliplyScalerMatrix(int16_t Nrows, int16_t Ncols, Flouble scaler, Flouble* inM, Flouble* outM);
 void getDiagonal(int16_t Nrows, int16_t Ncols, Flouble* inM, Flouble* diag);
 void extractRandT(svPoseEstimation_t *sv, Matrix3D_f M, Flouble* R, Flouble* t);
 void scale2UnitMatrix(Matrix3D_f* M);
 Flouble estimateH_2d_DLT_SRV(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv, Matrix3D_f* H, int16_t numPoints, int16_t a_nrows, int16_t a_ncols);
 void normalizeData_2d(Point2D_f *points, Point2D_f* pointsnorm, Matrix3D_f* T, Matrix3D_f* TInv, int16_t numPoints);
 void solveDelta(int16_t Nrows, int16_t Ncols, Flouble* A, Flouble* Q, Flouble* R, Flouble *x, Flouble *b, Flouble* u);
 void randomPerm(int32_t max_index, uint32_t *array_in, int32_t count, uint32_t *array_out);
 Flouble computePEGeometricError(Point2D_f points1, Point2D_f points2, Matrix3D_f M);


#endif
