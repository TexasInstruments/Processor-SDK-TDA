/*
* Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "tidlruntime_compile.h"
#include <cstring>

TIDLRT_CompileSession::TIDLRT_CompileSession(const std::map<std::string, std::string> &sessOptions)
{
#ifdef BUILD_COMPILE
	int32_t status = 0;
    m_session_created = 0;
	m_tidl_tools_path = "";

	std::map<std::string, std::string> options = trimStringMap(sessOptions);

	// Load libraries
	if(options.find("tidlToolsPath") != options.end())
    {
        m_tidl_tools_path = options.at("tidlToolsPath");
    }
    else if(options.find("tidl_tools_path") != options.end())
    {
        m_tidl_tools_path = options.at("tidl_tools_path");
    }
	else
	{
		char *env = getenv("TIDL_TOOLS_PATH");
		if (env != NULL)
		{
			m_tidl_tools_path.assign(env);
		}
	}

	if (m_tidl_tools_path == "")
	{
		printf("[TIDLRT_CompileSession] ERROR - tidl_tools_path is required. Either provide as option 'tidl_tools_path' or set 'TIDL_TOOLS_PATH' env variable\n");
        return;
	}

	std::string  lib_path = m_tidl_tools_path + "/tidl_model_import.so";
	m_import_ops.lib = dlopen(lib_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if(!m_import_ops.lib)
    {
        printf("[TIDLRT_CompileSession] ERROR - %s\n", dlerror());
        return;
    }

	// Load function from library
	m_import_ops.TIDL_createImportSession = reinterpret_cast<decltype(m_import_ops.TIDL_createImportSession)>(dlsym(m_import_ops.lib, "TIDL_createImportSession"));
	m_import_ops.TIDL_runImportSession = reinterpret_cast<decltype(m_import_ops.TIDL_runImportSession)>(dlsym(m_import_ops.lib, "TIDL_runImportSession"));
	m_import_ops.setDefaultParams = reinterpret_cast<decltype(m_import_ops.setDefaultParams)>(dlsym(m_import_ops.lib, "setDefaultParams"));

	if(m_import_ops.TIDL_createImportSession == nullptr ||
	   m_import_ops.TIDL_runImportSession == nullptr ||
	   m_import_ops.setDefaultParams == nullptr)
	{
		printf("[TIDLRT_CompileSession] ERROR - %s\n", dlerror());
        return;
	}

	// Parse options into params
	m_import_ops.setDefaultParams(&m_compile_option.m_options);
	m_compile_option.m_options.numCores = -1;
	strcpy((char *)m_compile_option.m_options.inData, "");
	strcpy((char *)m_compile_option.m_options.quantParamsPrototxtFile, "");

	status = m_compile_option.TIDLRT_CompileSession_Options_Parse(options);
	if (status == -1)
	{
		printf("[TIDLRT_CompileSession] ERROR - Invalid options provided\n");
		return;
	}
	//set inElementType back to -1 if not parsed
	for(auto &entry : m_compile_option.m_optionMap)
	{
		if(m_compile_option.m_options.modelType == 2 )
		{
			if(entry.key.first == "inElementType")
			{
				if(entry.parseType == -1)
				{
					for (int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++) 
					{
						m_compile_option.m_options.inElementType[i] = -1;
					}
				}
			}
		}
	}

	// Initialize import session with options and validate the params
	status = m_import_ops.TIDL_createImportSession(&m_import_state, &m_compile_option.m_options);
	if (status == 0)
    {
        m_session_created = 1;
    }
    else
    {
        printf("[TIDLRT_CompileSession] ERROR - Failed to create compilation session, status=%d\n", status);
    }
#else
	printf("[TIDLRT_CompileSession] Compilation APIs are not built\n");
#endif

	return;
}

#ifdef BUILD_COMPILE
int32_t TIDLRT_CompileSession::TIDLRT_CompileSession_Run(calibTensor *in[] , size_t size)
{
	int32_t status = 0;

	if(m_session_created == 1)
	{
		status = m_import_ops.TIDL_runImportSession(&m_import_state , in, size);
	}
	else
	{
		status = -1;
	}

    return status;
}
#endif

int32_t TIDLRT_CompileSession::TIDLRT_CompileSession_Delete()
{
    int32_t status = 0;
    return status;
}

void TIDLRT_CompileSession::TIDLRT_CompileSession_DumpOptions()
{
#ifdef BUILD_COMPILE
	m_compile_option.TIDLRT_CompileSession_Options_Dump();
#else
	printf("[TIDLRT_CompileSession] Compilation APIs are not built\n");
#endif
}

TIDLRT_CompileSession::~TIDLRT_CompileSession()
{
    TIDLRT_CompileSession_Delete();
}
