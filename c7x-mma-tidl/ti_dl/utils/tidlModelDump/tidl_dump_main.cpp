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
@file      tidl_dump_main.cpp
@brief     Dump the TIDL network representation to stdout
@details   Invoke tidl_dump.out with no arguments to get usage summary
*/
#include "itidl_ti.h"
#include "tidl_dump.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

static sTIDL_Network_t *read_network(const char *net_filename);
static sTIDL_IOBufDesc_t *read_IOParams(const char *iodesc_filename);
static void readLayerInfoFile(const char *liFileName, int nLayers, 
                              TIDL_dump::sLayerSpecificInfo_t *layersInfo, int inferenceMode);
static const char *get_arg(int argc, char *argv[], int index);
static void usage();

//-------------------------------------------------------------------------
// 
int main(int32_t argc, char *argv[])
{
  const char *network_filename = NULL;
  const char *io_filename = NULL;
  const char *output_filename = NULL;
  TIDL_dump::options_t options;
  
  for (int i = 1; i < argc; ++i)
  {
     if (strcmp(argv[i], "-model") == 0 || (strcmp(argv[i], "-m") == 0))
        network_filename = get_arg(argc, argv, ++i);
     else if (strcmp(argv[i], "-io") == 0 || (strcmp(argv[i], "-i") == 0))
        io_filename = get_arg(argc, argv, ++i);
     else if (strcmp(argv[i], "-output") == 0 || (strcmp(argv[i], "-o") == 0))
        output_filename = get_arg(argc, argv, ++i);
     else if (strcmp(argv[i], "-perfsim") == 0 || (strcmp(argv[i], "-p") == 0))
        options["perfsim"] = true;
     else if (strcmp(argv[i], "-full") == 0)
        options["full_weights"] = true;
        
     else 
        printf("unknown option: %s\n", argv[i]);
  }
  // No files! Print usage summary
  if (!network_filename && !io_filename)
  {
     usage();
     exit(1);
  }
  // Create output stream on either file or cout
  std::ostream* os = &std::cout;
  std::ofstream ofile;
  if (output_filename)
  {
    ofile.open(output_filename);
    if (!ofile.is_open())
    {
      printf("cannot open output file %s\n", output_filename);
      exit(1);
    }
    os = &ofile;
  }

  sTIDL_Network_t *network = nullptr;
  sTIDL_IOBufDesc_t *IOParams = nullptr;
  TIDL_dump::sLayerSpecificInfo_t layersInfo[TIDL_NUM_MAX_LAYERS];

  // Read network file
  if (network_filename)
  {
     network = read_network(network_filename);
     // Read layer info file, to get layer names
     std::string liFileName = std::string(network_filename) + ".layer_info.txt";
     readLayerInfoFile(liFileName.c_str(), network->numLayers, layersInfo, network->inferenceMode);
  }

  // Read IOBufDesc file
  if (io_filename)
     IOParams = read_IOParams(io_filename);

  // Instantiate the dumper 
  TIDL_dump dumper(*os, network, IOParams, layersInfo, options);

  // Invoke the dumper to print the network
  dumper.dumpNet();
  if (IOParams)
     dumper.dumpIODesc();

  return 0;
}

static const char *get_arg(int argc, char *argv[], int index)
{
   if (index >= argc)
   {
      printf("missing argument to %s\n", argv[index-1]);
      usage();
      exit(1);
   }
   return argv[index];
}

static void usage()
{
   printf("Usage: tidl_dump <options>\n");
   printf("Options:\n");
   printf("  -model|-m <TIDL Network File>  Specify TIDL model (.bin)\n");
   printf("  -io|-i <IOBufDescFile>         Print network input/output info\n");
   printf("  -perfsim|-p                    Include data flow info from perfsim tool\n");
}

// Load the network from a binary file
sTIDL_Network_t *read_network(const char *net_filename)
{
  FILE *net_file = fopen(net_filename, "rb");
  if (!net_file)
  {
     printf("cannot open network file %s\n", net_filename);
     exit(1);
  }
  fseek(net_file, 0, SEEK_END);
  int32_t model_size = ftell(net_file);
  fseek(net_file, 0, SEEK_SET);
  sTIDL_Network_t *network = (sTIDL_Network_t *)malloc(model_size);
  if (fread(network, model_size, 1, net_file) != 1)
  {
     printf("fail to read network file %s\n", net_filename);
     exit(1);
  }
  fclose(net_file);

  return network;
}

// Load the IO descriptor .bin file. This specifies the shape and format
// of the input and output tensors.
sTIDL_IOBufDesc_t *read_IOParams(const char *iodesc_filename)
{
  FILE *iodesc_file = fopen(iodesc_filename, "rb");
  if (!iodesc_file)
  {
     printf("cannot open IODesc file %s\n", iodesc_filename);
     exit(1);
  }
  sTIDL_IOBufDesc_t *IOParams =
     (sTIDL_IOBufDesc_t *)malloc(sizeof(sTIDL_IOBufDesc_t));
  if (fread(IOParams, sizeof(sTIDL_IOBufDesc_t), 1, iodesc_file) != 1)
  {
     printf("fail to read IODesc file %s\n", iodesc_filename);
     exit(1);
  }
  fclose(iodesc_file);

  return IOParams;
}

// Read the layer info file, and fill in layer names map
void readLayerInfoFile(const char *liFileName, int nLayers, 
                       TIDL_dump::sLayerSpecificInfo_t *layersInfo, int inferenceMode)
{
  FILE *fpNames = fopen(liFileName, "r");
  if (!fpNames)
    return;
  for (int i = 0; i < nLayers; i++)
  {
    char name[300];
    int temp1, temp2, temp3 = -1;

    if(inferenceMode != TIDL_inferenceModeLowLatency) /*single core inference*/
    {
      if (fscanf(fpNames, "%d %d %s", &temp1, &temp2, name) != 3)
      {
         printf("parsing error reading layer info file %s\n", liFileName);
         return;
      }
    }
    else
    {
      if (fscanf(fpNames, "%d %d %d %s", &temp1, &temp2, &temp3, name) != 4)
      {
         printf("parsing error reading layer info file %s\n", liFileName);
         return;
      }
    }
    layersInfo[i].layerName = name;

    layersInfo[i].multiCoreMode = temp3 != -1 
                                  ? temp3 
                                  : TIDL_NOT_MULTI_CORE;
  }
  fclose(fpNames);
}
