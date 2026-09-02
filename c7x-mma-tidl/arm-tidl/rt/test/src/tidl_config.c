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
#include "configparser.h"
#include "tidl_config.h"
#pragma RESET_MISRA ("required")


tidl_net_config gParams;

sTokenMapping gsTokenMap_tidl_net[MAX_ITEMS_TO_PARSE] =
{
        {(char *)"noZeroCoeffsPercentage",   &gParams.noZeroCoeffsPercentage         , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"updateNetWithStats",       &gParams.updateNetWithStats             , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"inFileFormat",             &gParams.inFileFormat                   , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"writeInput",               &gParams.writeInput                     , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"writeOutput",              &gParams.writeOutput                    , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"compareRef",               &gParams.compareRef                     , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"numFrames",                &gParams.numFrames                      , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"startFrameIdx",            &gParams.startFrameIdx                  , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"preProcType",              &gParams.preProcType                    , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"postProcType",             &gParams.postProcType                   , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"postProcDataId",           &gParams.postProcDataId                 , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"totNumClasses",            &gParams.totNumClasses                  , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"writeBinsAsHeader",        &gParams.writeBinsAsHeader              , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"targetPriority",           &gParams.targetPriority                 , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"coreNum",                  &gParams.coreNum                        , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"coreStartIdx",             &gParams.coreStartIdx                   , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"reservedCtrl",             &gParams.reservedCtrl                   , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"flowCtrl",                 &gParams.flowCtrl                       , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"inResizeMode",             &gParams.inResizeMode                   , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"numItrPerf",               &gParams.numItrPerf                     , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"debugTraceLevel",          &gParams.debugTraceLevel                , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"writeTraceLevel",          &gParams.writeTraceLevel                , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"logStats",                 &gParams.logStats                       , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"enableLayerPerfTraces",    &gParams.enableLayerPerfTraces          , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"dumpNetInitBackupData",    &gParams.dumpNetInitBackupData          , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"initOnly",                 &gParams.initOnly                       , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char* )"performanceTestcase",      &gParams.performanceTestcase            , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char* )"inFileFormatRandomType",   &gParams.inFileFormatRandomType         , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"computeChecksum",          &gParams.computeChecksum                , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        { (char*)"forceNegativeTest",        &gParams.forceNegativeTest              , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"enableCodeCoverage",       &gParams.enableCodeCoverage             , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"releaseIOTensorsAtCreate", &gParams.releaseIOTensorsAtCreate       , INT_32, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"quantRangeExpansionFactor",&gParams.quantRangeExpansionFactor      ,  FLOAT, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"quantRangeUpdateFactor",   &gParams.quantRangeUpdateFactor         ,  FLOAT, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"maxPreEmptDelay",          &gParams.maxPreEmptDelay                ,  FLOAT, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"tempBufferDir",            &gParams.tempBufferDir                  , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"netBinFile",               &gParams.netBinFile                     , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"paramsBinFile",            &gParams.paramsBinFile                  , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"meanImage",                &gParams.meanImage                      , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"inData",                   &gParams.inData                         , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"outData",                  &gParams.outData                        , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"traceDumpBaseName",        &gParams.traceDumpBaseName              , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"testCaseName",             &gParams.testCaseName                   , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"testCaseDesc",             &gParams.testCaseDesc                   , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"ioConfigFile",             &gParams.ioConfigFile                   , STRING, SINGLE, false, false, NULL, NULL, NULL},
        {(char *)"inDimsFile",               &gParams.inDimsFile                     , STRING, SINGLE, false, false, NULL, NULL, NULL},
} ;





