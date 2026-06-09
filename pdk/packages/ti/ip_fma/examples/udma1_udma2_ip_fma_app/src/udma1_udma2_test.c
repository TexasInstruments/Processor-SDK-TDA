/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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

/**
 *  \file udma1_udma2_test.c
 *
 *  \brief UDMA1 and UDMA2 safety mechanism example application. 
 *   
 *   Performing a DMA transfer from source buffer on memory location
 *   0x9555_5555 to destination buffer on memory location 0xAAAA_AAAA, and
 *   back. Then another one from 0x9555_5555 to 0x41CE_3200 and back. This
 *   tests basic UDMA functionalities and all address lines, which covers
 *   UDMA1. Besides that it calculates CRC for the initial source buffer
 *   payload data, embedes it into the buffer, does the mentioned DMA 
 *   transfer and then at the destination side checks the validity of the
 *   transfered data using the CRC. Also checks if overrun happened during
 *   the transfer. This covers UDMA2.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <ti/drv/udma/udma.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/udma/examples/udma_apputils/udma_apputils.h>
#include <ti/csl/csl_crc.h>
#include <string.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * Application test parameters
 */
/** \brief Number of bytes to copy and buffer allocation */
#define UDMA_TEST_APP_NUM_BYTES         (128U)
/** \brief Padding size for testing the overrun */
#define PADDING_SIZE                    (64U)
/** \brief Size of buffers that includes payload and padding */
#define BUFFER_SIZE                     (UDMA_TEST_APP_NUM_BYTES + PADDING_SIZE)

/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much memcpy operations */
#define UDMA_TEST_APP_RING_ENTRIES      (1U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define UDMA_TEST_APP_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * UDMA_TEST_APP_RING_ENTRY_SIZE)
/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_RING_MEM_SIZE_ALIGN ((UDMA_TEST_APP_RING_MEM_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define UDMA_TEST_APP_TRPD_SIZE         ((sizeof(CSL_UdmapTR15) * 2U) + 4U)
/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_TRPD_SIZE_ALIGN   ((UDMA_TEST_APP_TRPD_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/* Size of payload data in buffer, used for CRC values calculation */
#define PAYLOAD_SIZE                    ((uint32_t) 120U)
/* Align the size of the payload buffer data for CRC calculation using UDMA transfer to MCRC block */
#define PAYLOAD_SIZE_ALIGN              ((PAYLOAD_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/* CRC channel parameters */
#define APP_CRC_CHANNEL                 (CRC_CHANNEL_1)
#define APP_CRC_CH_CCITENR_MASK         (CRC_INTR_CH1_CCITENR_MASK)
#define APP_CRC_WATCHDOG_PRELOAD_VAL    ((uint32_t) 0U)
#define APP_CRC_BLOCK_PRELOAD_VAL       ((uint32_t) 0U)

/* CRC size parameters */
#define APP_CRC_PATTERN_SIZE            ((uint32_t) 4U)
#define APP_CRC_SECT_CNT                ((uint32_t) 1U)

/* Use MCU NAVSS/peripherals for MCU domain cores. Rest all uses Main NAVSS */
#define APP_CRC_BASE                    (CSL_MCU_NAVSS0_MCRC_BASE)


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief This function unifies complete DMA transfer test.
 * 
 *  Does the complete job of DMA buffers preparation, transfer execution using
 *  the given UDMA channel, validation and checking, in compliance with UDMA1 
 *  and UDMA2 requirements.
 * 
 *  \param  chHandle     UDMA channel handle.
 * 
 *  \return \ref         Udma_ErrorCodes
 */
static int32_t Udma1Udma2App_MemcpyTest(Udma_ChHandle chHandle);

/**
 * \brief   This function does one single DMA transfer.
 * 
 *  Clears the destination buffers and initializes padding region of it.
 *  Ensures cache coherency by writing back buffers from cache before DMA
 *  and invalidating the cache after it. Actual DMA transfer is performed
 *  by invoking \ref Udma1Udma2App_UdmaMemcpy function using the given
 *  UDMA channel.
 *  
 *  \param  chHandle     UDMA channel handle.
 *  \param  destBuf      Pointer to destination buffer.
 *  \param  srcBuf       Pointer to destination buffer.
 * 
 *  \return \ref         Udma_ErrorCodes
 */                                       
static int32_t Udma1Udma2App_SingleTransfer(Udma_ChHandle chHandle,
                                          uint8_t *destBuf,
                                          uint8_t *srcBuf);

/**
 * \brief   This function actually does the DMA transfer memcpy.
 * 
 *  Invokes the actual DMA transfer by updating TRDP, and submitting it to
 *  channel. The function makes sure the transfer is done by waiting for
 *  return descriptor in completion ring using the semaphor and then popping 
 *  the response from that completion ring. 
 * 
 *  \param  chHandle     UDMA channel handle.
 *  \param  destBuf      Pointer to destination buffer.
 *  \param  srcBuf       Pointer to destination buffer.
 *  \param  length       Transfer size in bytes.
 * 
 *  \return \ref         Udma_ErrorCodes
 */
static int32_t Udma1Udma2App_UdmaMemcpy(Udma_ChHandle chHandle,
                                        void *destBuf,
                                        void *srcBuf,
                                        uint32_t length);

/**
 * \brief   This function calculates the CRC utilizing DMA and MCRC blocks.
 * 
 *  Invokes the DMA transfer from \ref srcBuf to the input of MCRC block
 *  by preparing the TRPD and submitting it to given UDMA channel. By
 *  invoking this transfer CRC signature value is calculated which can be 
 *  obtained using the CRC API.
 * 
 *  \param  chHandle     UDMA channel handle.
 *  \param  srcBuf       Pointer to destination buffer.
 *  \param  length       Transfer size in bytes.
 * 
 *  \return \ref         Udma_ErrorCodes
 */ 
static int32_t Udma1Udma2App_UdmaCrc(Udma_ChHandle chHandle,
                                     void *srcBuf,
                                     uint32_t length);

/**
 * \brief   Function for extraction payload and embbeded CRC from buffer.
 * 
 *  Pointer to buffer \ref buf has a payload and CRC values embedded inside
 *  of it. This function extract them and then calculates new CRC values
 *  based on the payload data. Finally it compares the calculated and 
 *  extracted CRC values and prints appropriate messages.
 * 
 *  \param  chHandle     UDMA channel handle.
 *  \param  buf          Pointer to buffer containing payload and CRC data
 * 
 *  \return \ref         Udma_ErrorCodes
 */ 
static int32_t Udma1Udma2App_ExtractPayloadAndCrc(Udma_ChHandle chHandle, 
                                                  uint8_t *buf);

/**
 * \brief   This function checks if overrun happend during DMA transfer.
 * 
 *  Pointer to buffer passed as a parameter has a padding region initialized 
 *  to some known values. Padding region comes right after the buffer payload
 *  data. After the DMA transfer this function is called to check if that
 *  region is modified and prints appropriate messages.
 * 
 *  \param  buf          Pointer to buffer with initialzed padding region
 * 
 *  \return \ref         Udma_ErrorCodes
 */ 
static int32_t Udma1Udma2App_CheckOverrun(uint8_t *buf);

/**
 * \brief   DMA-comletion event callback function.
 * 
 *  This function is invoked by the UDMA driver when an event that
 *  indicates DMA completion happens. It signals the waiting task
 *  by posting some application level semaphor. This callback runs
 *  in interrupt context.
 * 
 *  \param  eventHandle     UDMA event handle
 *  \param  eventType       Event type reported by the driver
 *  \param  appData         Application data pointer provided at registration
 *                          Currently unused  
 * 
 *  \return None
 */ 
static void Udma1Udma2App_UdmaEventDmaCb(Udma_EventHandle eventHandle,
                                         uint32_t eventType,
                                         void *appData);
/**
 * \brief   UDMA teardown-completion event callback function.
 * 
 *  This function is invoked by the UDMA driver whean a teardown packet 
 *  completion event is generated for the registered channel. On receiving
 *  a teardown event (UDMA_EVENT_TYPE_TEARDOWN_PACKET), it dequeues the
 *  teardown response from the teardown completion queue. 
 * 
 *  \param  eventHandle     UDMA event handle
 *  \param  eventType       Event type reported by the driver
 *  \param  appData         Application data pointer provided at registration
 *                          Currently unused 
 * 
 *  \return None
 */ 
static void Udma1Udma2App_UdmaEventTdCb(Udma_EventHandle eventHandle,
                                        uint32_t eventType,
                                        void *appData);

/**
 * \brief   UDMA driver init function.
 *  
 *  \param  drvHandle   UDMA driver handle pointer passed during
 *                      #Udma_init
 *  \return 
 */ 
static int32_t Udma1Udma2App_Init(Udma_DrvHandle drvHandle);

/**
 * \brief   UDMA driver deinit function/
 *  
 *  \param  drvHandle   UDMA driver handle pointer passed during
 *                      #Udma_init
 *  \return 
 */  
static int32_t Udma1Udma2App_Deinit(Udma_DrvHandle drvHandle);

/**
 * \brief   This function does UDMA parameters, channel and CRC configuration.
 * 
 *  Initializes the UDMA channel and CRC channel. Creates a semaphore used for
 *  completion waiting. Initializes the channel paremeters on opens it.
 *  Registers all events and finally enables the channel.
 * 
 *  \param  drvHandle   UDMA driver handle
 *  \param  chHandle    UDMA channel handle

 * 
 *  \return \ref        Udma_ErrorCodes
 */ 
static int32_t Udma1Udma2App_Create(Udma_DrvHandle drvHandle, 
                                    Udma_ChHandle chHandle);

/**
 * \brief   UDMA driver cleanup function.
 * 
 *  This function disables UDMA channel, unregisters all events, closes
 *  the channel and deletes semaphor used for waiting on completion.
 * 
 *  \param  drvHandle   UDMA driver handle
 *  \param  chHandle    UDMA channel handle

 * 
 *  \return \ref        Udma_ErrorCodes
 */ 
static int32_t Udma1Udma2App_Delete(Udma_DrvHandle drvHandle, 
                                    Udma_ChHandle chHandle);

/**
 * \brief   Initializes UDMA TRPD for a linear DMA transfer.
 * 
 *  This function prepares a single Transfer Request (TR) of type 15 inside
 *  the given TR packet descriptor (TRPD) memory. It configures the TR to 
 *  copy \ref length bytes from \ref srcBuf to \ref destBuf using linear 
 *  addressing, enables completion events, and associates the TRPD with the
 *  channel's completion queue ring.
 * 
 *  \param  chHandle    UDMA channel handle
 *  \param  pTrpdMem    Pointer to TRPD memory buffer to be populated
 *  \param  destBuf     Pointer to the DMA transfer destination buffer
 *  \param  srcBuf      Pointer to the DMA transfer source buffer
 *  \param  length      Size of the DMA transfer 
 * 
 *  \return None
 */ 
static void Udma1Udma2App_UdmaTrpdInit(Udma_ChHandle chHandle,
                                       uint8_t *pTrpdMem,
                                       const void *destBuf,
                                       const void *srcBuf,
                                       uint32_t length);

/**
 * \brief   Initializes UDMA TRPD to stream data into an MCRC block.
 * 
 *  This function prepares a single Type-15 Transfer Request (TR) inside the
 *  provided TR packet descriptor (TRPD) to feed a source buffer into a CRC 
 *  hardware block (e.g. MCRC) via DMA. 
 * 
 *  \param  chHandle    UDMA channel handle
 *  \param  pTrpdMem    Pointer to TRPD memory buffer to be populated
 *  \param  destBuf     Pointer to the DMA transfer destination buffer, 
 *                      in this case address of the CRC PSA signature
 *                      register
 *  \param  srcBuf      Pointer to the DMA transfer source buffer
 *  \param  length      Size of the DMA transfer           
 * 
 *  \return None
 */ 
static void Udma1Udma2App_CrcUdmaTrpdInit(Udma_ChHandle chHandle,
                                          uint8_t *pTrpdMem,
                                          const void *srcBuf,
                                          const void *destBuf,
                                          uint32_t length);
/**
 * \brief   Application level function for printing string.
 * 
 *  \param  str     String to be printed      
 * 
 *  \return None
 */ 
static void Udma1Udma2App_Print(const char *str);

/**
 * \brief   Application level function for printing string and one unsigned
 *          integer number.
 * 
 *  \param  str     String to be printed
 *  \param  num     Unsigned integer number to be printed
 * 
 *  \return None
 */ 
static void Udma1Udma2App_PrintNum(const char *str, uint32_t num);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*
 * UDMA driver objects
 */
struct Udma_DrvObj      gUdmaDrvObj;
struct Udma_ChObj       gUdmaChObj;
struct Udma_EventObj    gUdmaCqEventObj;
struct Udma_EventObj    gUdmaTdCqEventObj;

/*
 * UDMA Memories
 */
static uint8_t gTxRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gTxCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gTxTdCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[UDMA_TEST_APP_TRPD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/*
 * CRC calculation buffer
 */
static uint8_t gCrcBuf[PAYLOAD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT), section(".data"))) = {1U};

/* UDMA Transfers buffers */
/** 
 * \brief Buffer set to section .udma_ddr_55 on address 0x9555_5555 in memory.
 */
static uint8_t gBufDdr_55Pattern[BUFFER_SIZE] __attribute__((section(".udma_ddr_55")));
/** 
 * \brief Buffer set to section .udma_ddr_aa on address 0xAAAA_AAAA in memory. 
 */
static uint8_t gBufDdr_AAPattern[BUFFER_SIZE] __attribute__((section(".udma_ddr_aa")));
/** 
 * \brief Buffer set to section .udma_sram on address 0x41CE3200 in memory. 
 */
static uint8_t gBufSram_41Pattern[BUFFER_SIZE] __attribute__((section(".udma_sram")));

/* Semaphore to indicate transfer completion */
static SemaphoreP_Handle gUdmaAppDoneSem = NULL;

/* Global test pass/fail flag */
static volatile int32_t gUdmaAppResult = UDMA_SOK;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * UDMA memcpy test
 */
int32_t Udma1Udma2_MemcpyMain(void)
{
    int32_t         retVal = UDMA_SOK;
    int32_t         status = UDMA_SOK;
    Udma_DrvHandle  drvHandle = &gUdmaDrvObj;
    Udma_ChHandle   chHandle = &gUdmaChObj;
    int32_t         initDone = 0;
    int32_t         createDone = 0;

    Udma1Udma2App_Print("UDMA memcpy application started...\n");

    retVal = Udma1Udma2App_Init(drvHandle);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA App init failed!!\n");
    }
    else
    {
        initDone = 1;
        Udma1Udma2App_Print("[Success] UDMA App init successful!\n");

        retVal = Udma1Udma2App_Create(drvHandle, chHandle);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA App create failed!!\n");
        }
        else
        {
            createDone = 1;
            Udma1Udma2App_Print("[Success] UDMA App create successful!\n\n");

            retVal = Udma1Udma2App_MemcpyTest(chHandle);
            if (UDMA_SOK != retVal)
            {
                Udma1Udma2App_Print("[Error] UDMA App memcpy test failed!!\n");
            }
            else
            {
                if (UTRUE == Udma_appIsUdmapStatsSupported())
                {
                    Udma_ChStats chStats;

                    retVal = Udma_chGetStats(chHandle, &chStats);
                    if (UDMA_SOK == retVal)
                    {
                        Udma1Udma2App_Print("\nUDMA App memcpy test statistics:\n");
                        Udma1Udma2App_PrintNum("Completed packet count       : %d\n", chStats.packetCnt);
                        Udma1Udma2App_PrintNum("Completed payload byte count : %d\n", chStats.completedByteCnt);
                        Udma1Udma2App_PrintNum("Started byte count           : %d\n", chStats.startedByteCnt);
                    }
                }
            }
        }
    }

    if (createDone)
    {
        status = Udma1Udma2App_Delete(drvHandle, chHandle);
        if (UDMA_SOK != status)
        {
            Udma1Udma2App_Print("[Error] UDMA App delete failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
        else
        {
            Udma1Udma2App_Print("[Success] UDMA App delete successful!\n");
        }
    }

    if (initDone)
    {
        status = Udma1Udma2App_Deinit(drvHandle);
        if (UDMA_SOK != status)
        {
            Udma1Udma2App_Print("[Error] UDMA App deinit failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
        else
        {
            Udma1Udma2App_Print("[Success] UDMA App deinit successful!\n");
        }
    }

    if ((UDMA_SOK == retVal) && (UDMA_SOK == gUdmaAppResult))
    {
        Udma1Udma2App_Print("\nUDMA memcpy using TR15 block copy Passed!!\n");
        Udma1Udma2App_Print("All tests have passed!!\n");
    }
    else
    {
        Udma1Udma2App_Print("\nUDMA memcpy using TR15 block copy Failed!!\n");
        Udma1Udma2App_Print("Some tests have failed!!\n");
    }

    return retVal;
}

static int32_t Udma1Udma2App_MemcpyTest(Udma_ChHandle chHandle)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    i;

    /* Pointers to the buffers involved in DMA transfer */
    uint8_t     *srcBuf;
    uint8_t     *destBuf;

    /* Pointer to the buffer used for CRC calculation */    
    uint8_t *crcSrcBuf = &gCrcBuf[0U];

    /* Fill the buffer with payload data */
    for (i = 0; i < PAYLOAD_SIZE; i++)
    {
        crcSrcBuf[i] = i;
    }

    /* Calculate the CRC signature value using the payload data */
    Udma1Udma2App_Print("Initial CRC calculation based on the payload data\n");
    retVal = Udma1Udma2App_UdmaCrc(chHandle, crcSrcBuf, PAYLOAD_SIZE);
    Udma1Udma2App_Print("\n");

    /* Get the CRC signature value*/
    crcSignature_t sectSignVal;
    CRCGetPSASectorSig(APP_CRC_BASE, APP_CRC_CHANNEL, &sectSignVal);

    /* DMA transfer from memory address 0x9555_5555 to 0xAAAA_AAAA */
    srcBuf  = &gBufDdr_55Pattern[0U];
    destBuf = &gBufDdr_AAPattern[0U];
    /* Append calculated CRC values at the end of srcBuf */
    memcpy(srcBuf, crcSrcBuf, PAYLOAD_SIZE);
    memcpy(srcBuf + PAYLOAD_SIZE, &sectSignVal.regL, sizeof(uint32_t));
    memcpy(srcBuf + PAYLOAD_SIZE + sizeof(uint32_t), &sectSignVal.regH, sizeof(uint32_t));
    /* DMA Transfer function call */
    retVal = Udma1Udma2App_SingleTransfer(chHandle, destBuf, srcBuf);

    if (UDMA_SOK == retVal)
    {
        /* Check the padding region for possible overrun during the DMA transfer */
        retVal = Udma1Udma2App_CheckOverrun(destBuf);
    }

    if (UDMA_SOK == retVal)
    {
        /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
        retVal = Udma1Udma2App_ExtractPayloadAndCrc(chHandle, destBuf);
    }

    Udma1Udma2App_Print("\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0xAAAA_AAAA to 0x9555_5555 */
        srcBuf  = &gBufDdr_AAPattern[0];
        destBuf = &gBufDdr_55Pattern[0];
        retVal = Udma1Udma2App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Udma1Udma2App_CheckOverrun(destBuf);
        }
        
        if (UDMA_SOK == retVal)
        {
             /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Udma1Udma2App_ExtractPayloadAndCrc(chHandle, destBuf);
        }

    }

    Udma1Udma2App_Print("\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0x9555_5555 to 0x41CE3200 */
        srcBuf  = &gBufDdr_55Pattern[0];
        destBuf = &gBufSram_41Pattern[0];
        retVal = Udma1Udma2App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Udma1Udma2App_CheckOverrun(destBuf);
        }

        if (UDMA_SOK == retVal)
        {
            /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Udma1Udma2App_ExtractPayloadAndCrc(chHandle, destBuf);
        }

    }

    Udma1Udma2App_Print("\n\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0x41CE3200 to 0x9555_5555 */
        srcBuf  = &gBufSram_41Pattern[0];
        destBuf = &gBufDdr_55Pattern[0];
        retVal = Udma1Udma2App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Udma1Udma2App_CheckOverrun(destBuf);
        }

        if (UDMA_SOK == retVal)
        {
            /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Udma1Udma2App_ExtractPayloadAndCrc(chHandle, destBuf);
        }
    }

    return (retVal);
}

static int32_t Udma1Udma2App_SingleTransfer(Udma_ChHandle chHandle, uint8_t *destBuf, uint8_t *srcBuf)
{
    int32_t retVal = UDMA_SOK;
    int16_t i;

    Udma1Udma2App_PrintNum("DMA Transfer of buffer from address 0x%08x", (uint32_t)(uintptr_t)srcBuf);
    Udma1Udma2App_PrintNum(" to address 0x%08x\n", (uint32_t)(uintptr_t)destBuf);
    

    /* Make sure destination buffer is set to all zeros */
    for (i = 0U; i < UDMA_TEST_APP_NUM_BYTES; i++)
    {
        destBuf[i] = 0U;
    }
    /* Initialize padding region of destination buffer */
    for (i = 0U; i < PADDING_SIZE; i++)
    {
        destBuf[UDMA_TEST_APP_NUM_BYTES+i] = 0xA5;
    }

    /* Writeback source and destination buffer */
    Udma_appUtilsCacheWb(srcBuf, UDMA_TEST_APP_NUM_BYTES);
    Udma_appUtilsCacheWb(destBuf, UDMA_TEST_APP_NUM_BYTES + PADDING_SIZE);

    /* Perform UDMA memcpy */
    retVal = Udma1Udma2App_UdmaMemcpy(
                    chHandle,
                    destBuf,
                    srcBuf,
                    UDMA_TEST_APP_NUM_BYTES);

    /* Invalidate destination buffer */
    Udma_appUtilsCacheInv(destBuf, UDMA_TEST_APP_NUM_BYTES + PADDING_SIZE);

    if (UDMA_SOK == retVal)
    {
        Udma1Udma2App_Print("... DMA transfer successful!\n");
    }

    return retVal;
}

static int32_t Udma1Udma2App_UdmaMemcpy(Udma_ChHandle chHandle,
                                        void *destBuf,
                                        void *srcBuf,
                                        uint32_t length)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t   *pTrResp, trRespStatus;
    uint64_t    pDesc = 0;
    uint8_t    *trpdMem = &gUdmaTrpdMem[0U];

    /* Update TR packet descriptor */
    Udma1Udma2App_UdmaTrpdInit(chHandle, trpdMem, destBuf, srcBuf, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(
                 Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] Channel queue failed!!\n");
    }

    if (UDMA_SOK == retVal)
    {
        /* Wait for return descriptor in completion ring - this marks the
         * transfer completion */
        SemaphoreP_pend(gUdmaAppDoneSem, SemaphoreP_WAIT_FOREVER);

        /* Response received in completion queue */
        retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &pDesc);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if (((uint64_t) Udma_appPhyToVirtFxn(pDesc, 0U, NULL)) != ((uint64_t) trpdMem))
        {
            Udma1Udma2App_Print("[Error] TR descriptor pointer returned doesn't "
                   "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        Udma_appUtilsCacheInv(&gUdmaTrpdMem[0U], UDMA_TEST_APP_TRPD_SIZE);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
        if (trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            Udma1Udma2App_Print("[Error] TR Response not completed!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static void Udma1Udma2App_UdmaEventDmaCb(Udma_EventHandle eventHandle,
                                         uint32_t eventType,
                                         void *appData)
{
    if (UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        SemaphoreP_post(gUdmaAppDoneSem);
    }
    else
    {
        gUdmaAppResult = UDMA_EFAIL;
    }

    return;
}

static void Udma1Udma2App_UdmaEventTdCb(Udma_EventHandle eventHandle,
                              uint32_t eventType,
                              void *appData)
{
    int32_t             retVal;
    CSL_UdmapTdResponse tdResp;

    if (UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&gUdmaChObj, &tdResp);
        if (UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
            gUdmaAppResult = UDMA_EFAIL;
        }
    }
    else
    {
        gUdmaAppResult = UDMA_EFAIL;
    }

    return;
}

static int32_t Udma1Udma2App_Init(Udma_DrvHandle drvHandle)
{
    int32_t         retVal;
    Udma_InitPrms   initPrms;
    uint32_t        instId;

    /* Use MCU NAVSS for MCU domain cores. Rest cores all uses Main NAVSS */
#if defined (BUILD_MCU1_0) || defined (BUILD_MCU1_1)
    instId = UDMA_INST_ID_MCU_0;
#else
    instId = UDMA_INST_ID_MAIN_0;
#endif

    /* UDMA driver init */
    retVal = UdmaInitPrms_init(instId, &initPrms);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA init prms init failed!!\n");
    }
    else
    {
        initPrms.virtToPhyFxn   = &Udma_appVirtToPhyFxn;
        initPrms.phyToVirtFxn   = &Udma_appPhyToVirtFxn;
        initPrms.printFxn       = &Udma1Udma2App_Print;
        retVal = Udma_init(drvHandle, &initPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA init failed!!\n");
        }
    }

    return (retVal);
}

static int32_t Udma1Udma2App_Deinit(Udma_DrvHandle drvHandle)
{
    int32_t     retVal;

    retVal = Udma_deinit(drvHandle);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA deinit failed!!\n");
    }

    return (retVal);
}

static int32_t Udma1Udma2App_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChTxPrms       txPrms;
    Udma_ChRxPrms       rxPrms;
    Udma_EventHandle    tdCqEventHandle;
    Udma_EventPrms      tdCqEventPrms;
    Udma_EventHandle    cqEventHandle;
    Udma_EventPrms      cqEventPrms;
    SemaphoreP_Params   semPrms;

    /* Configure CRC channel */
    CRCInitialize(
        APP_CRC_BASE,
        APP_CRC_CHANNEL,
        APP_CRC_WATCHDOG_PRELOAD_VAL,
        APP_CRC_BLOCK_PRELOAD_VAL);

    SemaphoreP_Params_init(&semPrms);
    gUdmaAppDoneSem = SemaphoreP_create(0, &semPrms);
    if (NULL == gUdmaAppDoneSem)
    {
        Udma1Udma2App_Print("[Error] Sem create failed!!\n");
        retVal = UDMA_EFAIL;
    }

    if (UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_TR_BLK_COPY;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.fqRingPrms.ringMem       = &gTxRingMem[0U];
        chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt       = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.cqRingPrms.ringMem       = &gTxCompRingMem[0U];
        chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.cqRingPrms.elemCnt       = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.tdCqRingPrms.ringMem     = &gTxTdCompRingMem[0U];
        chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.elemCnt     = UDMA_TEST_APP_RING_ENTRIES;

        /* Open channel for block copy */
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA channel open failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Config TX channel */
        UdmaChTxPrms_init(&txPrms, chType);
        retVal = Udma_chConfigTx(chHandle, &txPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA TX channel config failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Config RX channel - which is implicitly paired to TX channel in
         * block copy mode */
        UdmaChRxPrms_init(&rxPrms, chType);
        retVal = Udma_chConfigRx(chHandle, &rxPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA RX channel config failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Register ring completion callback */
        cqEventHandle = &gUdmaCqEventObj;
        UdmaEventPrms_init(&cqEventPrms);
        cqEventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
        cqEventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        cqEventPrms.chHandle          = chHandle;
        cqEventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        cqEventPrms.eventCb           = &Udma1Udma2App_UdmaEventDmaCb;
        retVal = Udma_eventRegister(drvHandle, cqEventHandle, &cqEventPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA CQ event register failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Register teardown ring completion callback */
        tdCqEventHandle = &gUdmaTdCqEventObj;
        UdmaEventPrms_init(&tdCqEventPrms);
        tdCqEventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
        tdCqEventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        tdCqEventPrms.chHandle          = chHandle;
        tdCqEventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        tdCqEventPrms.eventCb           = &Udma1Udma2App_UdmaEventTdCb;
        retVal = Udma_eventRegister(drvHandle, tdCqEventHandle, &tdCqEventPrms);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA Teardown CQ event register failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] UDMA channel enable failed!!\n");
        }
    }

    return (retVal);
}

static int32_t Udma1Udma2App_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal, tempRetVal;
    uint64_t            pDesc;
    Udma_EventHandle    cqEventHandle;
    Udma_EventHandle    tdCqEventHandle;

    retVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA channel disable failed!!\n");
    }

    /* Flush any pending request from the free queue */
    while (1)
    {
        tempRetVal = Udma_ringFlushRaw(
                         Udma_chGetFqRingHandle(chHandle), &pDesc);
        if (UDMA_ETIMEOUT == tempRetVal)
        {
            break;
        }
    }

    /* Unregister all events */
    cqEventHandle = &gUdmaCqEventObj;
    retVal += Udma_eventUnRegister(cqEventHandle);
    
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA event unregister failed!!\n");
    }

    tdCqEventHandle = &gUdmaTdCqEventObj;
    retVal += Udma_eventUnRegister(tdCqEventHandle);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA event unregister failed!!\n");
    }

    retVal += Udma_chClose(chHandle);
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] UDMA channel close failed!!\n");
    }

    if (gUdmaAppDoneSem != NULL)
    {
        SemaphoreP_delete(gUdmaAppDoneSem);
        gUdmaAppDoneSem = NULL;
    }

    return (retVal);
}

static void Udma1Udma2App_UdmaTrpdInit(Udma_ChHandle chHandle,
                             uint8_t *pTrpdMem,
                             const void *destBuf,
                             const void *srcBuf,
                             uint32_t length)
{
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)UdmaUtils_getTrpdTr1Pointer(pTrpdMem, 0U);
    uint32_t *pTrResp = (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_15, 1U, cqRingNum);

    /* Setup TR */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, 15)                                            |
                    CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U)                                          |
                    CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U)                                             |   /* NA */
                    CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION)|
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, 0x25U)                                       |   /* This will come back in TR response */
                    CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);
    pTr->icnt0    = length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = pTr->icnt0;
    pTr->dim2     = (pTr->icnt0 * pTr->icnt1);
    pTr->dim3     = (pTr->icnt0 * pTr->icnt1 * pTr->icnt2);
    pTr->addr     = (uint64_t) Udma_appVirtToPhyFxn(srcBuf, 0U, NULL);
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */
    pTr->dicnt0   = length;
    pTr->dicnt1   = 1U;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = pTr->dicnt0;
    pTr->ddim2    = (pTr->dicnt0 * pTr->dicnt1);
    pTr->ddim3    = (pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2);
    pTr->daddr    = (uint64_t) Udma_appVirtToPhyFxn(destBuf, 0U, NULL);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(pTrpdMem, UDMA_TEST_APP_TRPD_SIZE);

    return;
}

static void Udma1Udma2App_Print(const char *str)
{
    UART_printf("%s", str);
    if (UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}
static void Udma1Udma2App_PrintNum(const char *str, uint32_t num)
{
    static char printBuf[200U];

    snprintf(printBuf, 200U, str, num);
    UART_printf("%s", printBuf);

    if (UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", printBuf);
    }

    return;
}

static void Udma1Udma2App_CrcUdmaTrpdInit(Udma_ChHandle chHandle,
                             uint8_t *pTrpdMem,
                             const void *srcBuf,
                             const void *destBuf,
                             uint32_t length)
{
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)UdmaUtils_getTrpdTr1Pointer(pTrpdMem, 0U);
    uint32_t *pTrResp = (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);
    uint32_t cCnt;

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_15, 1U, cqRingNum);

    /* Setup TR */
    cCnt = 1;
    while ((length / cCnt) > 0x7FFFU)
    {
        cCnt = cCnt * 2;
    }
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, 15)                                            |
                    CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U)                                          |
                    CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U)                                             |   /* NA */
                    CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION)|
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, 0x25U)                                       |   /* This will come back in TR response */
                    CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);
    pTr->icnt0    = length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = pTr->icnt0;
    pTr->dim2     = (pTr->icnt0 * pTr->icnt1);
    pTr->dim3     = (pTr->icnt0 * pTr->icnt1 * pTr->icnt2);
    pTr->addr     = (uint64_t) Udma_appVirtToPhyFxn(srcBuf, 0U, NULL);
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */
    pTr->dicnt0   = APP_CRC_PATTERN_SIZE;
    pTr->dicnt1   = (length / pTr->dicnt0) / cCnt;
    pTr->dicnt2   = cCnt;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = 0U;
    pTr->ddim2    = 0U;
    pTr->ddim3    = 0U;
    pTr->daddr    = (uint64_t) Udma_appVirtToPhyFxn(destBuf, 0U, NULL);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(pTrpdMem, UDMA_TEST_APP_TRPD_SIZE);

    return;
}

static int32_t Udma1Udma2App_UdmaCrc(Udma_ChHandle chHandle,
                                     void *srcBuf,
                                     uint32_t length)
{
    int32_t               retVal = UDMA_SOK;
    uint32_t             *pTrResp, trRespStatus;
    uint64_t              pDesc = 0;
    uint8_t              *trpdMem = &gUdmaTrpdMem[0U];
    crcSignature_t        sectSignVal;
    crcSignatureRegAddr_t psaSignRegAddr;
    uint32_t              patternCnt;

    sectSignVal.regL = 0U;
    sectSignVal.regH = 0U;
    patternCnt = length / APP_CRC_PATTERN_SIZE;

    /* Writeback source buffer */
    Udma_appUtilsCacheWb(srcBuf, PAYLOAD_SIZE);

    /* Get CRC PSA signature register address */
    CRCGetPSASigRegAddr(APP_CRC_BASE, APP_CRC_CHANNEL, &psaSignRegAddr);
    CRCChannelReset(APP_CRC_BASE, APP_CRC_CHANNEL);
    CRCConfigure(APP_CRC_BASE, APP_CRC_CHANNEL, patternCnt, APP_CRC_SECT_CNT, CRC_OPERATION_MODE_SEMICPU);

    /* Update TR packet descriptor */
    Udma1Udma2App_CrcUdmaTrpdInit(chHandle, trpdMem, srcBuf, (void *)(uintptr_t) psaSignRegAddr.regL, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if (UDMA_SOK != retVal)
    {
        Udma1Udma2App_Print("[Error] Channel queue failed!!\n");
    }

    if (UDMA_SOK == retVal)
    {
        /* Wait for return descriptor in completion ring - this marks the
         * transfer completion */
        SemaphoreP_pend(gUdmaAppDoneSem, SemaphoreP_WAIT_FOREVER);

        /* Response received in completion queue */
        retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &pDesc);
        if (UDMA_SOK != retVal)
        {
            Udma1Udma2App_Print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if (((uint64_t) Udma_appPhyToVirtFxn(pDesc, 0U, NULL)) != ((uint64_t) trpdMem))
        {
            Udma1Udma2App_Print("[Error] TR descriptor pointer returned doesn't "
                   "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        Udma_appUtilsCacheInv(&gUdmaTrpdMem[0U], UDMA_TEST_APP_TRPD_SIZE);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
        if (trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            Udma1Udma2App_Print("[Error] TR Response not completed!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        uint32_t intrStatus;

        while (1U)
        {
            CRCGetIntrStatus(APP_CRC_BASE, APP_CRC_CHANNEL, &intrStatus);
            if ((intrStatus & APP_CRC_CH_CCITENR_MASK) == 0x1U)
            {
                break;
            }
            /* Wait here till CRC compression complete is set. */
        }

        CRCGetPSASectorSig(APP_CRC_BASE, APP_CRC_CHANNEL, &sectSignVal);
        Udma1Udma2App_PrintNum("...CRC Signature Value - L: 0x%08x, ", sectSignVal.regL);
        Udma1Udma2App_PrintNum("H: 0x%08x\n", sectSignVal.regH);
        
        CRCClearIntr(APP_CRC_BASE, APP_CRC_CHANNEL, CRC_CHANNEL_IRQSTATUS_RAW_MAIN_ALL);
    }

    return (retVal);
}

static int32_t Udma1Udma2App_ExtractPayloadAndCrc(Udma_ChHandle chHandle, uint8_t *buf)
{
    int32_t retVal = UDMA_SOK;

    if (UDMA_SOK == retVal)
    {
        /* Pointer to the buffer prepared for UDMA CRC transfer*/
        uint8_t *crcDestBuf = &gCrcBuf[0];
        /* Fill the crcDestBuf with only the payload data from buf */
        memcpy(crcDestBuf, buf, PAYLOAD_SIZE);

        /* Calculate CRC from the destination buffer payload data */
        Udma1Udma2App_Print("Calculate CRC values on destination side: \n");
        retVal = Udma1Udma2App_UdmaCrc(chHandle, crcDestBuf, PAYLOAD_SIZE);

        /* Get the CRC signature value */
        crcSignature_t sectSignVal;
        CRCGetPSASectorSig(APP_CRC_BASE, APP_CRC_CHANNEL, &sectSignVal);

        if (UDMA_SOK == retVal)
        {
            /* Compare the embedded and calculated CRC signature values */
            Udma1Udma2App_Print("Compare embedded and calculated CRC values on destination side: \n");
            /* After the UDMA memcpy, extract CRCs and compare them */
            uint32_t *crcPtr = (uint32_t *)(buf + PAYLOAD_SIZE);

            if ((crcPtr[0] == sectSignVal.regL) && (crcPtr[1] == sectSignVal.regH))
            {
                Udma1Udma2App_Print("...CRC Check Successful. Signature values MATCH!\n");
                retVal = UDMA_SOK;
            }
            else
            {
                Udma1Udma2App_Print("...CRC Check Fail. Signature values DO NOT MATCH!\n");
                retVal = UDMA_EFAIL;
            }

        }
    }

    return retVal;
}

static int32_t Udma1Udma2App_CheckOverrun(uint8_t *buf)
{
    uint32_t i;
    int32_t retVal = UDMA_SOK;
    

    Udma1Udma2App_Print("Check the padding/guard region of the destination buffer for overrun:\n");

    for (i = 0U; i < PADDING_SIZE; i++)
    {
        if (buf[UDMA_TEST_APP_NUM_BYTES + i] != 0xA5U)
        {
            Udma1Udma2App_PrintNum("... Overrun Check Fail. Padding region at +%u bytes ", i);
            Udma1Udma2App_PrintNum("(addr=%p) unexpected!\n", (uint32_t)(uintptr_t)&buf[UDMA_TEST_APP_NUM_BYTES + i]);

            retVal = UDMA_EFAIL;
        }
    }
    
    if (UDMA_SOK == retVal)
    {
        Udma1Udma2App_Print("... Overrun Check Successful. Padding/guard region intact!\n");
    }
    
    return retVal;
}