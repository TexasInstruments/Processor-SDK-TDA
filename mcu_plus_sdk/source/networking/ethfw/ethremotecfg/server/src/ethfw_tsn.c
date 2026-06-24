/*
 *
 * Copyright (c) 2024-2025 Texas Instruments Incorporated
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

/*!
 *  \file ethfw_tsn.c
 *
 *  \brief This file contains the TSN and gPTP implementation of ETHFW
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x108

/* TSN header files */
#if defined(MCU_PLUS_SDK)
#include <tsn_buildconf/sitara_buildconf.h>
#else
#include <tsn_buildconf/jacinto_buildconf.h>
#endif
#include <tsn_gptp/tilld/lld_gptp_private.h>
#include <tsn_uniconf/yangs/yang_db_runtime.h>
#include <tsn_gptp/gptpconf/gptpgcfg.h>
#include <tsn_uniconf/yangs/ieee1588-ptp-tt_access.h>
#include <tsn_gptp/gptpconf/xl4-extmod-xl4gptp.h>
#include <tsn_uniconf/uc_dbal.h>
#include <tsn_uniconf/ucman.h>
#include <tsn_gptp/gptpman.h>
#include <tsn_unibase/unibase_binding.h>

#if defined(AVTP_ENABLED) && defined(SOC_AM62DX)
#include <sitara/ethfw_avtp.h>
#endif

/* Enet LLD header files */
#include <enet.h>
#include <utils/include/enet_apputils.h>
#include <include/mod/cpsw_cpts.h>

/* EthFw header files */
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <ethremotecfg/server/include/ethfw_tsn.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! Logging task - very low priority so it doesn't interfere with gPTP stack */
#define ETHFW_TSN_LOGGER_TASK_PRIORITY                              (1U)

/*! gPTP stack priority - should be higher than other non-critical tasks which could interfere */
#define ETHFW_TSN_GPTP_TASK_PRIORITY                                (2U)

/*! gPTP Task name */
#define ETHFW_TSN_GPTP_TASK_NAME                                    "gPTP Task"

/*! TSN stack size and alignment */
#define ETHFW_TSN_TASK_STACK_SIZE                                   (16U * 1024U)
#define ETHFW_TSN_TASK_STACK_ALIGN                                  (32U)

/*! Uniconf stack priority */
#define ETHFW_TSN_UC_TASK_PRIORITY                                  (2U)

/*! Uniconf Task name */
#define ETHFW_TSN_UC_TASK_NAME                                      "Uniconf Task"

/*! Uniconf stack size and alignment */
#define ETHFW_TSN_UC_TASK_STACK_SIZE                                (16U * 1024U)
#define ETHFW_TSN_UC_TASK_STACK_ALIGN                               (32U)

/*! Log task's buffer stack size and alignment */
#define ETHFW_TSN_LOGGER_TASK_STACK_SIZE                            (2U * 1024U)
#define ETHFW_TSN_LOGGER_TASK_STACK_ALIGN                           (32U)

/*! Size of the buffer used for storing logs before printing */
#define ETHFW_TSN_BUFFER_SIZE                                       (8960U)

/*! Max size of the TSN log print buffer length */
#define ETHFW_TSN_TRACE_MAX_BUFFER_SIZE                             (250U)

#define MAX_KEY_SIZE                                                (32U)
#define MAX_BUFFER_SIZE                                             (256U)

#if defined(ETHFW_EST_DEMO_SUPPORT)
#define ETHFW_TSN_EST_TASK_PRIORITY                                 (1U)
#define ETHFW_TSN_EST_TASK_NAME                                     "EthFw EST demo Task"

#define ETHFW_TSN_EST_STACK_SIZE                                    (16U * 1024U)
#define ETHFW_TSN_EST_STACK_ALIGN                                   (32U)
#endif

/* Macro to round off time to second boundary */
#define ETHFW_PPS_TIMESYNC_TIME_SEC_TO_NS                           (1000000000U)

/* Macro to start GenF compare time in future - 1sec */
#define ETHFW_PPS_TIMESYNC_COMP_TIME_NS                             (1000000000U)

/* Log Buffer end line macro */
#define ETHFW_LOG_BUFFER_ENDLINE                                    "\r"

#define IEEE1588_PTP_TT_RW_Y IEEE1588_PTP_TT_func(ydbia->dbald)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* Container structure for database name and value pairs for yang configurations */
typedef struct EthFwTsn_DbNameVal_s
{
    char *name;
    char *val;
} EthFwTsn_DbNameVal;

/* Container structure for database name and value pairs, for non-yang configurations */
typedef struct EthFwTsn_DbIntVal_s
{
    char *name;
    int item;
    int val;
} EthFwTsn_DbIntVal;

typedef struct EthFwTsn_dbArgs_s
{
    uc_dbald *dbald;
    uc_notice_data_t *ucntd;
} EthFwTsn_dbArgs;

typedef int32_t (*EthFwTsn_OnModuleDBInit)(EthFwTsn_dbArgs *dbArgs);

typedef void *(*EthFwTsn_OnModuleStart)(void *arg1);

typedef struct EthFwTsn_GptpOpt_s
{
    char *devlist;
    const char **confFiles;
    uint32_t domainNum;
    uint32_t domains[GPTP_MAX_DOMAINS];
    uint32_t instNum;
    uint32_t numConf;
} EthFwTsn_GptpOpt;

/* Container structure for Uniconf config params */
typedef struct EthFwTsn_UniconfCfg_s
{
    char *dbName;
    ucman_data_t ucData;
    UC_NOTICE_SIG_T ucReadySem;
    bool dbInitFlag;
} EthFwTsn_UniconfCfg;

/* Container structure for TSN Modules config params */
typedef struct EthFwTsn_ModuleCfg_s
{
    bool stopFlag;
    uint32_t taskPriority;
    CB_THREAD_T hTaskHandle;
    const char *taskName;
    uint8_t *stackBuffer;
    uint32_t stackSize;
    EthFwTsn_OnModuleDBInit onModuleDBInit;
    EthFwTsn_OnModuleStart onModuleRunner;
    void *onModuleRunnerArgs;
    bool enable;
} EthFwTsn_ModuleCfg;

/*
 * \brief Structure holding gPTP and TSN configs
 */
typedef struct EthFwTsn_Obj_s
{
    /* Enet driver handle for this peripheral type/instance */
    Enet_Handle hEnet;

    /* Processing core Id */
    uint32_t coreId;

    /* Enet instance type */
    uint32_t enetType;

    /* Enet instance id */
    uint32_t instId;

    /* Whether TSN stack has been initialized or not */
    bool tsnInit;

    /* To run log Task in the loop*/
    bool logTaskrun;

    /* TSN stack netdev info */
    EthFwTsn_NetDevInfo netDevInfo;

    /* Mutex handle used for TSN stack logging */
    EthFwOsal_MutexHandle hLogMutex;

    /* TSN stack logging task handle */
    EthFwOsal_TaskHandle hLogTask;

    /* TSN logger task stack buffer */
    uint8_t logTaskStackBuf[ETHFW_TSN_LOGGER_TASK_STACK_SIZE] __attribute__ ((aligned(ETHFW_TSN_LOGGER_TASK_STACK_ALIGN)));

    /* Buffer used to accumulate the log messages given by stack until 'log_task' flushes them */
    uint8_t logBuf[ETHFW_TSN_BUFFER_SIZE];

    /* Buffer used to store string to be printed via app's print function */
    uint8_t printBuf[ETHFW_TSN_BUFFER_SIZE];

    /* Trace max buffer */
    char traceBuf[ETHFW_TSN_TRACE_MAX_BUFFER_SIZE];

    /* gPTP config callback from app */
    EthFwTsn_configPtpCb configPtpCb;

    /* gPTP config callback argument */
    void *configPtpCbArg;

    /* Uniconf Module Config */
    EthFwTsn_UniconfCfg ucCfg;
} EthFwTsn_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t EthFwTsn_logBuffer(bool flush, const char *str);

static void EthFwTsn_startLogTask(void);

/* Can be read from cfg files, or in case of no db file is specified, init runtime config */
static int32_t EthFwTsn_initDb(EthFwTsn_UniconfCfg *ucCfg);

static int32_t EthFwTsn_startModTask(EthFwTsn_ModuleCfg *modCfg, uint32_t moduleIdx);

static int32_t EthFwTsn_startMod(void);

static int32_t EthFwTsn_uniconfInit(EthFwTsn_dbArgs *dbArgs);

static int32_t EthFwTsn_gptpDbInit(EthFwTsn_dbArgs *dbArgs);

static void *EthFwTsn_gptpTask(void *args);

static void *EthFwTsn_uniconfTask(void *args);

static int32_t EthFwTsn_gptpNonYangConfig(uint8_t instance);

static int32_t EthFwTsn_gptpYangConfig(uint32_t instance,
                                       uint32_t domain);

static void EthFwTsn_cfgGptpDefaultDs(uint32_t instance,
                                      uint32_t domain,
                                      bool dbInitFlag);

static void EthFwTsn_cfgGptpPortDs(uint32_t instance,
                                   uint32_t domain,
                                   int portIndex,
                                   bool dbInitFlag);

static uint64_t EthFwTsn_getCurrentTime(void);

static int32_t EthFwTsn_genPPS(EthFwTsn_PpsConfig *ppsConfig);

static void EthFwTsn_gptpUpdateDomainMap(int instance, int domain);

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

extern uint8_t IEEE1588_PTP_TT_func(uc_dbald *dbald);

#if defined(ETHFW_EST_DEMO_SUPPORT)
extern void *EthFw_estDemoTask(void *arg1);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

EthFwTsn_Obj gEthFwTsnObj;

/* Uniconf Stack buffer */
uint8_t gUniconfStackBuf[ETHFW_TSN_UC_TASK_STACK_SIZE] __attribute__ ((aligned(ETHFW_TSN_UC_TASK_STACK_ALIGN)));

/* gPTP task stack buffer */
uint8_t gPtpStackBuf[ETHFW_TSN_TASK_STACK_SIZE] __attribute__ ((aligned(ETHFW_TSN_TASK_STACK_ALIGN)));

#if defined(ETHFW_EST_DEMO_SUPPORT)
static uint8_t gEthFwEstDemoStackBuf[ETHFW_TSN_EST_STACK_SIZE]__attribute__ ((aligned(ETHFW_TSN_EST_STACK_ALIGN)));
#endif

typedef struct EthFwTsn_DbKeyVal_IntItem
{
    uint8_t key;
    uint32_t val;
    uint8_t sz;   // value size 1 (bool) or 4 (uint32_t)
    bool rw; // true: rw, false: ro
} EthFwTsn_DbKeyVal_IntItem_t;

/* Default values for gptp port data set */
static EthFwTsn_DbKeyVal_IntItem_t gGptpPortDsInt[] =
{
    // rw
    {IEEE1588_PTP_TT_LOG_ANNOUNCE_INTERVAL, 0, sizeof(uint32_t),true},
    {IEEE1588_PTP_TT_GPTP_CAP_RECEIPT_TIMEOUT, 3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_ANNOUNCE_RECEIPT_TIMEOUT, 3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_SYNC_RECEIPT_TIMEOUT, 3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_INITIAL_LOG_ANNOUNCE_INTERVAL, 0, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_INITIAL_LOG_SYNC_INTERVAL, -3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_INITIAL_LOG_PDELAY_REQ_INTERVAL, 0, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_INITIAL_LOG_GPTP_CAP_INTERVAL, 3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_MGT_LOG_GPTP_CAP_INTERVAL, 3, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_ALLOWED_LOST_RESPONSES, 9, sizeof(uint32_t), true}, // 802.1 2020 default config
    //{IEEE1588_PTP_TT_ALLOWED_LOST_RESPONSES, 3, sizeof(uint32_t), true}, // 2011-backward compatible config
    {IEEE1588_PTP_TT_ALLOWED_FAULTS, 9, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_MEAN_LINK_DELAY_THRESH, 0x27100000, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_PORT_ENABLE, true, 1, true}, // bool
    // ro
    {IEEE1588_PTP_TT_LOG_SYNC_INTERVAL, -3, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_MINOR_VERSION_NUMBER, 1, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_CURRENT_LOG_SYNC_INTERVAL, -3, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_CURRENT_LOG_GPTP_CAP_INTERVAL, 3, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_CURRENT_LOG_PDELAY_REQ_INTERVAL, 0, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_INITIAL_ONE_STEP_TX_OPER, 1, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_CURRENT_ONE_STEP_TX_OPER, 1, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_MGT_ONE_STEP_TX_OPER, 1, sizeof(uint32_t), false},
    {IEEE1588_PTP_TT_USE_MGT_LOG_GPTP_CAP_INTERVAL, false, 1, false}, // bool
    {IEEE1588_PTP_TT_USE_MGT_ONE_STEP_TX_OPER, false, 1, false}, // bool

};

/* Default values for gptp default data set */
static EthFwTsn_DbKeyVal_IntItem_t gGptpDefaultDsInt[] =
{
    // rw
    {IEEE1588_PTP_TT_PRIORITY1, 248, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_PRIORITY2, 248, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_EXTERNAL_PORT_CONFIG_ENABLE, false, 1, true},
    // ro
    {IEEE1588_PTP_TT_TIME_SOURCE, 0xA0, sizeof(uint32_t), false}, // "internal-oscillator"
    {IEEE1588_PTP_TT_PTP_TIMESCALE, true, 1, false}, // "internal-oscillator"
};

static EthFwTsn_DbKeyVal_IntItem_t gGptpDefaultDsClkQuality[] =
{
    {IEEE1588_PTP_TT_CLOCK_CLASS, 248, sizeof(uint32_t), true}, // "cc-default"
    {IEEE1588_PTP_TT_CLOCK_ACCURACY, 0x22, sizeof(uint32_t), true}, //"ca-time-accurate-to-250-ns"
    {IEEE1588_PTP_TT_OFFSET_SCALED_LOG_VARIANCE, 0x436a, sizeof(uint32_t), true},
};

static EthFwTsn_DbKeyVal_IntItem_t gGptpTsCorrectionPortDs[] =
{
    {IEEE1588_PTP_TT_INGRESS_LATENCY, 0x00, sizeof(uint32_t), true},
    {IEEE1588_PTP_TT_EGRESS_LATENCY, 0x00, sizeof(uint32_t), true},
};

/* Default values for gptp non-yang data set */
static EthFwTsn_DbIntVal gGptpNonYangDs[] =
{
    {"SINGLE_CLOCK_MODE", XL4_EXTMOD_XL4GPTP_SINGLE_CLOCK_MODE, 1},
    {"USE_HW_PHASE_ADJUSTMENT", XL4_EXTMOD_XL4GPTP_USE_HW_PHASE_ADJUSTMENT, 1},
    {"CLOCK_COMPUTE_INTERVAL_MSEC", XL4_EXTMOD_XL4GPTP_CLOCK_COMPUTE_INTERVAL_MSEC, 100},
    {"FREQ_OFFSET_IIR_ALPHA_START_VALUE", XL4_EXTMOD_XL4GPTP_FREQ_OFFSET_IIR_ALPHA_START_VALUE, 1},
    {"FREQ_OFFSET_IIR_ALPHA_STABLE_VALUE", XL4_EXTMOD_XL4GPTP_FREQ_OFFSET_IIR_ALPHA_STABLE_VALUE, 4},
    {"PHASE_OFFSET_IIR_ALPHA_START_VALUE", XL4_EXTMOD_XL4GPTP_PHASE_OFFSET_IIR_ALPHA_START_VALUE, 1},
    {"PHASE_OFFSET_IIR_ALPHA_STABLE_VALUE", XL4_EXTMOD_XL4GPTP_PHASE_OFFSET_IIR_ALPHA_STABLE_VALUE, 4},
    {"MAX_DOMAIN_NUMBER", XL4_EXTMOD_XL4GPTP_MAX_DOMAIN_NUMBER, GPTP_MAX_DOMAINS},
#ifdef GPTP_QUICKSYNC
    {"QUICK_SYNC_ALGO", XL4_EXTMOD_XL4GPTP_QUICK_SYNC_ALGO, 1},
    {"SKIP_FREQADJ_COUNT_MAX", XL4_EXTMOD_XL4GPTP_SKIP_FREQADJ_COUNT_MAX, 2},
    {"PHASE_OFFSET_ADJUST_BY_FREQ", XL4_EXTMOD_XL4GPTP_PHASE_OFFSET_ADJUST_BY_FREQ, 500},
    {"FREQ_OFFSET_STABLE_PPB", XL4_EXTMOD_XL4GPTP_FREQ_OFFSET_STABLE_PPB, 500},
    {"FREQ_OFFSET_UPDATE_MRATE_PPB", XL4_EXTMOD_XL4GPTP_FREQ_OFFSET_UPDATE_MRATE_PPB, 5},

#ifdef GPTP_MASTER
        {"STATIC_PORT_STATE_SLAVE_PORT", XL4_EXTMOD_XL4GPTP_STATIC_PORT_STATE_SLAVE_PORT, 0}, // master in disable BMCA
#else
        {"SUPPORT_RUNTIME_NOTICE_CHECK", XL4_EXTMOD_XL4GPTP_SUPPORT_RUNTIME_NOTICE_CHECK, 1}, // only slave can trigger signaling
        {"STATIC_PORT_STATE_SLAVE_PORT", XL4_EXTMOD_XL4GPTP_STATIC_PORT_STATE_SLAVE_PORT, 1}, // slave in disable BMCA
#endif
    {"CLOCK_COMPUTE_INTERVAL_MSEC", XL4_EXTMOD_XL4GPTP_CLOCK_COMPUTE_INTERVAL_MSEC, 20},
#else
    {"SKIP_FREQADJ_COUNT_MAX", XL4_EXTMOD_XL4GPTP_SKIP_FREQADJ_COUNT_MAX, 0},
    {"CLOCK_COMPUTE_INTERVAL_MSEC", XL4_EXTMOD_XL4GPTP_CLOCK_COMPUTE_INTERVAL_MSEC, 100},
    // If the phase offset between the GM and the local clock exceeds the threshold PHASE_OFFSET_ADJUST_BY_FREQ,
    // phase offset will be applied; otherwise, the clock rate is adjusted to match the phase.
    #ifdef ENET_ENABLE_PER_ICSSG
    /* Increase this value to 10E6 for ICSSG because Nudge API is not supported. */
    {"PHASE_OFFSET_ADJUST_BY_FREQ", XL4_EXTMOD_XL4GPTP_PHASE_OFFSET_ADJUST_BY_FREQ, 1000000}, // in ns
    #else
    {"PHASE_OFFSET_ADJUST_BY_FREQ", XL4_EXTMOD_XL4GPTP_PHASE_OFFSET_ADJUST_BY_FREQ, 100000}, // in ns
    #endif

#endif
#if GPTP_MAX_DOMAINS == 2
    {"CMLDS_MODE", XL4_EXTMOD_XL4GPTP_CMLDS_MODE, 1},
    {"SECOND_DOMAIN_THIS_CLOCK", XL4_EXTMOD_XL4GPTP_SECOND_DOMAIN_THIS_CLOCK, 1}
#endif
};

EthFwTsn_ModuleCfg gModCfgTable[ETHFWTSN_MAX_TASK_IDX] =
{
        [ETHFWTSN_GPTP_TASK_IDX] =
        {
            .enable = BFALSE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_GPTP_TASK_PRIORITY,
            .taskName = ETHFW_TSN_GPTP_TASK_NAME,
            .stackBuffer = gPtpStackBuf,
            .stackSize = sizeof(gPtpStackBuf),
            .onModuleDBInit = EthFwTsn_gptpDbInit,
            .onModuleRunner = EthFwTsn_gptpTask,
            .onModuleRunnerArgs = NULL,
        },
        [ETHFWTSN_UNICONF_TASK_IDX] =
        {
            .enable = BFALSE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_UC_TASK_PRIORITY,
            .taskName = ETHFW_TSN_UC_TASK_NAME,
            .stackBuffer = gUniconfStackBuf,
            .stackSize = sizeof(gUniconfStackBuf),
            .onModuleDBInit = EthFwTsn_uniconfInit,
            .onModuleRunner = EthFwTsn_uniconfTask,
            .onModuleRunnerArgs = NULL,
        },
#if (AVTP_ENABLED)
        [ETHFWTSN_AVTPD_TASK_IDX]={
            .enable = BTRUE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_AVTPD_TASK_PRIORITY,
            .taskName = "avtpd_task",
            .stackBuffer = gAvtpdStackBuf,
            .stackSize = sizeof(gAvtpdStackBuf),
            .onModuleDBInit = NULL,
            .onModuleRunner = EnetApp_avtpdTask,
        },
        /* Autoamp Apps */
        /* TX apps */
        [ETHFWTSN_AAF_AUTOAMP_APP_TX_CLASSA_TASK_IDX]={
            .enable = BFALSE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_AUTOAMP_APP_TASK_PRIORITY,
            .taskName = "autoAmpApp_TxTask",
            .stackBuffer = gTxStackBuf,
            .stackSize = sizeof(gTxStackBuf),
            .onModuleDBInit = EnetApp_autoAmpAppInit,
            .onModuleRunner = EnetApp_talkerTask,
        },
        /* RX apps */
        [ETHFWTSN_AUTOAMP_APP_RX_TASK_IDX]={
            .enable = BFALSE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_AUTOAMP_APP_RX_TASK_PRIORITY,
            .taskName = "autoAmpApp_Rx",
            .stackBuffer = gRx1StackBuf,
            .stackSize = sizeof(gRx1StackBuf),
            .onModuleDBInit = NULL,
            .onModuleRunner = EnetApp_ListenerTask,
        },
#endif
#if defined(ETHFW_EST_DEMO_SUPPORT)
        [ETHFWTSN_EST_TASK_IDX] =
        {
            .enable = BFALSE,
            .stopFlag = BTRUE,
            .taskPriority = ETHFW_TSN_EST_TASK_PRIORITY,
            .taskName = ETHFW_TSN_EST_TASK_NAME,
            .stackBuffer = gEthFwEstDemoStackBuf,
            .stackSize = sizeof(gEthFwEstDemoStackBuf),
            .onModuleDBInit = NULL,
            .onModuleRunner = EthFw_estDemoTask,
            .onModuleRunnerArgs = &gEthFwTsnObj.netDevInfo,
        },
#endif
};

/* Max domains supported is defined as GPTP_MAX_DOMAINS in jacinto_buildconf.h.
   Please refer to the same for more details */
static EthFwTsn_GptpOpt gGptpOpt =
{
    .confFiles = NULL,
    .domainNum = GPTP_MAX_DOMAINS,
#if GPTP_MAX_DOMAINS == 1U
    .domains = {0},
#elif GPTP_MAX_DOMAINS == 2U
    .domains = {0, 1},
#else
    #error "Only support 2 domains"
#endif
    .instNum = 0U,
    .numConf = 0U,
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void EthFwTsn_logTask(void *a0)
{
    int32_t len;
    int32_t maxLen = sizeof(gEthFwTsnObj.traceBuf);
    int32_t retLen = 0U;
    int32_t i;

    while (gEthFwTsnObj.logTaskrun)
    {
        EthFwOsal_lockMutex(gEthFwTsnObj.hLogMutex);
        len = strlen((char *)gEthFwTsnObj.logBuf);
        if (len > 0U)
        {
            memcpy(gEthFwTsnObj.printBuf, gEthFwTsnObj.logBuf, len);
            gEthFwTsnObj.logBuf[0U] = 0U;
            gEthFwTsnObj.printBuf[len] = 0U;
        }
        EthFwOsal_unlockMutex(gEthFwTsnObj.hLogMutex);

        if (len > 0U)
        {
            i = 0U;
            /* The print function will take a long time, we should not
             * call it inside the mutex lock. */
            do
            {
                retLen = snprintf((char *)&gEthFwTsnObj.traceBuf, maxLen, "%s", (char *)&gEthFwTsnObj.printBuf[i]);
                EthFwTrace_print("%s", gEthFwTsnObj.traceBuf);
                i += maxLen;
            }
            while (retLen > 0U && i <= len);
        }

        EthFwOsal_sleepTaskinMsecs(100U);
    }
}

static int32_t EthFwTsn_logBuffer(bool flush, const char *str)
{
    int32_t usedLen;
    int32_t bufSizeLeft;
    int32_t loglen = strlen(str);
    int32_t status = ENET_SOK;

    if(loglen > 0)
    {
        EthFwOsal_lockMutex(gEthFwTsnObj.hLogMutex);

        usedLen = strlen((char *)gEthFwTsnObj.logBuf);
        bufSizeLeft = sizeof(gEthFwTsnObj.logBuf)-usedLen;
        if (bufSizeLeft > loglen)
        {
            snprintf((char *)&gEthFwTsnObj.logBuf[usedLen], bufSizeLeft, "%s" ETHFW_LOG_BUFFER_ENDLINE, str);
        }
        else
        {
            snprintf((char *)&gEthFwTsnObj.logBuf[0], sizeof(gEthFwTsnObj.logBuf), "log overflow!\n");
        }

        EthFwOsal_unlockMutex(gEthFwTsnObj.hLogMutex);
    }
    return status;
}

static void EthFwTsn_startLogTask(void)
{
    EthFwOsal_TaskParams params;

    gEthFwTsnObj.hLogMutex = EthFwOsal_createMutex();

    /* Create logging task for gPTP stack */
    EthFwOsal_initTaskParams(&params);
    params.priority  = ETHFW_TSN_LOGGER_TASK_PRIORITY;
    params.stack     = &gEthFwTsnObj.logTaskStackBuf[0];
    params.stacksize = sizeof(gEthFwTsnObj.logTaskStackBuf);
    params.name      = "ETHFW Log Task";

    gEthFwTsnObj.hLogTask = EthFwOsal_createTask(&EthFwTsn_logTask, &params);
    if (NULL == gEthFwTsnObj.hLogTask)
    {
        ETHFWTRACE_ERR(ENET_EFAIL, "Failed to create log task");
        EnetAppUtils_assert(BFALSE);
    }
}

static void *EthFwTsn_gptpTask(void *args)
{
    int32_t i;
    int32_t status = ETHFW_SOK;
    const char *netdevs[ETHFW_TSN_IFNAMSIZ];
    EthFwTsn_ModuleCfg *mod = &gModCfgTable[ETHFWTSN_GPTP_TASK_IDX];

    for (i = 0U; i < gEthFwTsnObj.netDevInfo.numNetDevs; i++)
    {
        netdevs[i] = gEthFwTsnObj.netDevInfo.netDevs[i];
    }

    /* This function start gPTP, it has a true loop inside */
    status = gptpman_run(gGptpOpt.instNum, netdevs, gEthFwTsnObj.netDevInfo.numNetDevs,
                            NULL, &mod->stopFlag);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "gptpman_run() failed");

    gptpgcfg_close(gGptpOpt.instNum);
    return NULL;
}

static void *EthFwTsn_uniconfTask(void *args)
{
    EthFwTsn_ModuleCfg *mod = &gModCfgTable[ETHFWTSN_UNICONF_TASK_IDX];
    EthFwTsn_UniconfCfg *uniConfCfg = &gEthFwTsnObj.ucCfg;
    const char *configFiles[2] = {ETHFW_TSN_INTERFACE_CONFFILE_PATH, NULL};

    uniConfCfg->ucData.ucmode      = UC_CALLMODE_THREAD|UC_CALLMODE_UNICONF;
    uniConfCfg->ucData.stoprun     = &mod->stopFlag;
    uniConfCfg->ucData.hwmod       = "";
    uniConfCfg->ucData.ucmanstart  = &uniConfCfg->ucReadySem;
    uniConfCfg->ucData.dbname      = uniConfCfg->dbName;
    uniConfCfg->ucData.configfiles = configFiles;
    uniConfCfg->ucData.numconfigfile = ETHFW_TSN_UC_CONF_FILE_NUM;

    return uniconf_main(&uniConfCfg->ucData);
}

void EthFwTsn_init(EthFwTsn_Config *tsnCfg)
{
    unibase_init_para_t params;
    int32_t status = ETHFW_SOK;

    gEthFwTsnObj.enetType       = tsnCfg->enetType;
    gEthFwTsnObj.instId         = tsnCfg->instId;
    gEthFwTsnObj.configPtpCb    = tsnCfg->configPtpCb;
    gEthFwTsnObj.configPtpCbArg = tsnCfg->configPtpCbArg;

    gEthFwTsnObj.hEnet = Enet_getHandle(gEthFwTsnObj.enetType, gEthFwTsnObj.instId);
    gEthFwTsnObj.coreId = EnetSoc_getCoreId();

    if (!gEthFwTsnObj.tsnInit)
    {
        /*refer to 'ub_logging.h for logging levels*/
        ubb_default_initpara(&params);
        params.ub_log_initstr    = "4,ubase:45,cbase:45,uconf:34,gptp:45";
        params.cbset.gettime64   = cb_lld_gettime64;
        params.cbset.console_out = EthFwTsn_logBuffer;
        gEthFwTsnObj.logTaskrun = BTRUE;

        EthFwTsn_startLogTask();

        unibase_init(&params);
        ubb_memory_out_init(NULL, 0);
        gEthFwTsnObj.ucCfg.dbName = ETHFW_TSN_UC_DBFILE_PATH;
        gEthFwTsnObj.ucCfg.dbInitFlag = BFALSE;
        gEthFwTsnObj.tsnInit = BTRUE;
    }

    status = EthFwTsn_genPPS(&tsnCfg->ppsConfig);
    ETHFWTRACE_WARN_IF((ETHFW_ENOTSUPPORTED == status),
                       "PPS via GenF is not supported for this SoC");
}

static uint64_t EthFwTsn_getCurrentTime(void)
{
    Enet_IoctlPrms prms;
    int32_t status;
    uint64_t tsVal = 0ULL;

    /* Software push event */
    ENET_IOCTL_SET_OUT_ARGS(&prms, &tsVal);
    ENET_IOCTL(gEthFwTsnObj.hEnet, gEthFwTsnObj.coreId,
               ENET_TIMESYNC_IOCTL_GET_CURRENT_TIMESTAMP, &prms, status);
    if (status != ENET_SOK)
    {
        tsVal = 0ULL;
    }

    return tsVal;
}

int32_t EthFwTsn_genPPS(EthFwTsn_PpsConfig *ppsConfig)
{
    int32_t status = ETHFW_SOK;
#if !defined(MCU_PLUS_SDK)
    Enet_IoctlPrms prms;
    CpswCpts_SetFxnGenInArgs setGenFInArgs;
    uint32_t tsrIn;
    uint32_t tsrOut;
    uint64_t cptsrefClk;
    CSL_IntrRouterCfg irRegs;

    irRegs.pIntrRouterRegs = (CSL_intr_router_cfgRegs *) CSL_TIMESYNC_INTRTR0_INTR_ROUTER_CFG_BASE;
    irRegs.pIntdRegs       = (CSL_intr_router_intd_cfgRegs *) NULL;
    irRegs.numInputIntrs   = ENET_TIMESYNCRTR_NUM_INPUT;
    irRegs.numOutputIntrs  = ENET_TIMESYNCRTR_NUM_OUTPUT;

    CSL_intrRouterCfgMux(&irRegs, ppsConfig->tsrIn, ppsConfig->tsrOut);
    ETHFWTRACE_DBG("TSR in=%d out=%d\n", ppsConfig->tsrIn, ppsConfig->tsrOut);

    if (ETHFW_SOK == status)
    {
        ENET_IOCTL_SET_OUT_ARGS(&prms, &cptsrefClk);
        ENET_IOCTL(gEthFwTsnObj.hEnet,
                   gEthFwTsnObj.coreId,
                   CPSW_CPTS_IOCTL_GET_CPTS_REFCLK,
                   &prms,
                   status);
        ETHFWTRACE_ERR_IF((ENET_SOK != status), status, "Failed to get CPTS RefClk Frequency");
    }
    if (ETHFW_SOK == status)
    {
        /* Configure GENF to generate pulse signal */
        /* Length should be same for both master and slave */
        setGenFInArgs.index  = ppsConfig->genfIdx;
        if (ppsConfig->ppsFreqHz != 0U)
        {
            setGenFInArgs.length = cptsrefClk/ppsConfig->ppsFreqHz;
        }
        else
        {
            setGenFInArgs.length = 0U;
        }

        /* GenF gets generated when currentTime is greater than/equal to compare time
         * compare time will be set to currentTime + 1sec buffer to make compare time
         * is always in future and follows seconds boundary */
        setGenFInArgs.compare = (((EthFwTsn_getCurrentTime()/ETHFW_PPS_TIMESYNC_TIME_SEC_TO_NS)+1)
                                 *ETHFW_PPS_TIMESYNC_TIME_SEC_TO_NS) + ETHFW_PPS_TIMESYNC_COMP_TIME_NS;
        setGenFInArgs.polarityInv = BTRUE;
        setGenFInArgs.ppmVal  = 0U;
        setGenFInArgs.ppmDir  = ENET_TIMESYNC_ADJDIR_INCREASE;
        setGenFInArgs.ppmMode = ENET_TIMESYNC_ADJMODE_DISABLE;

        ENET_IOCTL_SET_IN_ARGS(&prms, &setGenFInArgs);
        ENET_IOCTL(gEthFwTsnObj.hEnet,
                   gEthFwTsnObj.coreId,
                   CPSW_CPTS_IOCTL_SET_GENF,
                   &prms,
                   status);
        ETHFWTRACE_ERR_IF((ENET_SOK != status), status, "Failed to set GenF");
        ETHFWTRACE_ERR_IF((ENET_ENOTSUPPORTED == status), status,
                           "Failed to reconfigure CPTS GenF due to hardware limitation");
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to generate PPS signal");
    }
#else
    status = ETHFW_ENOTSUPPORTED;
#endif

    return status;
}

void EthFwTsn_deInit(void)
{
    uint32_t i;

    gEthFwTsnObj.logTaskrun = BFALSE;

    if (gEthFwTsnObj.hLogTask != NULL)
    {
        EthFwOsal_deleteTask(&gEthFwTsnObj.hLogTask);
        gEthFwTsnObj.hLogTask = NULL;
    }
    if (gEthFwTsnObj.hLogMutex != NULL)
    {
        EthFwOsal_deleteMutex(gEthFwTsnObj.hLogMutex);
        gEthFwTsnObj.hLogMutex = NULL;
    }
    if (gEthFwTsnObj.ucCfg.ucReadySem != NULL)
    {
        CB_SEM_DESTROY(&gEthFwTsnObj.ucCfg.ucReadySem);
    }
    if (gEthFwTsnObj.tsnInit)
    {
        for (i = 0; i < ETHFWTSN_MAX_TASK_IDX; i++)
        {
            EthFwTsn_stopModule(i);
        }
    }

    unibase_close();

    gEthFwTsnObj.tsnInit    = BFALSE;
    gEthFwTsnObj.logTaskrun = BFALSE;
}

int EthFwTsn_restartTsnModule(int moduleIdx)
{
    EthFwTsn_ModuleCfg *mod;
    EthFwTsn_dbArgs dbargs;
    int res = ETHFW_SOK;
    int timeout_ms = 500;
    res = uniconf_ready(gEthFwTsnObj.ucCfg.dbName, UC_CALLMODE_THREAD, timeout_ms);
    if (res != ETHFW_SOK)
    {
        ETHFWTRACE_ERR(res, "The uniconf must be run first !");
    }

    if (res == ETHFW_SOK)
    {
        dbargs.dbald = uc_dbal_open(gEthFwTsnObj.ucCfg.dbName, "w", UC_CALLMODE_THREAD);
        if (!dbargs.dbald)
        {
            ETHFWTRACE_ERR(res, "Failed to open DB!");
            res = ETHFW_EFAIL;
        }
    }

    if (res == ETHFW_SOK)
    {
        mod = &gModCfgTable[moduleIdx];
        if ((mod->enable == BTRUE) && (mod->onModuleDBInit != NULL))
        {
            mod->onModuleDBInit(&dbargs);
            res = EthFwTsn_startModule(moduleIdx);
        }
        else
        {
            ETHFWTRACE_ERR(ETHFW_EFAIL, "%s: Module %s is not enabled",__func__, mod->taskName);
            res = ETHFW_EFAIL;
        }
    }
    return res;
}

void EthFwTsn_stopModule(uint32_t moduleIdx)
{
    if ((moduleIdx >= 0) && (moduleIdx < ETHFWTSN_MAX_TASK_IDX))
    {
        EthFwTsn_ModuleCfg *mod;
        mod = &gModCfgTable[moduleIdx];
        if (mod->hTaskHandle != NULL)
        {
            mod->stopFlag = BTRUE;
            CB_THREAD_JOIN(mod->hTaskHandle, NULL);
            mod->hTaskHandle = NULL;
            ETHFWTRACE_INFO("Task: %s is terminated.", mod->taskName);
        }
    }
}

int32_t EthFwTsn_startModule(uint32_t moduleIdx)
{
    int32_t status = ETHFW_SOK;

    if ((moduleIdx >= 0) && (moduleIdx < ETHFWTSN_MAX_TASK_IDX))
    {
        EthFwTsn_ModuleCfg *mod;
        mod = &gModCfgTable[moduleIdx];

        if ((mod->enable == BTRUE) && (mod->stopFlag == BTRUE))
        {
            status = EthFwTsn_startModTask(mod, moduleIdx);
            ETHFWTRACE_INFO_IF((ETHFW_SOK==status),"Task: %s is created.", mod->taskName);
        }
    }
    else
    {
        status = ETHFW_EFAIL;
    }
    return status;
}

int32_t EthFwTsn_initTimeSyncPtp(const uint8_t *hostMacAddr,
                                 uint32_t portMask)
{
    lld_ethdev_t ethdevs[MAX_NUMBER_ENET_DEVS] = {0};
    Enet_MacPort macPort;
    int32_t status = ETHFW_SOK;
    uint32_t i;
    uint32_t j = 0U;
    const LLDTsyncTsSource tsSource = LLDTSYNC_TS_SOURCE_CPTS;

    if (portMask != 0U)
    {
        for (i = 0U; i < ETHFW_TSN_CFG_NUM_MAC_PORTS; i++)
        {
            if (ENET_IS_BIT_SET(portMask, i))
            {
                macPort = ENET_MACPORT_DENORM(i);

                /* Linking each MAC port with an interface name */
                snprintf(&gEthFwTsnObj.netDevInfo.netDevs[j][0], ETHFW_TSN_IFNAMSIZ, "tilld%d", i + 1);
                gEthFwTsnObj.netDevInfo.gPtpNetDevs[j] = &gEthFwTsnObj.netDevInfo.netDevs[j][0];
                ethdevs[j].netdev  = gEthFwTsnObj.netDevInfo.netDevs[j];
                ethdevs[j].macport = macPort;
                memcpy(&ethdevs[j].srcmac, hostMacAddr, ENET_MAC_ADDR_LEN);

                ETHFWTRACE_INFO("Enable gPTP on MAC port %u (%s)",
                                ENET_MACPORT_ID(macPort), gEthFwTsnObj.netDevInfo.gPtpNetDevs[j]);
                j++;
            }
        }
    }

    gEthFwTsnObj.netDevInfo.numNetDevs = j;

    ETHFWTRACE_WARN_IF(gEthFwTsnObj.netDevInfo.numNetDevs == 0U,
                       "gPTP is not enabled on any MAC port");
    ETHFWTRACE_DBG_IF(hostMacAddr == NULL,
                      "TSN stack will allocate %u MAC addrs from MAC pool", j);

    /* Filling netdev table where each entry consists of an interface,
     * its MAC port and mac addr (if any) */
    if (gEthFwTsnObj.netDevInfo.numNetDevs > 0U)
    {
        if (status == ETHFW_SOK)
        {
            status  = cb_lld_init_devs_table(ethdevs, gEthFwTsnObj.netDevInfo.numNetDevs,
                                             (Enet_Type) gEthFwTsnObj.enetType,
                                             gEthFwTsnObj.instId, tsSource);

            if (ETHFW_SOK != status)
            {
                status = ETHFW_EFAIL;
                ETHFWTRACE_ERR(status, "Failed to int devs table");
            }
        }
        if (ETHFW_SOK == status)
        {
            status = EthFwTsn_startMod();
            ETHFWTRACE_INFO_IF((ETHFW_SOK == status), "TimeSync PTP enabled");
        }
    }
    return status;
}

static int EthFwTsn_uniconfInit(EthFwTsn_dbArgs *dbargs)
{
    int i;

    for (i = 0; i < gEthFwTsnObj.netDevInfo.numNetDevs; i++)
    {
        uint8_t up=1;
		UB_LOG(UBL_DEBUG, "use network device:%s\n", gEthFwTsnObj.netDevInfo.netDevs[i]);
		YDBI_SET_ITEM(ifk1vk0, (char*)gEthFwTsnObj.netDevInfo.netDevs[i],
			      IETF_INTERFACES_ENABLED, YDBI_CONFIG,
			      &up, 1, YDBI_PUSH_NOTICE);
    }
    return 0;
}

static int32_t EthFwTsn_gptpDbInit(EthFwTsn_dbArgs *dbArgs)
{
    int32_t status = ETHFW_SOK;
    uint32_t i;
    EthFwTsn_UniconfCfg *cfg = &gEthFwTsnObj.ucCfg;

    for (i = 0; i < gGptpOpt.domainNum; i++)
    {
        EthFwTsn_gptpUpdateDomainMap(gGptpOpt.instNum, gGptpOpt.domains[i]);
    }

    status = gptpgcfg_init(cfg->dbName, gGptpOpt.confFiles, gGptpOpt.instNum, true,
                           EthFwTsn_gptpNonYangConfig);

    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                      "gptpgcfg_init() error in %s", __func__);

    if (gGptpOpt.numConf == 0)
    {
        /* There is no config file is specified, set config file for gptp*/
        for (i = 0; i < gGptpOpt.domainNum; i++)
        {
            status = EthFwTsn_gptpYangConfig(gGptpOpt.instNum,
                                             gGptpOpt.domains[i]);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                  "Failed to set gptp run time config");
        }
    }

    return status;
}

static void EthFwTsn_gptpUpdateDomainMap(int instance, int domain)
{
    EthFwTsn_UniconfCfg *cfg = &gEthFwTsnObj.ucCfg;
    if (!cfg->dbInitFlag)
    {
        yang_db_item_access_t *ydbia=ydbi_access_handle();
        uint16_t dmap;
        uint8_t aps[]={IEEE1588_PTP_TT_RW_Y, IEEE1588_PTP_TT_PTP,
		IEEE1588_PTP_TT_INSTANCE_DOMAIN_MAP, 255};
	    yang_db_access_para_t dbpara={((instance | domain) != 0) ? YANG_DB_ACTION_APPEND:YANG_DB_ACTION_CREATE,
                                        YANG_DB_ONHW_NOACTION,
		                                NULL, aps, NULL, NULL, &dmap, sizeof(uint16_t)};
        dmap=instance<<8|domain;
        if(yang_db_action(ydbia->dbald, NULL, &dbpara)!=0){
            ETHFWTRACE_ERR(ETHFW_EFAIL,"%s:Can't create instance|domainmap=0x%04x\n",
                            __func__, dmap);
        }
    }
}

static int32_t EthFwTsn_gptpNonYangConfig(uint8_t instance)
{
    uint32_t i;
    int32_t status;
    EthFwTsn_gPTPConfigArg cbArgs;

    cbArgs.inst = instance;

    /* Let app overwrite any gPTP configuration parameters */
    if (gEthFwTsnObj.configPtpCb != NULL)
    {
        gEthFwTsnObj.configPtpCb((void *)&cbArgs);
    }

    for (i = 0U; i < sizeof(gGptpNonYangDs)/sizeof(gGptpNonYangDs[0]); i++)
    {
        status = gptpgcfg_set_item(instance, gGptpNonYangDs[i].item,
                                   YDBI_CONFIG, &gGptpNonYangDs[i].val,
                                   sizeof(gGptpNonYangDs[i].val));

        ETHFWTRACE_DBG_IF(status == ETHFW_SOK,"%s:XL4_EXTMOD_XL4GPTP_%s=%d\n", __func__,
                               gGptpNonYangDs[i].name, gGptpNonYangDs[i].val);

        if ((status != ETHFW_SOK))
        {
            ETHFWTRACE_ERR(ETHFW_EFAIL,"%s: failed to set nonyang param: %s\n",
                            __func__, gGptpNonYangDs[i].name);
            break;
        }
    }
    return status;
}

static int32_t EthFwTsn_gptpYangConfig(uint32_t instance,
                                       uint32_t domain)
{
    uint32_t i, res = 0;
    EthFwTsn_UniconfCfg *cfg = &gEthFwTsnObj.ucCfg;

    do {
        /* set for default-ds */
        EthFwTsn_cfgGptpDefaultDs(instance, domain, cfg->dbInitFlag);

        // portindex starts from 1
        for (i = 0; i < gEthFwTsnObj.netDevInfo.numNetDevs; i++)
        {
            /* skip setting of 'rw' yang configs when db is already initialized */
            if (!cfg->dbInitFlag)
            {
                gptpgcfg_set_yang_port_item(instance,
                                            IEEE1588_PTP_TT_UNDERLYING_INTERFACE,
                                            255,
                                            i+1,
                                            domain, YDBI_CONFIG,
                                            gEthFwTsnObj.netDevInfo.netDevs[i],
                                            strlen(gEthFwTsnObj.netDevInfo.netDevs[i])+1,
                                            YDBI_NO_NOTICE);
            }

            /* set for port-ds */
            EthFwTsn_cfgGptpPortDs(instance, domain, i+1, cfg->dbInitFlag);
        }

        /* skip setting of 'rw' yang configs when db is already initialized */
        if (!cfg->dbInitFlag)
        {
            /* disable performance by default */
            bool enable=false;
            gptpgcfg_set_yang_item(
                    instance,
                    IEEE1588_PTP_TT_PERFORMANCE_MONITORING_DS,
                    IEEE1588_PTP_TT_ENABLE, 255,
                    domain, YDBI_CONFIG,
                    &enable, 1, YDBI_NO_NOTICE);
        }
    } while (0);

    return res;
}

static void EthFwTsn_cfgGptpDefaultDs(uint32_t instance,
                                      uint32_t domain,
                                      bool dbInitFlag)
{
    int i;
    for (i = 0; i < sizeof(gGptpDefaultDsInt)/sizeof(gGptpDefaultDsInt[0]); i++)
    {
        if (gGptpDefaultDsInt[i].rw)
        {
            if (!dbInitFlag)
            {
                gptpgcfg_set_yang_defaultds_item(instance,
                                                 gGptpDefaultDsInt[i].key,
                                                 255,
                                                 domain,
                                                 YDBI_CONFIG,
                                                 &gGptpDefaultDsInt[i].val,
                                                 gGptpDefaultDsInt[i].sz,
                                                 YDBI_NO_NOTICE);
            }
        }
        else
        {
            gptpgcfg_set_yang_defaultds_item(instance,
                                             gGptpDefaultDsInt[i].key,
                                             255,
                                             domain,
                                             YDBI_STATUS,
                                             &gGptpDefaultDsInt[i].val,
                                             gGptpDefaultDsInt[i].sz,
                                             YDBI_NO_NOTICE);
        }
    }

    if (!dbInitFlag)
    {
        for (i = 0; i < sizeof(gGptpDefaultDsClkQuality)/sizeof(gGptpDefaultDsClkQuality[0]); i++)
        {
            gptpgcfg_set_yang_defaultds_item(instance,
                                             IEEE1588_PTP_TT_CLOCK_QUALITY,
                                             gGptpDefaultDsClkQuality[i].key,
                                             domain,
                                             YDBI_CONFIG,
                                             &gGptpDefaultDsClkQuality[i].val,
                                             gGptpDefaultDsClkQuality[i].sz,
                                             YDBI_NO_NOTICE);
        }
    }

}

static void EthFwTsn_cfgGptpPortDs(uint32_t instance,
                                   uint32_t domain,
                                   int portIndex,
                                   bool dbInitFlag)
{
    uint32_t i;
    for (i = 0; i < sizeof(gGptpPortDsInt)/sizeof(gGptpPortDsInt[0]); i++)
    {
        if (gGptpPortDsInt[i].rw)
        {
            if (!dbInitFlag)
            {
                gptpgcfg_set_yang_port_item(instance, IEEE1588_PTP_TT_PORT_DS,
                                        gGptpPortDsInt[i].key, portIndex,
                                        domain, YDBI_CONFIG,
                                        &gGptpPortDsInt[i].val, gGptpPortDsInt[i].sz,
                                        YDBI_NO_NOTICE);
            }
        }
        else
        {
            gptpgcfg_set_yang_port_item(instance, IEEE1588_PTP_TT_PORT_DS,
                                        gGptpPortDsInt[i].key, portIndex,
                                        domain, YDBI_STATUS,
                                        &gGptpPortDsInt[i].val, gGptpPortDsInt[i].sz,
                                        YDBI_NO_NOTICE);
        }
    }

    for (i = 0; i < sizeof(gGptpTsCorrectionPortDs)/sizeof(gGptpTsCorrectionPortDs[0]); i++)
    {
        gptpgcfg_set_yang_port_item(instance, IEEE1588_PTP_TT_TIMESTAMP_CORRECTION_PORT_DS,
                            gGptpTsCorrectionPortDs[i].key, portIndex,
                            domain, YDBI_CONFIG,
                            &gGptpTsCorrectionPortDs[i].val, gGptpTsCorrectionPortDs[i].sz,
                            YDBI_NO_NOTICE);
    }
}


static int32_t EthFwTsn_startMod(void)
{
    uint32_t i;
    int32_t status = ETHFW_SOK;
    EthFwTsn_ModuleCfg *mod;

    if (CB_SEM_INIT(&gEthFwTsnObj.ucCfg.ucReadySem, 0U, 0U) != ETHFW_SOK)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to initialize ucReadySem semaphore!");
    }
    if (gEthFwTsnObj.tsnInit)
    {
        for (i = ETHFWTSN_UNICONF_TASK_IDX; i < ETHFWTSN_MAX_TASK_IDX; i++)
        {
            mod = &gModCfgTable[i];
            if (mod->enable == BFALSE)
            {
                status = EthFwTsn_startModTask(mod, i);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                  "Failed to start Task for moduleIdx %u", i);
            }
        }
    }

    return status;
}

static int32_t EthFwTsn_startModTask(EthFwTsn_ModuleCfg *modCfg, uint32_t moduleIdx)
{
    cb_tsn_thread_attr_t attr;
    int32_t status = ETHFW_SOK;

    if ((gEthFwTsnObj.ucCfg.ucReadySem != NULL) && (modCfg->onModuleRunner != NULL))
    {
        /* Set the flags before creating the thread.
           If the thread starts immediately (due to high priority),
           it will see the correct state. */
        modCfg->stopFlag = BFALSE;
        modCfg->enable = BTRUE;
        cb_tsn_thread_attr_init(&attr, modCfg->taskPriority,
                                modCfg->stackSize, modCfg->taskName);
        cb_tsn_thread_attr_set_stackaddr(&attr, modCfg->stackBuffer);

        status = CB_THREAD_CREATE(&modCfg->hTaskHandle, &attr,
                                  modCfg->onModuleRunner, modCfg->onModuleRunnerArgs);
        if (ETHFW_SOK != status)
        {
            /* Undo the flags if creation failed */
            modCfg->stopFlag = BTRUE;
            modCfg->enable = BFALSE;
            status = ETHFW_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to create %s task!\n", modCfg->taskName);
            EnetAppUtils_assert(BFALSE);
        }

        else
        {
            if (moduleIdx == ETHFWTSN_UNICONF_TASK_IDX)
            {
                /* initDb must be run right after UNICONF is started and
                   before starting any other tasks. */
                status = EthFwTsn_initDb(&gEthFwTsnObj.ucCfg);
            }
        }
    }
    else
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to create %s task!\n", modCfg->taskName);
        EnetAppUtils_assert(BFALSE);
    }

    return status;
}

static int32_t EthFwTsn_initDb(EthFwTsn_UniconfCfg *ucCfg)
{
    EthFwTsn_ModuleCfg *mod;
    EthFwTsn_dbArgs dbArgs;
    uint32_t i;
    int32_t status = ETHFW_SOK;
    uint32_t timeout_ms = 1000U;

    memset(&dbArgs, 0, sizeof(EthFwTsn_dbArgs));

    do
    {
        /*waiting for the uniconf to be ready */
        status = CB_SEM_WAIT(&ucCfg->ucReadySem);
        if (ETHFW_SOK != status)
        {
            ETHFWTRACE_ERR(status, "Failed to wait for the uniconf");
            break;
        }

        status = uniconf_ready(ucCfg->dbName, UC_CALLMODE_THREAD, timeout_ms);
        if (status)
        {
            ETHFWTRACE_ERR(status, "The uniconf must be run first!");
            break;
        }

        dbArgs.dbald = uc_dbal_open(ucCfg->dbName, "w", UC_CALLMODE_THREAD);
        if (!dbArgs.dbald)
        {
            status = ETHFW_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to open DB!\n");
            break;
        }

        for (i = 0U; i < ETHFWTSN_MAX_TASK_IDX; i++)
        {
            mod = &gModCfgTable[i];
            if (mod->onModuleDBInit != NULL)
            {
                status = mod->onModuleDBInit(&dbArgs);
                if (ETHFW_SOK != status)
                {
                    ETHFWTRACE_ERR(ETHFW_EFAIL, "Module DB Init failed for ModuleIdx %u", i);
                    break;
                }
            }
        }
    } while (0U);

    if (dbArgs.dbald)
    {
        uc_dbal_close(dbArgs.dbald, UC_CALLMODE_THREAD);
    }

    return status;
}
