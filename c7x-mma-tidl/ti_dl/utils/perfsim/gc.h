/*
*******************************************************************************
*
* Module Name: Graph compiler - Primarilly designed for execution plan for DNN
* based graphs, but should scale for any graph based execution planner
*
* "Performance simulator - Predicts time for overall graph execution for TI's
* C7x + MMA processor architecture. It models the performance of different 
* processing elements of graph and provides a summarized output of performance
*
* Copyright (C) 2015-21 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*******************************************************************************
*/

/**
******************************************************************************
*  @file     gc.h
*
*  @brief    Interface Header file for Graph Compiler for C7x + MMA
*
*  @author   PKS
*
*  @version  0.1 - June 2021: Initial Version [PKS]
*
*****************************************************************************
*/

/*
@Todo
1. Remove rounding bits and have a better way for processing functions control variables
   to be pass thru
2. Grouped convolution jacintonet11v2 test case itself uses

*/
#ifndef _GC_H_
#define _GC_H_

#include "itidl_ti.h"
#include "common.h"


/**
* @defgroup graphCompiler Graph Compiler
*/

/**
* @addtogroup graphCompiler
* @{
*/


/*
CRITICAL DATA OBJECTS and RELATIONSHIP

_____sGCDataBase_t___________________________________________________________
|                                                                             |
|     Buffer Node Data Base                                                   |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|    |     |     |     |     |     |     |     |     |............. |     |   |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|                                                                             |
|    sWorkloadUnit_t Data Base                                                |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|    |     |     |     |     |     |     |     |     |............. |     |   |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|                                                                             |
|    proc Link Data Base                                                      |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|    |     |     |     |     |     |     |     |     |............. |     |   |
|    +-----+-----+-----+-----+-----+-----+-----+-----+--------------+-----+   |
|                                                                             |
|_____________________________________________________________________________|


 ___________ sWorkloadSuperGroup_t____________________________________________________________________________________________________
|                                                                                                                                     |
|   ________________sWorkloadGroup_t______________________________________________________________________________________________    |
|  |                                                                                                            |     |     |     |   |
|  |     id, dependid[],                                                                                        |     |     |     |   |
|  |                                                                                                            |     |     |     |   |
|  |	 ________________sWorkloadSubGroup_t__________________________________________________________________    |     |     |     |   |
|  |	|                                                                                |      |      |      |   |     |     |     |   |
|  |	|      _____________sWorkloadUnit_t____________________________________________  |      |      |      |   |     |     |     |   |
|  |	|     |                                                |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |  numLinks, numIter, layerId, networkId         |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |  Links                                         |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |  ________________________________________      |     |     |....|     |  |      | .... |      |   |     | ... |     |   |
|  |	|     |  |      |      |      |      |..........|      |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |  |______|______|______|______|__________|      |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |											                           |     |     |    |     |  |      |      |      |   |     |     |     |   |
|  |	|     |________________________________________________|_____|_____|____|_____|  |      |      |      |   |     |     |     |   |
|  |	|     											                                                     |      |      |      |   |     |     |     |   |
|  |  |________________________________________________________________________________|______|______|______|   |     |     |     |   |
|  |                                                                                                            |     |     |     |   |
|  |____________________________________________________________________________________________________________|_____|_____|_____|   |
|  																													                                                                          |
|_____________________________________________________________________________________________________________________________________|


<> Joint,
+--------+         x~~~~~~~x
| Buffer |         | Link  |
+--------+         x~~~~~~~x
_____________________________________________________________________________________________________________________________________
| ------------------------------------------------------------------------------------------------------------------------------------|
|                                +-----+         x~~~~~x        +-----+                                                               |
|                                | FMF |<>----<> |  X  |<>----<>| FMP |                                                               |
|                                +-----+         x~~~~~x        +-----+                                                               |
|                                                                    <>                                                               |
|                                                                    |      x~~~~~x         +-----+         x~~~~~x        +-----+    |
|                                                                    +----<>|  P  |<>----<> | FMP |<>----<> |  X  |<>----<>| FMF |    |
|                                                                    |      x~~~~~x         +-----+         x~~~~~x        +-----+    |
|                                                                    <>                                                               |
| +-----+         x~~~~~x        +-----+         x~~~~~x         +-----+                                                              |
| | C_F |<>----<> |  X  |<>----<>| C_P |<>----<> |  X  | <>----<>| C_F |                                                              |
| +-----+         x~~~~~x        +-----+         x~~~~~x         +-----+                                                              |
|_____________________________________________________________________________________________________________________________________|
*/

#define MAX_WORKLOAD_PER_CORE     (10000)    /**< Maximum number of Workload per Core*/
#define MAX_SRC                   (TIDL_NUM_IN_BUFS)       /**< Maximum number of input source*/
#define MAX_SINK                  (8)                      /**< Maximum number of output sink*/
#define MAX_INPUT_LINKS           (15U)                      /**< Maximum number of supported input links to a Workload*/
#define MAX_SCRATCH_MEM_REGIONS_PER_WL   (1)

/**
* @defgroup workloadObject  Workload Objects
* @ingroup  graphCompiler
* @{
*/

/*****************************************************************************
* There are 13 data buffers and 7 transfer links as shown below per workload *
*----------+------+--------------+-----+-----------+------+------------------*
* Type    :| SRC  |              |STAGE|           | DST  |                  *
*----------|------|--------------|-----|-----------|------|------------------*
* IN      :| [Bx] | ........(T0).|..x..|.........> |[B0]  |                  *
* WTNext  :| [B1] | ........(T1).|..x..|.........> |[B2]  |                  *
* WT      :| [B3] | ...(T2).....>|[B4].|..(T3)...> |[B5]  |                  *
* OUT     :|      |              |[B6] |...(T4)..> |[B7]  |                  *
* CTX_ST  :| [B8] | ........(T5).|..x..|.........> |[B9]  |                  *
* CTX_RST :| [B10]| ........(T6).|..x..|.........> |[B11] |                  *
*----------+------+--------------+-----+-----------+------+------------------*
* Please note that IN(SRC) is a special buffer which internally can have     *
* multiple buffers                                                           *
* Then there is 1 Processor links which connects the 3 out of 13 buffers to  *
* processing element as shown below, where B5 = weight, B0 = i/p and B6 = o/p*
* [B0] ---------\                                                            *
*                  ==> (Processing) ----------> [B6]                         *
* [B5] ---------/                                                            *
*                                                                            *
* During a particular workload processing some of the links might not be     *
* active and same is the case for some of the buffers                        *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
* Above use case demands a representation in form of nodes and links         *
* so a generic software view/components are defined with below approach      *
*                                                                            *
* +------ +                                                      +------ +   *
* |       |  +------ +  +------------------------- +  +------ +  |       |   *
* | Node  |--| Joint |--|        Link              |--| Joint |--| Node  |   *
* |       |  +------ +  +------------------------- +  +------ +  |       |   *
* |       |                                                      |       |   *
* +------ +                                                      +------ +   *
*                                                                            *
* A Node can be a Buffer Node, and Link is a generic link                    *
* Joints connects the link to Nodes and makes a full pipe with src and sink  *
* Since the use case demands multi source situation, Joints on source side   *
* are multi instances                                                        *
* In this design, we chose to use xfer link always as a link which connects  *
* one source to one sink. i.e for a xfer link the joints on even source side *
* are single instance. However processing link can have multi source joint   *
* On processing link the source joints are multi-input if the link is going  *
* to use all the input simultaneously ( eg. eltwise add). There are case of  *
* Plinks even with multi-input but exercising only single joint (eg. concat) *

* Joints have a flow property which indicates the granularity at which       *
* information is transmitted from source node(s) to sink node                *
******************************************************************************
* In order to define the use case mentioned in first section, a data base of *
*  buffer nodes is created and then for each workload unit is a list         *
* of ordered links                                                           *
* so a execution engine should perform below operation                       *
* link[0] -> Link[1] --> link [2]-->.........-> link[N]                      *
******************************************************************************/
/**** { All structures and enums definition related to Nodes. starts */

/**<
********************************************************************************
* @struct sPadParams_t                                                         *
* @brief  This structure provides the information for padding values in buffer *
*         the unit of values is elements ( not bytes)                          *
* @param cols : Array holding the pad values in col direction, this array      *
*               holds 3 values : Actual physical space for padding, required & *
*               to be filled                                                   *
* @param rows : Array holding the pad values in row (top & bottom) direction,  *
*               this array holds 3 values : Actual physical space for padding, *
*               required & to be filled                                        *
*******************************************************************************/
typedef struct
{
  int32_t cols;
  int32_t rows;
} sPadParams_t;


/**
*  @enum       eBufShapeType
*  @brief      define macros for different shapes of buffer
*/
  #define BUF_LINEAR     0   /**< Linear buffer */
  #define BUF_CIRCULAR   1   /**< Circular buffer */
  #define BUF_PING_PONG  2   /**< Ping Pong buffer */


/**
*  @enum       eBufType
*  @brief      Enumeration for different types of buffer, This is more of
*              informational tag attached to each buf node and mostly useful
*              for debug, also helps in getting the right meta data
*/
typedef enum {
  BUF_FM_FULL = 0,    /**< Entire feature map  ( mostly DDR/MSMC)   */
  BUF_FM_IN_PART,     /**< Part of feature map ( always L2)         */
  BUF_FM_OUT_PART,    /**< Part of feature map output ( mostly MSMC)*/
  BUF_COEFF_FULL,     /**< coeffs - full data                       */
  BUF_COEFF_IN_STAGE, /**< Intermediate buffer for weights (coeffs) */
  BUF_COEFF_IN_FINAL, /**< Final buffer space for  weights (coeffs) */
  BUF_COEFF_IN_ADV,   /**< Advance transfer weights (coeffs)        */
  BUF_BIAS_FULL,      /**< Bias - full Data (in DDR)                */
  BUF_BIAS_IN_FINAL,  /**< Final buffer space for bias/scale/shift  */
  BUF_CTXT,           /**< Context information                      */
  BUF_SCRATCH,        /**< Scratch memory needed during inference   */
  BUF_UNKNOWN,        /**< Not known buffer type                    */
  MAX_BUF_PER_LINK,   /**< Maximum buffers per Link                 */
} eBuftype;


typedef enum
{
  ZERO_D = 0,
  ONE_D,
  TWO_D,
  THREE_D,
  FOUR_D
} eDim;

typedef enum
{
  ROW_DIR = 0,        /** Along row **/
  COL_DIR    ,         /**Along column**/
  MAX_DIR_TYPES,  /**Maximum type of directions **/
}eDir;

typedef enum
{
  TOP_DIR = 0,
  BOTTOM_DIR,
  LEFT_DIR,
  RIGHT_DIR,
  MAX_DIRECTION_TYPES,
}eDirection;

/**<
********************************************************************************
* @struct sFeatureMetaParams_t                                                 *
* @brief  This structure provides the meta information for feature map data    *
*         valid for buf type BUF_FM_xxxxxx                                     *
* @param width    : Width of complete feature map : even for BUF_FM_xx_PART    *
* @param height   : height of complete feature map : even for BUF_FM_xx_PART   *
* @param channels : channels of the feature map ( in case of BUF_FM_IN_PART and*
*                   BUF_FM_OUT_PART it will be the number of channels the part *
*                   buffer is accommodating                                    *
* @param numBatch  :num of batches of the feature map                          *   
*                   (in case of BUF_FM_IN_PART and                             *
*                   BUF_FM_OUT_PART it will be the number of batch the part    *
*                   buffer is accommodating)                                   *
* @param batchPad  :amount of Pad between batches along width                  *
* @param layout   : layout of buffer, refer eTIDL_TensorLayout                 *
*******************************************************************************/
typedef struct
{
  int32_t width;
  int32_t height;
  int32_t channel;
  int32_t numBatch;
  int32_t batchPad;
  int32_t layout;
}sFeatureMetaParams_t;

/**<
*****************************************************************************
* @struct sCoeffMetaParams_t                                                *
* @brief  This structure provides the meta information for coeff  data      *
*         valid for buf type BUF_COEFF_xxxxxx                               *
* @param Fr    : Filter kernel size in ver direction                        *
* @param Fc    : Filter kernel size in hor direction                        *
* @param Ni    : Number of input channels, actual value represented by the  *
*                buffer and can be lesser from the total channels           *
* @param No    : Number of output channels, actual value represented by the *
*                buffer and can be lesser from the total channels           *
* @param group : Number of groups                                           *
****************************************************************************/
typedef struct
{
  int32_t Fr;
  int32_t Fc;
  int32_t Ni;
  int32_t No;
  int32_t group;
}sCoeffMetaParams_t;

/**<
*****************************************************************************
* @struct sGenericMetaParams_t                                                  *
* @brief  This structure provides the meta information for generic  data    *
*         valid for buf type not in (BUF_FM_xxxxxx , BUF_COEFF_xxxxxx)      *
* @param rsvd   : Reserved filed - not defined                              *
****************************************************************************/
typedef struct
{
  int32_t rsvd;
}sGenericMetaParams_t;

/**<
******************************************************************************
* @struct sBufMetaParams_t                                                        *
* @brief  This union provides the meta information for different buffer types*
*         and bufType variable in parent structure should be used to identify*
*         which specific structure to be used                                *
* @param featMetaParams   : meta data structure for feature maps               *
* @param coeffMetaParams  : meta data structure for coeffs                     *
* @param genMetaParams    : meta data structure for generic buffers            *
****************************************************************************/
typedef union
{
  sFeatureMetaParams_t  featMetaParams  ;
  sCoeffMetaParams_t    coeffMetaParams ;
  sGenericMetaParams_t  genMetaParams   ;
} sBufMetaParams_t;

/**<
********************************************************************************
* @struct sBufParams_t                                                         *
* @brief  This structure holds the information for buffer nodes                *
*                                                                              *
* Legends in figure below                                                      *
* (o)padded region, (-)actual pixels,(+)boundary of feature width,(x)junk data *
*                                                                              *
* 1 feature plane 2D is as shown below is considered in form of linear 1D      *
* +o+oooooooo+                                                                 *
* +o+--------+                                                                 *
* +o+        +                                                                 *
* +o+        + => +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx       *
* +o+        +                                                                 *
* +o+--------+                                                                 *
* +o+oooooooo+oxxxx                                                            *
* 2D representation       ==> 1D representation                                *
*                                                                              *
* So tensor (multiple feature planes) is represented in 2D form as below       *
*                                                                              *
* <------------- Linearized 1D Feature Plane------------>                      *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  ^                    *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  |                    *
* .                                                       |                    *
* .                                                       Ni (# Feature Planes)*
* .                                                       |                    *
* +o+oooooooo+o+--------+o+--------+....o+--------+oxxxx  ^                    *
* Physical buffer Size = bufWidth*bufHeight (not always, but mostly)           *
*                                                                              *
* @param dataId       : Unique ID to the buffer in a buffer data base          *
* @param dataType     : data type of buffer like 8/16-bit refer @eDataType     *
* @param space        : memory space of buffer: L2/DDR/MSMC, refer @eMemType   *
* @param coreId       : c7x core where this buffer belongs, for shared         *
                        memories this can be NOT_VALID/Shared                  *
* @param baseMem      : memory address assuming start as 0                     *
* @param bufWidth     : Buffer width (including junk elements), unit: element  *
*                       -1 indicates that it is not relevant                   *
* @param bufHeight    : Buffer height(Mostly Ni), unit: element                *
*                       -1 indicates that it is not relevant                   *
* @param activeBufWidth: Pixels in width excluding junk data                   *
*                       -1 indicates that it is not relevant                   *
* @param bufSize    : Physical buffer size, not necessary width*height,        *
*                       explain later why? - TBD, unit: element                *
* @param type         : refer @eBufType                                        *
* @param shapeType    : refer @eBufShapeType                                   *
* @param padParams    : Padding Parameters, refer @sPadParams_t                *
* @param metaParams   : Meta data , refer @sBufMetaParams_t                    *
*******************************************************************************/
typedef struct {
  int64_t dataId;
  int32_t dataType;
  int32_t space;
  int32_t coreId;
  int32_t baseMem;
  int32_t bufWidth;
  int32_t bufHeight;
  int32_t activeBufWidth;
  int32_t bufSize;
  int32_t type;
  int32_t shapeType;
  float32_tidl tensorScale;
  int32_t tensorZeroPoint;
  sPadParams_t padParams;
  sBufMetaParams_t metaParams;
} sBufParams_t;

/**
*  @enum       eProcDependencyType
*  @brief      Enumeration for different processing nodes as per dependency type
*/
typedef enum
{
  PROC_DEPEND_POINT = 0,/**< Point to point dependency, example RELU       */
  PROC_DEPEND_RECT,     /**< Spatial dependency, example Pooling           */
  PROC_DEPEND_CUBE      /**< Spatial and depth dependency, eg non-DWS Conv */
} eProcDependencyType;

/**
*  @enum       eProcSourceType
*  @brief      Enumeration for different processing nodes as per number of source
*/
typedef enum
{
  PROC_SRC_SINGLE_TENSOR = 0, /**< Single input tensor, example conv, pool   */
  PROC_SRC_MULTI_TENSOR,     /**< Multi input tensor, example Concat, Eltwise*/
} eProcSourceType;

/**<
********************************************************************************
* @struct sProcParams_t                                                        *
* @brief  This structure holds the processing node type related info           *
*                                                                              *
* @param dependency: type of source dependency @eProcDependencyType            *
* @param roundBits : Amount of rounding bits - not at all clean,adding now to  *
*                    make progress but there can be many control variables of  *
*                    processing and need better mechanism                      *
*******************************************************************************/
typedef struct {
  int32_t procDependencyType;
  int32_t roundBits;
  int32_t scratchBufDBIdx[(int32_t)MEMTYPE_TOTAL - 1][MAX_SCRATCH_MEM_REGIONS_PER_WL];
  int32_t outPadZeros[MAX_DIR_TYPES];
  int32_t outPadRequired[MAX_DIR_TYPES];
  int32_t outPadToFill[MAX_DIR_TYPES];
  int32_t padVirtual[MAX_DIRECTION_TYPES];
} sProcParams_t;

/**** All structures and enums definition related to Nodes ends } */

/**** {All structures and enums definition related to Link/joints start */

#define MAX_LINKS_PER_WL  ((MAX_INPUT_LINKS+ 1U)*2U)
/**
*  @enum       eLinkType
*  @brief      Enumeration for different types of Links, This is critical
*              for processing chain to get implemented. 
*/
typedef enum {
  LINK_PROC = 0,           /**< Processing Link                              */
  LINK_XFER,   /**< transfer Link            */
  MAX_LINKS_TYPE           /**< MAX LInk types                               */
} eLinkType;

/**
*  @enum       eFlowStage
*  @brief      Enum to indicate the stage of flow of information from a Joint
*              Typically the information moves in 3 stages: pipeup, pipeline 
*               and pipedown
*/
typedef enum
{
  FLOW_PIPEUP = 0,
  FLOW_PIPELINE,
  FLOW_PIPEDOWN,
  MAX_FLOW_STAGES
} eFlowStage;

/**<
*****************************************************************************
* @struct sJoint_t                                                          *
* @brief  This structure holds the information for joints                   *
*                                                                           *
* @param type       : For future usage - not decided the purpose            *
* @param bufDBindex : Index in the data base to get the node to which joint *
*                     is attached                                           *
* @param parentbufDBindex : Index in the data base to get the ancestor nodes*
*                           0th index - immediate ancestor                  *
*                           1st index - 2nd level ancestor                  *
*                           if parent not present :                         *
*                           set parentbufIdx = its own bufIdx               *
* @param childbufDBindex  : Index in the database to get 2 immediate        *
*                           children nodes :                                *
*                           0th index - 1st immediate child (1st level)     *
*                           1st index - 2nd immediate child (1st level).    *
*     if 2 children present   -> both indexes populated with child bufindex *
*     if 1 child present      -> [0]:childs index [1]:NOT_VALID             *
*     if no children present  -> [0] its own bufIndex [1]:NOT_VALID         *
* @param offset     : offset within the node                                *
* @param superOffset: Some times joint move after completion of a entire    *
*                     flow. So instead of creating multiple joints and      *
*                     increase memory foot print this parameter tells that  *
*                     how much joint shall move after one flow completion   *
*                     finalOffset =  offset +  superOffset * flowIndex      *
*                     where flowIndex is maintained by processing chain     *
*                     For 1st iteration of WL corresponding to normal       *
*                     tile, add this value to the joint offset.             *
*                     "First Normal Tile Offset Difference"                 *
* -->+-----+  flowIndex = 0, offset = 0 , superOffset = X, First Tile       *
*    |     |                                                                *
* -->|-----|  flowIndex = 0, offset = 2, superOffset = 4, Normal Tile (0)   *
*    |     |                                                                *
*    |     |                                                                *
*    |     |                                                                *
* -->|-----|  flowIndex = 1, offset = "", superOffset = "", Normal Tile (1) *
*    |     |                                                                *
*    |     |                                                                *
*    |     |                                                                *
* -->|-----|  flowIndex = 2, offset = "", superOffset =  "", Normal Tile (2)*
*    |     |                                                                *
*    |     |                                                                *
*    |     |                                                                *
* -->|-----|  flowIndex = 0, offset = 14, superOffset = X, Last Tile        *
*    |     |                                                                *
*    +-----+                                                                *
*                                                                           *
* @param flowCnt   :  This object holds the information for flow maintained *
*                     by pipe connected to this joint, Information which    *
*                     flows via this joint is referred as flow unit and     *
*                     indicated by icntx and flowUnitBoundary               *
*                     Since the flow can be different initially to prime the*
*                     node compared to steady state and then different at   *
*                     the end, so 3 stages are defined and indicated by     *
*                     eFlowStage unit : elements ( not bytes)               *
* @param flowDim   :  This object holds the jump after each icnt,           *
*                     elements(not bytes)                                   *
* @param flowUnitBoundary: indicates the qunatum unit of transaction by this*
*                     joint- it can be 1D, 2D,.... refer eDim               *
*                     for 1D case icnt0 is the  amount of data exchange in  *
*                     each iteration. similarly for 2D, it is icnt0 * icnt1,*
*                     and so on....                                         *
* @param freq   : Indicates how many times the joint is transitioning. Freq *
*                 is the unit represented by iCNT of this joint             *
* @param repeat : Indicates how many times the joint repeats the pattern, if*
*                 repeat !=1 means the effective freq is repeat*freq        *
*                 repeat !=1 implies 2 cases - (A) address are also reset - *
*                 mostly in DL use case when weights fetched multiple times *
*                 for such case repeatOffset shall be zero                  *
*                 (B) address are not reset and continues but the pipeup,   *
*                 pipeline and pipedown state repeats - eg grouped conv in  *
*                 DL for such case repeatOffset shall be non-zero           *
* @param repeatOffset : Indicates that after a full cycle of freq, how much *
*                 does the joint moves before repeating the pattern         *
* @param period : Indicates for how many iterations a joint stays at one    *
*                 place                                                     *
****************************************************************************/
typedef struct {
  int32_t type;
  int32_t bufDBindex;
  int32_t parentBufDBIndex[2];
  int32_t childBufDBIndex[2];
  int32_t offset;
  int32_t superOffset;
  int32_t flowCnt[MAX_FLOW_STAGES][(int32_t)FOUR_D + 1];
  int32_t flowDim[MAX_FLOW_STAGES][(int32_t)FOUR_D-1];
  int32_t flowUnitBoundary;
  int32_t freq; 
  int32_t repeat;
  int32_t repeatOffset;
  int32_t period;
} sJoint_t;

#define NUM_SRC_STATIC          (MAX_SRC)
#define NUM_LINKS_PER_WL_STATIC (MAX_LINKS_PER_WL)

/**<
*******************************************************************************
* @struct sLink_t                                                             *
* @brief  This structure defines the complete pipe, it has joints for both src*
*         and sink, joints has all the info to get connected to a node and    *
*         then the pipe flow properties are also defined in this object       *
* @param src    : Array of joints connected to multiple source                *
* @param sink   : Joint connected to sink                                     *
* @param type   : Type  of Link: refer @eLinkType                             *
* @param numSrc : Number of source joints                                     *
* @param delay  : Indicates initial delay for link to be active               *
* @param freq   : Indicates how many times the link is transitioning to active*
*                 state  including all stages indicated by eFlowStage, please *
*                 note that this is not about how many times link will be     *
*                 active, one link can be active for lets say during all      *
*                 iterations but may have only 1 tarnsition and shall be      *
*                 referred as freq = 1                                        *
* @param repeat : Indicates how many times the link repeats the pattern, if   *
*                 repeat !=1 means the effective freq is repeat*freq          *
*                 repeat !=1 implies 2 cases - (A) address are also reset -   *
*                 mostly in DL use case when weights fetched multiple times   *
*                 for such case link type shall be LINK_XFER_MULTI_PASS_FR    *
*                 (B) address are not reset and continues but the pipeup,     *
*                 pipeline and pipedown state repeats - eg grouped conv in DL *
*                 for such case link type shall be LINK_XFER_MULTI_PASS_PR    *
* @param period : Indicates for how many iterations a link get to finish its  *
*                 task for one sub-unit. IN other words for xfer link, it is  *
*                 number of iterations b/w DMA trigger and wait               *
* @param dbIndex: Index in sGCDataBase_t:procLinksDB for Proc Link            *
*                 Don't care for transfer link as of now                      *
******************************************************************************/
typedef struct
{
  int64_t  id;
  int32_t  type ;
  int32_t  subType;
  int32_t  delay;
  int32_t  freq;
  int32_t  repeat;
  int32_t  period;
  int32_t  dbIndex;
  int32_t  numSrc;
  int8_t   configureTrigger;
  sJoint_t sink;
  sJoint_t src[MAX_SRC];
} sLink_t;

/**** All structures and enums definiton related to Link/joints ends } */

/**<
*****************************************************************************
* @struct sWorkloadUnit_t                                                   *
* @brief  This structure holds the information for a single workload unit   *
* @param dependId  : The ID of workload on which it has dependency,         *
*                   for eg. start processing only when depending WL finishes*
* @param wlExecId  : The ID of current workload which is same across all    * 
*                    cores not same as workload index                       *
* @param metaData  : Stores metadata information related to WL eg. numSplit *
* @param multiCoreMetaData  : Stores metadata information related to MultiC7x Execution *
                              eg. numCh which indicated number of channel to be executed*
                              by that core                                       *
* @param networkId : Id to indicate that to which network workload belongs  *
* @param layerId   : Id to indicate that to which layer workload belongs    *
* @param subLayerId: For a given network, given layer - there can be one or *
*                    multiple workload unit, so we need to define subLayerId*
*                    this is mainly useful when a workload is divided among*
*                    different processors - channel wise or spatially       *
* @param numLinks  : Number of active links in the workload                 *
* @param numIter   : Number of iterations for workload unit to finish       *
* @param link      : workload is represented by a list of  links            *
* @param isWLNOP   : if current workload is NOP or not                      *

 @Todo : May be we keep this as dynamic size instead of fix size            *
****************************************************************************/

typedef struct
{
  int32_t dependId[MAX_CORES];
  int32_t wlExecId;
  int64_t metadata;
  int64_t metadataExt;
  int32_t networkId;
  int32_t layerId;
  int32_t subLayerId;
  int32_t numIter  ;
  int32_t isWLNOP;
  void*   privInfo ;
  sProcParams_t procParam;
#ifdef SOC_TDA54
    int32_t layerCycles;
    int32_t kernelCycles;
    int32_t coreLoopCycles;
    int32_t dmaPipeUpCycles;
    float ddrBandwidthRead;
    float ddrBandwidthWrite;
#endif
  int32_t numLinks ;
  int16_t linkOffset[MAX_LINKS_PER_WL];
  sLink_t dummyLinkSpace[NUM_LINKS_PER_WL_STATIC];
} sWorkloadUnit_t;

#ifdef SOC_TDA54
typedef struct 
{
  int32_t layerCycles;
  int32_t kernelCycles;
  int32_t coreLoopCycles;
  int32_t dmaPipeUpCycles;
  float ddrBandwidthRead;
  float ddrBandwidthWrite;
}sTmpPerfData;
#endif

/**<
*******************************************************************************************
* @struct sTensorDims_t                                                                   *
* @brief  This structure holds the dimensions of a tensor                                 *
* @param width     : Tensor width                                                         *
* @param height    : Tensor height                                                        *
* @param channel   : Number of channels in tensor per betch                               *
* @param batch     : number of batches in tensor                                          *

 @Todo : May be we keep this as dynamic size instead of fix size                          *
******************************************************************************************/
typedef struct
{
  int32_t width;
  int32_t height;
  int32_t channel;
  int32_t batch;
  int32_t layout;
} sTensorDims_t;

/**<
*******************************************************************************************
* @struct sTensorPosition_t                                                               *
* @brief  This structure holds the starting position of output subtensor with respect to  *
          base memory of physical buffer                                                  * 
* @param startWidth      : start position in Width dimension                              *
* @param startHeight     : start position in Height dimension                             *
* @param startChannel    : start position in Channel dimension                            *
* @param startBatch      : start position in Batch dimension                              *

 @Todo : May be we keep this as dynamic size instead of fix size                          *
******************************************************************************************/
typedef struct
{
  int32_t startWidth;
  int32_t startHeight;
  int32_t startChannel;
  int32_t startBatch;
} sTensorPosition_t;

typedef enum
{
DB_WL = 0,      //Workload database
DB_AUXWL,   //Auxillary Workload database
DB_MAX
} eDataBaseType;

typedef enum
{
  DB_COMMON_BUF = 0,      //buffer database shared across all cores
  DB_COMMON_MAX
} eCommonDataBaseType;

/**<
*******************************************************************************************
* @struct sDBParams_t                                                                     *
* @brief  This structure holds the information about each dataBase in dbpayload           *
* @param type                   : type of dataBase refer  @eDataBaseType                  *
* @param size                   : size of the dataBase in Bytes, inside dbPayload         *
* @param numElements            : Number of entries in the Database                       * 
*                                 (A database is an array of structures )                 *
* @param  startOffset           : the starting offset with dbPayload as base address,     *
*                                  from where the database starts                         *
******************************************************************************************/
typedef struct 
{
int32_t type; //BUF, WL, Aux WL
int32_t size;
int32_t numElements;
int32_t startOffset;
} sDBParams_t ;

/**<
*******************************************************************************************
* @struct sWorkloadUnitAuxilary_t                                                         *
* @brief  This structure holds the debug information for a single workload unit           *
* @param OutFullTensorDims      : Full tensor dims of layer output data                   *
* @param OutSubTensorDims       : Sub tensor dims of Workload (sub-layer)output data      *
* @param OutSubTensorPosition   : Sub tensor position of Workload (sub-layer)output data  *

 @Todo : May be we keep this as dynamic size instead of fix size                          *
******************************************************************************************/
typedef struct
{
  sTensorDims_t outFullTensorDims;
  sTensorDims_t outSubTensorDims;
  sTensorPosition_t outSubTensorPosition;
} sWorkloadUnitAuxilary_t;

#define MAX_BUF_NODES_PER_CORE  ((int32_t)MAX_BUF_PER_LINK * MAX_WORKLOAD_PER_CORE)
#define MAX_PROC_LINKS_PER_CORE (64) /**< Assuming type of processing (layers in context of TIDL) supported */

#define AVERAGE_SIZE_OF_A_WL_ACROSS_DB  (sizeof(sWorkloadUnit_t) / 50U)
#define MAX_DB_COMMON_SIZE              ((uint32_t)MAX_BUF_NODES_PER_CORE * MAX_CORES *  sizeof(sBufParams_t))
#define MAX_DB_SIZE                     ((uint32_t)MAX_WORKLOAD_PER_CORE  * (AVERAGE_SIZE_OF_A_WL_ACROSS_DB + sizeof(sWorkloadUnitAuxilary_t)))
#define MAX_GC_DB_SIZE                  (sizeof(sGCCommonDataBase_t) + (sizeof(sGraphCompilerOutArgsCore_t) * MAX_CORES))
/**<
*****************************************************************************
* @struct sGCDataBase_t                                                     *
* @brief  This structure holds the data base of all key information to      *
*         represent a processing chain                                      *
* @param dbSize         : Total size of database in bytes                   *
* @param dbParams       : Holds information about each database             *
*                         that is part of the payload                       *
* @param dbPayLoad      : holds each  database  sequentially.               *
*                         offset to access each database inside it          *
*                         is obtained from dbParams                         *
****************************************************************************/

typedef struct
{
int32_t dbSize;
sDBParams_t dbParams[DB_MAX];
int8_t dbPayLoad[MAX_DB_SIZE];
} sGCDataBase_t;


typedef struct
{
  int32_t dbSize;
  sDBParams_t dbParams[DB_COMMON_MAX];
  int8_t dbPayLoad[MAX_DB_COMMON_SIZE];
} sGCCommonDataBase_t;


/**<
*******************************************************************************
* @struct sWLDataBase_t                                                       *
* @brief  This structure holds the data base of all workload units            *
* @param count     : Number of workload units                                 *
* @param pWLList   : pointer to workload unit List                            *
******************************************************************************/
typedef struct {
  int32_t count;
  int32_t wlOffset[MAX_WORKLOAD_PER_CORE];
  sWorkloadUnit_t *pWLList;
} sWLDataBase_t;

/**<
*******************************************************************************
* @struct sWLAuxilaryDataBase_t                                               *
* @brief  This structure holds debug information for all workload units       *
* @param count        : Number of workload units                              *
* @param pWLAuxList   : pointer to Auxilary workload unit List                *
******************************************************************************/
typedef struct {
  int32_t count;
  sWorkloadUnitAuxilary_t *pWLAuxList;
} sWLAuxilaryDataBase_t;

/**<
*******************************************************************************
* @struct sBufDataBase_t                                                      *
* @brief  This structure holds the data base of all buffer nodes              *
* @param count     : Number of nodes                                          *
* @param pNodeList : pointer to List of nodes                                 *
******************************************************************************/
typedef struct {
  int32_t count;
  sBufParams_t *pBufList;
} sBufDataBase_t;

/**<
*******************************************************************************
* @struct sGCDataBaseInfo_t                                                   *
* @brief  This structure holds the data base  information of all nodes        *
* @param bufDB  : Data base for all buffer nodes                              *
* @param wlDB   : Data base for all workload units                            *
* @param wlDB   : Data base for all auxillary workload units                  *
******************************************************************************/
typedef struct {
  sWLDataBase_t    wlDB;
  sWLAuxilaryDataBase_t wlAuxDB;
} sGCDataBaseInfo_t;

typedef struct {
  sBufDataBase_t   bufDB;
} sGCCommonDataBaseInfo_t;
/**<
*****************************************************************************
* @struct sWorkloadSubGroup_t                                               *
* @brief  This structure holds the workload sub group position in the       *
*         workload unit data base @workLoadUnitDB                           *
* @param startIndexWLUnitDB : Start Index in the workload unit data base    *
* @param endIndexWLUnitDB   : End Index in the workload unit data base      *
* @param count              : How many times start to end workload units    *
*                             needs to be executed                          *
* There are cases when the same set of workload units repeatedly work on a  *
* part of tensor,then this parameter plays an important role. Eg            *
* 10 layers are part of a group and each layer  has 7 parts for processing: *
* 1 x First +  5 x Normal + 1 x Last then the workload units would be       *
*  0...9 : 1st tile processing across these 10 layers, cnt=1                *
* 10..19 : center 5 tiles processing across 10 layers, cnt =5 , superOffset *
*          in buf node data base should be used to indicate the right offset*
* 20..29 : Last tile processing across these 10 layers, cnt=1               *
****************************************************************************/
typedef struct
{
  int32_t startIndexWLUnitDB;
  int32_t endIndexWLUnitDB;
  int32_t count;
} sWorkloadSubGroup_t;

/**<
*****************************************************************************
* @struct sWorkloadGroup_t                                                  *
* @brief  Object for a group of workloads and its dependency information    *
* @param activateGroup : A workload sub group to be called to activate and  *
*                        shall be called only after deactivation of the grp *
* @param subGroup      : A workload sub group represented by the start and  *
*                        end position in workload unit data base            *
*                        for now considering that we can manage the group   *
*                        having contiguous workloads from workload data base*
*                        but if not then need to make this as array of more *
*                        entries                                            *
* @param id            : Unique ID for this workload with below format      *
*                        CORENUM (4 bits)_(STAGE_4bits)_(GRPID_24bits)      *
*                        Lazy to define enums and macros for set and get    *
*                        but STAGE indicates if it is input (0),            *
*                        intermediate (1) or Final (2) workload group.      *
*                        Final (2) workload group means that output of it   *
*                        can be given to application and similarly 0 means  *
*                        that it is entry point and buffer is supplied by   *
*                        application                                        *
* @param dependId      : Indicates that on which  work group does this work *
*                        group depend on. -1 indicates no-dependency        *
****************************************************************************/
typedef struct
{

  sWorkloadSubGroup_t activateGroup ;
  sWorkloadSubGroup_t subGroup[TIDL_MAX_NUM_GROUPS*3U]; //Points to the workload unit data base
  int32_t numSubGroups;
} sWorkloadGroup_t;

#define MAX_BATCH_GROUPS (TIDL_NUM_MAX_LAYERS)
#define MAX_WORKLOAD_GRP_PER_CORE (8) //!< For now limiting workload group to be 8 per core
/**<
*****************************************************************************
* @struct sWorkLoadSuperGroup_t                                             *
* @brief  Object for entire workload                                        *
* @param workloadGrpList : List of workload groups scheduled per core       *
****************************************************************************/
typedef struct {
  sWorkloadGroup_t workloadGrpList[MAX_WORKLOAD_GRP_PER_CORE];
  int32_t numGroups;
} sWorkloadSuperGroup_t;

typedef struct{
  int32_t bufDBIndexToBackup[TIDL_NUM_MAX_LAYERS * 3][20];
  int32_t backupBuffersCount[TIDL_NUM_MAX_LAYERS * 3];
  float64_tidl procTime[TIDL_NUM_MAX_LAYERS * 3];
  float64_tidl preEmptWorstCaseDelay;
} sPreEmption_t;

/**<
****************************************************************************
* @struct sGraphCompilerOutArgs_t                                          *
* @brief  Output object from Graph Compiler                                *
* @param simConfig : Input configuration parameters                        *
* @param memorySizeRequirement : Memory size requirement per core          *
* @param superWorkload         : Complete workload distribution information*
* @param dataBase              : Data base being referred by workloads     *
****************************************************************************/
typedef struct
{
  int32_t               memoryRequirementPerCore[SUPERMEMTYPE_TOTAL];
  sWorkloadSuperGroup_t superWorkload;
  sGCDataBaseInfo_t     dataBaseInfo;
  sPreEmption_t         preEmptionInfo;
  sGCDataBase_t         dataBase;
} sGraphCompilerOutArgsCore_t;

typedef struct
{
  int32_t  size;
  int32_t  offset;
}sDataBaseMemoryRegionInfo_t;

/**
*  @enum       eDataBaseType
*  @brief      Enumeration for different types of databases
*/
typedef enum
{
  DATABASE_COMMON = 0,   /**< Common Database. Contains 1> BufferDatase */
  DATABASE_GCOUT_1,     /**< DataBase for GC Output of 1st Core */
  DATABASE_GCOUT_2,     /**< DataBase for GC Output of 2nd Core */
  DATABASE_GCOUT_3,     /**< DataBase for GC Output of 3rd Core */
  DATABASE_GCOUT_4,     /**< DataBase for GC Output of 4th Core */
  MAX_DATABASE_TYPE     /**< Maximum types of DataBase */
} eGCDataBaseType;

typedef struct
{
  int32_t memoryRequirementGlobal[SUPERMEMTYPE_TOTAL];
  sGCCommonDataBaseInfo_t   commonDatabaseInfo;
  sDataBaseMemoryRegionInfo_t dataBaseRegionInfo[MAX_DATABASE_TYPE];
  int32_t totalDataBaseSize;
  int8_t dataBase[MAX_GC_DB_SIZE];
} sGraphCompilerOutArgs_t;

/** @} */ // end of group workloadObject
/** @} */ // end of group graphCompiler

#endif
