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

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdint.h>
extern "C"
{
#include "configparser.h"
}
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#pragma RESET_MISRA ("required")

sTokenStringSubst booleanSubst[] = {
	{"true",  {1, 0.0f}},
	{"false", {0, 0.0f}},
	{NULL,    {0, 0.0f}},
};

sTokenStringSubst modelTypeSubst[] = {
	{"caffe",      {0, 0.0f}},
	{"tensorflow", {1, 0.0f}},
	{"onnx",       {2, 0.0f}},
	{"tflite",     {3, 0.0f}},
	{NULL,         {0, 0.0f}},
};

sTokenStringSubst postProcTypeSubst[] = {
	{"disabled",       {0, 0.0f}},
	{"classification", {1, 0.0f}},
	{"detection",      {2, 0.0f}},
	{"segmentation",   {3, 0.0f}},
	{NULL,             {0, 0.0f}},
};

sTokenMapping gsTokenMap_tidl_import_config[MAX_ITEMS_TO_PARSE] =
{
        {(char* )"randParams",             &gParams.randParams            , INT_32, SINGLE, false, false, NULL,
          "Use random model weights and values: 1 = true / 0 = false (default)", NULL},
        {(char *)"modelType",              &gParams.modelType             , INT_32, SINGLE, false, false, &modelTypeSubst[0],
          "Model type: see --help-modelType",
          "Model type to be imported. The following types are currently supported\n"
          "    0 : Caffe (.caffemodel and .prototxt files)\n"
          "    1 : Tensorflow (.pb files)\n"
          "    2 : ONNX (.onnx files)\n"
          "    3 : tfLite (.tflite files)\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2 or 3) indicating which\n"
          "model type is being imported\n"
          "\n"
          "The default value is \"0\" (Caffe models)\n"},
        {(char *)"quantizationStyle",      &gParams.quantizationStyle     , INT_32, SINGLE, false, false, NULL,
          "Quantization method: see --help-quantizationStyle",
          "Quantization method used for quantizing parameters. The following types are currently supported\n"
          "    0 : Caffe-jacinto specific\n"
          "    1 : Dynamic\n"
          "    2 : Fixed\n"
          "    3 : Power-of-2 Dynamic\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2 or 3) indicating which\n"
          "quantization type to be used\n"
          "\n"
          "The default value is \"2\" (Fixed)\n"},
        {(char *)"calibrationOption",      &gParams.calibrationOption     , INT_32, SINGLE, false, false, NULL,
          "Calibration Options available: see --help-calibrationOption",
          "Various Calibration options supported by TIDL. The following types are currently supported\n"
          "    1 : TIDL_CalibOptionActivationRange\n"
          "    2 : TIDL_CalibOptionWeightRange\n"
          "    4 : TIDL_CalibOptionBiasCalibration\n"
          "    8 :TIDL_CalibOptionPerChannelWeightQuantization\n"
          "    64 :TIDL_CalibOptionBiasRange\n"
          "Recommended options are :\n"
          "0 : Simple calibration ( Default)\n"
          "1 : Histogram based activation range collection \n"
          "7 : Advanced bias calibration\n"
          "\n"
          "This parameter accepts an integer indicating which calibration option to be used.\n"
          "User can choose multiple options together by doiing or of these bit fields.\n"
          "\n"
          "The default value is \"0\" (Fixed). \n"},
        {(char *)"preQuantizedModel",      &gParams.preQuantizedModel     , INT_32, SINGLE, false, false, NULL,
          "Enable preQuantizedModel (ONNX Models only): 0 -> False, 1 -> True",
          " "},
        {(char *)"enableBiasFusion",      &gParams.enableBiasFusion     , INT_32, SINGLE, false, false, NULL,
          "Enable enableBiasFusion (ONNX Models only): 0 -> False, 1 -> True",
          " "},
        {(char *)"softmax16BitScaleUpdate",      &gParams.softmax16BitScaleUpdate     , INT_32, SINGLE, false, false, NULL,
          "Enable softmax16BitScaleUpdate: 0 -> False, 1 -> True",
          " "},
        {(char *)"use16BitForTopK",      &gParams.use16BitForTopK     , INT_32, SINGLE, false, false, NULL,
          "Enable use16BitForTopK: 0 -> False, 1 -> True",
          " "},
        {(char *)"activationRangeMethod",      &gParams.activationRangeMethod     , INT_32, SINGLE, false, false, NULL,
          "Activation Range Methods available: see --help-activationRangeMethod",
          "Various Activation range methods supported by TIDL. The following types are currently supported\n"
          "    0 : TIDL_ActivationRangeMethodHistogram\n"
          "\n"
          "This parameter accepts an integer indicating which activation range method to be used.\n"
          "This option is only applicable if calibrationOption is set to TIDL_CalibOptionActivationRange \n"
          "The default value is \"0\" (Fixed)\n"},
        {(char *)"weightRangeMethod",      &gParams.weightRangeMethod     , INT_32, SINGLE, false, false, NULL,
          "Weight Range Methods available: see --help-weightRangeMethod",
          "Various Weight range methods supported by TIDL. The following types are currently supported\n"
          "    0 : TIDL_WeightRangeMethodHistogram\n"
          "    1 : TIDL_WeightRangeMethodMedian\n"
          "\n"
          "This parameter accepts an integer indicating which weight range method to be used.\n"
          "This option is only applicable if calibrationOption is set to TIDL_CalibOptionWeightRange \n"
          "The default value is \"1\" (Fixed)\n"},
        {(char *)"percentileActRangeShrink",      &gParams.percentileActRangeShrink     , FLOAT, SINGLE, false, false, NULL,
          "Definition of percentileActRangeShrink: see --help-percentileActRangeShrink",
          "This parameter is only applicable when activationRangeMethod is \n"
          "TIDL_ActivationRangeMethodHistogram. This is percentile of the total number \n"
          "of elements in a activation tensor which needs to be discarded from both\n"
          "side of activation distribution. If input is unsigned then this is applied\n"
          "to only one side of activation distribution. For example\n"
          "percentileRangeShrink = 0.01, means to discard 1/10000 elements\n"
          "from both or one side of activation distribution.\n"
          "\n"
          "The default value is \"0.01\" (Fixed)\n"},
        {(char *)"percentileWtRangeShrink",      &gParams.percentileWtRangeShrink     , FLOAT, SINGLE, false, false, NULL,
          "Definition of percentileWtRangeShrink: see --help-percentileWtRangeShrink",
          "This parameter is only applicable when weightRangeMethod is \n"
          "TIDL_WeightRangeMethodHistogram. This is percentile of the total number \n"
          "of elements in a weight filter which needs to be discarded from both\n"
          "side of weight distribution. If input is unsigned then this is applied\n"
          "to only one side of weight distribution. For example\n"
          "percentileRangeShrink = 0.01, means to discard 1/10000 elements\n"
          "from both or one side of weight distribution.\n"
          "\n"
          "The default value is \"0.01\" (Fixed)\n"},
        {(char *)"biasCalibrationFactor",      &gParams.biasCalibrationFactor     , FLOAT, SINGLE, false, false, NULL,
          "Definition of biasCalibrationFactor: see --help-biasCalibrationFactor",
          "This is contribution which is used to update the bias in each iteration based \n"
          "on the difference of actual mean with respect to the mean after quantization. \n"
          "\n"
          "The default value is \"0.05\" (Fixed)\n"},
          {(char *)"biasCalibrationIterations",      &gParams.biasCalibrationIterations     , INT_32, SINGLE, false, false, NULL,
          "Definition of biasCalibrationIterations: see --help-biasCalibrationIterations",
          "This parameter is only applicable when calibrationOption is \n"
          "TIDL_CalibOptionBiasCalibration. This is the number of iterations \n"
          "for which bias calibration will be iteratively run. \n"
          "\n"
          "The default value is \"100\" (Fixed)\n"},
        {(char *)"quantRoundAdd",          &gParams.quantRoundAdd         , INT_32, SINGLE, false, false, NULL,
          "Percentage roll-off to be added for rounding (during quantization): default (50) minimum (0) maximum (100)", NULL},
        {(char *)"numParamBits",           &gParams.numParamBits          , INT_32, SINGLE, false, false, NULL,
          "Bit-depth for model parameters (weights and bias): default (8), minimum (0), maximux (16)", NULL},
        {(char *)"numFeatureBits",         &gParams.numFeatureBits        , INT_32, SINGLE, false, false, NULL,
          "Bit-depth for layer activation: default (8), minimum (0), maximum (16)", NULL},
        {(char *)"inFileFormat",           &gParams.inFileFormat          , INT_32, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"postProcType",           &gParams.postProcType          , INT_32, SINGLE, false, false, &postProcTypeSubst[0],
          "Type of post-processing: see --help-postProcType",
          "Post-processing on output tensor. The following types are currently supported:\n"
          "    0 : No post-processing\n"
          "    1 : Classification (TOP-1 and TOP-5 accuracy)\n"
          "    2 : Object detection (Draw bounding-boxes around detected objects)\n"
          "    3 : Semantic segmentation (Per-pixel color blending)\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2 or 3) indicating which\n"
          "type of post-processing to be applied on the output tensor\n"
          "\n"
          "The default value is \"0\" (No post-processing)\n"},
        {(char *)"postProcDataId",         &gParams.postProcDataId        , INT_32, SINGLE, false, false, NULL,
          "Output tensor index for post processing: minimum (0) default (0) maximum (number of output tensors)", NULL},
        {(char *)"numFrames",              &gParams.numFrames             , INT_32, SINGLE, false, false, NULL,
          "Number of input frames: see --help-numFrames",
          "Number of input frames for validating import. This value can be set to -1\n"
          "for auto-detecting the number of frames\n"
          "\n"
          "The default value is \"-1\"\n"},
        {(char *)"numFramesBiasCalibration",              &gParams.numFramesBiasCalibration             , INT_32, SINGLE, false, false, NULL,
          "Number of input frames: see --help-numFramesBiasCalibration",
          "Number of input frames for running bias calibration in import.\n"
          "This number can be different from numFrames. This value can be set to -1\n"
          "for auto-detecting the number of frames\n"
          "\n"
          "The default value is \"-1\"\n"},
        {(char *)"addDataConvertToNet",              &gParams.addDataConvertToNet             , INT_32, SINGLE, false, false, NULL,
          "Adds layers to the net for formating the user provided data buffers as per the needs of network: see --help-addDataConvertToNet",
          "Adds layers to the net for formating the user provided data buffers as per the needs of network,  \n"
          "This can be used when the user has the float int tensor and needs to use infernce library for converting the \n"
          " The same to fixed point tensot before passing to first layer in the net \n"
          " This is a bit field Parameter,  Bit 0 would add for inputs and Bit 1 would add for outputs\n"
          "\n"
          "The default value is \"0 \" - IO format layers are not added\n"},

        {(char *)"foldBnInConv2D",         &gParams.foldBnInConv2D        , INT_32, SINGLE, false, false, &booleanSubst[0],
          "Flag to merge batch-normalization into previous Conv2D : 1 = true (default) / 0 = false ", NULL},
        {(char *)"foldEltWiseInConv2D",    &gParams.foldEltWiseInConv2D   , INT_32, SINGLE, false, false, &booleanSubst[0],
          "Flag to merge element-wise into previous Conv2D : 1 = true / 0 = false (default)",
          "Boolean flag to merge element-wise operations in previous\n"
          "convolution layer (if possible)\n"
          "\n"
          "This feature is currently not supported. Must be set to \"0\"\n"},
        {(char *)"foldMaxPoolInConv2D",    &gParams.foldMaxPoolInConv2D   , INT_32, SINGLE, false, false, NULL,
          "Flag to merge max-pool operation into previous Conv2D : 1 = true / 0 = false (default)",
          "Boolean flag to merge max-pool operations in previous convolution\n"
          "layer (if possible)\n"
          "\n"
          "This feature is currently not supported. Must be set to \"0\"\n"},
        {(char *)"foldDepthToSpaceInConv2D",    &gParams.foldDepthToSpaceInConv2D   , INT_32, SINGLE, false, false, NULL,
          "Flag to merge depth to space operation into previous Conv2D : 1 = true (default) / 0 = false ",
          "Boolean flag to merge depth to space operations in previous convolution\n"
          "layer (if possible)\n"},
        {(char *)"metaArchType",           &gParams.metaArchType          , INT_32, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"foldPreBnConv2D",        &gParams.foldPreBnConv2D       , INT_32, SINGLE, false, false, NULL,
          "Flag to merge batch-normalization into previous Conv2D :  0 = false, 1 = true (default), 2 = Merge batchnorm in convolution and add pad layer before merged batchnorm layer with padded value equal to mean of batchnorm layer ", NULL},
        {(char *)"debugTraceLevel",        &gParams.debugTraceLevel       , INT_32, SINGLE, false, false, NULL,
          "Trace level: minimum (0) default (0) maximum (2)", NULL},
        {(char *)"writeTraceLevel",        &gParams.writeTraceLevel       , INT_32, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"compileConstraintsFlag", &gParams.compileConstraintsFlag, INT_32, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"executeNetworkCompiler", &gParams.executeNetworkCompiler, INT_32, SINGLE, false, false, NULL,
          "Flag to execute network-compiler after import: 1 = true (default) / 0 = false", NULL},
        {(char *)"quantRangeUpdateFactor", &gParams.quantRangeUpdateFactor, FLOAT,  SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"quantRangeExpansionFactor", &gParams.quantRangeExpansionFactor, FLOAT,  SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inElementType",          &gParams.inElementType[0]      , INT_32, MULTI,  false, false, NULL,
          "Format of each input feature (multi-param): see --help-inElementType",
          "Format of each input feature. The following types are currently supported:"
          "    0 : unsigned char\n"
          "    1 : signed char\n"
          "    2 : unsigned short\n"
          "    3 : signed short\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2 or 3) indicating the type as described above\n"
          "\n"
          "The default value is \"0\" (unsigned char)"
        },
        {(char *)"actMethod",          &gParams.actMethod[0]      , INT_32, MULTI,  false, false, NULL,
          "Flag to indicate the activation method of each activation function (multi-param): see --help-actMethod",
          "Flag to indicate the activation method of each activation function. The following actMethod are currently supported:"
          "    0 : TIDL_SW_NONLUT  (Optimized c7x implementation for 8-bit and 16-bit can be choosen for all devices)\n"
          "    1 : TIDL_SW_SINGLEPOINT_LUT (Software Single Point Look up impelemntation for 8-bit and 16-bit can be choosen for all devices but not fast as TIDL_HW_LUT or TIDL_SW_NONLUT)\n"
          "    2 : TIDL_HW_LUT (Optimized c7x LUT implementation for 8-bit and 16-bit can be choosen for all devices except am62a)\n"
          "\n"
          "This parameter accepts an integer (0, 1, or 2) indicating the Activation Method as described above\n"
          "\n"
          "The default value is \"-1\" (Will choose the best/predefined flow for the particular device)"
        },
        {(char *)"actType",        &gParams.actType       , STRING, SINGLE, false, false, NULL,
          "List of Activation fucntions supported by TIDL", NULL},
        {(char *)"NetInElementType",          &gParams.NetInElementType[0]      , INT_32, MULTI,  false, false, NULL,
          "ElementType for each input feature, which lets user decide the outElementType of the input data convert layer. The following types are currently supported:"
          "    0 : unsigned char\n"
          "    1 : signed char\n"
          "    2 : unsigned short\n"
          "    3 : signed short\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2 or 3) indicating the type as described above\n"
          "\n"
          "The default value is \"-1\" (option not used)"
        },
        {(char *)"rawDataInElementType",          &gParams.rawDataInElementType[0]      , INT_32, MULTI,  false, false, NULL,
          "Raw data Format of each input feature (multi-param): see --help-rawDataInElementType",
          "Raw data Format of each input feature. The following types are currently supported:"
          "    0 : unsigned char\n"
          "    1 : signed char\n"
          "    2 : unsigned short\n"
          "    3 : signed short\n"
          "    6 : float\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2,3, 6) indicating the type as described above\n"
          "This parameter is only applicable when inFileFormat is 1 or 3\n"
          "\n"
          "The default value is \"0\" (unsigned char)"
        },
        {(char *)"inZeroPoint",          &gParams.inZeroPoint[0]      , INT_32, MULTI,  false, false, NULL,
          "Zero point of each fixed point input feature (multi-param): see --help-inZeroPoint",
          "Zero point of each fixed point input feature. The following types are currently supported:"
          "\n"
          "The default value is \"0\" (int)"
        },
        {(char *)"inLayout",          &gParams.inLayout[0]      , INT_32, MULTI,  false, false, NULL,
          "Data Layout of each input feature (multi-param): see --help-inLayout",
          "Data Layout of each input feature. The following types are currently supported:"
          "    0 : TIDL_LT_NCHW\n"
          "    1 : TIDL_LT_NHWC\n"
          "\n"
          "The default value is \"TIDL_LT_NCHW\")"
        },
        {(char *)"inTensorScale",          &gParams.inQuantFactor[0]      , FLOAT, MULTI,  false, false, NULL,
          "Tensor scale of each input feature (multi-param) alias for inQuantFactor: see --help-inTensorScale",
          "Tensor scale of each input feature (multi-param) in float"
          "This is alias for inQuantFactor \n"
          "The default value is \"1.0\" (float)"
        },
        {(char *)"outElementType",          &gParams.outElementType[0]      , INT_32, MULTI,  false, false, NULL,
          "Raw data Format of each output feature (multi-param): see --help-outElementType",
          "Raw data Format of each output feature. The following types are currently supported:"
          "    0 : unsigned char\n"
          "    1 : signed char\n"
          "    2 : unsigned short\n"
          "    3 : signed short\n"
          "    6 : float\n"
          "\n"
          "This parameter accepts an integer (0, 1, 2,3, 6) indicating the type as described above\n"
          "\n"
          "The default value is \"0\" (unsigned char)"
        },
        {(char *)"outZeroPoint",          &gParams.outZeroPoint[0]      , INT_32, MULTI,  false, false, NULL,
          "Zero point of each fixed point ouput feature (multi-param): see --help-outZeroPoint",
          "Zero point of each fixed point ouput feature. The following types are currently supported:"
          "\n"
          "The default value is \"0\" (int)"
        },
        {(char *)"outLayout",          &gParams.outLayout[0]      , INT_32, MULTI,  false, false, NULL,
          "Data Layout of each ouput feature (multi-param): see --help-outLayout",
          "Data Layout of each ouput feature. The following types are currently supported:"
          "    0 : TIDL_LT_NCHW\n"
          "    1 : TIDL_LT_NHWC\n"
          "\n"
          "The default value is \"TIDL_LT_NCHW\")"
        },
        {(char *)"outTensorScale",          &gParams.outTensorScale[0]      , FLOAT, MULTI,  false, false, NULL,
          "Tensor scale of each ouput feature (multi-param): see --help-outTensorScale",
          "Tensor scale of each ouput feature (multi-param) in float"
          "\n"
          "The default value is \"1.0\" (float)"
        },
        {(char *)"outElementSize",          &gParams.outElementSize[0]      , INT_32, MULTI,  false, false, NULL,
          "Format of each output feature (multi-param): see --help-outElementSize",
          "Format of each output feature. The following types are currently supported:"
          "    1 : 1 Byte for 8 bit output\n"
          "    2 : 2 Byte for 16 bit output\n"
          "\n"
          "This parameter accepts an integer (1 or 2) indicating the type as described above\n"
          "\n"
          "The default value is chosen based on the quantization used. For 8 bit quantization output will be"
          "8 bit and similarly for 16bit it will be 16 bit"
        },
        {(char *)"inQuantFactor",          &gParams.inQuantFactor[0]      , FLOAT,  MULTI,  false, false, NULL,
          "Scale factor for each input feature (multi-param): default (1.0)", NULL},
        {(char *)"inMean",                 &gParams.inMean[0]             , FLOAT,  MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inScale",                &gParams.inScale[0]            , FLOAT,  MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inDataNorm",             &gParams.inDataNorm[0]         , INT_32, MULTI,  false, false, NULL,
          "Data normalization flag for each input feature (multi-param): 1 = true / 0 = false (default)", NULL},
        {(char *)"inDataPadInTIDL",             &gParams.inDataPadInTIDL[0]         , INT_32, MULTI,  false, false, NULL,
          "Flag to enable padding for input within TIDL based on consumer layers' requirement  (multi-param): 1 = true / 0 = false (default)", NULL},
        {(char *)"inWidth",                &gParams.inWidth[0]            , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inHeight",               &gParams.inHeight[0]           , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inNumChannels",          &gParams.inNumChannels[0]      , INT_32, MULTI,  false, false, NULL,
          "Number of channel for each input feature (multi-param)", NULL},
        {(char *)"inDIM2",                 &gParams.inDIM2[0]             , INT_32, MULTI,  false, false, NULL,
          "Number of dim2 for each input feature (multi-param)", NULL},
        {(char *)"inDIM1",                &gParams.inDIM1[0]              , INT_32, MULTI,  false, false, NULL,
          "Number of dim1 for each input feature (multi-param)", NULL},
        {(char *)"numBatches",             &gParams.numBatches[0]         , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inferenceMode",          &gParams.inferenceMode         , INT_32, SINGLE,  false, false, NULL,
          "Default 0 - Inference using single c7x/MMA core, 1 - High throughput batch processing mode : Multiple frames parallely infer on multiple cores,\
           2 - Low latency mode : Single batch inference using network split across multiple cores", NULL},
        {(char *)"numCores",               &gParams.numCores              , INT_32, SINGLE,  false, false, NULL,
          "Specifies number of cores to be used for inference. Default is 1. This parameter overrides the NUMCORES parameter set as part of device_config.cfg", NULL},
        {(char *)"inDataFormat",           &gParams.inDataFormat[0]       , INT_32, MULTI,  false, false, NULL,
          "Color format for each input feature (multi-param): 0 = BGR planar / 1 = RGB planar (default)", NULL},
        {(char *)"inResizeType",           &gParams.inResizeType[0]       , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"resizeWidth",            &gParams.resizeWidth[0]        , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"resizeHeight",           &gParams.resizeHeight[0]       , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"reserved",               &gParams.reserved[0]           , INT_32, MULTI,  false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inData",                 &gParams.inData                , STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inStrVal",               &gParams.inStrVal              , CPP_STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"inDataNamesList",        &gParams.inDataNamesList       , STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"outDataNamesList",       &gParams.outDataNamesList      , STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"metaLayersNamesList",    &gParams.metaLayersNamesList   , STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"perfSimTool",            &gParams.perfSimTool           , STRING, SINGLE, false, false, NULL,
          "Path to network compiler tool (path-param): default ${TIDL_INSTALL_PATH}/ti_dl/utils/perfsim/ti_cnnperfsim.out", NULL},
        {(char *)"graphVizTool",           &gParams.graphVizTool          , STRING, SINGLE, false, false, NULL,
          "Path to graph visualisation tool (path-param): default ${TIDL_INSTALL_PATH}/ti_dl/utils/tidlModelGraphviz/out/tidl_graphVisualiser.out", NULL},
        {(char *)"modelDumpTool",          &gParams.modelDumpTool         , STRING, SINGLE, false, false, NULL,
          "Path to model dump tool (path-param): default ${TIDL_INSTALL_PATH}/ti_dl/utils/tidlModelDump/out/tidl_dump.out", NULL},
        {(char *)"perfSimConfig",          &gParams.perfSimConfig         , STRING, SINGLE, false, false, NULL,
          "Path to configuration file for network compiler tool (path-param): see --help-perfSimConfig",
          "Path to configuration file for network compiler tool\n"
          "\n"
          "If none provided, The default config at ${TIDL_INSTALL_PATH}/ti_dl/test/testvecs/config/import/device_config.cfg\n"
          "is used. The user must provide a different set of settings if required\n"},
        {(char *)"msmcSizeKB",             &gParams.msmcSizeKB, INT_32, SINGLE, false, false, NULL,
          "Size of the L3 (MSMC) memory in KB which can be used by TIDL: -1 (default), in default case\n"
          "the size will be picked from the size given in perfSimConfig file.", NULL},
        {(char *)"deviceName",             &gParams.deviceName, INT_32, SINGLE, false, false, NULL,
          "Id for the device -1 (default), TDA4VMID = 0 , TIDL_TDA4AEP = 1,  TIDL_TDA4AM = 2, TIDL_TDA4AMPlus = 3\n"
          "For default case device name will be picked from the perfSimConfig file", NULL},
        {(char *)"tidlStatsTool",          &gParams.tidlStatsTool         , STRING, SINGLE, false, false, NULL,
          "Path to statistics and range collection tool (path-param): default ${TIDL_INSTALL_PATH}/ti_dl/test/tidl_quant_stats_tool.out", NULL},
        {(char *)"inputNetFile",           &gParams.inputNetFile          , STRING, SINGLE, false, false, NULL,
          "Path to input model network definition file (path-param)", NULL},
        {(char *)"inputParamsFile",        &gParams.inputParamsFile       , STRING, SINGLE, false, false, NULL,
          "Path to input model parameters file for caffe networks (path-param)", NULL},
        {(char *)"outputNetFile",          &gParams.outputNetFile         , STRING, SINGLE, false, false, NULL,
          "Path to output TIDL model file (path-param)", NULL},
        {(char *)"quantParamsPrototxtFile",          &gParams.quantParamsPrototxtFile         , STRING, SINGLE, false, false, NULL,
          "Path to quantization parameters of the output model (path-param)", NULL},
        {(char *)"fileNameGrpInfo",        &gParams.fileNameGrpInfo       , STRING, SINGLE, false, false, NULL,
          "HELP TBD", NULL},
        {(char *)"outputParamsFile",       &gParams.outputParamsFile      , STRING, SINGLE, false, false, NULL,
          "Path to output TIDL buffer descriptor file (path-param)", NULL},
        {(char *)"negativeTestUpdate",       &gParams.negativeTestUpdate      , STRING, SINGLE, false, false, NULL,
          "Path to model corrupter description", NULL},
        {(char *)"executeQuantsTool",      &gParams.executeQuantsTool     ,INT_32,  SINGLE, false, false, NULL,
          "Flag to execute quantization tool after import: 1 = true (default) / 0 = false", NULL},
        {(char *)"enableHighResOptimization", &gParams.enableHighResOptimization, INT_32, SINGLE, false, false, NULL,
          "Flag to enable optimization for higher resolution: 1 = enable / 0 = disable (default)", NULL},
        {(char *)"enableCustomLayers", &gParams.enableCustomLayers, INT_32, SINGLE, false, false, NULL,
          "Flag to enable custom layers in network: 1 = enable / 0 = disable (default)", NULL},
        {(char *)"inYuvFormat",            &gParams.inYuvFormat[0] , INT_32, MULTI, false, false, NULL,
          "Flag for Input YUV Format: 0 = YUV420_NV12 / -1 = Not Valid(default)", NULL},
        {(char *)"ddrLayers",             &gParams.ddrLayers[0]    , INT_32, MULTI, false, false, NULL,
          "Layers to move to DDR in GC: list of layer IDs, terminated by -1", NULL},
        {(char *)"singleCoreLayersNames",        &gParams.singleCoreLayersNames       , STRING, SINGLE, false, false, NULL,
          "Layer to infer on single core : list of data Names", NULL},
        {(char *)"spatialSplitLayersNames",        &gParams.spatialSplitLayersNames   , STRING, SINGLE, false, false, NULL,
          "Layer to infer with spatial split : list of data Names", NULL},
        {(char *)"channelSplitLayersNames",        &gParams.channelSplitLayersNames   , STRING, SINGLE, false, false, NULL,
          "Layer to infer with channel split : list of data Names", NULL},
        {(char *)"outputFeature16bitNamesList",      &gParams.outputFeature16bitNamesList       , STRING, SINGLE, false, false, NULL,
          "List of names of the layers (separated by comma or space or tab) as in the original model whose feature/activation output"
          "user wants to be in 16 bit. "
          "Note that if for a given layer feature/activations is in 16 bit then parameters will automatically become 16 bit"
          "As TIDL merges certain layers, this list should correspond to the last layer in the merged output. User can find this mapping"
          "by running the import tool once without this parameter which generates <outputNetFile>.layer_info.txt where outputNetFile is "
          "same as given by the user in the import config file. This *.layer_info.txt file contains three columns the first one is the "
          "layer number, second one is unique data id and last one is the name as given in the original network's model format. This third"
          "column gives the name which will be present in the TIDL imported network after merging any layer. User should use this value for"
          "outputFeature16bitNamesList", NULL},
        {(char *)"mp16bitResolvedFeatureNamesList",      &gParams.mp16bitResolvedFeatureNamesList       , STRING, SINGLE, false, false, NULL,
          "List of names of the layers (separated by comma or space or tab) as in the original model whose feature/activation output"
          "user wants to be in 16 bit. "
          "Note that if for a given layer feature/activations is in 16 bit then parameters will automatically become 16 bit"
          "As TIDL merges certain layers, this list should correspond to the last layer in the merged output. User can find this mapping"
          "by running the import tool once without this parameter which generates <outputNetFile>.layer_info.txt where outputNetFile is "
          "same as given by the user in the import config file. This *.layer_info.txt file contains three columns the first one is the "
          "layer number, second one is unique data id and last one is the name as given in the original network's model format. This third"
          "column gives the name which will be present in the TIDL imported network after merging any layer. User should use this value for"
          "mp16bitResolvedFeatureNamesList", NULL},
        {(char *)"c7xFirmwareVersion",      &gParams.c7xFirmwareVersion       , STRING, SINGLE, false, false, NULL,
          "C7x firmware version to compile models for", NULL},
        {(char *)"params16bitNamesList",       &gParams.params16bitNamesList      , STRING, SINGLE, false, false, NULL,
          "List of names of the output layers (separated by comma or space or tab) as in the original model whose parameters user"
          "wants to be in 16 bit. This is not the name of the parameter of the layer but is expected to be the output name of the layer. "
          "Note that, if a given layers feature/activation is in 16 bit then parameters will automatically become 16 bit"
          "even if its not part of this list"
          "As TIDL merges certain layers, this list should correspond to the last layer in the merged output. User can find this mapping"
          "by running the import tool once without this parameter which generates <outputNetFile>.layer_info.txt where outputNetFile is "
          "same as given by the user in the import config file. This *.layer_info.txt file contains three columns the first one is the "
          "layer number, second one is unique data id and last one is the name as given in the original network's model format. This third"
          "column gives the name which will be present in the TIDL imported network after merging any layer. User should use this value for"
          "params16bitNamesList", NULL},
          {(char *)"nativeLayerNamesList",  &gParams.nativeLayerNamesList , STRING, SINGLE, false, false, NULL,
          "Comma-separated list of output layer names to run with native precision.  Use this to optimize performance for specific layers.", NULL},
          {(char* )"mixedPrecisionFactor",             &gParams.mixedPrecisionFactor, FLOAT, SINGLE, false, false, NULL,
          "This parameter is used when customer wants to enable the automated mixed precision feature. It is defined as mixedPrecisionFactor = "
          "(Acceptable latency with mixed precision / Latency with 8 bit inference), e.g. if acceptable latency is 1.2 times the 8 bit "
          "inference latency, the automated mixed precision algorithm finds the most optimal layers to set to 16 bits to gain accuracy "
          "improvement while making sure latency factor set by this parameter is met. Can be set to any float value ", NULL},
          {(char* )"modelGroupId",             &gParams.modelGroupId, INT_8, SINGLE, false, false, NULL,
          "This parameter is used when user wants to enable the DDR optimization for same network but being used for different resolution."
          "User can set the same gorupId for same models and TIDL-RT will share the pool of memories which can be shared for such models"
          "For example a application writer want mobilenet model to be imported for 1024x512 and 1024x1024 resolution, then he/she can import"
          "both of these models with same group id, lets say group id is 1.  while importing these 2 different situation of same model archicture"
          "Important Note: User can concurrently run both of these models in a system with optimal use of DDR memory, but if user is"
          " trying to pre-empt models from same group then user should not try to create this pre-emption level more than TIDL_MAX_PREEMPTION_LEVEL_WITH_GROUPID"
          " and also assign the modelGroupId different for the network beyond preemption level boundary"
          " example: Lets say user has 8 different resolution of same models and TIDL_MAX_PREEMPTION_LEVEL_WITH_GROUPID is 4, in that situation, there are 2 possibilities:"
          " case 1: modelGroupId are defined as 1 1 1 1 2 2 2 2, and lets say models are referred as M0,M1,.....M7"
          " case 2: modelGroupId are defined as (1 1 1 1) (1 1 1 1), and lets say models are referred as M0,M1,.....M7"
          " IN case 1 any model can interrupt any other model, but in case 2, first 4 models (M0,M1,M2,M3) can interrupt each other"
          " and second 4 can models ((M4,M5,M6,M7)) can interrupt each other"
          " the order in which models are created using TIDLRT_create are the order of model numbering: M0, M1,... and so on"
          " case 2 is more optimal from memory usage point of view but needs careful creation order"
          " User can set to any value b/w 1 to 127, default value is 0 which means this optimization is not used", NULL},
          {(char *)"logFileName",             &gParams.logFileName, STRING, SINGLE, false, false, NULL,
          "Log file name for storing the logs", NULL},
          {(char *)"partialInitDuringCompile",      &gParams.partialInitDuringCompile     , INT_8, SINGLE, false, false, NULL,
          "Enable partial init during model compilation: 0 -> Disable (Default), 1 -> Enables",
          " "},
          {(char *)"batchMode", &gParams.batchMode, INT_32, SINGLE, false, false, NULL,
          "batchMode=0 (Default) Disable batch stitching, batchMode=1 Enable stitching", NULL},
          {(char *)"graphOptimizationLevel", &gParams.graphOptimizationLevel, INT_32, SINGLE, false, false, NULL,
          "graphOptimizationLevel=0 (TIDL_OPTIMIZE_LEVEL_BASIC, default) Enable only Basic optimizations, graphOptimizationLevel=1 (TIDL_OPTIMIZE_LEVEL_EXTENDED) Enable extended and basic optimizations to execute", NULL},
          {(char *)"enableShapeFolding", &gParams.enableShapeFolding, INT_32, SINGLE, false, false, NULL,
          "enableShapeFolding=0 (Default) Disable shape folding, enableShapeFolding=1 Enable shape folding", NULL},
          {(char *)"optimizeBatchNormHigherDims", &gParams.optimizeBatchNormHigherDims, INT_32, SINGLE, false, false, NULL,
          "optimizeBatchNormHigherDims=0 (Default) Disable higher dims fuse for batchnorm, optimizeBatchNormHigherDims=1 Enable higher dims fuse for batchnorm", NULL},
          {(char *)"forceSpatialSplitInLowLatencyMode", &gParams.forceSpatialSplitInLowLatencyMode, INT_32, SINGLE, false, false, NULL,
          "forceSpatialSplitInLowLatencyMode=1 Force spatial split in low latency mode", NULL},
          {(char *)"forceBatchSplitInLowLatencyMode", &gParams.forceBatchSplitInLowLatencyMode, INT_32, SINGLE, false, false, NULL,
          "forceBatchSplitInLowLatencyMode=1 Force batch split in low latency mode", NULL},
          {(char *)"optimizeTopKAxis", &gParams.optimizeTopKAxis, INT_32, SINGLE, false, false, NULL,
          "optimizeTopKAxis=1 (Default) Enabled top axis optimization", NULL},
          {(char *)"packetizeMode",    &gParams.packetizeMode   , INT_32, SINGLE, false, false, NULL,
          "This parameter is used when user wants to enable the Packetization of the weights buffer for the convolution layers", NULL},
          {(char *)"ncTempInfoDir",             &gParams.ncTempInfoDir, STRING, SINGLE, false, false, NULL,
          "Path to dump NC info files", NULL},
          {(char *)"enableTFROptimization", &gParams.enableTFROptimization, INT_32, SINGLE, false, false, NULL,
            "Flag to enable Experimental TFR Optimizations: 0 = disable (default) / 1 = Using Float Range for Calibartion / 2 = Using Global Histogram for calibration", NULL},
          {(char *)"enableConcatNoOp", &gParams.enableConcatNoOp, INT_32, SINGLE, false, false, NULL,
            "Flag to enable concat no op optimizations: 0 = disable / 1 = Enable (default)", NULL},
          {(char *)"enableZeroPointForOutputDataConvert",      &gParams.enableZeroPointForOutputDataConvert     , INT_32, SINGLE, false, false, NULL,
            "Enable zero-point for Output DataConvert: 0 -> False, 1 -> True",
            " "},
} ;

/*
Common memory across multiple handles:
allocate always a memory chunk which is N * backup meory where N is maxPreemptLevel (provided during create time)
This is not really tied to an handle but a gloabl memory requsted by TIDL on a core

Common memory across reoslutions but same network ID

1. Memory can be more dynamic to avoid order constraint
    [
      Create (M0: 1) : Mem: 8b
      Create (M1: 1) : Mem: 8b
      Create (M2: 1) : Mem: 8b
      Create (M3: 1) : Mem: 8b
    ],
    [
      Create (M4: 1) : Mem: 0MB, reuse from M0
      Create (M5: 1) : Mem: 0MB, reuse from M1
      Create (M6: 1) : Mem: 0MB, reuse from M2
      Create (M7: 1) : Mem: 0MB, reuse from M1
    ]

    TIDL: accpet maxPreemptLevel during create time and always request for contextSize * maxPreemptLevel
    then orderig constraint can be removed. Has to be a common variable across all the instances...

    Can have a context level setting in openVX node and can apply to all ndoes - provide some reference (Jesse/Lucas)

    12 models: 12*8MB : 96 MB (context memory:) - current state, others: <10-20 MB... Resnet50: 25 MB, mobilenet: 4MB... 4 MB*12 :48 MB ==> 4MB....10 MB
    12 modesl : 8*8MB : 64 MB (context memory:) (without any groupId)
    12 models : 4*8MB (with groupId and some constraints) + additonal saving

    8*8MB
   (<8)*8MB
  with contarint of 4
    create 3 groups, memory requirement would be 8 * 8MB


    For each group: 8MB*4 , worst case 8MB *8 :
    + 8 MB *4

    8MB * 8 = 64 MB.... 8MB *4
    8 MB: scratch memory
    same group ID across M1...M5 is not allowed in below case of pre-emption
     M1 <start...... 8mb
                    M2<start.........8mb
                                     M3<start......8mb
                                                   M4<start......                  .......end>
                                                                 M5<start......end>

    Above use case : worst DDR case : can be exercised with having different groupId M1,M2,M3,...
    Above can be exercised with having M1,M2,M3,M4 as one group Id, M5,M6, M7,...

*/



