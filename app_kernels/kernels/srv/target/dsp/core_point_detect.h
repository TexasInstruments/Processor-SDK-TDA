#ifndef __CORE_POINT_DETECT_H__
#define __CORE_POINT_DETECT_H__

/* Funcs /struct for Point Detect */
typedef struct
{
    /* Shashank-1: Update buffer pointer definitions */
    uint32_t *buf_IntegralImg_ptr;
    uint32_t buf_IntegralImg_size;
    uint32_t *buf_IntegralRows_ptr;
    uint32_t buf_IntegralRows_size;
    uint8_t  *buf_grayLumaFrame_ptr;
    uint32_t buf_grayLumaFrame_size;

    /* Manually added buffers */
    uint8_t  *buf_bwLumaFrame_ptr;
    uint32_t buf_bwLumaFrame_size;

    int16_t *buf_candFPId_ptr;
    int32_t buf_candFPId_size;

    int16_t *buf_candFPx_ptr;
    int32_t buf_candFPx_size;

    int16_t *buf_candFPy_ptr;
    int32_t buf_candFPy_size;

    SV_ACDetect_FPBoundaryPos *buf_svFPBoundaryPosStruct_ptr;
    int32_t buf_svFPBoundaryPosStruct_size;

    int16_t  *buf_intOutCenter_ptr;
    uint32_t buf_intOutCenter_size;

    int16_t *buf_outCenterNum_ptr;
    uint32_t buf_outCenterNum_size;

    double *buf_outCenter_ptr;
    uint32_t buf_outCenter_size;

    int16_t *buf_candidCenter_ptr;
    uint32_t buf_candidCenter_size;

    double   *buf_boundX_ptr;
    uint32_t buf_boundX_size;

    double   *buf_boundY_ptr;
    uint32_t buf_boundY_size;

    double   *buf_tempBoundX_ptr;
    uint32_t  buf_tempBoundX_size;

    double   *buf_tempBoundY_ptr;
    uint32_t  buf_tempBoundY_size;

    double   *buf_fcBoundX_ptr;
    uint32_t  buf_fcBoundX_size;

    double   *buf_fcBoundY_ptr;
    uint32_t  buf_fcBoundY_size;

    double   *buf_tempCorner_ptr;
    uint32_t  buf_tempCorner_size;

    double   *buf_tempCorner1_ptr;
    uint32_t  buf_tempCorner1_size;

    double   *buf_tempCorner2_ptr;
    uint32_t  buf_tempCorner2_size;

    double   *buf_line1PtsX_ptr;
    uint32_t  buf_line1PtsX_size;

    double   *buf_line1PtsY_ptr;
    uint32_t  buf_line1PtsY_size;

    double   *buf_line2PtsX_ptr;
    uint32_t  buf_line2PtsX_size;

    double   *buf_line2PtsY_ptr;
    uint32_t  buf_line2PtsY_size;

    int32_t  *buf_candidCorners_ptr[MAX_FP_ALL];      
    uint32_t  buf_candidCorners_size;

    int32_t  *buf_finalCorners_ptr[MAX_INPUT_CAMERAS][FP_TO_DETECT];
    uint32_t  buf_finalCorners_size;

    double   *buf_matchScore_ptr;
    uint32_t  buf_matchScore_size;

    //This is used to pass image information
    VXLIB_bufParams2D_t img_prms;

} tivxPointDetectParams;


 uint16_t svGetFinderPatterns(svPointDetect_t *sv, svLdcLut_t *ldcLut, tivxPointDetectParams *prms,  uint8_t *in_image, uint8_t *out_bw_image,svACDetectStructFinalCorner_t *finalCornerSet );
void svBinarizeImage(svPointDetect_t* sv, tivxPointDetectParams *prms, uint8_t *in_image, uint8_t *bwLumaFrame);
 uint8_t svCheckAllDirections(SV_ACDetect_FPBoundaryPos* svFPBoundaryPosStruct);
 uint8_t svCheckPatternProfileLowDia(svPointDetect_t* sv, uint8_t* bwLumaFrame, int16_t posy, int16_t posx, int16_t height, int16_t width, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosA, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosB, double slope, uint8_t bScreen);
 uint8_t svCheckPatternProfileHiDia(svPointDetect_t* sv, uint8_t* bwLumaFrame, int16_t posy, int16_t posx, int16_t height, int16_t width, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosA, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosB, double slope, uint8_t bScreen);
 int32_t svSearchFinderPatterns(svPointDetect_t* sv, tivxPointDetectParams *prms);
 int16_t svSearchCluster(svPointDetect_t* sv, tivxPointDetectParams *prms, int16_t y, int16_t x, int16_t numCenters);
 int16_t svFindFPCentroids(svPointDetect_t* sv, tivxPointDetectParams *prms, int16_t numPts); 
 void svRandomPerm(int32_t max_index, int32_t *array_in, int32_t count, int32_t *array_out);
 double svRansacLineFit(svPointDetect_t* sv,tivxPointDetectParams *prms ,double* inY, double* inX, double* fcY, double* fcX, double* corner, int16_t numPts, LensDistortionCorrection *ldc);
 double Calculate1DErrorForPoint(double ptx, double pty, uint8_t NaNslope, double slope, double icpt, double xIcpt, uint8_t bHorDist);
 double svAssociatePointsToLine(svPointDetect_t* sv,tivxPointDetectParams *prms, uint8_t* NaNslope, double* slope, double* icpt, double* xIcpt, int16_t totalPoints, uint8_t errTh, uint8_t candLineIdx[128][4], uint8_t* LineIdxArr, uint8_t* numOutlier);
 void svGetLineFrom2Pts(svPointDetect_t* sv,tivxPointDetectParams *prms, uint8_t* NaNslope, double* slope, double* icpt, double* xIcpt, uint8_t lineIdx);
 double svGetLineFitError(double* linePtsY, double* linePtsX, uint8_t* NaNSlope, double* XValue, double* slope, double* icpt, int16_t numPtLine,  uint8_t bHorDist);
 double GetLineAfterRansac(svPointDetect_t* sv, tivxPointDetectParams *prms, uint8_t* optLineIdxArr, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t totalPoints, uint8_t lineIdx);
 void svGetCornerFromLine(double* corner, uint8_t* NaNSlope, double* slope, double*icpt, double* XValue, uint8_t lineIdx1st, uint8_t lineIdx2nd, uint8_t cornerIdx);
 LDC_status LDC_UndistToDist(LensDistortionCorrection* ldc, dtype point_in[2], dtype point_out[2]);
 uint8_t svCheckFPCondition(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inputCorner);
 void svGetInitialLines(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, double* fcY, double* fcX, int16_t numPts, int16_t* lineStart, int16_t* lineEnd);
 double svRefineFPCorner(svPointDetect_t* sv,tivxPointDetectParams *prms, double* inY, double* inX, double *fcY, double* fcX, double* corner, int16_t numPts, int16_t* lineStart, int16_t* lineEnd, LensDistortionCorrection *ldc, uint8_t bFisheye);
 double svUpdateBoundaryPoints(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, int16_t numPts, int16_t* startPos, int16_t* endPos, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t lineIdx1st, uint8_t lineIdx2nd, uint8_t bHorDist1, uint8_t bHorDist2);
 double svGetLine(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, int16_t numPts, int16_t* startPos, int16_t* endPos, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t lineIdx, uint8_t bHorDist);
 int16_t svRemoveNoiseFP(svPointDetect_t* sv,svLdcLut_t *ldcLut,  tivxPointDetectParams *prms,  int16_t numFPs,svACDetectStructFinalCorner_t *finalCornerSet );

 void svInterpolateMissingBoundary(double* inY, double* inX, double *tempY, double *tempX, int16_t numPts);
// uint32_t myrandPE(void);

#endif
