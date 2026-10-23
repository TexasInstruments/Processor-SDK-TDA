/*
 * Module Name        :TIDL
 *
 * Module Description :TI Deep learning Library module is TI's CNN/DNN
 *                     acceleration on EVE and DSP
 *
 * Copyright (C) 2016-2019 Texas Instruments Incorporated - http://www.ti.com/
 *
 * ALL RIGHTS RESERVED
 *
 *
 */

/**
 *  \file tidl_io.c
 *
 *  \brief This file contains the corruption of the input/output of the data in network structure
 */

#include "itidl_rt.h"
#include "tidl_tb_utils.h"
#include "gc_helper.h"

#if HOST_EMULATION
sTIDL_Network_t * net;

int32_t tidl_writeNetw(sTIDL_Network_t * net, uint8_t * fileString, int32_t modelSize)
{
  TI_FILE * fptr;
  fptr = FOPEN((const char *)fileString, "wb+");
  if(fptr)
  {
    FWRITE(net,1, modelSize, fptr);
    FCLOSE(fptr);
    return 0;
  }
  else
  {
    tidl_tb_printf(0, "Could Not Open Files %s\n",fileString);
    return 1;
  }
}

int32_t modifyNetworkParams(char * fileName,  char * inputLayer, char * mdata , sTIDL_IOBufDesc_t * gIOParams )
{
  uint32_t tidlModelSize = tidl_getFileSize((uint8_t*)fileName);
  TIDLTB_ASSERT_EXIT(tidlModelSize != 0);
  net = (sTIDL_Network_t *)malloc(tidlModelSize);
  tidl_readFile((sTIDL_Network_t *)net, (uint8_t*)fileName);
  
  printf("net version %x", net->netVersion);
  if(strcmp(mdata , "netVersion") == 0)
  {
    net->netVersion = 0;
    printf("N/w set %x \n",net->netVersion);
  }

  for (int i = 0; i < (*net).numLayers; i++)
  {
    //Differentiate here whether the inelement or outElement needs to be update
    if(strcmp(mdata , "IN") == 0)
    {
      
      if(net->TIDLLayers[i].layerType == atoi(inputLayer))
      {
        if(net->TIDLLayers[i].layerType == TIDL_InnerProductLayer)
        {
          net->TIDLLayers[i-1].outData.elementType = 10;
        }
        else
        {
          for(int j = 0 ; j < net->TIDLLayers[i].numInBufs; j++)
          {
            net->TIDLLayers[i-(j+1)].outData.elementType = 10;
            printf("Value modified is %d \n", net->TIDLLayers[i-(j+1)].outData.elementType);
          }
        }
      }

    }
    else if(strcmp(mdata , "OUT") == 0)
    {
      if(net->TIDLLayers[i].layerType == atoi(inputLayer))
      {
        net->TIDLLayers[i].outData.elementType = 10;
      }
      else if((atoi(inputLayer) == TIDL_DepthToSpaceLayer ) && 
              (net->TIDLLayers[i].layerType == TIDL_ConvolutionLayer) &&
              (net->TIDLLayers[i].layerParams.convParams.enableDepthToSpace == true))
      {
        net->TIDLLayers[i].outData.elementType = 10;
      }
    }
    
    if(((strcmp(mdata , "gatherindices")) == 0) && (net->TIDLLayers[i].layerType == TIDL_GatherLayer))
    {
      net->TIDLLayers[i-1].outData.elementType = 0;
    }

    if(((strcmp(mdata , "mode")) == 0) && (net->TIDLLayers[i].layerType == TIDL_ResizeLayer))
    {
      net->TIDLLayers[i].layerParams.resizeParams.mode = 5;
    }

    if(((strcmp(mdata , "resizeratio")) == 0) && (net->TIDLLayers[i].layerType == TIDL_ResizeLayer))
    {
      /* initialize resizeRatio by 1.0 */
      for(int32_t j = 0; j < (int32_t)TIDL_DIM_MAX; j++)
      {
        net->TIDLLayers[i].layerParams.resizeParams.resizeRatio[j] = 1.0;
      }
      net->TIDLLayers[i].layerParams.resizeParams.resizeRatio[TIDL_DIM_WIDTH] = 9;
      net->TIDLLayers[i].layerParams.resizeParams.resizeRatio[TIDL_DIM_HEIGHT] = 9;
    }
    
    if(((strcmp(mdata , "axis")) == 0) && (net->TIDLLayers[i].layerType == TIDL_SoftMaxLayer))
    {
      
      net->TIDLLayers[i].layerParams.softMaxParams.axis = 9;
    }
    if((strcmp(mdata , "stride")) == 0 && (net->TIDLLayers[i].layerType == TIDL_Deconv2DLayer)  )
    {
      net->TIDLLayers[i].layerParams.convParams.strideW = 10;
      net->TIDLLayers[i].layerParams.convParams.strideH = 10;
    }

    if((strcmp(mdata , "kernel")) == 0 && (net->TIDLLayers[i].layerType == TIDL_Deconv2DLayer)  )
    {
      net->TIDLLayers[i].layerParams.convParams.kernelH = 10;
      net->TIDLLayers[i].layerParams.convParams.kernelW = 10;
    }
    if(strcmp(mdata , "changeact") == 0 && (net->TIDLLayers[i].layerType == TIDL_BatchNormLayer)  )
    {
      net->TIDLLayers[i].actParams.actType = 10;
    }
    
    if((strcmp(mdata , "kernel")) == 0 && (net->TIDLLayers[i].layerType == TIDL_PoolingLayer)  )
    {
      net->TIDLLayers[i].layerParams.poolParams.kernelH = 5;
      net->TIDLLayers[i].layerParams.poolParams.kernelW = 5;
    }
    
    if((strcmp(mdata , "stride")) == 0 && (net->TIDLLayers[i].layerType == TIDL_PoolingLayer)  )
    {
      net->TIDLLayers[i].layerParams.poolParams.strideH = 3;
      net->TIDLLayers[i].layerParams.poolParams.strideW = 3;
    }

    if((strcmp(mdata , "strideW")) == 0 && (net->TIDLLayers[i].layerType == TIDL_PoolingLayer)  )
    {
      net->TIDLLayers[i].layerParams.poolParams.strideW = 4;
    }

    if((strcmp(mdata , "axis")) == 0 && (net->TIDLLayers[i].layerType == TIDL_SliceLayer)  )
    {
      net->TIDLLayers[i].layerParams.sliceParams.axis = 7;
    }
    if(net->TIDLLayers[i].layerType == TIDL_FlattenLayer && (strcmp(mdata , "layercorrupt") == 0))
    {
      net->TIDLLayers[i].layerType = 50;
    }

    if(net->TIDLLayers[i].layerType == TIDL_ReduceLayer && (strcmp(mdata , "axis") == 0))
    {
      net->TIDLLayers[i].layerParams.reduceParams.axis = 5;
    }

    if(net->TIDLLayers[i].layerType == TIDL_ConcatLayer && (strcmp(mdata , "axis") == 0))
    {
      net->TIDLLayers[i].layerParams.concatParams.axis = TIDL_DIM_BATCH;
    }

    if(net->TIDLLayers[i].layerType == TIDL_LayerNormLayer && (strcmp(mdata , "axis") == 0))
    {
      net->TIDLLayers[i].layerParams.layerNormParams.axis = TIDL_DIM_BATCH;
    }
    
    if(net->TIDLLayers[i].layerType == TIDL_EltWiseLayer && (strcmp(mdata , "eltWiseType") == 0))
    {
      net->TIDLLayers[i].layerParams.eltWiseParams.eltWiseType = TIDL_DIM_WIDTH;
    }
    if(net->TIDLLayers[i].layerType == TIDL_InnerProductLayer && (strcmp(mdata , "indata") == 0) )
    {
      net->TIDLLayers[i].inData[0] = 4;
    }
    
    if( (strcmp(mdata , "dataflowinfo") == 0) )
    {
      net->dataFlowInfo = 0;
    }
    
    if(net->TIDLLayers[i].layerType == TIDL_ConcatLayer && (strcmp(mdata, "dataId") == 0))
    {
      net->TIDLLayers[i].outData.dataId = 0;
    }
    if(net->TIDLLayers[i].layerType == TIDL_DataConvertLayer  && (strcmp(mdata, "outzeropoint") == 0))
    {
      net->TIDLLayers[i].layerParams.dataConvertParams.outZeroPoint = 9;
      net->TIDLLayers[i].layerParams.dataConvertParams.inZeroPoint = 9;
    }
  }

  uint32_t status = tidl_writeNetw(net, (uint8_t*)fileName, tidlModelSize );
  free(net);
  return status; 
  
}

void * TIDL_getNetObjBufPtr( sTIDL_Network_t *pNet,
                             int32_t netMember)
{
  void* netMemberSrcAddr = NULL;
  void* netMemberDstAddr = NULL;
  uint32_t size = 0;
  uint32_t offset = 0;

  if (netMember >= (int32_t)TIDL_NET_TOTAL_BUF)
  {
    printf ("Member not available as part of net structure \n");
    assert(0);
  }

  size   = pNet->netBufInfo[netMember].size;
  offset = pNet->netBufInfo[netMember].offset;

  netMemberDstAddr = netMemberSrcAddr = (uint8_t *)get_int8_t_pointer((int8_t *)(pNet), offset);

  (void)size;

  return netMemberDstAddr ;
}


int32_t modifyWorkloadParams(char * fileName,  char * inputLayer, char * mdata , sTIDL_IOBufDesc_t * gIOParams )
{
  uint32_t status = 0;
  uint32_t tidlModelSize = tidl_getFileSize((uint8_t*)fileName);
  TIDLTB_ASSERT_EXIT(tidlModelSize != 0);
  net = (sTIDL_Network_t *)malloc(tidlModelSize);
  tidl_readFile((sTIDL_Network_t *)net, (uint8_t*)fileName);
 
  sGCHelperHandle *gcHelperHandle = NULL;
  sGCHelperHandle gcHelperHandleObj;
  gcHelperHandle = &gcHelperHandleObj;
  uint32_t relativeCoreId = 0;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGraphCompilerOutArgsCore_t *gcOut = NULL;
  sGCCommonDataBase_t     *gcCommonDataBase = NULL;
  sWorkloadGroup_t *wlGrp = NULL;
 
  gcOutArgs    = (sGraphCompilerOutArgs_t *) TIDL_getNetObjBufPtr( net,  TIDL_NET_GC_BUF);
  gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs , DATABASE_GCOUT_1+relativeCoreId);
  sGCDataBase_t *gcDatabase   = &gcOut->dataBase;
  gcCommonDataBase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs,DATABASE_COMMON);
  wlGrp  = &gcOut->workload;
  sBufParams_t* bufNodesDB = (sBufParams_t*) &gcCommonDataBase->dbPayLoad[gcCommonDataBase->dbParams[DB_COMMON_BUF].startOffset];
  sWorkloadUnit_t* workLoadUnitDB = (sWorkloadUnit_t*) &gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_WL].startOffset];
 
  sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) &gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_AUXWL].startOffset];
  gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
  gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
  gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;  
 
  (void)initGCHelperHandle(gcHelperHandle, net, &gcOut->dataBaseInfo, &gcOutArgs->commonDatabaseInfo);
   
  if(gcHelperHandle != NULL)
  {
    sWorkloadUnit_t *pWLUnit =  getWLUnitPtr(gcHelperHandle , atoi(inputLayer));
    int32_t bufDBindex = NOT_VALID;
    uint32_t linkIdx = 0;
    sLink_t *linkPtrList[MAX_LINKS_PER_WL];
 
    for( linkIdx = 0; linkIdx < (uint32_t)pWLUnit->numLinks; linkIdx++)
    {
      getLinkPtrs(pWLUnit, NOT_VALID, linkIdx, linkPtrList);
      sLink_t * pLink = linkPtrList[0];

      //Case to corrupt workload to get into the condition in workload_unit_exec. 
      if((strcmp(mdata , "offset") == 0))
      {
        pLink->src[0].offset = 100000000;
      }
      else if((strcmp(mdata , "pad") == 0))
      {
        //padCReq
        pWLUnit->procParam.outPadRequired[COL_DIR] = 2;
        //padRZeros
        pWLUnit->procParam.outPadZeros[ROW_DIR] = 0;
        //padRFillZeros
        pWLUnit->procParam.outPadToFill[ROW_DIR] = 0;
      }
      else if((strcmp(mdata , "flowunitboundarysrc") == 0))
      {
        pLink->src->flowUnitBoundary = 3;
      }
      else if((strncmp(mdata , "flowunitboundaryboth", 20) == 0))
      {
        char strData[25];
        strcpy(strData , mdata);
        char* subString  = strtok(strData,"-");
        subString  = strtok(NULL,"-");
        pLink->src->flowUnitBoundary = atoi(subString);
        pLink->sink.flowUnitBoundary = atoi(subString);
      }
      else if((strcmp(mdata , "freq") == 0) && (pLink->subType == LINK_X_FM_OUT_PART_TO_FULL))
      {
        pLink->freq = 2;
      }
      else if ((strcmp(mdata , "IN") == 0)  &&( ( pLink->subType == LINK_X_FM_IN_FULL_TO_PART) || ( pLink->subType == LINK_P)))
      {
        for(int32_t srcIdx = 0; srcIdx < pLink->numSrc; srcIdx++)
        {
          bufDBindex = pLink->src[srcIdx].bufDBindex;
          sBufParams_t * bufParams = getBufParamsFromBufIndex(gcHelperHandle, bufDBindex);
          bufParams->dataType = 6;
                     
        }
      }
      else if ((strcmp(mdata , "bufdbindex") == 0)  && ( pLink->subType == BUF_FM_FULL) )
      {
        
        pLink->sink.bufDBindex = -1;
        printf("Setting bufDbindex %d \n",pLink->sink.bufDBindex);
      }
      else if((strncmp(mdata , "elementtype", 11) == 0))
      {
        char strData[25];
        strcpy(strData , mdata);
        char* subString  = strtok(strData,"-");
        subString  = strtok(NULL,"-");//printf("%s", strtok(NULL, " = ");  
        if((strcmp(subString, "src") == 0) && (pLink->subType == LINK_X_FM_OUT_PART_TO_FULL))
        {

          sJoint_t *srcJoint = &pLink->src[linkIdx];
          sBufParams_t *srcBufParams = getBufParamsFromBufIndex(gcHelperHandle, srcJoint->bufDBindex);
          srcBufParams->dataType = 10;
        }
        else if((strcmp(subString, "dst") == 0 ) && (pLink->subType == LINK_X_FM_OUT_PART_TO_FULL))
        {
          sJoint_t *sinkJoint = &pLink->sink;
          sBufParams_t *sinkBufParams = getBufParamsFromBufIndex(gcHelperHandle, sinkJoint->bufDBindex);
          sinkBufParams->dataType = 10;
        }
        else if(strcmp(subString, "both") == 0)
        {
          sJoint_t *srcJoint = &pLink->src[linkIdx];
          sJoint_t *sinkJoint = &pLink->sink;
          sBufParams_t *srcBufParams = getBufParamsFromBufIndex(gcHelperHandle, srcJoint->bufDBindex);
          sBufParams_t *sinkBufParams = getBufParamsFromBufIndex(gcHelperHandle, sinkJoint->bufDBindex);
          srcBufParams->dataType = 10;
          sinkBufParams->dataType = 9;
        }
        
      }  
      else if( (strcmp(mdata , "OUT") == 0) && (( pLink->subType == LINK_X_FM_IN_FULL_TO_PART) || ( pLink->subType == LINK_P)))
      {
        
        bufDBindex = pLink->sink.bufDBindex;
        sBufParams_t * bufParams = getBufParamsFromBufIndex(gcHelperHandle, bufDBindex);
          
        bufParams->dataType = 6;
      }
      else if((strcmp(mdata , "linkpipeup") == 0))
      {
        pLink->src[0].flowCnt[FLOW_PIPEUP][0] = pLink->src[0].flowCnt[FLOW_PIPELINE][0];
        pLink->src[0].flowCnt[FLOW_PIPELINE][0] =0;
      } 
      
    }
  }

  status = tidl_writeNetw(net, (uint8_t*)fileName, tidlModelSize );

  (void)status;
  (void)wlGrp;

  free(net);
  return 0;
}

int32_t tidlNegativeCase(int32_t argc, char **argv)
{
  int32_t status = 0;
  sTIDL_IOBufDesc_t  gIOParams;
  if(strcmp(argv[1], "NNW") == 0)
  {
    status = modifyNetworkParams((char *)argv[2], (char *)argv[3], (char *)argv[4], &gIOParams);
  }
  else if(strcmp(argv[1], "NWL") == 0)
  {
    status = modifyWorkloadParams((char *)argv[2], (char *)argv[3], (char *)argv[4], &gIOParams);
  }
  else
  {
    printf("[NEG] argument not supported \n");
  }
  return status;  
}
#endif
