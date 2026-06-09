/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter owns
 * or controls to make, have made, use, import, offer to sell and sell
 * ("Utilize") this software subject to the terms herein.  With respect to the
 * foregoing patent license, such license is granted solely to the extent that
 * any such patent is necessary to Utilize the software alone. The patent
 * license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI
 * Devices"). No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this
 * license (including the above copyright notice and the disclaimer and (if
 * applicable) source code license limitations below) in the documentation
 * and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted
 * provided that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this
 * software is permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI
 * Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the
 * software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are
 * met:
 *
 * *       any redistribution and use of the source code, including any
 * resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the
 * source code and any resulting derivative works, are licensed by TI for use
 * only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its
 * suppliers
 *
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "ctools_wrapper.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h> // For usleep
#include <utils/console_io/include/app_log.h>

typedef enum
{
    FILTER_UINT32,
    FILTER_UINT16,
    FILTER_UINT8,
    FILTER_BOOL
} FilterType;

typedef enum
{
    PROMPT_PROBE,
    PROMPT_ROUTE_ID
} PromptType;

typedef struct
{
    const char*   prompt;
    void*         field;
    FilterType    type;
    unsigned long defaultValue;
    int           isHex;
} FilterInput;

static const char* getBoardSpecificPrompt(PromptType type);
static unsigned long
getUlongWithDefault(const char* prompt, unsigned long defaultValue, int isHex);

/*
 * As the application is about getting the DDR bandwidth, we need to configure
 * the VBUSM.C probes. So, we will configure OP Code here, instead of direction
 * (DIR) filter. Note: Trace data may be unpredictable if filters are not
 * configured correctly.
 */
void runCtoolsDdrBw()
{
    cpt2_options_t cpt2Options = g_defaultCpt2Options;
    cpt2pb_id_t    cpt2ProbeId;
    int            probeIndex = -1;
    char           probeInput[16];
    uint32_t       bufferSize;
    uint32_t*      pBuffer;
    unsigned long  timePeriod;

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }

    appLogPrintf(
        "[CTOOLS] Enter filter values of probe configuration. If you press "
        "just enter to the filters, default values will be used.\n\n");
    appLogPrintf("%s", getBoardSpecificPrompt(PROMPT_PROBE));
    fgets(probeInput, sizeof(probeInput), stdin);
    probeInput[strcspn(probeInput, "\n")] = 0;
    if (strlen(probeInput) == 0)
    {
        cpt2ProbeId = eCpTracer2_DDR_Probe;
    }
    else
    {
        if (sscanf(probeInput, "%d", &probeIndex) == 1 && probeIndex >= 0 &&
            probeIndex < NUM_CPT2_PROBES)
        {
            cpt2ProbeId = g_cpt2_table[probeIndex].probe_id;
        }
        else
        {
            appLogPrintf(
                "Invalid probe number, using default (EMIF0 Probe).\n");
            cpt2ProbeId =
                eCpTracer2_DDR_Probe; /* Selects EMIF0 as default probe */
        }
    }

    appLogPrintf(
        "[Info] For EMIF probe selection, the maximum TBR buffer size "
        "is 65536 bytes. If you set a value less than this, only that "
        "much data will be captured; if you set a higher value, only "
        "65536 bytes will be returned.\n");
    bufferSize =
        getUlongWithDefault("Enter buffer size (in bytes): ", 65536, 0);
    pBuffer = (uint32_t*)malloc(bufferSize);
    if (pBuffer == NULL)
    {
        appLogPrintf("Cannot allocate %u byte buffer to read TBR\n",
                     bufferSize);
        return;
    }

    timePeriod = getUlongWithDefault("Enter time to run (min): ", 1, 0);

    FilterInput filterInputs[] = {
        {"Enter sampling period (hexadecimal value, Valid Range: 0x00001000 - "
         "0x00FFFFFF, Default Value = 0x00FFFFFF): ",
         &cpt2Options.sampling_period, FILTER_UINT32,
         g_defaultCpt2Options.sampling_period, 1},
        {"Enter mode (0=LATENCY, 1=THROUGHPUT, 2=TRANSACTION, "
         "Default=THROUGHPUT): ",
         &cpt2Options.mode, FILTER_UINT32, 1, 1},
        {getBoardSpecificPrompt(PROMPT_ROUTE_ID), &cpt2Options.filt_route_id,
         FILTER_UINT16, g_defaultCpt2Options.filt_route_id, 1},
        {"Enter route id mask (hexadecimal value, 16 bits): ",
         &cpt2Options.filt_route_id_mask, FILTER_UINT16,
         g_defaultCpt2Options.filt_route_id_mask, 1},
        {"Enter OP code (hexadecimal value, 8 bits, Read opcodes: 0x00-0x07, "
         "Write opcodes: 0x08-0x0E): ",
         &cpt2Options.filt_opcode, FILTER_UINT8,
         g_defaultCpt2Options.filt_opcode, 1},
        {"Enter OP code mask (hexadecimal value, 8 bits, Default = 0x0, "
         "Bits[5:3] must be set to 111b for VBUSM.C probes for "
         "Throughput/Latency Statistics): ",
         &cpt2Options.filt_opcode_mask, FILTER_UINT8,
         g_defaultCpt2Options.filt_opcode_mask, 1},
        {"Enter exclude address range (0=No, 1=Yes, Default = 0): ",
         &cpt2Options.filt_addr_low_range_exclude, FILTER_BOOL,
         g_defaultCpt2Options.filt_addr_low_range_exclude, 0},
        {"Enter filter address low LSB (hexadecimal value, 32 bits, Default "
         "Value = 0x0): ",
         &cpt2Options.filt_addr_low_lsb, FILTER_UINT32,
         g_defaultCpt2Options.filt_addr_low_lsb, 1},
        {"Enter filter address low MSB (hexadecimal value, 16 bits, Default = "
         "0x0): ",
         &cpt2Options.filt_addr_low_msb, FILTER_UINT16,
         g_defaultCpt2Options.filt_addr_low_msb, 1},
        {"Enter filter address high LSB (hexadecimal value, 32 bits, Default "
         "Value = 0xFFFFFFFF): ",
         &cpt2Options.filt_addr_high_lsb, FILTER_UINT32,
         g_defaultCpt2Options.filt_addr_high_lsb, 1},
        {"Enter filter address high MSB (hexadecimal value, 16 bits, Default "
         "Value = 0xFFFF): ",
         &cpt2Options.filt_addr_high_msb, FILTER_UINT16,
         g_defaultCpt2Options.filt_addr_high_msb, 1},
        // Add more filters here as needed
    };
    int numFilters = sizeof(filterInputs) / sizeof(filterInputs[0]);

    for (int filterIdx = 0; filterIdx < numFilters; ++filterIdx)
    {
        unsigned long value =
            getUlongWithDefault(filterInputs[filterIdx].prompt,
                                filterInputs[filterIdx].defaultValue,
                                filterInputs[filterIdx].isHex);
        switch (filterInputs[filterIdx].type)
        {
        case FILTER_UINT32:
            *(uint32_t*)filterInputs[filterIdx].field = (uint32_t)value;
            break;
        case FILTER_UINT16:
            *(uint16_t*)filterInputs[filterIdx].field = (uint16_t)value;
            break;
        case FILTER_UINT8:
            *(uint8_t*)filterInputs[filterIdx].field = (uint8_t)value;
            break;
        case FILTER_BOOL:
            *(int*)filterInputs[filterIdx].field = (value != 0);
            break;
        }
    }

    // Special handling for mode
    switch (cpt2Options.mode)
    {
    case 0:
        cpt2Options.mode = eCPT2_MODE_LATENCY;
        break;
    case 1:
        cpt2Options.mode = eCPT2_MODE_THROUGHPUT;
        break;
    case 2:
        cpt2Options.mode = eCPT2_MODE_TRANSACTION;
        break;
    default:
        cpt2Options.mode = eCPT2_MODE_THROUGHPUT;
        break;
    }

    /*
     * Note: There could be other VBUSM.C probes, but this code only fixes the
     * opcode mask for the EMIF probe (eCpTracer2_DDR_Probe) when the user
     * selects default values. It does not apply this fix for other VBUSM.C
     * probes.
     */
    if (cpt2ProbeId == eCpTracer2_DDR_Probe &&
        (cpt2Options.mode == eCPT2_MODE_THROUGHPUT ||
         cpt2Options.mode == eCPT2_MODE_LATENCY))
    {
        // Bits 5:3 mask is 0x38 (0b00111000)
        if ((cpt2Options.filt_opcode_mask & 0x38) != 0x38)
        {
            appLogPrintf("[Warning] For EMIF probe in throughput/latency "
                         "mode, opcode mask bits [5:3] must be set to 111b. "
                         "Overriding to set these bits.\n");
            cpt2Options.filt_opcode_mask |= 0x38;
        }
    }

    if (cpt2Options.sampling_period < 0x00001000 ||
        cpt2Options.sampling_period > 0x00FFFFFF)
    {
        appLogPrintf("Invalid sampling period. Using default value: 0x%08X\n",
                     g_defaultCpt2Options.sampling_period);
        cpt2Options.sampling_period = g_defaultCpt2Options.sampling_period;
    }

    appCpt2EnableProbe(cpt2Options, cpt2ProbeId);
    // To prevent integer overflow issues
    for (unsigned long i = 0; i < timePeriod; i++) {
    usleep(60 * 1000000); // Sleep for 1 minute at a time
    }
    appCpt2DisableProbe(cpt2Options, pBuffer, bufferSize / 4);
    free(pBuffer);
}

/**
 * @brief Returns a board-specific prompt string for user input.
 *
 * This function provides context-sensitive prompts for different SoCs and
 * prompt types (e.g., probe selection, route ID selection). The prompts
 * include example Route IDs for user reference, but are not exhaustive.
 *
 * @param type  The type of prompt to return (e.g., PROMPT_PROBE,
 * PROMPT_ROUTE_ID).
 * @return      A constant string with the appropriate prompt.
 */
static const char* getBoardSpecificPrompt(PromptType type)
{
    switch (type)
    {
    case PROMPT_PROBE:
#ifdef SOC_J784S4
        return "Enter Probe Number (per device.h in J784S4: EMIF0 = 9, EMIF1 "
               "= 10, EMIF2 = 16, EMIF3 = 15, Default = EMIF0): ";
#elif defined(SOC_J742S2)
        return "Enter Probe Number (per device.h in J742S2: EMIF0 = 2, EMIF1 "
               "= 3, Default = EMIF0): ";
#elif defined(SOC_J721S2)
        return "Enter Probe Number (per device.h in J721S2: EMIF0 = 9, EMIF1 "
               "= 10, Default = EMIF0): ";
#elif defined(SOC_J721E)
        return "Enter Probe Number (per device.h in J721E: EMIF0 = 10, "
               "Default = EMIF0): ";
#elif defined(SOC_J722S)
        return "Enter Probe Number (per device.h in J722S: DDR NRT = 4, DDR RT = "
               "3, Default = DDR NRT): ";
#else
        return "Enter Probe Number (see device.h for your SoC): ";
#endif

    case PROMPT_ROUTE_ID:
#ifdef SOC_J784S4
        return "[Info] Example J784S4 Route IDs: Use route id = "
               "0x20/0x24/0x28/0x2C "
               "(for C7-0/1/2/3) with mask = 0xFFC to track one specific C7x core; "
               "for ALL C7-x cores, use route id = 0x20 with mask = 0xFF0.\n"
               "Enter route id (hexadecimal value, Valid Range: 0 to 0xFFF): ";
#elif defined(SOC_J742S2)
        return "[Info] Example J742S2 Route IDs: Use route id = "
               "0x20/0x24/0x28 "
               "(for C7-0/1/2) with mask = 0xFFC to track one specific C7x core; "
               "for ALL C7-x cores, use route id = 0x20 with mask = 0xFF0.\n"
               "Enter route id (hexadecimal value, Valid Range: 0 to 0xFFF): ";
#elif defined(SOC_J721S2)
        return "[Info] Example J721S2 Route IDs: C7x-0=0x00C, C7x-1=0x015, "
               "A72-CORE0=0x000, A72-CORE1=0x001, DSS0_DMA=0xA20, "
               "DSS0_FBDC=0xA21.\nEnter route id (hexadecimal value, Valid "
               "Range: 0 to 0xFFF): ";
#elif defined(SOC_J721E)
        return "[Info] Example J721E Route IDs: C7x-CORE0=0x008, "
               "C7x-Cache=0x00C, "
               "C66SS0_MDMA=0x240, C66SS0_CFG=0x241, C66SS1_MDMA=0x242, "
               "C66SS1_CFG=0x243, DSS0_DMA=0xA20, DSS0_FBDC=0xA21.\nEnter "
               "route id (hexadecimal value, Valid Range: 0 to 0xFFF): ";
#elif defined(SOC_J722S)
        return "[Info] Example J722S Route IDs: Use route id = "
               "0x20/0x30 "
               "(for C7-0/1) with mask = 0xFF8 to track one specific C7x core; "
               "for ALL C7-x cores, use route id = 0x20 with mask = 0xFE8.\n"
               "Enter route id (hexadecimal value, Valid Range: 0 to 0xFFF): ";
#else
        return "Enter route id (hexadecimal value): ";
#endif
    default:
        return "";
    }
}

/**
 * @brief Prompts the user for an unsigned long input value, with support for a
 * default value and optional hexadecimal input.
 *
 * This function displays a prompt to the user and reads input from stdin. If
 * the user provides a valid value, it is parsed as either hexadecimal or
 * decimal based on the isHex flag. If the input is empty or invalid, the
 * specified default value is returned.
 *
 * @param prompt        The prompt message to display to the user.
 * @param defaultValue  The value to return if the user input is empty or
 * invalid.
 * @param isHex         If non-zero, input is parsed as hexadecimal; otherwise,
 * as decimal.
 *
 * @return The user-provided value if valid, otherwise the default value.
 */
static unsigned long getUlongWithDefault(const char*   prompt,
                                         unsigned long defaultValue, int isHex)
{
    char input[32];
    appLogPrintf("%s", prompt);
    if (fgets(input, sizeof(input), stdin))
    {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0)
            return defaultValue;
        int           result;
        unsigned long value = defaultValue;
        if (isHex)
            result = sscanf(input, "%lx", &value);
        else
            result = sscanf(input, "%lu", &value);
        if (result != 1)
        {
            appLogPrintf("Invalid input. Using default value!");
            return defaultValue;
        }
        return value;
    }
    return defaultValue;
}