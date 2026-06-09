/*
 *  Copyright (C) 2024-2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ELF_U_PARSER_H_
#define __ELF_U_PARSER_H_

/* tielfup or TI ELF Micro Parser */

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <drivers/bootloader/bootloader_elf.h>

/* ========================================================================== */
/*                             Macros & Typedefs                              */
/* ========================================================================== */

#define ELFUP_PARSER_STATE_INIT     (0U)
#define ELFUP_PARSER_STATE_IDEN0    (1U)
#define ELFUP_PARSER_STATE_IDEN1    (2U)
#define ELFUP_PARSER_STATE_IDEN2    (3U)
#define ELFUP_PARSER_STATE_IDEN3    (4U)
#define ELFUP_PARSER_STATE_PHT      (5U)
#define ELFUP_PARSER_STATE_END      (6U)
#define ELFUP_PARSER_STATE_ERROR    (7U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef union ELFUP_ELFH32_u_t
{
    Bootloader_ELFH32 ELFH;
    uint8_t buff[sizeof(Bootloader_ELFH32)];
} ELFUP_ELFH;

typedef union ELFUP_ELFPH32_u_t
{
    Bootloader_ELFPH32 ELFPH;
    uint8_t buff[sizeof(Bootloader_ELFPH32)];
} ELFUP_ELFPH;

typedef struct ELFUP_State_s_t
{
    uint32_t stateNext;
    uint32_t stateNow;
    uint32_t statePrev;
    uint32_t elfFileStartOffset;
    uint32_t genericOffsetCounter;
    uint8_t maxPhtSize;
    uint8_t headerCnt;
    ELFUP_ELFH ELFHeader;
    ELFUP_ELFPH *pht;
}ELFUP_Handle;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * @brief Initilizer the ELF parser state machine
 *
 * Internally this parser works based on mili-State machine design and works
 * on each character of an ELF file. This design is made so that the ELF
 * file which is being recieved over some interface (CAN/Ethernet/etc.) can be
 * parsed on the fly, since entire file is not available at a time. Other design
 * constraint is that this should not do any dynamic allocation. Since the size of
 * program header table  is not known before hand, therefore, applcaition should
 * allocate some space where program header table can be stored and this is done
 * by passing that via pPhtArr and pPhtArrSize.
 *
 * @param handle pointer to handle of parser
 * @param pPhtArr pointer to an array of ELFUP_ELFPH
 * @param pPhtArrSize size of pPhtArr
 * @return int16_t System_SUCCSESS if all okay
 */
int16_t ELFUP_init(ELFUP_Handle *handle, ELFUP_ELFPH *pPhtArr, uint8_t pPhtArrSize);

/**
 * @brief Update the parser with new data 
 * 
 * There are 2 ways this function returns an error. 1st is 
 * when wrong arguments are passed that is when this function will 
 * return SystemP_FAILURE. Other error is the failure to parse in case 
 * the ELF file is corrupted or is incorrect. This error is stored in 
 * handle.stateNext. In case of failture to parse, handle.stateNext
 * will have value of ELFUP_PARSER_STATE_ERROR.
 * 
 * @param handle pointer to handle of parser
 * @param byte byte of new data
 * @return int16_t SystemP_FAILURE if there was any failure of API
 */
int16_t ELFUP_update(ELFUP_Handle *handle, uint8_t byte);

/**
 * @brief Check if a byte is part of program segment or not.
 * 
 * @param handle pointer to handle of parser
 * @param offset offset of the byte whose info is required.
 * @param phtInfo pointer to memory area where the header info will be written.
 * @return int16_t SystemP_FAILURE if there was any failure of API
 */
int16_t ELFUP_isPartOfSegment(ELFUP_Handle *handle, uint32_t offset, ELFUP_ELFPH *phtInfo);

#ifdef __cplusplus
}
#endif

#endif /* __ELF_U_PARSER_H_ */