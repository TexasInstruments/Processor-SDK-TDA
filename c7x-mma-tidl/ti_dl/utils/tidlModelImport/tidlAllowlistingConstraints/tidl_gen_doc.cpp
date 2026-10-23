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

#include "tidl_constraint_table.h"
#include "../tidl_parse_tflite_jumptable.h"
#include "../tidl_parse_onnx_jumptable.h"
#include "../tidl_parse_tvm_jumptable.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <map>
using namespace std;

/* Internal Documentation: TIDL Layers to Onnx, TfLite and TVM Operators and Constraints */
void operatorMapping();
/* Onnx Operator Constraints and mapping to TIDL layers */
void onnxConstraints();
/* TfLite Operator Constraints and mapping to TIDL layers */
void tfliteConstraints();
/* TVM Operator Constraints and mapping to TIDL layers */
void tvmConstraints();

map<int32_t, vector<string>> tfLiteMappings;
map<int32_t, vector<string>> onnxMappings;
map<int32_t, vector<string>> tvmMappings;

/* Generate all documentations */
int main()
{
    operatorMapping();
    onnxConstraints();
    tfliteConstraints();
    tvmConstraints();
}


void operatorMapping()
{
    ofstream file("operator_mappings.md");
    file << "# Operator Mappings (TIDL-RT):" << endl << endl;
    file << "<center>" << endl << endl;
    file << "| S. No. | TIDL Layer Type | Onnx Ops. | TfLite Ops. | TVM Ops. | Constraints |" << endl;
    file << "|:------:|:----------------|:----------|:------------|:---------|:------|" << endl;

    for (auto jtEntry : tidlConstraintTable)
    {
        tfLiteMappings.insert({jtEntry.first, vector<string>{}});
        onnxMappings.insert({jtEntry.first, vector<string>{}});
        tvmMappings.insert({jtEntry.first, vector<string>{}});
    }

    for (auto jtEntry: tidlParseTfliteJumpTable)
    {
        auto tfliteMappingsEntry = tfLiteMappings.find(jtEntry.second.first);
        if (tfliteMappingsEntry != tfLiteMappings.end())
        {
            tfliteMappingsEntry->second.push_back(jtEntry.second.second.substr(14));
        }
    }
    for (auto jtEntry: tidlParseOnnxJumpTable)
    {
        auto onnxMappingsEntry = onnxMappings.find(jtEntry.second.first);
        if (onnxMappingsEntry != onnxMappings.end())
        {
            onnxMappingsEntry->second.push_back(jtEntry.second.second);
        }
    }
    for (auto jtEntry: tidlParseTVMJumpTable)
    {
        auto tvmMappingsEntry = tvmMappings.find(jtEntry.second.first);
        if (tvmMappingsEntry != tvmMappings.end())
        {
            tvmMappingsEntry->second.push_back(jtEntry.second.second);
        }
    }

    int index = 1;
    for (auto jtEntry : tidlConstraintTable)
    {
        file << "| " << to_string(index) << " | " << jtEntry.second.displayname << " | <ul> ";

        for (string opName : onnxMappings[jtEntry.first])
        {
            file << "<li> " << opName << " </li>";
        }

        file << " </ul> | <ul> ";

        for (string opName : tfLiteMappings[jtEntry.first])
        {
            file << "<li> " << opName << " </li>";
        }

        file << " </ul> | <ul> ";

        for (string opName : tvmMappings[jtEntry.first])
        {
            file << "<li> " << opName << " </li>";
        }

        file << " </ul> | <ul> ";
        vector<TidlConstraint> constraints = jtEntry.second.constraints;
        for (auto constraint : constraints)
        {
            if (constraint.comments.size() != 0) file << "<li> " << constraint.comments << " </li>";
        }
        file << "</ul> | " << endl;
        index++;
    }
    file << endl << "</center>" << endl;
    file.close();    
}

void onnxConstraints()
{
    ofstream file("onnx_operators.md");
    file << "# ONNX Operator Mappings:" << endl << endl;
    file << "<center>" << endl << endl;
    file << "| S. No. | Onnx Operator | TIDL Layer | Constraints |" << endl;
    file << "|:------:|:--------------|:-----------|:------|" << endl;

    int index = 1;
    for(auto onnxMap: onnxMappings)
    {
        TidlConstraintTableEntry jtEntry = tidlConstraintTable.find(onnxMap.first)->second;
        if(onnxMap.second.size() == 0)
        {
            /* No mapping from onnx node to this layer in parsing */
            continue;
        }
        file << "| " << to_string(index) << " | ";
        for (int i = 0; i < onnxMap.second.size(); i++)
        {
            file << onnxMap.second[i];
            if(i != onnxMap.second.size() - 1)
            {
                file << "/";
            }
        }
        file << " | " << jtEntry.displayname << " | <ul> ";
        vector<TidlConstraint> constraints = jtEntry.constraints;
        for (auto constraint : constraints)
        {
            if (constraint.onnxComment.size() == 0)
            {
                continue;
            }
            file << "<li> " << constraint.onnxComment << " </li>";
        }
        file << "</ul> | " << endl;
        index++;
    }

    file << endl << "</center>" << endl;
    file.close();    
}

void tvmConstraints()
{
    ofstream file("tvm_operators.md");
    file << "# TVM Operator Mappings:" << endl << endl;
    file << "<center>" << endl << endl;
    file << "| S. No. | Onnx operator | TI TVM Operator | TIDL Layer | Constriants |" << endl;
    file << "|:------:|:--------------|:-----------------|:-----------|:------|" << endl;

    int index = 1;
    for(auto tvmMap: tvmMappings)
    {
        TidlConstraintTableEntry jtEntry = tidlConstraintTable.find(tvmMap.first)->second;
        if(tvmMap.second.size() == 0)
        {
            /* No mapping from tvm node to this layer in parsing */
            continue;
        }
        vector<string> onnxOpNames = onnxMappings[tvmMap.first];
        file << "| " << to_string(index) << " | ";
        for (int i = 0; i < onnxOpNames.size(); i++)
        {
            file << onnxOpNames[i];
            if(i != onnxOpNames.size() - 1)
            {
                file << "/";
            }
        }
        file << " | ";
        for (int i = 0; i < tvmMap.second.size(); i++)
        {
            file << tvmMap.second[i];
            if(i != tvmMap.second.size() - 1)
            {
                file << "/";
            }
        }
        file << " | " << jtEntry.displayname << " | <ul> ";
        vector<TidlConstraint> constraints = jtEntry.constraints;
        for (auto constraint : constraints)
        {
            if (constraint.onnxComment.size() == 0)
            {
                continue;
            }
            file << "<li> " << constraint.onnxComment << " </li>";
        }
        file << "</ul> | " << endl;
        index++;
    }

    file << endl << "</center>" << endl;
    file.close();    
}

void tfliteConstraints()
{
    ofstream file("tflite_operators.md");
    file << "# TfLite Operator Mappings:" << endl << endl;
    file << "<center>" << endl << endl;
    file << "| S. No. | TfLite Operator | TIDL Layer | Constraints |" << endl;
    file << "|:------:|:----------------|:-----------|:------|" << endl;

    int index = 1;
    for(auto tfliteMap: tfLiteMappings)
    {
        TidlConstraintTableEntry jtEntry = tidlConstraintTable.find(tfliteMap.first)->second;
        if(tfliteMap.second.size() == 0)
        {
            /* No mapping from tflite node to this layer in parsing */
            continue;
        }
        file << "| " << to_string(index) << " | ";
        for (int i = 0; i < tfliteMap.second.size(); i++)
        {
            file << tfliteMap.second[i];
            if(i != tfliteMap.second.size() - 1)
            {
                file << "/";
            }
        }
        file << " | " << jtEntry.displayname << " | <ul> ";
        vector<TidlConstraint> constraints = jtEntry.constraints;
        for (auto constraint : constraints)
        {
            if (constraint.tfliteComment.size() == 0)
            {
                continue;
            }
            file << "<li> " << constraint.tfliteComment << " </li>";
        }
        file << "</ul> | " << endl;
        index++;
    }

    file << endl << "</center>" << endl;
    file.close();    
}