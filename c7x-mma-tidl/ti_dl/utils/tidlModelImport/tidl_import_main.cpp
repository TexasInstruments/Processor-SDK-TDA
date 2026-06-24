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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <float.h>
#ifdef _WIN32
#include <asprintf.h>
#endif

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "string"
#include "tidl_import_diag.h"
#include "tidl_import_session.h"

#define __MODULE__ ""

struct deferredOption {
  char *opt;
  char **args;
};

void printTokenHelpAndExit(char *appname, char *token)
{
  int c = 0;

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "* %s --help-%s\n", appname, token);
  while(gsTokenMap_tidl_import_config[c].tokenName) {
    if(!strcmp(token, gsTokenMap_tidl_import_config[c].tokenName)) {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "--%-28s : %s\n", token, gsTokenMap_tidl_import_config[c].shortHelp);
      if(gsTokenMap_tidl_import_config[c].mandatory)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "[Mandatory parameter]\n");
      }
      else
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "[Optional parameter]\n");
      }
      if(gsTokenMap_tidl_import_config[c].longHelp)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", gsTokenMap_tidl_import_config[c].longHelp);
      }
      else
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s does not have extended help", token);
      }
      exit(0);
    }
    c++;
  }

  /* Should not reach here */
  exit(0);
}

void printHelpAndExit(char *appname)
{
  int c = 0;

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Usage:");
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "* %s -h", appname);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "* %s --help", appname);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "* %s --help-<parameter>", appname);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "* %s <config-file> [OPTIONS]", appname);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "This tool reads import parameters from the config file.\n\n"
         "A parameter read from config file can be overridden by adding\n"
         "\"--<parameter> <arg0> [<arg1> ...]\" to the command line options\n\n"
         "A parameter can also be added (if not provided by config file)\n"
         "by adding \"--<parameter> <arg0> [<arg1> ...]\" to the command\n"
         "line options\n\n"
         "The list of supported import parameters is given below.\n"
         "You can get details about each parameter by running the application\n"
         "with a --help-<parameter> option.\n\n"
         "For example, to know more about \"modelType\" parameter, run the\n"
         "application as:\n"
         "%s --help-modelType\n", appname);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nMandatory import parameters:\n");
  while(gsTokenMap_tidl_import_config[c].tokenName) {
    if(gsTokenMap_tidl_import_config[c].mandatory)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "--%-28s: %s", gsTokenMap_tidl_import_config[c].tokenName, gsTokenMap_tidl_import_config[c].shortHelp);
    }
    c++;
  }
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Supported optional import parameters:");
  c = 0;
  while(gsTokenMap_tidl_import_config[c].tokenName) {
    if(!gsTokenMap_tidl_import_config[c].mandatory)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "--%-28s: %s", gsTokenMap_tidl_import_config[c].tokenName, gsTokenMap_tidl_import_config[c].shortHelp);
    }
    c++;
  }

  exit(0);
}

bool isOptionAToken(const char *opt)
{
  int c = 0;
  while(gsTokenMap_tidl_import_config[c].tokenName) {
    if(!strcmp(opt, gsTokenMap_tidl_import_config[c].tokenName))
      return true;
    c++;
  }

  return false;
}

bool isOptionATokenHelp(const char *opt, char **ret_token, bool leadingDashes)
{
  int c = 0;
  const char *helpToken = leadingDashes ? "--help-" : "help-";

  while(gsTokenMap_tidl_import_config[c].tokenName) {
    char *token = gsTokenMap_tidl_import_config[c].tokenName;
    if(strstr(opt, token) &&
        strlen(opt) == strlen(helpToken) + strlen(token) &&
        !strncmp(opt, helpToken, strlen(helpToken))) {
      *ret_token = token;
      return true;
    }
    c++;
  }

  return false;
}

/* Convenience function to add a deferredOption to a NULL-terminated list */
struct deferredOption *addDeferredOption(struct deferredOption *d_opts, struct deferredOption *new_d_opt)
{
  int i = 0;

  while(d_opts[i].opt)
  {
    i++;
  }

  /* old Sentinel */
  i++;

  d_opts = (struct deferredOption *)realloc(d_opts, sizeof(struct deferredOption) * (i + 1));

  /* new sentinel */
  memset(&d_opts[i], 0, sizeof(struct deferredOption));
  /* new option */
  d_opts[i - 1].opt = new_d_opt->opt;
  d_opts[i - 1].args = new_d_opt->args;

  return d_opts;
}

/* Convenience function to add an option argumnet to a NULL-terminated list */
char **addArg(char **args, char *new_arg)
{
  int i = 0;

  while(args[i])
  {
    i++;
  }

  /* old Sentinel */
  i++;

  args = (char **)realloc(args, sizeof(char *) * (i + 1));

  /* new sentinel and arg */
  args[i] = NULL;
  args[i - 1] = new_arg;

  return args;
}

/* Convenience function to add an option to a NULL-terminated list */
struct option *addOption(struct option *lopts, char *opt, int flag)
{
  int i = 0;

  while(lopts[i].name)
  {
    i++;
  }

  /* old Sentinel */
  i++;

  lopts = (struct option *)realloc(lopts, sizeof(struct option) * (i + 1));

  /* new sentinel */
  memset(&lopts[i], 0, sizeof(struct option));
  /* new option */
  lopts[i - 1].name = strdup(opt);
  lopts[i - 1].has_arg = flag;
  lopts[i - 1].flag = NULL;
  lopts[i - 1].val = 0;

  return lopts;
}

int32_t parseOptimizationLevel(tidl_import_config& gParams)
{
  if(gParams.graphOptimizationLevel == TIDL_OPTIMIZE_LEVEL_EXTENDED)
  {
    if(gParams.enableShapeFolding == -1)
    {
      gParams.enableShapeFolding = 1;
    }
    // Other higher optimization flag can be added here in future
  }
  else
  {
    if(gParams.enableShapeFolding == -1)
    {
      gParams.enableShapeFolding = 0;
    }
    // Other higher optimization flag can be added here in future
  }
  return 0;
}

int32_t main(int32_t argc, char *argv[])
{
  int32_t c;
  int lindex;
  struct option *lopts;
  struct deferredOption *d_opts;
  char *token = NULL;
  int32_t status = 0;
  int32_t runQuantStep = 0;
  FILE * fp;
  if(argc < 2)
  {
    printHelpAndExit(argv[0]);
  }

  /*Build a list of options that can be parsed */
  lopts = (struct option *)calloc(sizeof(struct option), 1);

  /* Add --help as an option */
  lopts = addOption(lopts, "help", no_argument);

  c = 0;
  while(gsTokenMap_tidl_import_config[c].tokenName)
  {
    char *str;
    lopts = addOption(lopts, gsTokenMap_tidl_import_config[c].tokenName, required_argument);

    asprintf(&str, "help-%s", gsTokenMap_tidl_import_config[c].tokenName);
    lopts = addOption(lopts, str, no_argument);
    free(str);

    c++;
  }

  if(!strcmp(argv[1], "--help"))
    printHelpAndExit(argv[0]);
  else if(!strcmp(argv[1], "-h"))
    printHelpAndExit(argv[0]);
  else if(isOptionATokenHelp((const char *)argv[1], &token, true))
    printTokenHelpAndExit(argv[0], token);

  /* Must be config file if it did not exit */
  fp = fopen(argv[1], "r");
  if(fp== NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open config  file : %s ",argv[1]);
    return(0);
  }
  fclose(fp);
  strcpy(inConfigFilename, argv[1]);

  /*
   * First stage, parse --help, -h and --help-<p>, and build
   * a list of deferred options
   *
   * Do not parse any of the --<p> options yet, as they
   * are supposed to override values later.
   */
  d_opts = (struct deferredOption *)calloc(sizeof(struct deferredOption), 1);
  optind = 2;

  while((c = getopt_long(argc, argv, "h", lopts, &lindex)) != -1)
  {
    const char *lopt = NULL;

    token = NULL;

    switch(c)
    {
      case 0:
        lopt = lopts[lindex].name;

        if(isOptionAToken(lopt)) {
          int k;
          struct deferredOption d_opt;

          d_opt.opt = (char *)lopt;
          d_opt.args = (char **)calloc(sizeof(char *), 1);

          d_opt.args = addArg(d_opt.args, optarg);

          k = 0;
          while(argv[optind + k] && strncmp("-", argv[optind + k], strlen("-"))) {
              d_opt.args = addArg(d_opt.args, argv[optind + k]);
              k++;
          }

          d_opts = addDeferredOption(d_opts, &d_opt);
        }

        else if(isOptionATokenHelp(lopt, &token, false))
          printTokenHelpAndExit(argv[0], token);
        else if(!strcmp(lopt, "help"))
          printHelpAndExit(argv[0]);
        else
          printHelpAndExit(argv[0]);

        break;
      case 'h':
        printHelpAndExit(argv[0]);
      default:
        printHelpAndExit(argv[0]);
    }
  }

  setDefaultParams(&gParams);

  status = readparamfile(argv[1], &gsTokenMap_tidl_import_config[0]) ;

  /* Set log file name */
  if (((const char*)gParams.logFileName)[0] != '\0')
  {
    gDiags.setLogFile((const char*)gParams.logFileName);
  }
  //determine if quantization parameters are to be exported
  string quantParamsPrototxtFile((char*)gParams.quantParamsPrototxtFile);

  if(quantParamsPrototxtFile != "" )
  {
    FILE* fd = fopen((char*)gParams.quantParamsPrototxtFile, "r");
    if(fd == NULL)
    {
      gParams.isQuantParamsToBeExported = 1;
    }
    else
    {
      gParams.isQuantParamsToBeExported = 0;
      fclose(fd);
    }
  }
  // file isnt provided by user, quant params wont be exported
  else
  {
    gParams.isQuantParamsToBeExported = 0;
  }


  if(status == -1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Parser Failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

 sTokenMapping deviceConfigTokenMap[] =
    {
      {(char* )"DEVICE_NAME", &gParams.deviceName, INT_32, SINGLE, false, false, NULL, "NR", NULL},
      {(char* )"NUMCORES", &gParams.numCores, INT_32, SINGLE, false, false, NULL, "NR", NULL},
      {(char* )NULL    , NULL , INT_32, SINGLE, false, false, NULL, NULL, NULL}
    };

  TIDL_IMPORT_CHECK_AND_RETURN(readparamfile((char *)gParams.perfSimConfig, &deviceConfigTokenMap[0]), "Failed to read device config");

  /* read device config before import config so import config can be used to override device config parameters */
  TIDL_IMPORT_CHECK_AND_RETURN(readparamfile(argv[1], &gsTokenMap_tidl_import_config[0]), "Parser Failed");

  /*
   * Second stage, parse deferred options and override
   */
  c = 0;
  while(d_opts[c].opt)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(readparamarg(d_opts[c].opt, &gsTokenMap_tidl_import_config[0], d_opts[c].args), "Parser Failed");
    c++;
  }

  TIDL_IMPORT_CHECK_AND_RETURN(checkMandatoryParams(&gsTokenMap_tidl_import_config[0]), "Missing one or more mandatory parameters");
  
  TIDL_IMPORT_CHECK_AND_RETURN(parseOptimizationLevel(gParams),"");
  
  // Create and run import sessions
  importState *state = (importState *) calloc(1, sizeof(importState));
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_createImportSession(state, NULL), "Failed to create Import session");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runImportSession(state), "Failed to run Import session");
  free((void *)state);

  std::quick_exit(0);
  return 0;
}
