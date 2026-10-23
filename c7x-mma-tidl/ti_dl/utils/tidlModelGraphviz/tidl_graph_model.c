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

TIDL_dump::sLayerSpecificInfo_t layersInfo[TIDL_NUM_MAX_LAYERS];

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
  "#3235d3ff",  //"TIDL_Bool",
  "#ff8c00",    //"TIDL_BFloat16"
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
  "#00010b",    //"GatherND",
  "#9d0b9a",    //"Cast" ,
  "#050935",    //"GatherElements",
  "#7baf00",    //"Shape",
  "#00b4d8",    //"Size",
  "#eab676",    //"Attention"
  "#9cc576",    //"NonZero"
  "#a14405",    //"GroupQueryAttention"
  "#1572be",    //"MatMulNBits"
  "#FF7F50",    //"RotaryEmbedding"
  "#8e44ad",    //"Cache"
  "#00acc1",    //"MemCopy"
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
    layersInfo[i].layerName.c_str(), pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]
    , pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1],pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH]
    );
  }
  else
  {
    sprintf(nodeName,"Layer %d Data ID : %d: %s %s\nInput Dimensions : %dx%dx%dx%dx%dx%d\nOutput Dimensions : %dx%dx%dx%dx%dx%d\n", i, (pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dataId), layerTypeName.c_str(),
    layersInfo[i].layerName.c_str(), pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_BATCH],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM1],pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_DIM2], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_NUMCH], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT], pTIDLNetStructure->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH]
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
   std::ostringstream oss;
   TIDL_dump dumper(oss, pTIDLNetStructure, /*io=*/nullptr, layersInfo, options);
   dumper.dumpLayer(layerNum);
   return oss.str();
}

static int32_t extractBlockId(const char *layerName)
{
    int N;
    if (layerName && sscanf(layerName, "/model/layers.%d/", &N) == 1)
        return N;
    return -1;
}

static bool hasBlockStructure(const sTIDL_Network_t *net)
{
    if (net->numLayers <= 800) return false;
    int blockCount = 0;
    for (int i = 0; i < net->numLayers; i++)
    {
        if (!layersInfo[i].layerName.empty() &&
            extractBlockId(layersInfo[i].layerName.c_str()) >= 0)
            blockCount++;
    }
    return (blockCount * 2) > net->numLayers;
}

/**
 * Cache input gated by cacheImport, output gated by
 * cacheExport per invoke call. Shared by tidltb_dotPrintNetInfo's flat
 * per-edge check and renderBlockSVG's three edge sites (intra-block,
 * cross-block/boundary input, forward-pointing boundary output) -- src may
 * be -1 (unresolved/generic-fallback producer) at the cross-block site, so
 * callers must guard on src >= 0 before calling.
 */
static bool tidl_isCacheOptionalEdge(const sTIDL_Network_t *net, int32_t src, int32_t dst, int32_t dstInSlot)
{
    return (net->TIDLLayers[dst].layerType == TIDL_CacheLayer &&
            net->TIDLLayers[dst].layerParams.cacheParams.isOptionalInput[dstInSlot] == 1) ||
           (net->TIDLLayers[src].layerType == TIDL_CacheLayer &&
            (net->TIDLLayers[dst].layerType == TIDL_DataLayer || net->TIDLLayers[dst].layerType == TIDL_MemCopyLayer));
}

static void setCommonGraphAttrs(Agraph_t *g)
{
    agattr(g, AGNODE, (char *)"color",     (char *)"#000000");
    agattr(g, AGNODE, (char *)"fontcolor", (char *)"#FFFFFF");
    agattr(g, AGNODE, (char *)"fontsize",  (char *)"10");
    agattr(g, AGNODE, (char *)"height",    (char *)"0.24");
    agattr(g, AGNODE, (char *)"style",     (char *)"rounded,filled");
    agattr(g, AGNODE, (char *)"penwidth",  (char *)"2");
    agattr(g, AGNODE, (char *)"fillcolor", (char *)"#000000");
    agattr(g, AGNODE, (char *)"shape",     (char *)"box");
    agattr(g, AGNODE, (char *)"label",     (char *)"");
    agattr(g, AGNODE, (char *)"id",        (char *)"");
    agattr(g, AGEDGE, (char *)"label",     (char *)"");
    agattr(g, AGEDGE, (char *)"fontsize",  (char *)"8");
    agattr(g, AGEDGE, (char *)"penwidth",  (char *)"0.7");
    agattr(g, AGEDGE, (char *)"arrowhead", (char *)"vee");
    agattr(g, AGEDGE, (char *)"arrowsize", (char *)"0.5");
    agattr(g, AGRAPH, (char *)"rankdir",   (char *)"TB");
    agattr(g, AGRAPH, (char *)"ranksep",   (char *)"0.1");
    agattr(g, AGEDGE, (char *)"style",     (char *)"solid");
}

/* Render one transformer block's layers as a sub-graph. Returns malloc'd SVG string (caller frees). */
static char* renderBlockSVG(const sTIDL_Network_t *net, sPerfSim_t *perSim,
                             int32_t blockId, const int32_t *layerToBlock)
{
    GVC_t *gvc2 = gvContext();
    char graphName[64];
    snprintf(graphName, sizeof(graphName), "block_%d", blockId);
    Agraph_t *g = agopen(graphName, Agdirected, NULL);
    setCommonGraphAttrs(g);

    Agnode_t *nodeArr[TIDL_NUM_MAX_LAYERS] = {NULL};
    Agnode_t *inputBoundary = NULL;
    char nodeName[512], edgeName[64];

    /* Create a node for each layer in this block */
    for (int i = 0; i < net->numLayers; i++)
    {
        if (layerToBlock[i] != blockId) continue;
        std::string info = tidl_NodeInfo(net, perSim, i);
        nodeArr[i] = agnode(g, (char*)info.c_str(), TRUE);
        agset(nodeArr[i], (char *)"fillcolor", (char*)TIDL_graphBgColors[net->TIDLLayers[i].layerType]);
        agset(nodeArr[i], (char *)"color",     (char*)TIDL_nodeElemTypeColors[net->TIDLLayers[i].outData.elementType]);
        std::string lt = TIDL_Strings::layerTypeShort(net->TIDLLayers[i].layerType);
        if (net->TIDLLayers[i].layerType == TIDL_EltWiseLayer)
            lt = TIDL_Strings::eltwiseOpTypeString(net->TIDLLayers[i].layerParams.eltWiseParams.eltWiseType);
        else if (net->TIDLLayers[i].layerType == TIDL_BatchNormLayer) {
            if (net->TIDLLayers[i].actParams.actType != TIDL_NoAct)
                lt = TIDL_Strings::actTypeShort(net->TIDLLayers[i].actParams.actType);
            else if (net->TIDLLayers[i].clipParams.isClipEnabled == 1)
                lt = "Clip";
        }
        else if (net->TIDLLayers[i].layerType == TIDL_ScatterElementsLayer &&
                 net->TIDLLayers[i].layerParams.scatterElementsParams.axis == -1)
            lt = "ScatterND";
        else if (net->TIDLLayers[i].layerType == TIDL_ArgOpLayer)
            lt = TIDL_Strings::argOpTypeString(net->TIDLLayers[i].layerParams.argOpParams.argOpType);
        else if (net->TIDLLayers[i].layerType == TIDL_LogicalOpLayer)
            lt = TIDL_Strings::logicalOpTypeString(net->TIDLLayers[i].layerParams.logicalOpLayerParams.operatorType);
        snprintf(nodeName, sizeof(nodeName), "%s [%d %d]", lt.c_str(), net->TIDLLayers[i].outData.dataId, i);
        agset(nodeArr[i],(char *) "label", nodeName);
    }

    /* Create edges.
     * External inputs are split into three visual categories:
     *   - Algorithm layers from other blocks  → one labeled node per source ("MatMul [N]\n(Block B)")
     *   - DataLayer inputs (seqlens, KV etc.) → one labeled node per source (tensor name)
     *   - ConstData (weights/biases)          → single grouped "Weights" node
     */
    Agnode_t *extAlgNodes[TIDL_NUM_MAX_LAYERS]  = {NULL}; /* one per cross-block algo source */
    Agnode_t *dataInputNodes[TIDL_NUM_MAX_LAYERS] = {NULL}; /* one per DataLayer source */
    Agnode_t *weightBoundary = NULL;
    for (int i = 0; i < net->numLayers; i++)
    {
        if (layerToBlock[i] != blockId) continue;
        for (int j = 0; j < net->TIDLLayers[i].numInBufs; j++)
        {
            int32_t src = tidl_getInLayer(net, net->numLayers, net->TIDLLayers[i].inData[j]);
            if (src >= 0 && layerToBlock[src] == blockId && nodeArr[src])
            {
                /* Intra-block edge */
                snprintf(edgeName, sizeof(edgeName), "E%d_%d", src, i);
                Agedge_t *e = agedge(g, nodeArr[src], nodeArr[i], edgeName, TRUE);
                std::string edgeHtml = TIDL_shapeHtmlLabel(&net->TIDLLayers[src].outData);
                char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
                agset(e, (char *)"label", edgeHtmlLabel);
                if (tidl_isCacheOptionalEdge(net, src, i, j))
                {
                    agset(e, (char *)"style", (char *)"dashed");
                }
            }
            else
            {
                /* Cross-block / external input */
                Agnode_t *bNode = NULL;
                bool addDimLabel = false;

                if (src >= 0 && net->TIDLLayers[src].layerType == TIDL_ConstDataLayer)
                {
                    /* Group all weight/bias constants into one node */
                    if (!weightBoundary)
                    {
                        weightBoundary = agnode(g, (char *)"Weights", TRUE);
                        agset(weightBoundary, (char *)"fillcolor", (char *)"#444444");
                        agset(weightBoundary, (char *)"label",     (char *)"Weights");
                    }
                    bNode = weightBoundary;
                }
                else if (src >= 0 && net->TIDLLayers[src].layerType == TIDL_DataLayer)
                {
                    /* DataLayer input: same appearance as real DataLayer nodes */
                    if (!dataInputNodes[src])
                    {
                        std::string srcInfo = tidl_NodeInfo(net, perSim, src);
                        std::string lt = TIDL_Strings::layerTypeShort(net->TIDLLayers[src].layerType);
                        char dataId[64], dataLabel[128];
                        snprintf(dataId,    sizeof(dataId),    "DI_%d", src);
                        snprintf(dataLabel, sizeof(dataLabel), "%s [%d %d]", lt.c_str(),
                                 net->TIDLLayers[src].outData.dataId, src);
                        dataInputNodes[src] = agnode(g, (char*)srcInfo.c_str(), TRUE);
                        agset(dataInputNodes[src], (char *)"id",        dataId);
                        agset(dataInputNodes[src], (char *)"fillcolor", (char*)TIDL_graphBgColors[net->TIDLLayers[src].layerType]);
                        agset(dataInputNodes[src], (char *)"color",     (char*)TIDL_nodeElemTypeColors[net->TIDLLayers[src].outData.elementType]);
                        agset(dataInputNodes[src], (char *)"label",     dataLabel);
                    }
                    bNode = dataInputNodes[src];
                    addDimLabel = true;
                }
                else if (src >= 0)
                {
                    /* Algorithm layer output from another block: label with type, dataId, block */
                    if (!extAlgNodes[src])
                    {
                        std::string lt = TIDL_Strings::layerTypeShort(net->TIDLLayers[src].layerType);
                        if (net->TIDLLayers[src].layerType == TIDL_EltWiseLayer)
                            lt = TIDL_Strings::eltwiseOpTypeString(net->TIDLLayers[src].layerParams.eltWiseParams.eltWiseType);
                        else if (net->TIDLLayers[src].layerType == TIDL_BatchNormLayer) {
                            if (net->TIDLLayers[src].actParams.actType != TIDL_NoAct)
                                lt = TIDL_Strings::actTypeShort(net->TIDLLayers[src].actParams.actType);
                            else if (net->TIDLLayers[src].clipParams.isClipEnabled == 1)
                                lt = "Clip";
                        }
                        else if (net->TIDLLayers[src].layerType == TIDL_ScatterElementsLayer &&
                                 net->TIDLLayers[src].layerParams.scatterElementsParams.axis == -1)
                            lt = "ScatterND";
                        else if (net->TIDLLayers[src].layerType == TIDL_ArgOpLayer)
                            lt = TIDL_Strings::argOpTypeString(net->TIDLLayers[src].layerParams.argOpParams.argOpType);
                        else if (net->TIDLLayers[src].layerType == TIDL_LogicalOpLayer)
                            lt = TIDL_Strings::logicalOpTypeString(net->TIDLLayers[src].layerParams.logicalOpLayerParams.operatorType);
                        std::string srcInfo = tidl_NodeInfo(net, perSim, src);
                        char extId[64], extLabel[128];
                        /* id encodes src layer index + source block for JS navigation */
                        snprintf(extId,    sizeof(extId),    "EXT_SRC_%d_BLK_%d", src, layerToBlock[src]);
                        snprintf(extLabel, sizeof(extLabel), "%s [%d %d]\n(Block %d)", lt.c_str(),
                                 net->TIDLLayers[src].outData.dataId, src, layerToBlock[src]);
                        extAlgNodes[src] = agnode(g, (char*)srcInfo.c_str(), TRUE);
                        agset(extAlgNodes[src], (char *)"id",        extId);
                        agset(extAlgNodes[src], (char *)"fillcolor", (char *)"#2a5070");
                        agset(extAlgNodes[src], (char *)"label",     extLabel);
                        agset(extAlgNodes[src], (char *)"style",     (char *)"rounded,filled,dashed");
                        agset(extAlgNodes[src], (char *)"penwidth",  (char *)"1");
                    }
                    bNode = extAlgNodes[src];
                    addDimLabel = true;
                }
                else
                {
                    /* src not found — generic fallback */
                    if (!inputBoundary)
                    {
                        inputBoundary = agnode(g, (char *)"Inputs", TRUE);
                        agset(inputBoundary, (char *)"fillcolor", (char *)"#555555");
                        agset(inputBoundary, (char *)"label",     (char *)"Inputs");
                    }
                    bNode = inputBoundary;
                }

                snprintf(edgeName, sizeof(edgeName), "EI%d_%d", i, j);
                Agedge_t *e = agedge(g, bNode, nodeArr[i], edgeName, TRUE);
                if (addDimLabel && src >= 0)
                {
                    std::string edgeHtml = TIDL_shapeHtmlLabel(&net->TIDLLayers[src].outData);
                    char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
                    agset(e, (char *)"label", edgeHtmlLabel);
                }
                if (src >= 0 && tidl_isCacheOptionalEdge(net, src, i, j))
                {
                    agset(e, (char *)"style", (char *)"dashed");
                }
            }
        }
    }

    /* Forward-pointing boundary nodes: layers in this block whose output feeds a
     * consumer resolved to a *different* block. Mirrors extAlgNodes above but in
     * the opposite direction, so a viewer can navigate forward to whatever
     * consumes this block's output. Dedup by destination layer, one proxy per
     * cross-block consumer (a consumer may have several inputs from this block,
     * but still only needs a single incoming proxy node here). */
    Agnode_t *extDstNodes[TIDL_NUM_MAX_LAYERS] = {NULL};
    for (int i = 0; i < net->numLayers; i++)
    {
        if (layerToBlock[i] != blockId) continue;
        /* Shared DataLayer/ConstData sources (e.g. attention_mask, seqlens, tied
         * weights) can get assigned into this block by the propagation heuristic
         * even though they're really global resources reused by many blocks.
         * Don't treat their fan-out to other blocks as a forward pointer. */
        if (net->TIDLLayers[i].layerType == TIDL_DataLayer ||
            net->TIDLLayers[i].layerType == TIDL_ConstDataLayer) continue;
        for (int j = 0; j < net->numLayers; j++)
        {
            if (layerToBlock[j] < 0 || layerToBlock[j] == blockId) continue;
            if (net->TIDLLayers[j].layerType == TIDL_DataLayer) continue;
            bool consumesOutput = false;
            for (int k = 0; k < net->TIDLLayers[j].numInBufs; k++)
            {
                if (net->TIDLLayers[j].inData[k] == net->TIDLLayers[i].outData.dataId)
                {
                    consumesOutput = true;
                    break;
                }
            }
            if (!consumesOutput) continue;

            if (!extDstNodes[j])
            {
                std::string lt = TIDL_Strings::layerTypeShort(net->TIDLLayers[j].layerType);
                if (net->TIDLLayers[j].layerType == TIDL_EltWiseLayer)
                    lt = TIDL_Strings::eltwiseOpTypeString(net->TIDLLayers[j].layerParams.eltWiseParams.eltWiseType);
                else if (net->TIDLLayers[j].layerType == TIDL_BatchNormLayer) {
                    if (net->TIDLLayers[j].actParams.actType != TIDL_NoAct)
                        lt = TIDL_Strings::actTypeShort(net->TIDLLayers[j].actParams.actType);
                    else if (net->TIDLLayers[j].clipParams.isClipEnabled == 1)
                        lt = "Clip";
                }
                else if (net->TIDLLayers[j].layerType == TIDL_ScatterElementsLayer &&
                         net->TIDLLayers[j].layerParams.scatterElementsParams.axis == -1)
                    lt = "ScatterND";
                else if (net->TIDLLayers[j].layerType == TIDL_ArgOpLayer)
                    lt = TIDL_Strings::argOpTypeString(net->TIDLLayers[j].layerParams.argOpParams.argOpType);
                else if (net->TIDLLayers[j].layerType == TIDL_LogicalOpLayer)
                    lt = TIDL_Strings::logicalOpTypeString(net->TIDLLayers[j].layerParams.logicalOpLayerParams.operatorType);
                std::string dstInfo = tidl_NodeInfo(net, perSim, j);
                char extId[64], extLabel[128];
                /* id encodes dst layer index + dest block for JS navigation */
                snprintf(extId,    sizeof(extId),    "EXT_DST_%d_BLK_%d", j, layerToBlock[j]);
                snprintf(extLabel, sizeof(extLabel), "%s [%d %d]\n(Block %d)", lt.c_str(),
                         net->TIDLLayers[j].outData.dataId, j, layerToBlock[j]);
                extDstNodes[j] = agnode(g, (char*)dstInfo.c_str(), TRUE);
                agset(extDstNodes[j], (char *)"id",        extId);
                agset(extDstNodes[j], (char *)"fillcolor", (char *)"#2a5070");
                agset(extDstNodes[j], (char *)"label",     extLabel);
                agset(extDstNodes[j], (char *)"style",     (char *)"rounded,filled,dashed");
                agset(extDstNodes[j], (char *)"penwidth",  (char *)"1");
            }

            snprintf(edgeName, sizeof(edgeName), "EO%d_%d", i, j);
            Agedge_t *e = agedge(g, nodeArr[i], extDstNodes[j], edgeName, TRUE);
            std::string edgeHtml = TIDL_shapeHtmlLabel(&net->TIDLLayers[i].outData);
            char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
            agset(e, (char *)"label", edgeHtmlLabel);
            if (tidl_isCacheOptionalEdge(net, i, j, 0))
            {
                agset(e, (char *)"style", (char *)"dashed");
            }
        }
    }

    gvLayout(gvc2, g, "dot");
    char *svgData = NULL;
    unsigned int svgLen = 0;
    gvRenderData(gvc2, g, "svg", &svgData, &svgLen);
    gvFreeLayout(gvc2, g);
    agclose(g);
    gvFreeContext(gvc2);

    /* Return a copy so caller owns it */
    char *result = NULL;
    if (svgData)
    {
        result = (char*)malloc(svgLen + 1);
        if (result) { memcpy(result, svgData, svgLen); result[svgLen] = '\0'; }
        gvFreeRenderData(svgData);
    }
    return result;
}

#define MAX_BLOCKS 256

int32_t tidltb_dotPrintNetInfoGrouped(sTIDL_Network_t *pTIDLNetStructure, const char *fname)
{
    /* --- 1. Assign layers to blocks --- */
    int32_t layerToBlock[TIDL_NUM_MAX_LAYERS];
    int32_t maxBlockId = -1;
    int32_t blockLayerCount[MAX_BLOCKS] = {0};

    for (int i = 0; i < pTIDLNetStructure->numLayers; i++)
        layerToBlock[i] = extractBlockId(layersInfo[i].layerName.c_str());

    /* Forward propagation: push block IDs from consumers backward to producers.
     * Handles custom-named intermediates (e.g. tidl_GQACustom) whose downstream
     * Slice/Reshape layers carry proper /model/layers.N/ names. */
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < pTIDLNetStructure->numLayers; i++)
        {
            if (layerToBlock[i] >= 0) continue;
            for (int j = i + 1; j < pTIDLNetStructure->numLayers; j++)
            {
                if (layerToBlock[j] < 0) continue;
                for (int k = 0; k < pTIDLNetStructure->TIDLLayers[j].numInBufs; k++)
                {
                    if (pTIDLNetStructure->TIDLLayers[j].inData[k] ==
                        pTIDLNetStructure->TIDLLayers[i].outData.dataId)
                    {
                        layerToBlock[i] = layerToBlock[j];
                        changed = true;
                        break;
                    }
                }
                if (layerToBlock[i] >= 0) break;
            }
        }
    }

    /* Backward propagation: push block IDs from producers forward to consumers.
     * Handles GQA/TopK/LSTM/GRU sibling outputs (present.N.key, present.N.value, etc.)
     * that have NO downstream consumers so the forward pass cannot reach them,
     * but whose producer (tidl_GQACustom) is already resolved to a block. */
    changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < pTIDLNetStructure->numLayers; i++)
        {
            if (layerToBlock[i] >= 0) continue;
            if (pTIDLNetStructure->TIDLLayers[i].numInBufs == NOT_VALID) continue;
            for (int j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
            {
                int32_t src = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers,
                                              pTIDLNetStructure->TIDLLayers[i].inData[j]);
                if (src >= 0 && layerToBlock[src] >= 0)
                {
                    layerToBlock[i] = layerToBlock[src];
                    changed = true;
                    break;
                }
            }
        }
    }

    /* Compute maxBlockId and count layers per block */
    for (int i = 0; i < pTIDLNetStructure->numLayers; i++)
    {
        if (layerToBlock[i] >= 0 && layerToBlock[i] < MAX_BLOCKS)
        {
            if (layerToBlock[i] > maxBlockId) maxBlockId = layerToBlock[i];
            blockLayerCount[layerToBlock[i]]++;
        }
    }

    if (maxBlockId < 0) return -1; /* shouldn't happen after hasBlockStructure check */

    sPerfSim_t *perSim = NULL;
    if (pTIDLNetStructure->dataFlowInfo != 0)
        perSim = (sPerfSim_t *)((int8_t *)(pTIDLNetStructure) + pTIDLNetStructure->dataFlowInfo);

    /* --- 2. Build collapsed summary graph --- */
    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen((char*)"summary", Agdirected, NULL);
    setCommonGraphAttrs(g);
    agattr(g, AGRAPH, (char *)"pad", (char *)"0.5");

    Agnode_t *inputNode = agnode(g, (char *)"Inputs", TRUE);
    agset(inputNode, (char *)"fillcolor", (char *)"#555555");
    agset(inputNode, (char *)"label", (char *)"Inputs");

    Agnode_t *blockNodes[MAX_BLOCKS] = {NULL};
    char blockKey[64], blockLabel[128], edgeKey[32];
    for (int b = 0; b <= maxBlockId; b++)
    {
        snprintf(blockKey,   sizeof(blockKey),   "BLOCK_ID:%d", b);
        snprintf(blockLabel, sizeof(blockLabel), "Block %d\n(%d layers)", b, blockLayerCount[b]);
        blockNodes[b] = agnode(g, blockKey, TRUE);
        agset(blockNodes[b], (char *)"fillcolor", (char *)"#00008b");
        agset(blockNodes[b], (char *)"label",     blockLabel);
        agset(blockNodes[b], (char *)"style",     (char *)"rounded,filled,dashed");
        agset(blockNodes[b], (char *)"penwidth",  (char *)"2");
    }

    Agnode_t *outputNode = agnode(g, (char *)"Output", TRUE);
    agset(outputNode, (char *)"fillcolor", (char *)"#555555");
    agset(outputNode, (char *)"label", (char *)"Output");

    snprintf(edgeKey, sizeof(edgeKey), "e_in_0");
    agedge(g, inputNode, blockNodes[0], edgeKey, TRUE);
    for (int b = 1; b <= maxBlockId; b++)
    {
        snprintf(edgeKey, sizeof(edgeKey), "e_%d_%d", b-1, b);
        agedge(g, blockNodes[b-1], blockNodes[b], edgeKey, TRUE);
    }
    snprintf(edgeKey, sizeof(edgeKey), "e_%d_out", maxBlockId);
    agedge(g, blockNodes[maxBlockId], outputNode, edgeKey, TRUE);

    /* --- 3. Render summary SVG to memory --- */
    gvLayout(gvc, g, "dot");
    char *summarySvg = NULL;
    unsigned int summaryLen = 0;
    gvRenderData(gvc, g, "svg", &summarySvg, &summaryLen);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);

    /* --- 4. Render per-block SVGs --- */
    char **blockSvgs = (char**)calloc(maxBlockId + 1, sizeof(char*));
    for (int b = 0; b <= maxBlockId; b++)
        blockSvgs[b] = renderBlockSVG(pTIDLNetStructure, perSim, b, layerToBlock);

    /* --- 5. Write HTML --- */
    char htmlFileName[MAX_FILE_NAME_SIZE];
    strcpy(htmlFileName, fname);
    strcat(htmlFileName, ".html");

    if (HTML_TEMPLATE == NULL || summarySvg == NULL)
    {
        printf("Could not generate HTML for grouped visualization.\n");
        if (summarySvg) gvFreeRenderData(summarySvg);
        for (int b = 0; b <= maxBlockId; b++) if (blockSvgs[b]) free(blockSvgs[b]);
        free(blockSvgs);
        return -1;
    }

    long templateSize = (long)strlen(HTML_TEMPLATE);

    /* Find mainSvg insertion point */
    const char *mainSvgMarker = "<div id=\"mainSvg\">";
    char *insertPos = strstr((char*)HTML_TEMPLATE, mainSvgMarker);
    if (!insertPos)
    {
        gvFreeRenderData(summarySvg);
        for (int b = 0; b <= maxBlockId; b++) if (blockSvgs[b]) free(blockSvgs[b]);
        free(blockSvgs);
        return -1;
    }
    insertPos += strlen(mainSvgMarker);

    /* Find block data placeholder */
    const char *blockPlaceholder = "<!--BLOCK_DATA_PLACEHOLDER-->";
    char *blockInsertPos = strstr((char*)HTML_TEMPLATE, blockPlaceholder);

    FILE *htmlFile = fopen(htmlFileName, "w");
    if (!htmlFile)
    {
        printf("Error creating HTML file: %s\n", htmlFileName);
        gvFreeRenderData(summarySvg);
        for (int b = 0; b <= maxBlockId; b++) if (blockSvgs[b]) free(blockSvgs[b]);
        free(blockSvgs);
        return -1;
    }

    /* Write template up to mainSvg insertion point, then summary SVG */
    fwrite(HTML_TEMPLATE, 1, insertPos - HTML_TEMPLATE, htmlFile);
    fwrite(summarySvg, 1, summaryLen, htmlFile);
    gvFreeRenderData(summarySvg);

    if (blockInsertPos)
    {
        /* Write rest of template up to block placeholder */
        fwrite(insertPos, 1, blockInsertPos - insertPos, htmlFile);
        /* Write per-block <template> elements */
        for (int b = 0; b <= maxBlockId; b++)
        {
            if (blockSvgs[b])
            {
                fprintf(htmlFile, "<template id=\"block_detail_%d\">", b);
                fwrite(blockSvgs[b], 1, strlen(blockSvgs[b]), htmlFile);
                fprintf(htmlFile, "</template>\n");
            }
        }
        /* Write rest of template after placeholder */
        fwrite(blockInsertPos + strlen(blockPlaceholder),
               1,
               templateSize - (blockInsertPos + strlen(blockPlaceholder) - HTML_TEMPLATE),
               htmlFile);
    }
    else
    {
        /* No placeholder found — just write rest of template */
        fwrite(insertPos, 1, templateSize - (insertPos - HTML_TEMPLATE), htmlFile);
    }

    fclose(htmlFile);
    for (int b = 0; b <= maxBlockId; b++) if (blockSvgs[b]) free(blockSvgs[b]);
    free(blockSvgs);

    return 0;
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
  sym = agattr(g, AGNODE, (char *)"color", (char *)"#000000");
  sym = agattr(g, AGNODE, (char *)"fontcolor", (char *)"#FFFFFF");
  sym = agattr(g, AGNODE, (char *)"fontsize", (char *)"10");
  sym = agattr(g, AGNODE, (char *)"height", (char *)"0.24");
  sym = agattr(g, AGNODE, (char *)"style", (char *)"rounded,filled");
  sym = agattr(g, AGNODE, (char *)"penwidth", (char *)"2");
  sym = agattr(g, AGNODE, (char *)"fillcolor", (char *)"#000000");
  sym = agattr(g, AGRAPH, (char *)"pad", (char *)"6.5,0.2,6.5,0.5");
  sym = agattr(g, AGRAPH, (char *)"rankdir", (char *)"TB");
  sym = agattr(g, AGRAPH, (char *)"ranksep", (char *)"0.1");
  sym = agattr(g, AGNODE, (char *)"label", (char *)"Default");
  sym = agattr(g, AGEDGE, (char *)"label", (char *)"Default");
  sym = agattr(g,AGEDGE,  (char *)"fontsize", (char *)"8");
  sym = agattr(g, AGEDGE, (char *)"penwidth", (char *)"0.7");
  sym = agattr(g, AGEDGE, (char *)"arrowhead", (char *)"vee");
  sym = agattr(g, AGEDGE, (char *)"arrowsize", (char *)"0.5");
  sym = agattr(g, AGEDGE, (char *)"style", (char *)"solid");
  sym = agattr(g, AGNODE, (char *)"shape", (char *)"box");
  strcpy(liFileName, fname);
  strcat(liFileName, ".layer_info.txt");
  fpNames = fopen(liFileName, "r");
  if (fpNames != NULL)
  {
    for (i = 0; i < pTIDLNetStructure->numLayers; i++)
    {
      char name[300] = {0};
      temp3 = -1;
      if(pTIDLNetStructure->inferenceMode != TIDL_inferenceModeLowLatency)/*single core inference*/
      {
        fscanf(fpNames, "%d %d %s", &temp1, &temp2, name);
      }
      else
      {
        fscanf(fpNames, "%d %d %d %s", &temp1, &temp2, &temp3, name);
      }
      layersInfo[i].layerName = name;
      layersInfo[i].multiCoreMode = temp3 != -1 
                                    ? temp3
                                    : TIDL_NOT_MULTI_CORE;
    }
    fclose(fpNames);
  }
  if (pTIDLNetStructure->dataFlowInfo != 0)
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
  sWorkloadGroup_t *wlGrp = NULL;
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
    
    wlGrp = &gcOut->workload;
  }

  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    std::string layerTypeName = TIDL_Strings::layerTypeShort(pTIDLNetStructure->TIDLLayers[i].layerType);
    if(pTIDLNetStructure->TIDLLayers[i].layerType == TIDL_DataLayer && pTIDLNetStructure->TIDLLayers[i].numInBufs == NOT_VALID)
    {
      std::string info = tidl_NodeInfo(pTIDLNetStructure, perSimInfo, i);
      n[i] = agnode(g, (char*)info.c_str(), TRUE);
      agset(n[i], (char *)"fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      agset(n[i], (char *)"color",(char*)TIDL_nodeElemTypeColors[pTIDLNetStructure->TIDLLayers[i].outData.elementType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], (char *)"label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        {
          std::string edgeHtml = TIDL_shapeHtmlLabel(&pTIDLNetStructure->TIDLLayers[inLayerIdx].outData);
          char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
          agset(e[i], (char *)"label", edgeHtmlLabel);
        }
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
                      wlGrp);

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
        if (pTIDLNetStructure->TIDLLayers[i].actParams.actType == TIDL_NoAct &&
            pTIDLNetStructure->TIDLLayers[i].clipParams.isClipEnabled == 1)
        {
          layerTypeName = "Clip";
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
      agset(n[i], (char *)"fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      agset(n[i], (char *)"color",(char*)TIDL_nodeElemTypeColors[pTIDLNetStructure->TIDLLayers[i].outData.elementType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], (char *)"label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        {
          std::string edgeHtml = TIDL_shapeHtmlLabel(&pTIDLNetStructure->TIDLLayers[inLayerIdx].outData);
          char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
          agset(e[i], (char *)"label", edgeHtmlLabel);
        }

        if (tidl_isCacheOptionalEdge(pTIDLNetStructure, inLayerIdx, i, j))
        {
          agset(e[i], (char *)"style", (char *)"dashed");
        }
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
      agset(n[i], (char *)"fillcolor", (char*)TIDL_graphBgColors[pTIDLNetStructure->TIDLLayers[i].layerType]);
      sprintf(nodeName, "%s [%d %d]", layerTypeName.c_str(), layerIdxCount, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
      agset(n[i], (char *)"label", (char*)nodeName);
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        sprintf(tempBuff, "EDGEFORLAYER%d%d", i, pTIDLNetStructure->TIDLLayers[i].outData.dataId);
        int32_t inLayerIdx = tidl_getInLayer(pTIDLNetStructure, pTIDLNetStructure->numLayers, pTIDLNetStructure->TIDLLayers[i].inData[j]);
        e[i] = agedge(g, n[inLayerIdx], n[i], (char*)tempBuff,TRUE);
        i1 = pTIDLNetStructure->TIDLLayers[i].inData[j];
        {
          std::string edgeHtml = TIDL_shapeHtmlLabel(&pTIDLNetStructure->TIDLLayers[inLayerIdx].outData);
          char *edgeHtmlLabel = agstrdup_html(g, (char*)edgeHtml.c_str());
          agset(e[i], (char *)"label", edgeHtmlLabel);
        }
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
        /* Pre-read layer info so hasBlockStructure can inspect layer names */
        char liFileName[MAX_FILE_NAME_SIZE];
        strcpy(liFileName, fname);
        strcat(liFileName, ".layer_info.txt");
        FILE *fpNames = fopen(liFileName, "r");
        if (fpNames != NULL)
        {
          for (int li = 0; li < net->numLayers; li++)
          {
            char name[300] = {0};
            int t1, t2, t3 = -1;
            if (net->inferenceMode != TIDL_inferenceModeLowLatency)
              fscanf(fpNames, "%d %d %s", &t1, &t2, name);
            else
              fscanf(fpNames, "%d %d %d %s", &t1, &t2, &t3, name);
            layersInfo[li].layerName = name;
            layersInfo[li].multiCoreMode = (t3 != -1) ? t3 : TIDL_NOT_MULTI_CORE;
          }
          fclose(fpNames);
        }

        if (hasBlockStructure(net))
          tidltb_dotPrintNetInfoGrouped(net, fname);
        else
          tidltb_dotPrintNetInfo(net, fname);
      }
    }
  }
  return 0;
}