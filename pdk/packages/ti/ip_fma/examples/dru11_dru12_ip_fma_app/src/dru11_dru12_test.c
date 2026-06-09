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
 *  \file dru11_dru12_test.c
 *
 *  \brief DRU11 and DRU12 safety mechanisms example application.
 *
 *  Performing a DMA transfer using MSMC DRU as DMA engine. Test DRU basic 
 *  functionalities and all address lines (DRU11) by performing DMA transfer
 *  from memory location on address 0x9555_5555 to memory location on address
 *  0xAAAA_AAAA, then reverse source and destination address and to the
 *  transfer again. Same is done for addresses 0x9555_5555 and 0x41CE_3200.
 *  To test information redundancy (DRU12) CRC signature is calculated for the
 *  initial source buffer payload, then it is embedded inside the buffer and
 *  DMA transfer is performed. At the destination side the validity of
 *  transfered data is checked using embedded CRC signature value and
 *  recalculated CRC value.
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
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * \
                                         UDMA_TEST_APP_RING_ENTRY_SIZE)
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
#define PAYLOAD_SIZE                    ((uint32_t) 124U)

/* CRC-32 (IEEE 802.3) parameters */
#define CRC32_INIT   (0xFFFFFFFFu)
#define CRC32_XOROUT (0xFFFFFFFFu)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief This function unifies complete DMA transfer test.
 * 
 *  Does the complete job of DMA buffers preparation, transfer execution using
 *  the given DMA channel, validation and checking, in compliance with DRU11 
 *  and DRU12 requirements.
 * 
 *  \param  chHandle     UDMA channel handle.
 * 
 *  \return \ref         Udma_ErrorCodes
 */
static int32_t Dru11Dru12App_MemcpyTest(Udma_ChHandle chHandle);

/**
 * \brief   This function does one single DMA transfer.
 * 
 *  Clears the destination buffers and initializes padding region of it.
 *  Ensures cache coherency by writing back buffers from cache before DMA
 *  and invalidating the cache after it. Actual DMA transfer is performed
 *  by invoking \ref Dru11Dru12App_UdmaMemcpy function using the given
 *  UDMA channel.
 *  
 *  \param  chHandle     UDMA channel handle
 *  \param  destBuf      Pointer to destination buffer
 *  \param  srcBuf       Pointer to destination buffer
 * 
 *  \return \ref         Udma_ErrorCodes
 */ 
static int32_t Dru11Dru12App_SingleTransfer(Udma_ChHandle chHandle,
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
 *  \param  chHandle     UDMA channel handle
 *  \param  destBuf      Pointer to destination buffer
 *  \param  srcBuf       Pointer to destination buffer
 *  \param  length       Transfer size in bytes
 * 
 *  \return \ref         Udma_ErrorCodes
 */
static int32_t Dru11Dru12App_UdmaMemcpy(Udma_ChHandle chHandle,
                                        void *destBuf,
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
 *  \param  chHandle     UDMA channel handle
 *  \param  buf          Pointer to buffer containing payload and CRC data
 * 
 *  \return \ref         Udma_ErrorCodes
 */
static int32_t Dru11Dru12App_ExtractPayloadAndCrc(Udma_ChHandle chHandle, 
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
static int32_t Dru11Dru12App_CheckOverrun(uint8_t *buf);

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
static void Dru11Dru12App_UdmaEventCb(Udma_EventHandle eventHandle,
                                      uint32_t eventType,
                                      void *appData);

/**
 * \brief   UDMA driver init function.
 * 
 *  This function initializes the UDMA driver and configures the MSMC DRU
 *  (SoC‑level DRU) hardware queues before any DRU channel is created.
 *
 *  The MSMC DRU is an external Unified Transfer Controller (UTC) located
 *  outside NAVSS, but all of its configuration and TR submission still go
 *  through the MAIN NAVSS UDMA driver. For this reason, the initialization
 *  always targets MAIN NAVSS. 
 * 
 *  \param  drvHandle   UDMA driver handle pointer passed during
 *                      #Udma_init
 *  \return \ref        Udma_ErrorCodes
 */ 
static int32_t Dru11Dru12App_Init(Udma_DrvHandle drvHandle);

/**
 * \brief   UDMA driver deinit function/
 *  
 *  \param  drvHandle   UDMA driver handle pointer passed during
 *                      #Udma_init
 *  \return \ref        Udma_ErrorCodes
 */ 
static int32_t Dru11Dru12App_Deinit(Udma_DrvHandle drvHandle);

/**
 * \brief   Creates and configures a UDMA channel for use with the MSMC DRU.
 *
 * This function performs all application‑level setup required to prepare a
 * UDMA channel for DRU11/DRU12 execution. It initializes channel parameters
 * for an external Unified Transfer Controller (UTC), assigns ring memories
 * for TR submission and completion, configures DRU‑specific UTC parameters
 * (ownership and DRU hardware queue), registers completion and teardown
 * events, and finally enables the channel.
 *
 * \param   drvHandle   UDMA driver handle pointer passed during
 *                      #Udma_init
 * \param   chHandle    UDMA channel handle.
 *
 * \return  \ref        Udma_ErrorCodes
 */

static int32_t Dru11Dru12App_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);

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
static int32_t Dru11Dru12App_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);

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
static void Dru11Dru12App_UdmaTrpdInit(Udma_ChHandle chHandle,
                                       uint8_t *pTrpdMem,
                                       const void *destBuf,
                                       const void *srcBuf,
                                       uint32_t length);

/**
 * \brief   Application level function for printing string.
 * 
 *  \param  str     String to be printed      
 * 
 *  \return None
 */
static void Dru11Dru12App_Print(const char *str);
 
/**
 * \brief   Application level function for printing string and one unsigned
 *          integer number.
 * 
 *  \param  str     String to be printed
 *  \param  num     Unsigned integer number to be printed
 * 
 *  \return None
 */
static void Dru11Dru12App_PrintNum(const char *str, uint32_t num);

/**
 * \brief   Compute CRC-32 (IEEE 802.3) over a buffer with cache maintanance.
 * 
 *  This function computes a software CRC-32 (polynomial 0x04C11DB7, reflected 
 *  form 0xEDB88320) over the input buffer \p buf of length \p length bytes.
 *  The calculation usees a 256-entry lookup table for performance and applies
 *  the standard initial value (0xFFFFFFFF) and final XOR (0xFFFFFFFF).
 * 
 * \param  buf     Pointer to the input buffer (byte-addressable). Must be a valid, accessible
 *                 memory region of at least \p length bytes. The pointer is treated as const
 *                 for CRC purposes; no payload modification is performed.
 * \param  length  Number of bytes to include in the CRC calculation.
 *
 * \return 32-bit CRC value computed over the buffer contents, using CRC-32 (IEEE 802.3)
 */
static uint32_t Dru11Dru12App_SwCrc(void *buf, uint32_t length);

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
static uint8_t gDruRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruTdCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[UDMA_TEST_APP_TRPD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/*
 * Application Buffers
 */
/* CRC calculation buffer */
static uint8_t gCrcBuf[PAYLOAD_SIZE];

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


/**
 * \brief  Precomputed CRC-32 (IEEE 802.3) lookup table (256 entries).
 *
 * This table encodes the effect of the reflected CRC-32 polynomial (0xEDB88320)
 * applied to each possible 8-bit value (0–255). It allows the CRC algorithm to
 * process one byte at a time using a fast table lookup instead of performing
 * 8 rounds of bitwise polynomial division for every byte.
 *
 * Each entry represents the CRC remainder obtained after feeding a single byte
 * through the CRC shift/XOR logic. Using this table reduces the overall CRC
 * computation cost from bit-level operations to a constant-time lookup per byte.
 *
 * The polynomial used is the reflected form of the IEEE CRC-32 generator
 * (0x04C11DB7), which matches the standard LSB-first, table-driven algorithm.
 */
static const uint32_t g_crc32Tbl[256] = {
    0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU, 0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
    0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U, 0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
    0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU, 0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
    0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU, 0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
    0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U, 0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
    0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U, 0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
    0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U, 0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
    0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U, 0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
    0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU, 0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
    0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U, 0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
    0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU, 0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
    0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU, 0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
    0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U, 0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
    0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U, 0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
    0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U, 0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
    0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U, 0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
    0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU, 0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
    0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U, 0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
    0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU, 0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
    0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU, 0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
    0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U, 0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
    0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U, 0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
    0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U, 0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
    0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U, 0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
    0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU, 0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
    0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U, 0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
    0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU, 0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
    0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU, 0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
    0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U, 0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
    0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U, 0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
    0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U, 0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
    0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U, 0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int32_t Dru11Dru12App_Test(void)
{
    int32_t         retVal = UDMA_SOK;
    int32_t         status = UDMA_SOK;
    Udma_DrvHandle  drvHandle = &gUdmaDrvObj;
    Udma_ChHandle   chHandle = &gUdmaChObj;

    int32_t initDone = 0;
    int32_t createDone = 0;

    Dru11Dru12App_Print("UDMA DRU application started...\n");

    retVal = Dru11Dru12App_Init(drvHandle);
    if (UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA App init failed!!\n");
    }
    else
    {
        initDone = 1;

        retVal = Dru11Dru12App_Create(drvHandle, chHandle);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA App create failed!!\n");
        }
        else
        {
            createDone = 1;

            retVal = Dru11Dru12App_MemcpyTest(chHandle);
            if(UDMA_SOK != retVal)
            {
                Dru11Dru12App_Print("[Error] UDMA App memcpy test failed!!\n");
            }
        }
    }

    if (createDone)
    {
        status = Dru11Dru12App_Delete(drvHandle, chHandle);
        if(UDMA_SOK != status)
        {
            Dru11Dru12App_Print("[Error] UDMA App delete failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }

    if(initDone)
    {
        status = Dru11Dru12App_Deinit(drvHandle);
        if(UDMA_SOK != status)
        {
            Dru11Dru12App_Print("[Error] UDMA App deinit failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }

    if(UDMA_SOK == retVal)
    {
        Dru11Dru12App_Print("\nUDMA DRU memcpy using TR15 block copy Passed!!\n");
        Dru11Dru12App_Print("All tests have passed!!\n");
    }
    else
    {
        Dru11Dru12App_Print("\nUDMA DRU memcpy using TR15 block copy Failed!!\n");
        Dru11Dru12App_Print("Some tests have failed!!\n");
    }

    return retVal;
}

static int32_t Dru11Dru12App_MemcpyTest(Udma_ChHandle chHandle)
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
    Dru11Dru12App_Print("Initial CRC calculation based on the payload data\n");
    uint32_t crcVal = Dru11Dru12App_SwCrc(crcSrcBuf, PAYLOAD_SIZE);
    Dru11Dru12App_Print("\n");

    /* DMA transfer from memory address 0x9555_5555 to 0xAAAA_AAAA */
    srcBuf  = &gBufDdr_55Pattern[0U];
    destBuf = &gBufDdr_AAPattern[0U];
    /* Append calculated CRC values at the end of srcBuf */
    memcpy(srcBuf, crcSrcBuf, PAYLOAD_SIZE);
    memcpy(srcBuf + PAYLOAD_SIZE, &crcVal, sizeof(uint32_t));
    /* DMA Transfer function call */
    retVal = Dru11Dru12App_SingleTransfer(chHandle, destBuf, srcBuf);

    if (UDMA_SOK == retVal)
    {
        /* Check the padding region for possible overrun during the DMA transfer */
        retVal = Dru11Dru12App_CheckOverrun(destBuf);
    }

    if (UDMA_SOK == retVal)
    {
        /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
        retVal = Dru11Dru12App_ExtractPayloadAndCrc(chHandle, destBuf);
    }

    Dru11Dru12App_Print("\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0xAAAA_AAAA to 0x9555_5555 */
        srcBuf  = &gBufDdr_AAPattern[0];
        destBuf = &gBufDdr_55Pattern[0];
        retVal = Dru11Dru12App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Dru11Dru12App_CheckOverrun(destBuf);
        }
        
        if (UDMA_SOK == retVal)
        {
             /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Dru11Dru12App_ExtractPayloadAndCrc(chHandle, destBuf);
        }

    }

    Dru11Dru12App_Print("\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0x9555_5555 to 0x41CE3200 */
        srcBuf  = &gBufDdr_55Pattern[0];
        destBuf = &gBufSram_41Pattern[0];
        retVal = Dru11Dru12App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Dru11Dru12App_CheckOverrun(destBuf);
        }

        if (UDMA_SOK == retVal)
        {
            /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Dru11Dru12App_ExtractPayloadAndCrc(chHandle, destBuf);
        }
    }

    Dru11Dru12App_Print("\n\n");

    if (UDMA_SOK == retVal)
    {
        /* DMA transfer from memory address 0x41CE3200 to 0x9555_5555 */
        srcBuf  = &gBufSram_41Pattern[0];
        destBuf = &gBufDdr_55Pattern[0];
        retVal = Dru11Dru12App_SingleTransfer(chHandle, destBuf, srcBuf);

        if (UDMA_SOK == retVal)
        {
            /* Check the padding region for possible overrun during the DMA transfer */
            retVal = Dru11Dru12App_CheckOverrun(destBuf);
        }

        if (UDMA_SOK == retVal)
        {
            /* Calculate CRC at the destination side and compare it to the CRC values embedded inside the buffer */
            retVal = Dru11Dru12App_ExtractPayloadAndCrc(chHandle, destBuf);
        }
    }

    return (retVal);
}

static int32_t Dru11Dru12App_SingleTransfer(Udma_ChHandle chHandle, 
                                            uint8_t *destBuf, 
                                            uint8_t *srcBuf)
{
    int32_t retVal = UDMA_SOK;
    int16_t i;

    Dru11Dru12App_PrintNum("DMA Transfer of buffer from address 0x%08x", (uint32_t)(uintptr_t)srcBuf);
    Dru11Dru12App_PrintNum(" to address 0x%08x\n", (uint32_t)(uintptr_t)destBuf);
    

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
    retVal = Dru11Dru12App_UdmaMemcpy(
                    chHandle,
                    destBuf,
                    srcBuf,
                    UDMA_TEST_APP_NUM_BYTES);

    /* Invalidate destination buffer */
    Udma_appUtilsCacheInv(destBuf, UDMA_TEST_APP_NUM_BYTES + PADDING_SIZE);

    if (UDMA_SOK == retVal)
    {
        Dru11Dru12App_Print("... DMA transfer successful!\n");
    }

    return retVal;
}

static int32_t Dru11Dru12App_UdmaMemcpy(Udma_ChHandle chHandle,
                                        void *destBuf,
                                        void *srcBuf,
                                        uint32_t length)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t   *pTrResp, trRespStatus;
    uint64_t    pDesc = 0;
    uint8_t    *trpdMem = &gUdmaTrpdMem[0U];

    /* Update TR packet descriptor */
    Dru11Dru12App_UdmaTrpdInit(chHandle, trpdMem, destBuf, srcBuf, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] Channel queue failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {
        /* Wait for return descriptor in completion ring - this marks the
         * transfer completion */
        SemaphoreP_pend(gUdmaAppDoneSem, SemaphoreP_WAIT_FOREVER);

        /* Response received in completion queue */
        retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &pDesc);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if(pDesc != ((uint64_t) trpdMem))
        {
            Dru11Dru12App_Print("[Error] TR descriptor pointer returned doesn't "
                   "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        Udma_appUtilsCacheInv(&gUdmaTrpdMem[0U], UDMA_TEST_APP_TRPD_SIZE);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
        if(trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            Dru11Dru12App_Print("[Error] TR Response not completed!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static void Dru11Dru12App_UdmaEventCb(Udma_EventHandle eventHandle,
                                      uint32_t eventType,
                                      void *appData)
{
    int32_t         retVal;
    CSL_UdmapTdResponse tdResp;

    if(UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        SemaphoreP_post(gUdmaAppDoneSem);
    }

    if(UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&gUdmaChObj, &tdResp);
        if(UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
        }
    }

    return;
}

static int32_t Dru11Dru12App_Init(Udma_DrvHandle drvHandle)
{
    int32_t             retVal;
    Udma_InitPrms       initPrms;
    uint32_t            instId;
    uint32_t            utcId;
    uint32_t            numQueue, queId;
    CSL_DruQueueConfig  queueCfg;

    /* Note: There is no external channel support in MCU NAVSS. So always use
     * main NAVSS even for MCU builds */
    /* UDMA driver init */
    instId = UDMA_INST_ID_MAIN_0;

    /* UDMA driver init*/
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.printFxn = &Dru11Dru12App_Print;

    retVal = Udma_init(drvHandle, &initPrms);
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA init failed!!\n");
    }

    /* Init all DRU queue */
    utcId = UDMA_UTC_ID_MSMC_DRU0;
    numQueue = Udma_druGetNumQueue(drvHandle, utcId);
   
    if(0U == numQueue)
    {
        Dru11Dru12App_Print("[Error] Invalid queue number!!\n");
    }
   
    UdmaDruQueueConfig_init(&queueCfg);
    
    for(queId = CSL_DRU_QUEUE_ID_0; queId < numQueue; queId++)
    {
        retVal = Udma_druQueueConfig(drvHandle, utcId, queId, &queueCfg);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] DRU queue config failed!!\n");
            break;
        }
    }

    return (retVal);
}

static int32_t Dru11Dru12App_Deinit(Udma_DrvHandle drvHandle)
{
    int32_t             retVal;

    retVal = Udma_deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA deinit failed!!\n");
    }

    return (retVal);
}

static int32_t Dru11Dru12App_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChUtcPrms      utcPrms;
    Udma_EventHandle    eventHandle;
    Udma_EventPrms      eventPrms;
    SemaphoreP_Params   semPrms;

    SemaphoreP_Params_init(&semPrms);
    gUdmaAppDoneSem = SemaphoreP_create(0, &semPrms);
    if(NULL == gUdmaAppDoneSem)
    {
        Dru11Dru12App_Print("[Error] Sem create failed!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.utcId                = UDMA_UTC_ID_MSMC_DRU0;
        chPrms.fqRingPrms.ringMem   = &gDruRingMem[0U];
        chPrms.cqRingPrms.ringMem   = &gDruCompRingMem[0U];
        chPrms.tdCqRingPrms.ringMem = &gDruTdCompRingMem[0U];
        chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = UDMA_TEST_APP_RING_ENTRIES;

        /* Open channel for DRU */
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA channel open failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Config UTC channel */
        UdmaChUtcPrms_init(&utcPrms);
        utcPrms.druOwner    = CSL_DRU_OWNER_UDMAC_TR;
        utcPrms.druQueueId  = CSL_DRU_QUEUE_ID_3;
        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA UTC channel config failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register ring completion callback */
        eventHandle = &gUdmaCqEventObj;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = chHandle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        eventPrms.eventCb           = &Dru11Dru12App_UdmaEventCb;
        retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register teardown ring completion callback */
        eventHandle = &gUdmaTdCqEventObj;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = chHandle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        eventPrms.eventCb           = &Dru11Dru12App_UdmaEventCb;
        retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA Teardown CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            Dru11Dru12App_Print("[Error] UDMA channel enable failed!!\n");
        }
    }

    return (retVal);
}

static int32_t Dru11Dru12App_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal, tempRetVal;
    uint64_t            pDesc;
    Udma_EventHandle    eventHandle;

    retVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA channel disable failed!!\n");
    }

    /* Flush any pending request from the free queue */
    while(1)
    {
        tempRetVal = Udma_ringFlushRaw(
                         Udma_chGetFqRingHandle(chHandle), &pDesc);
        if(UDMA_ETIMEOUT == tempRetVal)
        {
            break;
        }
    }

    /* Unregister all events */
    eventHandle = &gUdmaTdCqEventObj;
    retVal += Udma_eventUnRegister(eventHandle);
    eventHandle = &gUdmaCqEventObj;
    retVal += Udma_eventUnRegister(eventHandle);
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA event unregister failed!!\n");
    }

    retVal += Udma_chClose(chHandle);
    if(UDMA_SOK != retVal)
    {
        Dru11Dru12App_Print("[Error] UDMA channel close failed!!\n");
    }

    if(gUdmaAppDoneSem != NULL)
    {
        SemaphoreP_delete(gUdmaAppDoneSem);
        gUdmaAppDoneSem = NULL;
    }

    return (retVal);
}

static void Dru11Dru12App_UdmaTrpdInit(Udma_ChHandle chHandle,
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
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_9, 1U, cqRingNum);

    /* Setup TR */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE)         |
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

static void Dru11Dru12App_Print(const char *str)
{
    UART_printf("%s", str);

    if(UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}

static void Dru11Dru12App_PrintNum(const char *str, uint32_t num)
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


static uint32_t Dru11Dru12App_SwCrc(void *buf, uint32_t length)
{

    int32_t crc = CRC32_INIT;
    uint8_t *p   = (uint8_t *)buf;
    uint32_t originalLength = length;

    /* Writeback source buffer before reading */
    Udma_appUtilsCacheWb(buf, length);

    /* Compute CRC */
    while (length > 0)
    {
        uint8_t idx = (uint8_t)((crc ^ *p++) & 0xFFU);
        crc = (crc >> 8) ^ g_crc32Tbl[idx];
        length--;
    }
    crc ^= CRC32_XOROUT;

    /* Invalidate buffer after computation */
    Udma_appUtilsCacheInv(buf, originalLength);

    Dru11Dru12App_PrintNum("... CRC value: 0x%08x\n", crc);

    return crc;
}

static int32_t Dru11Dru12App_ExtractPayloadAndCrc(Udma_ChHandle chHandle, uint8_t *buf)
{
    int32_t retVal = UDMA_SOK;

    if (UDMA_SOK == retVal)
    {
        /* Pointer to the buffer prepared for UDMA CRC transfer*/
        uint8_t *crcDestBuf = &gCrcBuf[0];
        /* Fill the crcDestBuf with only the payload data from buf */
        memcpy(crcDestBuf, buf, PAYLOAD_SIZE);

        /* Calculate CRC from the destination buffer payload data */
        Dru11Dru12App_Print("Calculate CRC values on destination side: \n");
        uint32_t crcVal;
        crcVal = Dru11Dru12App_SwCrc(crcDestBuf, PAYLOAD_SIZE);

        if (UDMA_SOK == retVal)
        {
            /* Compare the embedded and calculated CRC signature values */
            Dru11Dru12App_Print("Compare embedded and calculated CRC values on destination side: \n");
            /* After the UDMA memcpy, extract CRCs and compare them */
            uint32_t extractedCrcVal = *((uint32_t *)(buf + PAYLOAD_SIZE));
            if (crcVal == extractedCrcVal)
            {
                Dru11Dru12App_Print("...CRC Check Successful. Signature values MATCH!\n");
            }
            else
            {
                Dru11Dru12App_Print("...CRC Check Fail. Signature values DO NOT MATCH!\n");
                retVal = UDMA_EFAIL;
            }
        }
    }

    return retVal;
}

static int32_t Dru11Dru12App_CheckOverrun(uint8_t *buf)
{
    uint32_t i;
    int32_t retVal = UDMA_SOK;
    

    Dru11Dru12App_Print("Check the padding/guard region of the destination buffer for overrun:\n");

    for (i = 0U; i < PADDING_SIZE; i++)
    {
        if (buf[UDMA_TEST_APP_NUM_BYTES + i] != 0xA5U)
        {
            Dru11Dru12App_PrintNum("... Overrun Check Fail. Padding region at +%u bytes ", i);
            Dru11Dru12App_PrintNum("(addr=%p) unexpected!\n", (uint32_t)(uintptr_t)&buf[UDMA_TEST_APP_NUM_BYTES + i]);

            retVal = UDMA_EFAIL;
        }
    }
    
    if (UDMA_SOK == retVal)
    {
        Dru11Dru12App_Print("... Overrun Check Successful. Padding/guard region intact!\n");
    }
    
    return retVal;
}
