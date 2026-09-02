#ifndef __SRV_COMMON__
#define __SRV_COMMON__




#define CALMAT_R_SHIFT 30 // Rotation Matrix in [R|T] Q1.30
#define CALMAT_T_SHIFT 10 // Translation Matrix in [R|T] Q21.10
#define CALMAT_H_SHIFT 20 // Perspective matrix precision H Q11.20
#define POW_2_CALMAT_R_SHIFT_INV    1.0/(1<<CALMAT_R_SHIFT)
#define POW_2_CALMAT_T_SHIFT_INV    1.0/(1<<CALMAT_T_SHIFT)
#define A_NROWS 16
#define A_NCOLS 9
#define A_SIZE  A_NROWS*A_NCOLS
#define U_SIZE  A_NROWS*A_NROWS
#define V_SIZE  A_NROWS*A_NROWS //A_NCOLS*A_NCOLS
#define FP_TO_DETECT          2   // # of charts to detect
#define NUM_CHART_CORNERS     8   // # of corners in a image
#define NUM_CORNERS_PER_CHART (NUM_CHART_CORNERS/FP_TO_DETECT) // # of corners / chart
#define ALGORITHM_PROCESS_OK 0
#define ALGORITHM_PROCESS_FAIL -1
#define ALGORITHM_PROCESS_DISABLED 2
#define LIN_ALG_FLOUBLE_TYPE 1 /* 0 = float, 1 = double */
/*Constants*/
#if LIN_ALG_FLOUBLE_TYPE == 0
        #define FLOUBLE_MAX FLT_MAX
        #define FLOUBLE_MIN FLT_MIN
        #define FLOUBLE_EPSILON FLT_EPSILON
#elif LIN_ALG_FLOUBLE_TYPE == 1
        #define FLOUBLE_MAX DBL_MAX
        #define FLOUBLE_MIN DBL_MIN
        #define FLOUBLE_EPSILON DBL_EPSILON
#endif

/*Functions*/
#if LIN_ALG_FLOUBLE_TYPE == 0
        #define FABS(x)  fabsf((x))
        #define SQRT(x)  sqrtf((x))
        #define COS(x)   cosf((x))
        #define SIN(x)   sinf((x))
        #define TAN(x)   tanf((x))
        #define ACOS(x)  acosf((x))
        #define ASIN(x)  asinf((x))
        #define ATAN(x)  atanf((x))
        #define EXP(x)   expf((x))
        #define POW(x,p) powf((x),(p))
        #define FLOOR(x) floorf((x)) 
#elif LIN_ALG_FLOUBLE_TYPE == 1
        #define FABS(x)  fabs((x))
        #define SQRT(x)  sqrt((x))
        #define COS(x)   cos((x))
        #define SIN(x)   sin((x))
        #define TAN(x)   tan((x))
        #define ACOS(x)  acos((x))
        #define ASIN(x)  asin((x))
        #define ATAN(x)  atan((x))
        #define EXP(x)   exp((x))
        #define POW(x,p)   pow((x),(p))
        #define FLOOR(x) floor((x)) 
#endif


#define MIN(a,b)   (((a)<(b))?(a):(b))
#define SIGNVAR(a) ((a) > 0.0 ? 1 : -1)
#define LIN_ALG_FLOUBLE_TYPE 1
#if LIN_ALG_FLOUBLE_TYPE == 0
typedef float Flouble;
#elif LIN_ALG_FLOUBLE_TYPE == 1
typedef double Flouble;
#else
#error("Unknown value for LIN_ALG_FLOUBLE_TYPE");
#endif



typedef struct  {
   svACDetectStructFinalCorner_t finalCameraCorners[MAX_INPUT_CAMERAS];

}  svACDetectStructFourCameraCorner_t;


/* This struct is only used for reading calmat from file */
typedef struct  {
   int32_t  numCameras;   /* 4 Bytes for number of Cameras */
   int32_t  calMatSize[MAX_INPUT_CAMERAS]; /* 4 Btyes /calmat size          */
   int32_t  calMatBuf [48 *MAX_INPUT_CAMERAS]; //Cross check 

}  svCalmat_t;


typedef struct
{
    double xPos[4];
    double yPos[4];

    double centerRun;
    double firstBrun;
    double secondBrun;

} SV_ACDetect_FPBoundaryPos;



/******************************************
Vector/Point
******************************************/
typedef struct {
        Flouble x;
        Flouble y;
} Point2D_f;

/******************************************
Matrix
******************************************/
typedef struct {
        Flouble xx, xy;
        Flouble yx, yy;
} Matrix2D_f;

/*****************************************
*Affine Transform:
* - matrix A
* - translation t
******************************************/
typedef struct {
        Matrix2D_f A;
        Point2D_f b;
} Affine2D_f;

/*****************************************
*Euclidean transform (Pose):
* - rotation matrix R (orthonormal)
* - translation t
******************************************/
typedef struct {
        Matrix2D_f R;
        Point2D_f t;
} Pose2D_f;

/******************************************
Vector/Point
******************************************/
typedef struct {
        Flouble x;
        Flouble y;
        Flouble z;
} Point3D_f;

/******************************************
Matrix
******************************************/
typedef struct {
        Flouble xx, xy, xz;
        Flouble yx, yy, yz;
        Flouble zx, zy, zz;
} Matrix3D_f;

/*****************************************
*Affine Transform:
* - matrix A
* - translation t
******************************************/
typedef struct {
        Matrix3D_f A;
        Point3D_f t;
} Affine3D_f;

/*****************************************
*Euclidean transform (Pose):
* - rotation matrix R (orthonormal)
* - translation t
******************************************/
typedef struct {
        Matrix3D_f R;
        Point3D_f t;
} Pose3D_f;

typedef struct {
    vx_int16 distCenterX;
    vx_int16 distCenterY;
    vx_int16 distFocalLength;
    Matrix3D_f K;
    Matrix3D_f invK;
} CameraIntrinsicParams;


typedef enum
{
    ESTIMATE_RT_DLT,
    ESTIMATE_RT_RANSAC
} rtEstimateMethod; 


/**************************************************************************************************/

#endif
