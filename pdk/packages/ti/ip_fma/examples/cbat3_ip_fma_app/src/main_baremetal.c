/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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
 *  \file  main_baremetal.c
 *
 *  \brief This application demonstrates the CBA‑T3 safety mechanism on a Cortex‑R5F
 *         by deliberately generating and handling a Data Abort caused by a CBASS
 *         firewall‑protected address region.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/board/board.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/csl/arch/r5/interrupt.h>
#include <sciclient.h>
#include <ti/csl/csl_cbass.h>
#include <ti/csl/src/ip/rat/V0/csl_rat.h>
#include <ti/csl/src/ip/rat/V0/cslr_rat.h>

#include <ip_fma_r5f.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Data Fault Status  Register function */
_DEFINE_COPROCR_READ_FUNC(dfsr, p15, 0, c5, c0, 0)
/**< Data Fault Address Register function */
_DEFINE_COPROCR_READ_FUNC(dfar, p15, 0, c6, c0, 0)

/**< Macros for DFSR bit fields */
#define DFSR_SD_BIT_MASK                (0x1000U)
#define DFSR_SD_BIT_SHIFT               (12U)
#define DFSR_RW_BIT_MASK                (0x800U)
#define DFSR_RW_BIT_SHIFT               (11U)

/**< Firewall definitions */
#define FW_REGION_ENABLE                (0xAU)
/**< Privid of R5F core */
#define FW_MCU_R5F0_PRIVID              (96U)

/**< Local R5F 32-bit address to be translated with RAT into 48-bit address */
#define FWL_TRANSLATED_R5F_START_ADDR   (0x50000000)

/**< Firewall region parameters */
#if defined (SOC_J784S4)
#define FWL_ID                          (CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_MSMC_DDR_0_ECC_AGGR_CFG_MSMC_ECC2_ID)
#define FWL_START_ADDR                  (CSL_COMPUTE_CLUSTER0_VBUSP_MSMC_DDR_0_ECC_AGGR_CFG_MSMC_ECC2_BASE)
#define FWL_END_ADDR                    (CSL_COMPUTE_CLUSTER0_VBUSP_MSMC_DDR_0_ECC_AGGR_CFG_MSMC_ECC2_BASE + CSL_COMPUTE_CLUSTER0_VBUSP_MSMC_DDR_0_ECC_AGGR_CFG_MSMC_ECC2_SIZE)
#elif defined (SOC_J721S2)
#define FWL_ID                          (CSL_STD_FW_COMPUTE_CLUSTER0_MSMC_DDR_ECC_AGGR_1__VBUSP_MSMC_DDR_0_ECC_AGGR_CFG_MSMC_ECC2_ID)
#define FWL_START_ADDR                  (CSL_COMPUTE_CLUSTER0_MSMC_DDR_0_ECC_AGGR2_BASE)
#define FWL_END_ADDR                    (CSL_COMPUTE_CLUSTER0_MSMC_DDR_0_ECC_AGGR2_BASE + CSL_COMPUTE_CLUSTER0_MSMC_DDR_0_ECC_AGGR2_SIZE)
#endif

/**
 *  \brief Data Abort Exception Handler Args
 */
typedef struct
{
    uint32_t cause;                         /**< Cause of the exception, 0 - read, 1 - write. */
    uint32_t type;                          /**< Type of the exception. */
    uint32_t externalAbortCause;            /**< Cause of the external abort(Only valid if the abort is external). */
    uint32_t address;                       /**< Address on which the read/write operation caused an exception. */
    uint32_t auxiliaryFaultStatusReg;       /**< Auxiliary Data Fault Status Register (ADFSR). */
} Cbat3App_DataAbortExptnHandlerArgs;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Structure containing the Exception Handlers. */
extern CSL_R5ExptnHandlers gExptnHandlers;

/* Data abort exception handler arguments. */
static Cbat3App_DataAbortExptnHandlerArgs gDataAbortExptnHandlerArgs;

/* Indicates if the data abort exception was invoked. */
bool gDataAbortExptnInvoked = false;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Registers a custom application-level handler for Data Abort exceptions.
 *
 * By default, the R5F runtime environment provides a weak Data Abort handler
 * that typically enters an infinite loop, providing no diagnostic info.
 * This function overwrites the 'dabtExptnHandler' member within the global
 * CSL_R5ExptnHandlers structure. When a Data Abort occurs, the low-level
 * hardware vector dispatcher will call this registered function instead of
 * the default.
 *
 * \param dataAbortExptnHandler  Pointer to the user-defined exception handler function.
 *
 * \retval  None
 */
static void Cbat3App_UpdateDataAbortExptnHandler(void* dataAbortExptnHandler);

/**
 * \brief Data Abort exception handler for Cortex-R5F.
 *
 * This handler is invoked on a Data Abort exception on the Cortex-R5F core.
 * It reads and decodes the Data Fault Status Register (DFSR) and the Data
 * Fault Address Register (DFAR), and reports the fault details over UART.
 *
 * The function:
 *  - Prints a header indicating that the Data Abort handler has started.
 *  - Reads DFSR and DFAR via IpFma_R5f_read_dfsr() and IpFma_R5f_read_dfar().
 *  - Decodes and prints:
 *      - SD (bit 12 of DFSR): external abort type (AXI Decode vs Slave error).
 *      - RW (bit 11 of DFSR): whether the abort was caused by a read or write.
 *      - STATUS (bits [10,3:0] of DFSR): type of fault (e.g. sync external abort).
 *  - Prints a footer indicating the handler has finished.
 *
 * This function is intended to be registered as the Data Abort exception
 * handler in the R5F vector table. It does not return any status and
 * currently only performs diagnostic reporting (no recovery).
 *
 * \param args Data abort handler arguments.
 */
static void Cbat3App_DataAbortExptnHandler(Cbat3App_DataAbortExptnHandlerArgs* args);

/**
 * \brief Configure RAT for 32‑bit to 48‑bit address translation used by CBA‑T3.
 *
 * This function configures RAT region 0 to translate a 32‑bit R5F‑accessible
 * address range into a 48‑bit system address range. The translated region
 * size and base addresses are taken from the FWL_* macros. If the region is
 * available and not already enabled, the function programs the RAT registers,
 * enables translation for the region, and prints the resulting mapping over
 * UART.
 *
 * The function only operates on RAT region 0 and will fail if:
 * - region 0 does not exist on the RAT instance, or
 * - region 0 translation is already enabled.
 *
 * \note This is a file‑local helper (static) and is not intended to be called
 *       from outside this translation unit.
 *
 * \return
 *  - #CSL_PASS   if the RAT region was configured successfully.
 *  - #CSL_EFAIL  if the region index is invalid, the region is already
 *                enabled, or the RAT configuration API reports failure.
 */
static int32_t Cbat3App_RatConfig();

/**
 * \brief Configure and enable a firewall region to block access.
 *
 * This function programs a CBASS firewall region via the DMSC TISCI
 * interface so that the specified address range is blocked (no
 * permissions granted). It sets the region control to enabled and
 * clears all permission registers for the given firewall and region.
 *
 * The request is sent using Sciclient_service() and the result is
 * indicated by the return value. On a successful TISCI ACK, a message
 * is printed via UART.
 *
 * \param fwl_id     Firewall instance ID to configure.
 * \param region     Region index within the specified firewall.
 * \param start_addr Start address of the region to block (inclusive).
 * \param end_addr   End address of the region to block (inclusive).
 *
 * \return
 *  - #CSL_PASS if the Sciclient request completed and was ACKed.
 *  - An error code (e.g. #CSL_EFAIL) if the request failed or was NACKed.
 */
static int32_t Cbat3App_FirewallBlock(uint16_t fwl_id,
                                      uint16_t region,
                                      uint64_t start_addr,
                                      uint64_t end_addr);

/**
 * \brief Configure and enable a firewall region to allow R5F access.
 *
 * This function programs a CBASS firewall region via the DMSC TISCI
 * interface so that the specified address range is accessible with full
 * permissions for the R5F master identified by FW_MCU_R5F0_PRIVID.
 * All three permission registers are populated to permit secure and
 * non-secure, supervisor and user read/write/debug access for this
 * master. The region control is set to enabled.
 *
 * The request is sent using Sciclient_service() and the result is
 * indicated by the return value. On a successful TISCI ACK, a message
 * is printed via UART.
 *
 * \param fwl_id     Firewall instance ID to configure.
 * \param region     Region index within the specified firewall.
 * \param start_addr Start address of the region to unblock (inclusive).
 * \param end_addr   End address of the region to unblock (inclusive).
 *
 * \return
 *  - #CSL_PASS if the Sciclient request completed and was ACKed.
 *  - An error code (e.g. #CSL_EFAIL) if the request failed or was NACKed.
 */
static int32_t Cbat3App_FirewallUnblock(uint16_t fwl_id,
                                        uint16_t region,
                                        uint64_t start_addr,
                                        uint64_t end_addr);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void Cbat3App_UpdateDataAbortExptnHandler(void* dataAbortExptnHandler)
{
    gExptnHandlers.dabtExptnHandler = dataAbortExptnHandler;
    gExptnHandlers.dabtExptnHandlerArgs = &gDataAbortExptnHandlerArgs;
}

static void Cbat3App_DataAbortExptnHandler(Cbat3App_DataAbortExptnHandlerArgs* args)
{
    // Read Data Fault Status Register
    uint32_t dfsrReg = IpFma_R5f_read_dfsr();
    // Read Data Fault Address Register
    uint32_t dfarReg = (uint32_t) IpFma_R5f_read_dfar();

    args->address = dfarReg;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bit [12] - SD (External abort type)
     * Distinguishes between an AXI Decode or Slave error on an external abort.
     * This bit is only valid for external aborts. For  all other aborts types
     * of abort, this bit is set to zero:
     *  0 = AXI Decode error (DECERR) or AHB  error caused the abort
     *  1 = AXI Slave  error (SLVERR) or unsupported exclusive access
     */
    uint8_t sd = (dfsrReg & DFSR_SD_BIT_MASK) >> DFSR_SD_BIT_SHIFT;
    args->externalAbortCause = sd;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bit [11] - RW
     * Indicates whether a read or write access caused an abort:
     *  0 = read  access caused the abort
     *  1 = write access caused the abort
     */
    uint8_t rw = (dfsrReg & DFSR_RW_BIT_MASK) >> DFSR_RW_BIT_SHIFT;
    args->cause = rw;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bits [10, 3:0] - STATUS
     * Indicates the type of fault generated
     *
     * -------------------------------------------------------------------------
     * Fault Type                              FSR[10,3:0]    FAR
     * -------------------------------------------------------------------------
     * Alignment                                0b00001        Valid
     *
     * Background                               0b00000        Valid
     *
     * Permission                               0b01101        Valid
     *
     * Synchronous External Abort               0b01000        Valid
     *
     * Asynchronous External Abort              0b10110        Unpredictable
     *
     * Synchronous Parity or ECC Error          0b11001        Valid
     *
     * Asynchronous Parity or ECC Error         0b11000        Unpredictable
     *
     * Debug Event                              0b00010        Unchanged
     * -------------------------------------------------------------------------
     */
    uint8_t status = ((dfsrReg >> 6) & 0x10) | (dfsrReg & 0x0F);
    args->type = status;

    /* Flag that a Data Abort exception has occurred */
    gDataAbortExptnInvoked = true;
}


static int32_t Cbat3App_RatConfig()
{
    bool statusTranslate = (bool)false;
    int32_t retVal = CSL_PASS;
    uint32_t ratRegion0   = 0;

    /**
    * The RAT module performs a region based address translation.
    * It translates a 32-bit input address into a 48-bit output address.
    */

    CSL_ratRegs *pRatRegs = (CSL_ratRegs *)CSL_MCU_R5FSS0_RAT_CFG_BASE;
    CSL_RatTranslationCfgInfo translationCfg;

    translationCfg.sizeInBytes       = FWL_END_ADDR - FWL_START_ADDR;       // size of the translated region
    translationCfg.baseAddress       = FWL_TRANSLATED_R5F_START_ADDR;       // 32-bit local base address
    translationCfg.translatedAddress = FWL_START_ADDR;                      // 48-bit base address 0x004d 0000 0800

    if (ratRegion0 < CSL_ratGetMaxRegions(pRatRegs))
    {
        if (CSL_ratIsRegionTranslationEnabled(pRatRegs, ratRegion0) == (bool)false)
        {
            CSL_ratEnableRegionTranslation(pRatRegs, ratRegion0);
            /* Set up RAT translation */
            statusTranslate = CSL_ratConfigRegionTranslation(pRatRegs, ratRegion0, &translationCfg);
        }
        else
        {
            retVal = CSL_EFAIL;
        }

        if ((bool)true == statusTranslate)
        {
            UART_printf("\n[CBA-T3 APP]: Successfully configured the RAT for 48-bit address 0x%08x%08x and R5F accessible 32-bit address 0x%08x \n", \
                        (uint32_t)(translationCfg.translatedAddress >> 32), (uint32_t)translationCfg.translatedAddress, translationCfg.baseAddress);
        }
    }
    else
    {
        retVal = CSL_EFAIL;
        UART_printf("\n ERROR: Configuration failed!!");
    }

    return retVal;
}

static int32_t Cbat3App_FirewallBlock(uint16_t fwl_id,
                                      uint16_t region,
                                      uint64_t start_addr,
                                      uint64_t end_addr)
{
    int retVal = CSL_PASS;

    struct tisci_msg_fwl_set_firewall_region_req request = {0};

    Sciclient_ReqPrm_t  reqParam    = {0};
    Sciclient_RespPrm_t respParam   = {0};

    request.fwl_id              = fwl_id;
    request.region              = region;
    request.n_permission_regs   = CSL_FW_NUM_CBASS_FW_EP_REGION_PERMISSION;
    request.control             = (FW_REGION_ENABLE & CSL_CBASS_ISC_EP_REGION_CONTROL_ENABLE_MASK);
    request.permissions[0]      = 0;
    request.permissions[1]      = 0;
    request.permissions[2]      = 0;
    request.start_address       = start_addr;
    request.end_address         = end_addr;

    reqParam.messageType    = (uint16_t) TISCI_MSG_SET_FWL_REGION;
    reqParam.flags          = (uint32_t) TISCI_MSG_FLAG_AOP | TISCI_MSG_FLAG_DEVICE_EXCLUSIVE;
    reqParam.pReqPayload    = (const uint8_t *) &request;
    reqParam.reqPayloadSize = (uint32_t) sizeof(request);
    reqParam.timeout        = SCICLIENT_SERVICE_WAIT_FOREVER;

    respParam.flags             = (uint32_t) 0; /* Populated by the API */
    respParam.pRespPayload      = (uint8_t *) 0;
    respParam.respPayloadSize   = (uint32_t) 0;

    retVal = Sciclient_service(&reqParam, &respParam);
    if ((CSL_PASS == retVal) && (respParam.flags == TISCI_MSG_FLAG_ACK))
    {
        UART_printf("Firewall sciclient success!\n");
    }

    return retVal;
}

static int32_t Cbat3App_FirewallUnblock(uint16_t fwl_id,
                                        uint16_t region,
                                        uint64_t start_addr,
                                        uint64_t end_addr)
{
    int retVal = CSL_PASS;

    struct tisci_msg_fwl_set_firewall_region_req request = {0};

    Sciclient_ReqPrm_t  reqParam    = {0};
    Sciclient_RespPrm_t respParam   = {0};

    request.fwl_id              = fwl_id;
    request.region              = region;
    request.n_permission_regs   = CSL_FW_NUM_CBASS_FW_EP_REGION_PERMISSION;
    request.control             = (FW_REGION_ENABLE & CSL_CBASS_ISC_EP_REGION_CONTROL_ENABLE_MASK);
    request.permissions[0]      = (FW_MCU_R5F0_PRIVID << CSL_CBASS_FW_EP_REGION_PERMISSION_PRIV_ID_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_DEBUG_SHIFT);
    request.permissions[1]      = (FW_MCU_R5F0_PRIVID << CSL_CBASS_FW_EP_REGION_PERMISSION_PRIV_ID_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_DEBUG_SHIFT);
    request.permissions[2]      = (FW_MCU_R5F0_PRIVID << CSL_CBASS_FW_EP_REGION_PERMISSION_PRIV_ID_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_SEC_USER_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_SUPV_DEBUG_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_WRITE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_READ_SHIFT)
                                    | (0U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_CACHEABLE_SHIFT)
                                    | (1U << CSL_CBASS_FW_EP_REGION_PERMISSION_NONSEC_USER_DEBUG_SHIFT);
    request.start_address       = start_addr;
    request.end_address         = end_addr;

    reqParam.messageType    = (uint16_t) TISCI_MSG_SET_FWL_REGION;
    reqParam.flags          = (uint32_t) TISCI_MSG_FLAG_AOP | TISCI_MSG_FLAG_DEVICE_EXCLUSIVE;
    reqParam.pReqPayload    = (const uint8_t *) &request;
    reqParam.reqPayloadSize = (uint32_t) sizeof(request);
    reqParam.timeout        = SCICLIENT_SERVICE_WAIT_FOREVER;

    respParam.flags             = (uint32_t) 0; /* Populated by the API */
    respParam.pRespPayload      = (uint8_t *) 0;
    respParam.respPayloadSize   = (uint32_t) 0;

    retVal = Sciclient_service(&reqParam, &respParam);
    if ((CSL_PASS == retVal) && (respParam.flags == TISCI_MSG_FLAG_ACK))
    {
        UART_printf("Firewall sciclient success!\n");
    }

    return retVal;
}


/*
 * Application main.
 */
 int main(void)
 {
    int status = CSL_PASS;

    /* Board initialization */
    Board_initCfg boardCfg = BOARD_INIT_UART_STDIO;

    if (BOARD_SOK == Board_init(boardCfg))
    {

        status = Cbat3App_RatConfig();

        if (CSL_PASS == status)
        {
            /* Register a custom Data Abort exception handler */
            Cbat3App_UpdateDataAbortExptnHandler((exptnHandlerPtr) Cbat3App_DataAbortExptnHandler);

            status = Cbat3App_FirewallBlock(FWL_ID,
                                            1U,
                                            FWL_START_ADDR,
                                            FWL_END_ADDR);
            UART_printf("\nFirewall now blocked\n");
        }

        if (CSL_PASS == status)
        {
            uint32_t regVal;

            UART_printf("Try to access register from the firewall protected region: \n");
            /* This will cause the exception to happen */
            regVal = CSL_REG32_RD(FWL_TRANSLATED_R5F_START_ADDR);

            bool exceptionInvokedForValidReasons = true;
            if (true == gDataAbortExptnInvoked)
            {
                if (FWL_TRANSLATED_R5F_START_ADDR == gDataAbortExptnHandlerArgs.address)
                {
                    UART_printf("\n Data abort exception was invoked on address %x!", gDataAbortExptnHandlerArgs.address);
                }
                else
                {
                    UART_printf("\n ERROR: Data abort exception was not invoked on correct address %x!", gDataAbortExptnHandlerArgs.address);
                    exceptionInvokedForValidReasons = false;
                }

                if (0x08 == gDataAbortExptnHandlerArgs.type)
                {
                    UART_printf("\n Data abort exception was invoked due to Synchronous External Abort");
                }
                else
                {
                    UART_printf("\n ERROR: Data abort exception was not invoked due to Synchronous External Abort!");
                    exceptionInvokedForValidReasons = false;
                }

                if (0U == gDataAbortExptnHandlerArgs.cause)
                {
                    UART_printf("\n Data abort exception was invoked due to read operation");
                }
                else
                {
                    UART_printf("\n ERROR: Data abort exception was not invoked due to read operation!");
                    exceptionInvokedForValidReasons = false;
                }

                if (0 == gDataAbortExptnHandlerArgs.externalAbortCause)
                {
                    UART_printf("\n AXI Decode error (DECERR) or AHB  error caused the abort\n");
                    exceptionInvokedForValidReasons = false;
                }
                else
                {
                    UART_printf("\n ERROR: AXI Slave  error (SLVERR) or unsupported exclusive access\n");
                }
            }
            else
            {
                UART_printf("\n ERROR: Exception was not invoked!!");
            }

            status = Cbat3App_FirewallUnblock(FWL_ID,
                                              1U,
                                              FWL_START_ADDR,
                                              FWL_END_ADDR);

            if (CSL_PASS == status)
            {
                UART_printf("\nFirewall now unblocked\n\n");

                UART_printf("Try reading some registers from the firewall protected address region: \n");
                regVal = CSL_REG32_RD(FWL_TRANSLATED_R5F_START_ADDR);
                UART_printf("-- Read register at %08x - value: %08x\n", FWL_TRANSLATED_R5F_START_ADDR, regVal);

                /* If firewall is unblocked succesfully and exception was called due to correct options. */
                if ((true == gDataAbortExptnInvoked) && (true == exceptionInvokedForValidReasons))
                {
                    UART_printf("All tests have passed!!\n");
                }
            }
            else
            {
                UART_printf("\nFirewall unblock failed!\n\n");
            }
        }
        else
        {
            UART_printf("\n ERROR: Firewall was not blocked!!");
        }
    }

    return status;
}
