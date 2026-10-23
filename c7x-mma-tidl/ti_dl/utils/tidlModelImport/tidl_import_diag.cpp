/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iterator>
#include <regex>

#include "tidl_import_diag.h"

/* Global diagnostics manager */
TIDL_ImportDiagnostics gDiags;
DiagList_t TIDL_ImportDiagnostics::gDiagList;

/*
 Implementation of TIDL_ModelDiagnostic
*/

// Construct a diagnostic using printf-style formatting
TIDL_ModelDiagnostic::TIDL_ModelDiagnostic(TIDL_ModelDiagnostic::Kind k, TIDL_ModelDiagnostic::Color c, const char* fmt, ...) : kind(k), color(c)
{
  va_list args;
  va_start(args, fmt);
  size_t len = vsnprintf(nullptr, 0, fmt, args);
  va_end(args);

  va_start(args, fmt);
  std::vector<char> buf(len+1);
  len = vsnprintf(buf.data(), buf.size(), fmt, args);
  va_end(args);
  msg = std::string(buf.data(), len);
  if(kind == TIDL_ModelDiagnostic::DK_PrintHeading || kind == TIDL_ModelDiagnostic::DK_PrintSubheading)
  {
    int32_t lineLen = (80 - (msg.length() + 2)) / 2;
    if(lineLen < 5)
      lineLen = 5;
    if(kind == TIDL_ModelDiagnostic::DK_PrintHeading)
    {
      std::string line(lineLen, '=');
      msg =  line + " " + msg + " " + line + "\n" ;
    }
    else
    {
      std::string line(lineLen, '-');
      msg =  line + " " + msg + " " + line;
    }
  }
}


// Emit diagnostic to fptr
void TIDL_ModelDiagnostic::emit(FILE* fptr) const
{
  // Remove ANSI from the message if not stdout
  if(fptr != stdout)
  {
    std::string cleanMsg = TIDL_ImportDiagnostics::removeAnsi(msg);
    fprintf(fptr, "%s\n", cleanMsg.c_str());
  }
  else
  {
    fprintf(fptr, "%s\n", msg.c_str());
  }
}
// return kind of diagnosis
int32_t TIDL_ModelDiagnostic::getKind() const
{
  return kind;
}

// return color of diagnosis
int32_t TIDL_ModelDiagnostic::getColor() const
{
  return color;
}

std::string TIDL_ModelDiagnostic::getString() const
{
  return msg;
}

std::map<std::string, std::string> FILE_CATEGORY{
  {"tidl_caffeImport_param.cpp",          "[PARSER]"},
  {"tidl_caffeImport_v2.cpp",             "[PARSER]"},
  {"tidl_import_common.cpp",              "[PARSER]"},
  {"tidl_import_common_model_check.cpp",  "[PARSER]"},
  {"tidl_import_common.h",                "[PARSER]"},
  {"tidl_import_common_model_check.h",    "[PARSER]"},
  {"tidl_onnxImport.cpp",                 "[PARSER]"},
  {"tidl_onnxrt_common.cpp",              "[PARSER]"},
  {"tidl_onnxRtImport_core.cpp",          "[PARSER]"},
  {"tidl_onnxRtImport_EP.cpp",            "[PARSER]"},
  {"tidl_onnxImport.h",                   "[PARSER]"},
  {"tidl_onnxrt_common.h",                "[PARSER]"},
  {"tidl_onnxRtImport_core.h",            "[PARSER]"},
  {"tidl_onnxRtImport_EP.h",              "[PARSER]"},
  {"tidl_caffeImport_param.h",            "[PARSER]"},
  {"tidl_tfImport.cpp",                   "[PARSER]"},
  {"tidl_tfLiteImport.cpp",               "[PARSER]"},
  {"tidl_tfLiteRtImport_core.cpp",        "[PARSER]"},
  {"tidl_tfLiteRtImport_delegate.cpp",    "[PARSER]"},
  {"tidl_tflrt_common.cpp",               "[PARSER]"},
  {"tidl_relayImport.cpp",                "[PARSER]"},
  {"tidl_tfLiteRtImport.h",               "[PARSER]"},
  {"tidl_tflrt_common.h",                 "[PARSER]"},
  {"tidl_import_quantize.h",              "[QUANTIZATION]"},
  {"tidl_import_quantize.cpp",            "[QUANTIZATION]"},
  {"tidl_quantParamsImport.cpp",          "[QUANTIZATION]"},
};

std::string TIDL_ImportDiagnostics::getModuleFromFile(std::string filename)
{
  if (FILE_CATEGORY.find(filename) != FILE_CATEGORY.end())
  {
    return FILE_CATEGORY[filename];
  }
  else
  {
    return "";
  }
}

std::string TIDL_ImportDiagnostics::removeAnsi(std::string str)
{
  std::regex re("\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])");
  std::stringstream result;
  std::regex_replace(std::ostream_iterator<char>(result), str.begin(), str.end(), re, "");
  return result.str();
}

// Return true if this diagnostic represents a condition that will prevent the network from operating properly on TIDL.
bool TIDL_ModelDiagnostic::isError() const
{
  return kind == DK_NotSupported || kind == DK_Error;
}

// Return true if this condition should not prevent offloading this node to TIDL as part of a subgraph.
bool TIDL_ModelDiagnostic::isOKToOffload() const
{
  switch (kind)
  {
    default:
    case DK_Error:
    case DK_NotSupported:
    case DK_NotVerified:   return false;

    case DK_Supported:
    case DK_NotPerformant:
    case DK_Info:          return true;
  }
}

void TIDL_createTable(std::ostream &stream,
                      std::vector<std::string> header,
                      std::vector<std::vector<std::string>> data,
                      int32_t padding,
                      std::vector<TIDL_table_align_t> columnAlignment,
                      bool printSeperator)
{
  int32_t numColumn = header.size();
  if(numColumn <= 0)
  {
    return;
  }
  for (auto &d: data)
  {
    if(d.size() != numColumn)
    {
      printf("No of headers columns does not match number of data column\n");
      return;
    }
  }

  bool useDynamicColumnResizing=false;
  int32_t i;
  std::vector<size_t> columnSizes;
  columnSizes.resize(numColumn);


  // Initialize each column width with header text sizes.
  for (i = 0; i < numColumn; i++)
  {
    std::string text = TIDL_ImportDiagnostics::removeAnsi(header[i]);
    columnSizes[i] = text.length();
  }

  // Get width for each column according to largest string present
  for (auto &d: data)
  {
    for(i = 0; i < d.size(); i++)
    {
      std::string text = TIDL_ImportDiagnostics::removeAnsi(d[i]);
      if(text.length() > columnSizes[i])
      {
        columnSizes[i] = text.length();
      }
    }
  }

  for (i = 0; i < columnSizes.size(); i++)
  {
    if(columnSizes[i] > (((80-(numColumn + 1))/numColumn) - (2*padding)))
    {
      useDynamicColumnResizing = true;
      break;
    }
  }

  if(!useDynamicColumnResizing)
  {
    for (i = 0; i < columnSizes.size(); i++)
    {
      columnSizes[i] = ((80-(numColumn + 1))/numColumn) - (2*padding);
    }
  }

  std::string paddingString(padding, ' ');

  int32_t totalWidth = 0;
  for (i = 0; i < columnSizes.size(); i++)
  {
    totalWidth += columnSizes[i] + (2 * padding);
  }
  totalWidth += numColumn + 1;
  std::string dashLine(totalWidth, '-');

  // Print the top of the table
  stream << dashLine << "\n";

  // Print center aligned header
  stream << "|";
  for (i = 0; i < numColumn; i++)
  {
    std::string text = TIDL_ImportDiagnostics::removeAnsi(header[i]);
    int32_t ansiExtra = header[i].length() - text.length();
    int32_t half = (columnSizes[i] / 2) - (text.length() / 2);
    if(half < 0)
    {
      half = 0;
    }
    stream << paddingString << std::setw(columnSizes[i]+ansiExtra) << std::left
              << std::string(half, ' ') + header[i] << paddingString << "|";
  }

  stream << "\n";

  // Print dash line at bottom of header
  stream << dashLine << "\n";

  if(data.size() <= 0)
  {
    return;
  }

  int32_t cnt = 0;
  // Print data
  for (auto &d : data)
  {
    stream << "|";

    // Set precision in case of float values;
    stream << std::fixed << std::setprecision(2);

    for (i = 0; i < d.size(); i++)
    {
        auto alignment = std::left;
        int32_t half = 0;
        std::string text = TIDL_ImportDiagnostics::removeAnsi(d[i]);
        int32_t ansiExtra = d[i].length() - text.length();
        if(i < columnAlignment.size())
        {
            alignment = columnAlignment[i] == ALIGN_RIGHT ? std::right : std::left;
            if(columnAlignment[i] == ALIGN_CENTER)
            {
                half = (columnSizes[i] / 2) - (text.length() / 2);
                if(half < 0)
                {
                  half = 0;
                }
            }
        }
        stream << paddingString << std::setw(columnSizes[i]+ansiExtra) << alignment
                  << std::string(half, ' ') + d[i] << paddingString << "|";
    }
    stream << "\n";
    if(printSeperator && cnt++ < data.size() - 1)
    {
      stream << dashLine << "\n";
    }
  }

  stream << dashLine;
}

/*Temporarily using printf() directly - needs to be wrapped by GLOBAL_REPORT otherwise won't be redirected to file*/
void TIDL_printProgressBar(float progress)
{
  int32_t i;
  std::string progressBar;
  printf("[");
  //progressBar << "[";
  int32_t pos = progress * 80;
  for(i = 0; i < 80; i++)
  {
    if (i < pos)
    {
      //progressBar << "=";
      printf("=");
    }
    else if(i == pos)
    {
      //progressBar << ">";
      printf(">");
    }
    else
    {
      //progressBar << " ";
      printf(" ");
    }
  }
  //progressBar << "]";
  printf("]");
  //progressBar << " " << (int)(progress * 100.0) << "%\r";
  printf(" %d%%\r",(int)(progress * 100.0));
  fflush(stdout);
}

void TIDL_printCompilationStages()
{
  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Model Compilation Started]");
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"Model compilation will perform the following stages:\n1. Parsing\n2. Graph Optimization\n3. Quantization & Calibration\n4. Memory Planning\n");
}

void TIDL_printVersionTable(std::string tidlToolsVer, std::string c7xFirmwareVer, std::string runtimeVer, std::string opSetVer)
{
  std::stringstream tableStream;
  std::string tableString;

  std::stringstream tidlToolsVersion;
  std::stringstream c7xFirmwareVersion;
  std::stringstream runtimeVersion;
  std::stringstream opSetVerSion;

  tidlToolsVersion << "\033[32;1m" << tidlToolsVer << "\033[0m";
  c7xFirmwareVersion << "\033[32;1m" << c7xFirmwareVer << "\033[0m";

  std::vector<std::string> header = {"TIDL Tools Version",tidlToolsVersion.str()};

  std::vector<std::vector<std::string>> data = {{"C7x Firmware Version",c7xFirmwareVersion.str()}};

  if(runtimeVer != "")
  {
    runtimeVersion << "\033[32;1m" << runtimeVer << "\033[0m";
    data.push_back({"Runtime Version", runtimeVersion.str()});
  }

  if(opSetVer != "")
  {
    opSetVerSion << "\033[32;1m" << opSetVer << "\033[0m";
    data.push_back({"Model Opset Version", opSetVerSion.str()});
  }

  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Version Summary]");
  TIDL_createTable(tableStream, \
                  header,
                  data, \
                  0, \
                  {ALIGN_CENTER,ALIGN_CENTER,ALIGN_CENTER,ALIGN_CENTER},
                  true);
  tableString = tableStream.str();
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"%s\n",tableString.c_str());
}