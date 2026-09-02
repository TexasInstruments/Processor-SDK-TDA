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

#ifndef TIDL_IMPORT_DIAG_H
#define TIDL_IMPORT_DIAG_H

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <list>
#include <utility>
#include <mutex>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>

#define TIDL_IMPORT_DIAGNOSIS_RETURN_OK (0)
#define TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL (-1)
#define TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL (1)

/* Handy macros */
// #define __MODULE__ TIDL_ImportDiagnostics::getModuleFromFile(__FILE__).c_str()
#define __MODULE__ ""

/*
*  This class represents a warning or error diagnostic that applies to a model:
*  unsupported layer, non-performant, etc. It can apply to a single layer
*  or the whole model.
*/

class TIDL_ModelDiagnostic
{
public:
  enum Kind
  {
    DK_Print,         // For normal print messages without any specific purpose or else use any of the below
    DK_PrintHeading,  // For heading prints
    DK_PrintSubheading,
    DK_Supported,     // Fully supported
    DK_Info,          // Fully supported, informational message
    DK_NotPerformant, // Supported, but may not perform well
    DK_NotVerified,   // Supported, but not verified
    DK_Error,         // Error in import process
    DK_NotSupported,  // Unsupported layer (error)
  };

  enum Color
  {
    DK_NoColor,
    DK_Red,
    DK_Green,
    DK_Yellow,
    DK_Magenta,
    DK_Cyan,
    DK_Purple,
    DK_Blue,
  };

  /* Construct a diagnostic, with printf-style formatting */
  TIDL_ModelDiagnostic(Kind k, Color c, const char* fmt, ...);
  /* Emit diagnostic to stdout */
  void emit(FILE* fptr) const;
  /* get kind of diagnosis */
  int32_t getKind() const;
  /* get color of diagnosis */
  int32_t getColor() const;
  /* True if diagnostic represents a fatal error (model will not run on TIDL) */
  bool isError() const;
  /* True if node should be whitelisted (part of subgraph offloaded to TIDL) */
  bool isOKToOffload() const;
  /* return diagnostic string collected */
  std::string getString() const;
private:
  Kind kind;
  Color color;
  std::string msg;
  int32_t logfd;
};

using DiagList_t = std::vector<TIDL_ModelDiagnostic>;

/* 
A global instance of this class manages diagnostics (errors and warnings)
that occur during the import process. Use this class to report and
keep track of errors.
Some guidelines:
  * Try to avoid using abort(). When invoked as an API (from an external
    framework, e.g. TVM, the import tool should return if possible.
  * Errors pertaining to import tool usage (parameters, missing files, etc)
    should be reported using error() or warning().
  * Errors pertaining to the input model (unsupported layers, etc) should
    be reported using model_diag().

Diagnostic message guidelines:
  * Describe things in terms that make sense to users. Avoid abbreviations and internal names.
  * Start messages with a capital letter, unless the first word is an identifier
  * Avoid ALL CAPS and EXCLAMATION POINTS!!!!! Errors are not that exciting.
  * Single-sentence messages should not end with a period
  * Please double-check the grammar and spelling. Users are expected to see these messages.
*/
class TIDL_ImportDiagnostics {
  static constexpr const char *sigil = "[IMPORT]";
  int errors = 0;
  int warnings = 0;
  std::mutex diag_mutex;
  const char* filename = 0;
  FILE* logPtr = stdout;
public:
  static DiagList_t gDiagList;
  /* Report an error that prevents successful import */
  template<typename ...Args>
  void error(Args&&... args) {
    std::lock_guard<std::mutex> guard(diag_mutex);
    print(31,0,std::forward<Args>(args)...);
    ++errors;
  }
  /* Report an warning. Successful import and execution of the model still possible. */
  template<typename ...Args>
  void warning(Args&&... args) {
    std::lock_guard<std::mutex> guard(diag_mutex);
    print(33,0,std::forward<Args>(args)...);
    ++warnings;
  }
  /* Report a fatal error that makes it impossible to continue (Exits the program.) */
  template<typename ...Args>
  void abort(Args&&... args) {
    print(31,1,std::forward<Args>(args)...);
    if(this->logPtr == stdout)
      fprintf(this->logPtr, "\033[31;1m%s Aborting\033[0m\n", sigil);
    else
      fprintf(this->logPtr, "%s Aborting\n", sigil);
    std::quick_exit(1);
  }

  /* Report an error on the input model, using a TIDL_ModelDiagnostic construction. */
  template<typename ...Args>
  void modelDiag(Args&& ... args) {
    std::lock_guard<std::mutex> guard(diag_mutex);
    postModelDiag(TIDL_ModelDiagnostic(std::forward<Args>(args)...));
  }
  /* Post and report an already-constructed TIDL_ModelDiagnostic. */
  void modelDiag(const TIDL_ModelDiagnostic &d) {
    postModelDiag(d);
  }

  void reportLastModeDiag() {
    modelDiag(gDiagList.back());
  }

  /* Display a summary report of all errors warnings encountered */
  void report() const {
    const char *elabel = errors == 1 ? "error" : "errors";
    const char *wlabel = warnings == 1 ? "warning" : "warnings";
    fprintf(this->logPtr, "%s %d %s, %d %s\n", sigil, errors, elabel, warnings, wlabel);
  }

  /* Return true if there are any errors that indicate the model was not successfully imported. */
  bool hasErrors() const {
    return errors > 0;
  }

  /* Get Module name from filename */
  static std::string getModuleFromFile(std::string);

  /* Remove ANSI Character from string */
  static std::string removeAnsi(std::string str);

  /* set logging file */
  bool setLogFile(const char* filename) {
    this->filename = filename;
    this->logPtr = std::fopen(filename, "w");
    if (this->logPtr == 0)
    {
      this->logPtr = stdout;
      warning("%s WARNING: Could not open %s for writing logs, double check the file permissions. Writing logs to stdout\n", __MODULE__, this->filename);
      return false;
    }
    fprintf(stdout, "Redirecting output to %s\n", this->filename);
    fprintf(this->logPtr, "Opening log file %s\n", this->filename);
    return true;
  }
  /* close log file and free file ptr */
  void freeFile()
  {
    if (logPtr != stdout && logPtr != NULL)
    {
      fprintf(this->logPtr, "Closing log file %s\n", this->filename);
      fclose(logPtr);
      logPtr = stdout;
    }
  }
  /* Destructor */
  ~TIDL_ImportDiagnostics()
  {
    freeFile();
  }
private:
  /* Given a model diagostic, report and tally it */
  void postModelDiag(const TIDL_ModelDiagnostic &d) {
    // std::lock_guard<std::mutex> guard(diag_mutex);   This apparently creates a deadlock, Why?
    int color=0;
    int font=0;

    // Set Font Color
    if(this->logPtr != stdout){
      color=0;  /* no coloring if writing logs to a file */
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Red){
      color = 31;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Magenta){
      color = 35;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_NoColor){
      color = 0;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Yellow){
      color = 33;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Blue){
      color = 34;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Purple){
      color = 35;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Green){
      color = 32;
    }else if(d.getColor()==TIDL_ModelDiagnostic::DK_Cyan){
      color = 36;
    }

    // Set Font Weight
    if(this->logPtr != stdout){
      font=0;  /* no weight if writing logs to a file */
    }else if(d.getKind()==TIDL_ModelDiagnostic::DK_NotSupported ||
             d.getKind()==TIDL_ModelDiagnostic::DK_Info ||
             d.getKind()==TIDL_ModelDiagnostic::DK_NotPerformant ||
             d.getKind()==TIDL_ModelDiagnostic::DK_PrintHeading ||
             d.getKind()==TIDL_ModelDiagnostic::DK_PrintSubheading){
      font = 1;
    }

    if(color != 0){
      if(font != 0)
        fprintf(this->logPtr, "\033[%d;%dm", color, font);
      else
        fprintf(this->logPtr, "\033[%dm", color);
    }

    if(d.getKind()!=TIDL_ModelDiagnostic::DK_Print && d.getKind()!=TIDL_ModelDiagnostic::DK_PrintHeading && d.getKind()!=TIDL_ModelDiagnostic::DK_PrintSubheading)
      fprintf(this->logPtr, "%s", sigil);
    d.emit(this->logPtr);

    if(color != 0){
      fprintf(this->logPtr, "\033[0m");
    }
    if (d.isError()) ++errors;
    else ++warnings;
  }
  /* Format and report an error message */
  void print(int color, int font, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if(this->logPtr != stdout){
      color = 0;
    }
    if(color != 0){
      if(font)
        fprintf(this->logPtr, "\033[%d;%dm", color, font);
      else
        fprintf(this->logPtr, "\033[%dm", color);
    }
    fprintf(this->logPtr, "%s", sigil);
    vfprintf(this->logPtr, fmt, args);

    if(color != 0){
      fprintf(this->logPtr, "\033[0m\n");
    }else{
      fprintf(this->logPtr, "\n");
    }
  }
};

/* Global diagnostics manager */
extern TIDL_ImportDiagnostics gDiags;

/* push log reports to a DiagList_t variable: */
/* Macro for unsupported layers/inputs/parameters */
#define TIDL_LOG_UNSUPPORTED(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_NotSupported, TIDL_ModelDiagnostic::DK_Magenta, "%s UNSUPPORTED: " Message " -- [%s, %d]", __MODULE__,  ##__VA_ARGS__, __FILE__, __LINE__);\
}

#define TIDL_LOG_UNSUPPORTED_AT(diags, index, Message, ...) {\
    TIDL_ModelDiagnostic newDiag(TIDL_ModelDiagnostic::DK_NotSupported, TIDL_ModelDiagnostic::DK_Magenta, "%s UNSUPPORTED: " Message " -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__); \
    if (index >= 0 && index < diags.size()) {\
        diags[index] = newDiag;\
    } else { \
        diags.emplace_back(newDiag); \
    } \
  }

/* push log reports to a DiagList_t variable: */
/* Macro for supported layers/inputs/parameters */
#define TIDL_LOG_SUPPORTED(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_Supported, TIDL_ModelDiagnostic::DK_NoColor, "%s SUPPORTED: " Message " -- [%s, %d]", __MODULE__,  ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Macro for generic error */
#define TIDL_LOG_ERROR(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_Error, TIDL_ModelDiagnostic::DK_Red, "%s ERROR: " Message" -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Macro for supported but not verified */
#define TIDL_LOG_NOT_VERIFIED(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_NotVerified, TIDL_ModelDiagnostic::DK_Yellow, "%s WARNING: " Message" -- [%s, %d]", __MODULE__,  ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Macro to be used in case of supported but may not perform well */
#define TIDL_LOG_NOT_PERFORMANT(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_NotPerformant, TIDL_ModelDiagnostic::DK_Blue, "%s SUGGESTION: " Message" -- [%s, %d]", __MODULE__,  ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Macro for additional information */
#define TIDL_LOG_INFO(diags, Message, ...) {\
diags.emplace_back(TIDL_ModelDiagnostic::DK_Info, TIDL_ModelDiagnostic::DK_Blue, "%s INFORMATION: " Message" -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* report directly in gDiags as unsupported: */
/* Macro to be used in case of unsupported layers/inputs/parameters */
#define TIDL_GLOBAL_REPORT_UNSUPPORTED(Message, ...) {\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_NotSupported, TIDL_ModelDiagnostic::DK_Magenta, "%s UNSUPPORTED: " Message" -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* For fully supported */
#define TIDL_GLOBAL_REPORT_SUPPORTED(Message, ...) {\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_Supported, TIDL_ModelDiagnostic::DK_NoColor, "%s SUPPORTED: " Message, __MODULE__, ##__VA_ARGS__);\
}

/* Macro to be used in case of supported but may not perform well */
#define TIDL_GLOBAL_REPORT_NOT_PERFORMANT(Message, ...) {\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_NotPerformant, TIDL_ModelDiagnostic::DK_Blue, "%s SUGGESTION: " Message, __MODULE__, ##__VA_ARGS__);\
}

/* Macro to print additional information */
#define TIDL_GLOBAL_REPORT_INFO(debugLevel, Message, ...) {\
if((debugLevel)>0)\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_Print, TIDL_ModelDiagnostic::DK_NoColor, Message, ##__VA_ARGS__);\
}

/* Macro to print headings */
#define TIDL_GLOBAL_REPORT_HEADING(debugLevel, Color, Message, ...) {\
if((debugLevel)>0)\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_PrintHeading, Color, Message, ##__VA_ARGS__);\
}

/* Macro to print subheadings */
#define TIDL_GLOBAL_REPORT_SUBHEADING(debugLevel, Color, Message, ...) {\
if((debugLevel)>0)\
gDiags.modelDiag(TIDL_ModelDiagnostic::DK_PrintSubheading, Color, Message, ##__VA_ARGS__);\
}

/* Macro to report an error that prevents successful import */
#define TIDL_GLOBAL_REPORT_ERROR(Message, ...) {\
gDiags.error("%s ERROR: " Message" -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Macro to report any type of warning - Successful import and execution of the model still possible.*/
#define TIDL_GLOBAL_REPORT_WARNING(Message, ...) {\
gDiags.warning("%s WARNING: " Message"", __MODULE__, ##__VA_ARGS__);\
}

/* Macro to report Fatal error (error and exit the program) */
#define TIDL_GLOBAL_REPORT_FATAL_AND_ABORT(Message, ...) {\
gDiags.abort("%s FATAL ERROR: " Message" -- [%s, %d]", __MODULE__, ##__VA_ARGS__, __FILE__, __LINE__);\
}

/* Wrapper for functions which return 0 for success & -1 for failure and want to exit from program if it is failure */
#define TIDL_IMPORT_CHECK_AND_ABORT(status, Message, ...){            \
  if((status)==TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL){                    \
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT(Message, ##__VA_ARGS__);       \
  }                                                                   \
}
/* Wrapper for functions which return 0 for success & -1 for failure and want to propagate up if it is failure */
#define TIDL_IMPORT_CHECK_AND_RETURN(status, Message, ...){                                                 \
  if((status)==TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL){                                                          \
    TIDL_GLOBAL_REPORT_ERROR(Message " - Failed in function: %s", ##__VA_ARGS__, __FUNCTION__);             \
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;                                                               \
  }                                                                                                         \
}

typedef enum {
    ALIGN_LEFT = 0,
    ALIGN_RIGHT,
    ALIGN_CENTER
} TIDL_table_align_t;

/* Helper function to create table on a given stream (ex: stringstream, cout, fout) */
void TIDL_createTable(std::ostream &stream, std::vector<std::string> header, std::vector<std::vector<std::string>> data, int32_t padding = 0, std::vector<TIDL_table_align_t> columnAlignment = {}, bool printSeperator = false);

/** Prints the version table */
void TIDL_printVersionTable(std::string tidlToolsVer, std::string c7xFirmwareVer, std::string runtimeVer = "", std::string opSetVer = "");

/** Prints the model compilation stages*/
void TIDL_printCompilationStages();

/** Print an in-place progress bar*/
void TIDL_printProgressBar(float progress);

#endif /* TIDL_IMPORT_DIAG_H */
