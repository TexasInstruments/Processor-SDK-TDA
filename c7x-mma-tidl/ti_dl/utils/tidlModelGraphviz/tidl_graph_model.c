/**
----------------------------------------------------------------------------
@file    tidl_graph_model.c
@brief     Visualisation utility for tidl_models (converts to dot
format (.gv) for graphviz)
----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
//#include <cstdlib>
#include <itidl_ti.h>
#include<cgraph.h>
#include<gvc.h>
#include "perfsim.h"
#include "tidl_dump.h"
#include "tidl_strings.h"
#include <sstream>
#include <string>
#include "tidl_graph_model_template.h"
#define MAX_LAYER_NODES 2048
#define MAX_LAYER_EDGES 2048
#define MAX_FILE_NAME_SIZE 1024
char TIDL_LayerNames[MAX_LAYER_NODES][300] = {0};
bool legacyMode = false;   // if true via -legacy, use "classic" node info
using namespace TIDL_Strings;
#include "gc.h"
#include "gc_helper.h"

const char *TIDL_graphFontColors[] =
{
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF",
  "FFFFFF"
};

const char *TIDL_nodeElemTypeColors[] =
{
  "#0f0f0f",    //"TIDL_UnsignedChar",
  "#0f0f0f",    //"TIDL_SignedChar",
  "#A84060",    //"TIDL_UnsignedShort",
  "#A84060",    //"TIDL_SignedShort",
  "#faf200",    //"TIDL_UnsignedWord",
  "#faf200",    //"TIDL_SignedWord",
  "#00fa04",    //"TIDL_SinglePrecFloat",
  "#bb00fa",    //"TIDL_UnsignedDoubleWord",
  "#bb00fa",    //"TIDL_SignedDoubleWord",
  "#3235d3ff",    //"TIDL_Bool"
};

const char *TIDL_graphBgColors[] =
{
  "#707070",    //"Data",
  "#375588",    //"Convolution",
  "#338533",    //"Pooling",
  "#4B1616",    //"ReLU",
  "#4B1616",    //"PReLU",
  "#209070",    //"EltWise",
  "#30C0D0",    //"InnerProduct",
  "#4B1616",    //"SoftMax",
  "#B07320",    //"BatchNorm",
  "#DC143C",    //"Bias",
  "#EE82EE",    //"Scale",
  "#802C4C",    //"Deconv2D",
  "#804000",    //"Concat",
  "#9932CC",    //"Split",
  "#6495ED",    //"Slice",
  "#B0C4DE",    //"Crop",
  "#778899",    //"Flatten",
  "#87CEEB",    //"DropOut",
  "#007F9F",    //"ArgMax",
  "#5499C7",    //"DetectionOutput",
  "#30C0B0",    //"ShuffleChannel" ,
  "#A84060",    //"Resize" ,
  "#BB8FCE",    //"RoiPooling",
  "#3366CC",    //"OdPostProcessing",
  "#A84060",    //"DepthToSpaceLayer",
  "#A84060",    //"Sigmoid",
  "#A84060",    //"PadLayer",
  "#A84060",    //"ColorConversionLayer",
  "#A84060",    //"ODOutputReformat",
  "#A84060",    //"DataConvertLayer",
  "#A84060",    //"Custom",
  "#A84060",    //"BatchReshape",
  "#A84060",    //"Reduce" ,
  "#A84060",    //"ScatterElements" ,
  "#B22222",    //"Squeeze" ,
  "#A84060",    //"Tanh" ,
  "#A84060",    //"HardSigmoid" ,
  "#A84060",    //"ELU" ,
  "#4B1616",    //"Reshape" ,
  "#800080",    //"ConstData" ,
  "#A84060",    //"Gather" ,
  "#008000",    //"Transpose" ,
  "#008000",    //"LayerNorm" ,
  "#4B1616",    //"GridSample" ,
  "#4B1616",    //"TopK" ,
  "#008a63",    //"DeformableConv",
  "#660066",    //"Tile",
  "#8a7be4ff",  //"LogicalOpLayer"
  "#4B1616",    //"RMSNormalizationLayer" ,
  "#00008b",    //"LSTM",
  "#00008b",    //"GRU",
  "#00008b",    //"RNN",
  "#A84060",    //"UnSuported" ,
  "#A84060",    //"PriorBox" ,
  "#A84060",    //"Permute" ,
  "#A84060",    //"Shape" ,
  "#A84060",    //"Clip" ,
  "#A84060",    //"Minimum" ,
  "#A84060",    //"LeakyRelu" ,
  "#A84060",    //"Identity" ,
  "#A84060",    //"BatchtoSpace" ,
  "#A84060",    //"SpacetoBatch" ,
  "#A84060",    //"Pack" ,
  "#A84060",    //"Dequantize" ,
  "#A84060",    //"Quantize" ,
  "#A84060",    //"Cast" ,
};

/**
----------------------------------------------------------------------------
@fn         tidl_graphGetModelSize
@brief      Function returns the size of the model binary

@param      fileString : Binary file name

@return     File size on success, -1 in case of an error
@remarks    None
----------------------------------------------------------------------------
*/
int64_t tidl_graphGetModelSize(char *fileString)
{
  FILE *fptr;
  int64_t netSize;
  fptr = fopen(fileString, "rb");
  if (fptr)
  {
    fseek(fptr, 0L, SEEK_END);
    netSize = ftell(fptr);
    fclose(fptr);
    return netSize;
  }
  else
  {
    printf("Could Not Open Files %s\n", fileString);
    return -1;
  }
}

/**
----------------------------------------------------------------------------
@fn         tidl_graphReadNet
@brief      Function reads the binary model and stores it in the opaque struct
of type sTIDL_Network_t (defined in itidl_ti.h)

@param      net : Pointer to the opaque structure
@param      fileString : Binary file name

@return     0 on success, -1 in case of an error
@remarks    None
----------------------------------------------------------------------------
*/
int32_t tidl_graphReadNet(sTIDL_Network_t * net, char * fileString)
{
  FILE *fptr;
  int64_t netSize = tidl_graphGetModelSize(fileString);
  fptr = fopen((const char *)fileString, "rb");
  if(fptr)
  {
    fread(net,1, netSize, fptr);
    fclose(fptr);
    return 0;
  }
  else
  {
    printf("Could Not Open Files %s\n",fileString);
    return -1;
  }
}

int32_t tidl_getInLayer(const sTIDL_Network_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId)
{
  int32_t i1, i2;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    for (i2 = 0; i2 < pOrgTIDLNetStructure->TIDLLayers[i1].numOutBufs; i2++)
    {
      if (pOrgTIDLNetStructure->TIDLLayers[i1].outData.dataId == dataId)
      {
        return (i1);
      }
    }
  }
  return (-1);
}
#if 0
int32_t tidl_getOutLayer(sTIDL_Network_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId)
{
  int32_t i1, i2;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    for (i2 = 0; i2 < pOrgTIDLNetStructure->TIDLLayers[i1].numInBufs; i2++)
    {
      if (pOrgTIDLNetStructure->TIDLLayers[i1].inData[i2].dataId == dataId)
      {
        return (i1);
      }
    }
  }
  return (-1);
}
#endif

// This is the original code that provides layer details shown when hovering over the node.
// It's now replaced by the new TIDL_dump mechanism. The intention is to delete this
// once everyone is satisfied with the content and format of the new description.
static void tidl_LegacyNodeInfo(char *nodeName, const sTIDL_Network_t* pTIDLNetStructure, const sPerfSim_t *perSimInfo, int i)
{
  char tempBuff[1024];
  std::string layerTypeName = TIDL_Strings::layerTypeShort(pTIDLNetStructure->TIDLLayers[i].layerType);

  int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[0]);

  if(inLayerIdx == -1)
  {
    inLayerIdx = i;
  }

  if(pTIDLNetStructure->TIDLLayers[i].numOutBufs != -1)
  {
    sprintf(nodeName,"Layer %d Data ID: %d: %s %s \nInput Dimensions : %dx%dx%dx%dx%dx%d\nOutput Dimensions : %dx%dx%dx%dx%dx%d\n", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId, layerTypeName.c_str(),
    TIDL_LayerNames[i], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],TIDL_LayerNames[i], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1],TIDL_LayerNames[i], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]
    , pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH]
    );
  }
  else
  {
    sprintf(nodeName,"Layer %d Data ID : %d: %s %s\nInput Dimensions : %dx%dx%dx%dx%dx%d\nOutput Dimensions : %dx%dx%dx%dx%dx%d\n", i, (pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dataId), layerTypeName.c_str(),
    TIDL_LayerNames[i], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]
    , pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH]
    );
  }
  strcat(nodeName,"In Data IDs: ");
  for (int j = 0 ; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
  {
    sprintf(tempBuff,"%d ", pTIDLNetStructure->TIDLLayers[i].inData[j]);
    strcat(nodeName,(const char*)tempBuff);
  }
  strcat(nodeName,"\n");

  sprintf(tempBuff,"actType : %d \n", pTIDLNetStructure->TIDLLayers[i].actParams.actType);
  strcat(nodeName,(const char*)tempBuff);
  sprintf(tempBuff,"elementType : %d \n", pTIDLNetStructure->TIDLLayers[i].outData.elementType);
  strcat(nodeName,(const char*)tempBuff);
  sprintf(tempBuff,"Min-Max : %f , %f \n", pTIDLNetStructure->TIDLLayers[i].outData.minTensorValue, pTIDLNetStructure->TIDLLayers[i].outData.maxTensorValue);
  strcat(nodeName,(const char*)tempBuff);

//Layer Specific information:
  switch(pTIDLNetStructure->TIDLLayers[i].layerType)
  {
    case 1: //Convolution
	    sprintf(tempBuff,"Kernel dimensions : %dx%d \nStrides: %dx%d \nDilations : %dx%d\nGroups : %d\n",pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.kernelW,
		    pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.kernelH, pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.strideW,
		    pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.strideH, pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.dilationW,
		    pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.dilationH,
	      pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.numGroups);
	    strcat(nodeName,(const char*)tempBuff);
	    break;

    case 2: //Pooling
    if(pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.kernelW != 0)
    {
      sprintf(tempBuff,"Kernel dimensions : %dx%d \nStrides: %dx%d \n", pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.kernelW, pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.kernelH,
	      pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.strideW, pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.strideH);
    }
    else
    {
      sprintf(tempBuff,"Global Pooling\n");
    }
    strcat(nodeName,(const char*)tempBuff);
    if(pTIDLNetStructure->TIDLLayers[i].layerParams.poolParams.poolingType==0)
    {
      strcat(nodeName,"Pooling type : Max Pooling\n");
    }
    else
    strcat(nodeName,"Pooling type : Average Pooling\n");
    break;
    case 11: //DeConvolution
		  sprintf(tempBuff,"Kernel dimensions : %dx%d \nStrides: %dx%d \nDilations : %dx%d\nGroups : %d\n",pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.kernelW,
			  pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.kernelH, pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.strideW,
			  pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.strideH, pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.dilationW,
			  pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.dilationH,
		    pTIDLNetStructure->TIDLLayers[i].layerParams.convParams.numGroups);
		  strcat(nodeName,(const char*)tempBuff);
		  break;

    case 12: //Concat
	    sprintf(tempBuff,"Axis : %d\n",pTIDLNetStructure->TIDLLayers[i].layerParams.concatParams.axis);
	    strcat(nodeName,(const char*)tempBuff);
	    break;

    default: break;
  }
  /* // This information might be derived from the sBufParams_t.
  if (perSimInfo)
  {
    sprintf(tempBuff, "\nSpace : %d \nBase Mem: %d (%5.3f) - %d (%5.3f) \nSize : %d (%5.3f) \nCh Pitch : %d\nPadRC_IO = %d, %d -> %d, %d\n", perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].space,
      perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].baseMem,
      (perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].baseMem/1000000.0),
      (perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].baseMem + perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].bufSize),
      (perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].baseMem + perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].bufSize)/1000000.0,
      perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].bufSize,
      perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].bufSize/1000000.0,
      perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].bufWidth,
      perSimInfo->sdataFlowInfo[i].bufInfo[IN_FEAT_MAP][READ].padR, perSimInfo->sdataFlowInfo[i].bufInfo[IN_FEAT_MAP][READ].padC,
      perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padR, perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padC);
    strcat(nodeName, (const char*)tempBuff);
#if ENABLE_DEBUG
    sprintf(tempBuff, "PadRC = %d, %d\nPadRCZeros = %d, %d\nPadRCFillZeros = %d, %d\n",
    perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padR, perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padC,
    perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padRZeros, perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padCZeros,
    perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padRFillZeros, perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padCFillZeros
    );
    strcat(nodeName, (const char*)tempBuff);
#endif
  }*/
}

// Provide layer information shown when hovering over a node
static std::string tidl_NodeInfo(const sTIDL_Network_t* pTIDLNetStructure, const sPerfSim_t *perSimInfo, int layerNum)
{
   if (legacyMode)
   {
      char nodeInfo[4096];
      tidl_LegacyNodeInfo(nodeInfo, pTIDLNetStructure, perSimInfo, layerNum);
      return std::string(nodeInfo);
   }
   // Use the TIDL_dump utility to provide layer information
   TIDL_dump::options_t options = {{"perfsim", true}};
   TIDL_dump::layerNames_t layerNames; 
   for (int i = 0; i < pTIDLNetStructure->numLayers; ++i)
      layerNames[i] = TIDL_LayerNames[i];
   std::ostringstream oss;
   TIDL_dump dumper(oss, pTIDLNetStructure, /*io=*/nullptr, &layerNames, options);
   dumper.dumpLayer(layerNum);
   return oss.str();
}

/**
----------------------------------------------------------------------------
@fn         tidltb_dotPrintNetInfo
@brief      Function creates a dot file from the information in the opaque
struct sTIDL_Network_t.

@param      pTIDLNetStructure : Pointer to the opaque structure
@param      fname : Binary file name

@return     0 on success
@remarks    None
----------------------------------------------------------------------------
*/
int32_t tidltb_dotPrintNetInfo(sTIDL_Network_t *pTIDLNetStructure, const char *fname)
{
  FILE *fpNames;
  int32_t i, j, i1, i2, temp1, temp2, temp3;
  int32_t status;
  Agnode_t *n[MAX_LAYER_NODES];
  Agedge_t *e[MAX_LAYER_EDGES];
  Agraph_t *g;
  char nodeName[4096], tempBuff[1024], fileName[1024], liFileName[1024], htmlFileName[1024];
  GVC_t *gvc;
  sPerfSim_t * perSimInfo = NULL;
  gvc = gvContext();

  //Default attributes:
  Agsym_t *sym;
  strcpy(fileName, fname);
  strcat(fileName, ".svg");
  strcpy(htmlFileName, fname);
  strcat(htmlFileName, ".html");
  g = agopen((char*)fname, Agdirected, NULL);
  sym = agattr(g, AGNODE,"color", "#000000");
  sym = agattr(g,AGNODE, "fontcolor", "#FFFFFF");
  sym = agattr(g,AGNODE, "fontsize", "10");
  sym = agattr(g,AGNODE, "height", "0.24");
  sym = agattr(g, AGNODE, "style", "rounded,filled");
  sym = agattr(g, AGNODE, "penwidth", "2");
  sym = agattr(g, AGNODE, "fillcolor", "#000000");
  sym = agattr(g, AGRAPH, "pad", "6.5,0.2,6.5,0.5");
  sym = agattr(g, AGRAPH, "rankdir", "TB");
  sym = agattr(g, AGRAPH, "ranksep","0.1");
  sym = agattr(g, AGNODE, "label","Default");
  sym = agattr(g, AGEDGE, "label","Default");
  sym = agattr(g,AGEDGE, "fontsize", "8");
  sym = agattr(g, AGEDGE, "penwidth", "0.7");
  sym = agattr(g, AGEDGE, "arrowhead", "vee");
  sym = agattr(g, AGEDGE, "arrowsize", "0.5");
  //sym = agattr(g,AGEDGE, "minlen", "1");
  sym = agattr(g, AGNODE, "shape", "box");
  strcpy(liFileName, fname);
  strcat(liFileName, ".layer_info.txt");
  fpNames = fopen(liFileName, "r");
  if (fpNames != NULL)
  {
    for (i = 0; i < pTIDLNetStructure->numLayers; i++)
    {
      char name[300];
      if(pTIDLNetStructure->inferenceMode != TIDL_inferenceModeLowLatency)/*single core inference*/
      {
        fscanf(fpNames, "%d %d %s", &temp1, &temp2, name);
      }
      else
      {
        fscanf(fpNames, "%d %d %d %s", &temp1, &temp2, &temp3, name);
      }
      strcpy(TIDL_LayerNames[i], name);
    }
    fclose(fpNames);
  }
  if (pTIDLNetStructure->dataFlowInfo != NULL)
  {
    perSimInfo = (sPerfSim_t *)((int8_t *)(pTIDLNetStructure) +pTIDLNetStructure->dataFlowInfo);
  }

  int32_t currAlgLayer = 0;
  int32_t wlRepeatIter;
  sWorkloadUnit_t  *workloadUnit = NULL;
  sGetLayerIdContext_t getLayerIdContext;
  int32_t layerIdxCount = 0;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGraphCompilerOutArgsCore_t *gcOut = NULL;
  sGCCommonDataBase_t *commonDatabase = NULL;
  sGCDataBase_t           *gcDataBase = NULL;
  sWorkloadSuperGroup_t *wlSuperGrp = NULL;
  int32_t visited[MAX_LAYER_NODES] = {0};
  sGCHelperHandle           *gcHelperHandle = NULL;
  sGCHelperHandle            gcHelperHandleObj;

  if ( pTIDLNetStructure->isGCInfoAvailable != 0 )
  {
    gcOutArgs  = (sGraphCompilerOutArgs_t *)get_int8_t_pointer((int8_t *)(pTIDLNetStructure), pTIDLNetStructure->netBufInfo[TIDL_NET_GC_BUF].offset);
    gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs , DATABASE_GCOUT_1);
    commonDatabase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs,DATABASE_COMMON);
    gcDataBase = &gcOut->dataBase;

    sBufParams_t* bufNodesDB = (sBufParams_t*) &commonDatabase->dbPayLoad[commonDatabase->dbParams[DB_COMMON_BUF].startOffset];
    sWorkloadUnit_t* workLoadUnitDB = (sWorkloadUnit_t*) &gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_WL].startOffset];
    // sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_BUF].size +
    //                                                                       gcDataBase->dbParams[DB_WL].size];
    sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) &gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_AUXWL].startOffset];

    gcHelperHandle = &gcHelperHandleObj;
    gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
    gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
    gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;
    (void)initGCHelperHandle(gcHelperHandle, pTIDLNetStructure, &gcOut->dataBaseInfo, &gcOutArgs->commonDatabaseInfo);
    
    wlSuperGrp = &gcOut->superWorkload;
  }

  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    std::string layerTypeName = TIDL_Strings::layerTypeShort(pTIDLNetStructure->TIDLLayers[i].layerType);
    if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_DataLayer && pTIDLNetStructure->TIDLLayers[i].numInBufs == NOT_VALID)
    {
      std::string info = tidl_NodeInfo(pTIDLNetStructure, perSimInfo, i);
      // printf("Layer %d: %s\n", pTIDLNetStructure->TIDLLayers[i].outData.dataId, layerTypeName.c_str());
      // printf("info:\n-------------------\n%s\n----------------------\n", info.c_str());

      n[i] = agnode(g, (char*)info.c_str(), TRUE);
      agset(n[i], "fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      agset(n[i],"color",(char*)TIDL_nodeElemTypeColors[pTIDLNetStructure->TIDLLayers[i].outData.elementType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], "label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        sprintf(tempBuff, " %dx%dx%dx%dx%dx%d", pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2],
                pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]);
        agset(e[i], "label", (char*)tempBuff);
      }
      visited[pTIDLNetStructure->TIDLLayers[i].outData.dataId] = 1;
      layerIdxCount++;
    }
  }

  getLayerIdInit(&getLayerIdContext,
                      0,
                      -1,
                      pTIDLNetStructure,
                      gcHelperHandle,
                      wlSuperGrp);

  while(1)
  {

    i = getLayerIdToExecute(&getLayerIdContext,
                                     0,
                                     &wlRepeatIter,
                                     &currAlgLayer,
                                     &workloadUnit);

    if (i == NOT_VALID)
    {
      break;
    }

    if( visited[pTIDLNetStructure->TIDLLayers[i].outData.dataId] != 1)
    {
      std::string layerTypeName = TIDL_Strings::layerTypeShort(pTIDLNetStructure->TIDLLayers[i].layerType);
      if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_ScatterElementsLayer)
      {
        if(pTIDLNetStructure->TIDLLayers[i].layerParams.scatterElementsParams.axis == -1)
          layerTypeName = "ScatterND";
      }

      if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_ArgOpLayer)
      {
        layerTypeName = TIDL_Strings::argOpTypeString(pTIDLNetStructure->TIDLLayers[i].layerParams.argOpParams.argOpType);    
      }

      if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_LogicalOpLayer)
      {
        layerTypeName = TIDL_Strings::logicalOpTypeString(pTIDLNetStructure->TIDLLayers[i].layerParams.logicalOpLayerParams.operatorType);    
      }

      /*Update SVG to reflect activation instead of batchnorm when necessary:*/
      if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_BatchNormLayer)
      {
        if(pTIDLNetStructure->TIDLLayers[i].actParams.actType != TIDL_NoAct)
        {
          layerTypeName = TIDL_Strings::actTypeShort(pTIDLNetStructure->TIDLLayers[i].actParams.actType);
        }
      }

      /*Update SVG to reflect operator name instead of EltWise */
      if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_EltWiseLayer)
      {
        layerTypeName = TIDL_Strings::eltwiseOpTypeString(pTIDLNetStructure->TIDLLayers[i].layerParams.eltWiseParams.eltWiseType);
      }

      std::string info = tidl_NodeInfo(pTIDLNetStructure, perSimInfo, i);
      // printf("Layer %d: %s\n", pTIDLNetStructure->TIDLLayers[i].outData.dataId, layerTypeName.c_str());
      // printf("info:\n-------------------\n%s\n----------------------\n", info.c_str());

      n[i] = agnode(g, (char*)info.c_str(), TRUE);
      agset(n[i], "fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      agset(n[i],"color",(char*)TIDL_nodeElemTypeColors[pTIDLNetStructure->TIDLLayers[i].outData.elementType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], "label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        sprintf(tempBuff, " %dx%dx%dx%dx%dx%d", pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2],
                pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]);
        agset(e[i], "label", (char*)tempBuff);
      }
      visited[pTIDLNetStructure->TIDLLayers[i].outData.dataId] = 1;
      layerIdxCount++;
    }
    
  }

  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    std::string layerTypeName = TIDL_Strings::layerTypeShort(pTIDLNetStructure->TIDLLayers[i].layerType);
    if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_DataLayer && pTIDLNetStructure->TIDLLayers[i].numInBufs != -1)
    {
      std::string info = tidl_NodeInfo(pTIDLNetStructure, perSimInfo, i);
      // printf("Layer %d: %s\n", pTIDLNetStructure->TIDLLayers[i].outData.dataId, layerTypeName.c_str());
      // printf("info:\n-------------------\n%s\n----------------------\n", info.c_str());

      n[i] = agnode(g, (char*)info.c_str(), TRUE);
      agset(n[i], "fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], "label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        sprintf(tempBuff, " %dx%dx%dx%dx%dx%d", pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2],
                pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]);
        agset(e[i], "label", (char*)tempBuff);
      }
      visited[pTIDLNetStructure->TIDLLayers[i].outData.dataId] = 1;
      layerIdxCount++;
    }
  }

  gvLayout(gvc, g, "dot");
  gvRenderFilename(gvc, g, "svg", fileName);
  
  if (HTML_TEMPLATE != NULL)
  {
    long templateSize = strlen(HTML_TEMPLATE);
    char *templateContent = (char*)malloc(templateSize + 1);
    if (templateContent == NULL)
    {
      printf("Could not read html template. Skip converting svg to html.\n");
      gvFreeLayout(gvc, g);
      agclose(g);
      return 0;
    }
    strcpy(templateContent, HTML_TEMPLATE);

    FILE *svgFile = fopen(fileName, "r");
    if (!svgFile)
    {
      printf("Error opening SVG file : %s. Skip converting svg to html.\n", fileName);
      gvFreeLayout(gvc, g);
      agclose(g);
      return 0;
    }
    fseek(svgFile, 0, SEEK_END);
    long svgSize = ftell(svgFile);
    fseek(svgFile, 0, SEEK_SET);
  
    char *svgContent = (char *)malloc(svgSize + 1);
    if (!svgContent)
    {
      printf("Memory allocation failed for SVG content. Skip converting svg to html.\n");
      fclose(svgFile);
      gvFreeLayout(gvc, g);
      agclose(g);
      return 0;
    }
    fread(svgContent, 1, svgSize, svgFile);
    svgContent[svgSize] = '\0';
    fclose(svgFile);
  
    char *insertPos = strstr(templateContent, "<div id=\"mainSvg\">");
    if (!insertPos) {
      printf("Could not find placeholder div with id 'mainSvg' in HTML template. Skip converting svg to html.\n");
      free(svgContent);
      gvFreeLayout(gvc, g);
      agclose(g);
      return 0;
    }
    insertPos = insertPos + strlen("<div id=\"mainSvg\">");
  
    FILE *htmlFile = fopen(htmlFileName, "w");
    if (!htmlFile) {
      printf("Error creating HTML file : %s. Skip converting svg to html.\n", htmlFileName);
      free(svgContent);
      gvFreeLayout(gvc, g);
      agclose(g);
      return 0;
    }
  
    fwrite(templateContent, 1, insertPos - templateContent, htmlFile);
    fwrite(svgContent, 1, svgSize, htmlFile);
    fwrite(insertPos, 1, templateSize - (insertPos - templateContent), htmlFile);
    
    fclose(htmlFile);
    free(svgContent);
    remove(fileName);
  }
  
  gvFreeLayout(gvc, g);
  agclose(g);
  return 0;
}

int main(int argc,char *argv[])
{
  int status,i;
  sTIDL_Network_t *net;
  char *fname;
  for(i = 1 ; i < argc; ++i)
     if (strcmp(argv[i],"-legacy") == 0)
         legacyMode = true;
  if(argc==1)
  {
    printf("Enter the tidl-model binary file!\n");
  }
  else
  {
    for(i = argc-1 ; i >= 1 ; i--)
    {
      fname = argv[i];
      if (fname[0] == '-')
        continue;
      int64_t tidlModelSize = tidl_graphGetModelSize(fname);
      if (tidlModelSize != -1)
      {
        net = (sTIDL_Network_t *)malloc(tidlModelSize);
    }
    status = tidl_graphReadNet(net,fname);
    if(status==-1)
      {
         printf("Error opening binary file %s\n",fname);
      }
    else
      {
        tidltb_dotPrintNetInfo(net,fname);
        //printf("Dot generated for %s\n",fname);
      }
    }
  }
  return 0;
}