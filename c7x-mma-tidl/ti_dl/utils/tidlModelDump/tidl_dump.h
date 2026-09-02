/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/**
@file      tidl_dump.h
@brief     This file defines a class used to render the TIDL network representation
@brief     in human-readbale format
*/
#ifndef _TIDL_DUMP_H
#define  _TIDL_DUMP_H
#include "itidl_ti.h"
#include "perfsim.h"
#include "indent_ostream.h"
#include <string>
#include <ostream>
#include <map>

// Build graphviz HTML inner content for a tensor shape label
std::string TIDL_shapeHtmlLabel(const sTIDL_DataParams_t *pData);

/*
 * This class provides a facility for dumping the TIDL network data structure
 * to an output stream as human-readable text.  
 *
 * Instantiate the class as follows:
 *   TIDL_dump d(os, net, io, layerNames, options);
 * where:
 *     os         - the output stream, e.g. std::cout or a std::ostringstream
 *     net        - pointer to sTIDL_Network_t
 *     io         - pointer to sTIDL_IOBufDesc_t (optional)
 *     layerNames - map of layer numbers to layer names (optional)
 *     options    - map of option keywords to value
 * Options:
 *     "perfsim" (true, false)   - include dataflow info from perfsim tool
 *     "full_weights" (true, false)  - include all weights
 *
 * After instantiating the class, invoke it as follows:
 *   d.dumpNet()               - dump the whole network
 *   d.dumpIODesc()            - dump the IOBufDesc
 *   d.dumpLayer(layerNum)     - dump one layer
 */
class TIDL_dump
{
public:
   typedef struct
   {
      std::string layerName;
      int32_t multiCoreMode;
   } sLayerSpecificInfo_t;

   typedef std::map<std::string, int> options_t;
   TIDL_dump(std::ostream &os,
             const sTIDL_Network_t *net,
             const sTIDL_IOBufDesc_t *io = nullptr,
             const sLayerSpecificInfo_t *layersInfo = nullptr,
             options_t user_options = {});

public:
   bool hasOption(const std::string &opt) 
      { return options.find(opt) != options.end() && options[opt] == true; } 
   void dumpNet();
   void dumpIODesc();
   void dumpLayer(int layerNum);
private:
   static const options_t default_options; 
   options_t options;
   indent_ostream os;
   const sTIDL_Network_t* pNet;
   const sTIDL_IOBufDesc_t *pIODesc;
   const sLayerSpecificInfo_t *layersInfo;
   const sGraphCompilerOutArgs_t *pSimInfo();

   void dumpTensorParams(const sTIDL_DataParams_t *pData);
   void dumpDataLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpConvolutionLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpDeconvolutionLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpPoolingLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpPoolingLayerParams(const sTIDL_PoolingParams_t *pPool);
   void dumpEltWiseLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpInnerProductLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpSoftMaxLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpBatchNormLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpBiasLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpConcatLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpSliceLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpCropLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpArgOpLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpDataConvertParams(const sTIDL_Layer_t *pLayer);
   void dumpTransposeParams(const sTIDL_Layer_t *pLayer);
   void dumpGridSampleParams(const sTIDL_Layer_t *pLayer);
   void dumpDeformConvParams(const sTIDL_Layer_t *pLayer);
   void dumpTopKParams(const sTIDL_Layer_t *pLayer);
   void dumpTileParams(const sTIDL_Layer_t *pLayer);
   void dumpLogicalOpLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpRMSNormalizationParams(const sTIDL_Layer_t *pLayer);
   void dumpShapeParams(const sTIDL_Layer_t *pLayer);
   void dumpDetectionOutputLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpShuffleChannelLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpResizeLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpRoiPoolingLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpDepthToSpaceLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpPadLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpLSTMParams(const sTIDL_Layer_t *pLayer);
   void dumpGRUParams(const sTIDL_Layer_t *pLayer);
   void dumpRNNParams(const sTIDL_Layer_t *pLayer);
   void dumpGatherNDParams(const sTIDL_Layer_t *pLayer);
   void dumpOdOutputReformatLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpOdPostProcessingLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpCustomLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpGatherElementsParams(const sTIDL_Layer_t *pLayer);
   void dumpNonZeroParams(const sTIDL_Layer_t *pLayer);
   void dumpGatherParams(const sTIDL_Layer_t *pLayer);
   void dumpUnsupportedLayerParams(const sTIDL_Layer_t *pLayer);
   void dumpActParams(const sTIDL_ActParams_t *pActParams);
   void dumpClipParams(const sTIDL_ClipParams_t *pClipParams);
   void dumpCalibParams(const sTIDL_CalibParams_t *pCalibParams);
   void dumpPerfSimInfo(int layerNum);

   // Dump per-channel weightScales and biasScales for asymmetric
   // (layerKernelType==0) Conv/Deconv/InnerProduct layers.
   // numWeightScaleChannels: entries in weightScales.
   // numBiasChannels: entries in biasScales.
   void dumpAsymQuantArrays(const sTIDL_Layer_t *pLayer,
                            int32_t numWeightScaleChannels,
                            int32_t numBiasChannels,
                            int32_t weightScalesOffset,
                            int32_t biasScalesOffset);

   // print multiple values using proper type
   template <typename Tw>
   void printItems(Tw *items, int n);

   // printf-style formatting to stream
   std::string format(const char *s, ...) const;
};
#endif  // _TIDL_DUMP_H
