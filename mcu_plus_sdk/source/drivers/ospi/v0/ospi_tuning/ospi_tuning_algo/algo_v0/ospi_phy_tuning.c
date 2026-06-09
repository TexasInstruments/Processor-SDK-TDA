#include "ospi_phy_tuning.h"

# ifndef MIN
# define MIN(x,y) \
    ({typeof (x) _x = (x);\
      typeof (y) _y = (y);\
      _x < _y ? _x : _y;})
# endif
# ifndef MAX
# define MAX(x,y) \
    ({typeof (x) _x = (x); \
      typeof (y) _y = (y); \
      _x > _y ? _x : _y; \
    })
# endif

typedef struct
{
    int txDLL;
    int rxDLL;
    int rdDelay;

} OSPI_PhyConfig;


static OSPI_PhyTuneWindowParams gTestDefaultTuningWindowDDR =
{
    .txDllLowWindowStart    = 28,
    .txDllLowWindowEnd      = 48,
    .txDllHighWindowStart   = 60,
    .txDllHighWindowEnd     = 96,
    .rxLowSearchStart       = 0,
    .rxLowSearchEnd         = 40,
    .rxHighSearchStart      = 24,
    .rxHighSearchEnd        = 107,
    .txLowSearchStart       = 16,
    .txLowSearchEnd         = 64,
    .txHighSearchStart      = 78,
    .txHighSearchEnd        = 127,
    .txDLLSearchOffset      = 8,
    .rxTxDLLSearchStep      = 4,
    .rdDelayMin             = 0,
    .rdDelayMax             = 4,
};

static int OSPI_phyIsPassingPoint(OSPI_PhyConfig *result, short int (*arr)[128][128])
{
    if(result->rxDLL < 0 || result->rxDLL > 127 || result->txDLL < 0 || result->txDLL > 127)
    {
        return OSPI_PHY_OLD_TUNING_FAILURE;
    }

    usleep(30);
    int passingValue = (*arr)[result->rxDLL][result->txDLL];

    if(passingValue == result->rdDelay)
    {
        return OSPI_PHY_OLD_TUNING_SUCCESS;
    }

    return OSPI_PHY_OLD_TUNING_FAILURE;
}

int OSPI_phyFindRxLow(OSPI_PhyConfig *start, OSPI_PhyConfig *result,short int (*arr)[128][128])
{
    int isPassingValue = OSPI_PHY_OLD_TUNING_SUCCESS;
    OSPI_PhyTuneWindowParams *phyTuneWindowParams = \
            &gTestDefaultTuningWindowDDR;

    result->txDLL = start->txDLL;
    result->rxDLL = start->rxDLL;
    result->rdDelay = start->rdDelay;

    isPassingValue = OSPI_phyIsPassingPoint(result, arr);

    while(isPassingValue == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        result->rxDLL += phyTuneWindowParams->rxTxDLLSearchStep;
        if(result->rxDLL > phyTuneWindowParams->rxLowSearchEnd && result->rxDLL < 128)
        {
            result->rxDLL = 127U;
            return OSPI_PHY_OLD_TUNING_FAILURE;
        }

        isPassingValue = OSPI_phyIsPassingPoint(result, arr);
    }

    return OSPI_PHY_OLD_TUNING_SUCCESS;
}

int OSPI_phyFindRxHigh(OSPI_PhyConfig *start, OSPI_PhyConfig *result,short int (*arr)[128][128])
{
    int isPassingValue = OSPI_PHY_OLD_TUNING_SUCCESS;
    OSPI_PhyTuneWindowParams *phyTuneWindowParams = \
            &gTestDefaultTuningWindowDDR;

    result->txDLL = start->txDLL;
    result->rxDLL = start->rxDLL;
    result->rdDelay = start->rdDelay;

    isPassingValue = OSPI_phyIsPassingPoint(result,  arr);

    while(isPassingValue == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        result->rxDLL -= phyTuneWindowParams->rxTxDLLSearchStep;
        if(result->rxDLL < phyTuneWindowParams->rxHighSearchStart && result->rxDLL >= 0)
        {
            result->rxDLL = 127U;
            return OSPI_PHY_OLD_TUNING_FAILURE;
        }
        isPassingValue = OSPI_phyIsPassingPoint(result, arr);
    }

    return OSPI_PHY_OLD_TUNING_SUCCESS;
}

int OSPI_phyFindTxLow(OSPI_PhyConfig *start, OSPI_PhyConfig *result, short int (*arr)[128][128])
{
    int isPassingValue = OSPI_PHY_OLD_TUNING_SUCCESS;
    OSPI_PhyTuneWindowParams *phyTuneWindowParams = \
            &gTestDefaultTuningWindowDDR;

    result->txDLL = start->txDLL;
    result->rxDLL = start->rxDLL;
    result->rdDelay = start->rdDelay;

    isPassingValue = OSPI_phyIsPassingPoint(result, arr);

    while(isPassingValue == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        result->txDLL += phyTuneWindowParams->rxTxDLLSearchStep;
        if(result->txDLL > phyTuneWindowParams->txLowSearchEnd && result->txDLL < 128)
        {
            result->txDLL = 127U;
            return OSPI_PHY_OLD_TUNING_FAILURE;
        }
        isPassingValue = OSPI_phyIsPassingPoint(result, arr);
    }

    return OSPI_PHY_OLD_TUNING_SUCCESS;
}

int OSPI_phyFindTxHigh(OSPI_PhyConfig *start, OSPI_PhyConfig *result, short int (*arr)[128][128])
{
    int isPassingValue = OSPI_PHY_OLD_TUNING_SUCCESS;
    OSPI_PhyTuneWindowParams *phyTuneWindowParams = \
            &gTestDefaultTuningWindowDDR;

    result->txDLL = start->txDLL;
    result->rxDLL = start->rxDLL;
    result->rdDelay = start->rdDelay;

    isPassingValue = OSPI_phyIsPassingPoint(result, arr);

    while(isPassingValue == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        result->txDLL -= phyTuneWindowParams->rxTxDLLSearchStep;
        if(result->txDLL < phyTuneWindowParams->txHighSearchStart && result->txDLL >= 0)
        {
            result->txDLL = 127U;
            return OSPI_PHY_OLD_TUNING_FAILURE;
        }
        isPassingValue = OSPI_phyIsPassingPoint(result, arr);
    }

    return OSPI_PHY_OLD_TUNING_SUCCESS;
}

int OSPI_phyFindOTP1(short int (*arr)[128][128], int32_t radius, OSPI_PhyConfig *otp)
{
    int status = OSPI_PHY_OLD_TUNING_SUCCESS;
    int searchResult = OSPI_PHY_OLD_TUNING_SUCCESS;
    OSPI_PhyConfig searchPoint;
    OSPI_PhyConfig bottomLeft = {0,0,0}, topRight = {0,0,0};
    OSPI_PhyConfig gapLow = {0,0,0}, gapHigh = {0,0,0};
    OSPI_PhyConfig rxLow = {0,0,0}, rxHigh = {0,0,0};
    OSPI_PhyConfig txLow = {0,0,0}, txHigh = {0,0,0};
    OSPI_PhyConfig backupPoint = {0,0,0}, backupCornerPoint = {0,0,0};
    OSPI_PhyConfig sec_rxLow = {0,0,0}, sec_rxHigh = {0,0,0};
    float slope;

    OSPI_PhyTuneWindowParams *phyTuneWindowParams = \
            &gTestDefaultTuningWindowDDR;

    // Variables for binary search and slope calculation
    /*
     * Finding RxDLL fails at some of the TxDLL values based on the HW platform.
     * A window of TxDLL values is used to find the RxDLL without errors.
     * This can increase the number of CPU cycles taken for the PHY tuning
     * in the cases where more TxDLL values need to be parsed to find a stable RxDLL.
     */

    /***************************** GOLDEN Primary Rx_Low Search **************/
    /*
     * To find the RxDLL boundaries, we fix a valid TxDLL and search through RxDLL range, rdDelay values
     * As we are not sure of a valid TxDLL we use a window of TxDLL values to find the RxDLL boundaries.
     */
    /*
           Rx_DLL
              ▲
              │   ++++++++++++++++
          127 │     ++++++++++++++
              │   x   ++++++++++++
              │   xx   +++++++++++
              │   xxx   ++++++++++
              │   xxxx   +++++++++
              │   xxxxx   ++++++++
              │ │ xxx│xx   +++++++
              │ │ xxx│xxx   ++++++
              │ │ xxx│xxxx   +++++
              │ │ xxx│xxxxx   ++++
              │ │ xxx│xxxxxx   +++
     Search   │ │ xxx│xxxxxxx   ++
     Rx_Low ──┼─┤►xxx│xxxxxxxx   +
              │ │    │
             ─┼─┼────┼────────────►  Tx_DLL
             0│ │    │           127
                │    │
                │    │

            Tx_Low   Tx_Low
            Start    End
    */

    searchPoint.txDLL = phyTuneWindowParams->txDllLowWindowStart;

    while(searchPoint.txDLL <= phyTuneWindowParams->txDllLowWindowEnd)
    {
        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
        searchPoint.rxDLL = phyTuneWindowParams->rxLowSearchStart;
        searchResult = OSPI_phyFindRxLow(&searchPoint, &rxLow, arr);

        while(!searchResult)
        {
            searchPoint.rdDelay++;
            if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
            {
                if(searchPoint.txDLL >= phyTuneWindowParams->txDllLowWindowEnd)
                {
                    status = OSPI_PHY_OLD_TUNING_FAILURE;
                    return status;
                }
                else
                {
                    break;
                }
            }
            searchResult = OSPI_phyFindRxLow(&searchPoint, &rxLow, arr);
        }

        if(rxLow.rxDLL != 127U)
        {
            break;
        }

        searchPoint.txDLL += phyTuneWindowParams->rxTxDLLSearchStep;
    }

    /***************************** GOLDEN Secondary Rx_Low Search *****************************/
    /* Search for one more rxLow at different txDll*/
    if(searchPoint.txDLL <= (phyTuneWindowParams->txDllLowWindowEnd - phyTuneWindowParams->txDLLSearchOffset))
    {
        searchPoint.txDLL += phyTuneWindowParams->txDLLSearchOffset;
    }
    else
    {
        searchPoint.txDLL = phyTuneWindowParams->txDllLowWindowEnd;
    }

    searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
    searchPoint.rxDLL   = phyTuneWindowParams->rxLowSearchStart;
    searchResult = OSPI_phyFindRxLow(&searchPoint, &sec_rxLow, arr);
    while(!searchResult)
    {
        searchPoint.rdDelay++;  /* For each TxDLL in the window, go through all the valid rdDelays until we find the RxLow */
        if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
        {
            if(searchPoint.txDLL >= phyTuneWindowParams->txDllLowWindowEnd)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                return status; /* Not able to find RxLow as there is no valid TxDLL in the TxDLL window */
            }
            else
            {
                break;
            }
        }
        searchResult = OSPI_phyFindRxLow(&searchPoint, &sec_rxLow, arr);
    }

    /*
     * Pick Minimum value of rxDLL between rxLow and sec_rxLow
     * Pick Minimum value of rdDelay(read_delay) between rxLow and sec_rxLow
    ┌────────┬───────────┬────────────────────────────────────┐
    │Primary │ Secondary │   Final  Point                     │
    │ Search │  Search   │                                    │
    │        │           │                                    │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Fail   │   Fail    │  Return Fail                       │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Fail   │   Pass    │  Return Fail                       │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Pass   │   Fail    │  Return Fail                       │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Pass   │   Pass    │ RxDll = Min(Primary, Secondary)    │
    │        │           │ RdDelay = Min(Primary, Secondary)  │
    │        │           │ TxDll = Primary                    │
    │        │           │                                    │
    └────────┴───────────┴────────────────────────────────────┘
     */

    rxLow.rxDLL = MIN(rxLow.rxDLL, sec_rxLow.rxDLL);
    rxLow.rdDelay = MIN(rxLow.rdDelay, sec_rxLow.rdDelay);

    /*
     * Reset the search point txDLL to continue the Rx_High search
     */
    searchPoint.txDLL = rxLow.txDLL;

    /***************************** GOLDEN Primary Rx_High Search *********************/
    /*
     * To find rxHigh we use the txDLL values of rxLow
     * Start the rdDelay (Read delay) from maximum and decrement it.
     * As these are valid values and rxHigh rdDelay is always >= rxLow rdDelay
     */
    /*
               Rx_DLL
                  ▲
              127 │   ▲+++++++++++++++
        Search    │   │ ++++++++++++++
       Rx_High────┼──►│   ++++++++++++
 on Fixed Tx_DLL  │   │x   +++++++++++
                  │   │xx   ++++++++++
                  │   │xxx   +++++++++
                  │   │xxxx   ++++++++
                  │   ▼xxxxx   +++++++
                  │   Xxxxxxx   ++++++
                  │   Xxxxxxxx   +++++
                  │   Xxxxxxxxx   ++++
                  │   Xxxxxxxxxx   +++
                  │   Xxxxxxxxxxx   ++
                  │   Xxxxxxxxxxxx   +
                  │
                 ─┼───────────────────►  Tx_DLL
                 0│                  127
    */

    searchPoint.rxDLL = phyTuneWindowParams->rxHighSearchEnd;
    searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;

    searchResult = OSPI_phyFindRxHigh(&searchPoint,&rxHigh, arr);

    while(!searchResult)
    {
        searchPoint.rdDelay--;
        if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
        {
            status = OSPI_PHY_OLD_TUNING_FAILURE;
            break;
        }
        searchResult = OSPI_phyFindRxHigh(&searchPoint, &rxHigh, arr);
    }


    /***************************** GOLDEN Secondary Rx_High Search *********************/
    /*
     * To find Secondary rxHigh we use the txDLL + Search_offset value of rxLow
     * Start the rdDelay (Read delay) from maximum and decrement it.
     * As these are valid values and rxHigh rdDelay is always >= rxLow rdDelay
     */
    if(searchPoint.txDLL <= (phyTuneWindowParams->txDllLowWindowEnd - phyTuneWindowParams->txDLLSearchOffset))
    {
        searchPoint.txDLL += phyTuneWindowParams->txDLLSearchOffset;
    }
    else
    {
        searchPoint.txDLL = phyTuneWindowParams->txDllLowWindowEnd;
    }

    searchPoint.rxDLL = phyTuneWindowParams->rxHighSearchEnd;
    searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;

    searchResult = OSPI_phyFindRxHigh( &searchPoint, &sec_rxHigh, arr);

    while(!searchResult)
    {
        searchPoint.rdDelay--;
        if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
        {
            status = OSPI_PHY_OLD_TUNING_FAILURE;
            break;
            /*
             * If we don't find a valid Secondary Rx_High, Don't return from tuning function
             * Check whether we have a valid Primary Rx_High and then take decision.
             */
        }
        searchResult = OSPI_phyFindRxHigh(&searchPoint, &sec_rxHigh, arr);
    }

    /*
     * Compare the Primary and Secondary point
     * Pick the point which has passing maximum rxDll
    ┌────────┬───────────┬────────────────────────────────────┐
    │Primary │ Secondary │   Final  Point                     │
    │ Search │  Search   │                                    │
    │        │           │                                    │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Fail   │   Fail    │  Return Fail                       │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Fail   │   Pass    │  Pick Secondary search             │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Pass   │   Fail    │  Pick Primary search               │
    ├────────┼───────────┼────────────────────────────────────┤
    │ Pass   │   Pass    │ If(secondary.rxDll > primary.rxDll)│
    │        │           │ Pick Secondary search point        │
    │        │           │ Else                               │
    │        │           │ Pick Primary search point          │
    └────────┴───────────┴────────────────────────────────────┘
    */

    if(sec_rxHigh.rxDLL != 127U)
    {
        if(rxHigh.rxDLL == 127U)
        {
            rxHigh = sec_rxHigh;
        }
        else
        {
            if(sec_rxHigh.rxDLL > rxHigh.rxDLL)
            {
                rxHigh = sec_rxHigh;
            }
        }
    }
    else
    {
        if(rxHigh.rxDLL == 127)
        {
            status = OSPI_PHY_OLD_TUNING_FAILURE;
            return status;
        }
    }

    /*
     * Check a different point if the rxLow and rxHigh are on the same rdDelay.
     * This avoids mistaking the metastability gap for an rxDLL boundary
     */
    if(rxLow.rdDelay == rxHigh.rdDelay)
    {
        /***************************** BACKUP Primary Rx_Low Search *********************/
        /*
         * Find the rxDLL boundaries using the TxDLL window at the higher end .
         * we start the window_end and decrement the TxDLL value until we find the valid point.
         */
        /*
           Rx_DLL
            ▲
            │   ++++++++++++++++
        127 │   ++++++++++++++++
            │   ++++++++++++++++
            │    +++++++++++++++
            │     +++++++++│++++│
            │      ++++++++│++++│
            │   x   +++++++│++++│
            │   xx   ++++++│++++│
            │   xxx   +++++│++++│
            │   xxxx   ++++│++++│
            │   xxxxx   +++│++++│
            │   xxxxxx   ++│++++│
            │   xxxxxxx   +│++++│         Search
            │   xxxxxxxx   │++++◄───────  Rx_Low
            │              │    │
           ─┼──────────────┼────┤► Tx_DLL
           0│              │    │   127
                           │    │
                   Tx_High        Tx_High
                   Start          End
        */

        searchPoint.txDLL = phyTuneWindowParams->txDllHighWindowEnd;

        /* Find rxDLL Min */
        while(searchPoint.txDLL >= phyTuneWindowParams->txDllHighWindowStart)
        {
            searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
            searchPoint.rxDLL = phyTuneWindowParams->rxLowSearchStart;
            searchResult = OSPI_phyFindRxLow(&searchPoint, &backupPoint, arr);

            while(!searchResult)
            {
                searchPoint.rdDelay++;
                if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
                {
                    if(searchPoint.txDLL <= phyTuneWindowParams->txDllHighWindowStart)
                    {
                        status = OSPI_PHY_OLD_TUNING_FAILURE;
                        return status;
                    }
                    else
                    {
                        break;
                    }
                }
                searchResult = OSPI_phyFindRxLow(&searchPoint, &backupPoint, arr);
            }

            if(backupPoint.rxDLL != 127U)
            {
                break;
            }

            searchPoint.txDLL -= phyTuneWindowParams->rxTxDLLSearchStep;
        }

        /***************************** BACKUP Secondary Rx_Low Search *********************/
        /* Search for one more rxLow at different txDll*/
        if (searchPoint.txDLL >= (phyTuneWindowParams->txDllHighWindowStart + phyTuneWindowParams->txDLLSearchOffset ))
        {
            searchPoint.txDLL -= phyTuneWindowParams->txDLLSearchOffset;
        }
        else
        {
            searchPoint.txDLL = phyTuneWindowParams->txDllHighWindowStart;
        }

        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
        searchPoint.rxDLL   = phyTuneWindowParams->rxLowSearchStart;
        searchResult = OSPI_phyFindRxLow(&searchPoint, &sec_rxLow, arr);
        while(!searchResult)
        {
            searchPoint.rdDelay++;  /* For each TxDLL in the window, go through all the valid rdDelays until we find the RxLow */
            if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
            {
                if(searchPoint.txDLL <= phyTuneWindowParams->txDllHighWindowStart)
                {
                    status = OSPI_PHY_OLD_TUNING_FAILURE;
                    return status; /* Not able to find RxLow as there is no valid TxDLL in the TxDLL window */
                }
                else
                {
                    break;
                }
            }
            searchResult = OSPI_phyFindRxLow(&searchPoint, &sec_rxLow, arr);
        }

        backupPoint.rxDLL = MIN(backupPoint.rxDLL, sec_rxLow.rxDLL);
        backupPoint.rdDelay = MIN(backupPoint.rdDelay, sec_rxLow.rdDelay);

        if(backupPoint.rxDLL < rxLow.rxDLL)
        {
            rxLow = backupPoint;
        }

        /*
         * Reset the search point txDLL to continue the Rx_High search
         */
        searchPoint.txDLL = backupPoint.txDLL;

        /***************************** BACKUP Primary Rx_High Search *********************/
        /*
         * Find rxDLL Max
         * Start the rdDelay (Read delay) from maximum and decrement it.
         */
        /*
        Rx_DLL
        127 ▲
            │   +++++++++++++++▲                Search Rx_High
            │   +++++++++++++++│◄────────────   on Fixed Tx_DLL
            │   +++++++++++++++│
            │    ++++++++++++++│
            │     +++++++++++++│
            │      ++++++++++++│
            │   x   +++++++++++▼
            │   xx   +++++++++++
            │   xxx   ++++++++++
            │   xxxx   +++++++++
            │   xxxxx   ++++++++
            │   xxxxxx   +++++++
            │   xxxxxxx   ++++++
            │   xxxxxxxx    ++++
            │
           ─┼────────────────────► Tx_DLL
           0│                       127
        */

        searchPoint.rxDLL = phyTuneWindowParams->rxHighSearchEnd;
        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;
        searchResult = OSPI_phyFindRxHigh(&searchPoint, &backupPoint, arr);

        while(!searchResult)
        {
            searchPoint.rdDelay--;
            if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                break;
            }
            searchResult = OSPI_phyFindRxHigh(&searchPoint, &backupPoint, arr);
        }

        /***************************** BACKUP Secondary Rx_High Search *********************/
        /*
         * Find rxDLL Max
         * Start the rdDelay (Read delay) from maximum and decrement it.
         */

        if (searchPoint.txDLL >= (phyTuneWindowParams->txDllHighWindowStart + phyTuneWindowParams->txDLLSearchOffset )){
            searchPoint.txDLL -= phyTuneWindowParams->txDLLSearchOffset;
        }
        else
        {
            searchPoint.txDLL = phyTuneWindowParams->txDllHighWindowStart;
        }

        searchPoint.rxDLL = phyTuneWindowParams->rxHighSearchEnd;
        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;
        searchResult = OSPI_phyFindRxHigh(&searchPoint, &sec_rxHigh, arr);

        while(!searchResult)
        {
            searchPoint.rdDelay--;
            if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                break;
            }
            searchResult = OSPI_phyFindRxHigh(&searchPoint, &sec_rxHigh, arr);
        }

        /*
         * Compare the Primary and Secondary point
         * Pick the point which has passing maximum rxDll
        ┌────────┬───────────┬────────────────────────────────────┐
        │Primary │ Secondary │   Final  Point                     │
        │ Search │  Search   │                                    │
        │        │           │                                    │
        ├────────┼───────────┼────────────────────────────────────┤
        │ Fail   │   Fail    │  Return Fail                       │
        ├────────┼───────────┼────────────────────────────────────┤
        │ Fail   │   Pass    │  Pick Secondary search             │
        ├────────┼───────────┼────────────────────────────────────┤
        │ Pass   │   Fail    │  Pick Primary search               │
        ├────────┼───────────┼────────────────────────────────────┤
        │ Pass   │   Pass    │ If(secondary.rxDll > primary.rxDll)│
        │        │           │ Pick Secondary search point        │
        │        │           │ Else                               │
        │        │           │ Pick Primary search point          │
        └────────┴───────────┴────────────────────────────────────┘
        */

        if(sec_rxHigh.rxDLL != 127U)
        {
            if(backupPoint.rxDLL == 127U)
            {
                backupPoint = sec_rxHigh;
            }
            else
            {
                if(sec_rxHigh.rxDLL > backupPoint.rxDLL)
                {
                    backupPoint = sec_rxHigh;
                }
            }
        }
        else
        {
            if(backupPoint.rxDLL == 127U)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                return status;
            }
        }

        if(backupPoint.rxDLL > rxHigh.rxDLL)
        {
            rxHigh = backupPoint;
        }
    }

    /***************************** GOLDEN Tx_Low Search *********************/
    /*
     * Look for txDLL boundaries at 1/4 of rxDLL window
     * Find txDLL Min
     */
    /*
                  Rx_DLL
                 127 ▲
                     │   ++++++++++++++++
          Rx_High    │     ++++++++++++++
              ───────┼──►x   ++++++++++++
                     │   xx   +++++++++++
                     │   xxx   ++++++++++
                     │   xxxx   +++++++++
                     │   xxxxx   ++++++++
      Fix Rx_DLL     │   xxxxxx   +++++++
     1/4 between     │   xxxxxxx   ++++++
  Rx_High and Rx_Low │   xxxxxxxx   +++++
               ──────┼─► ◄───┬──►    ++++
                     │   xxxx│xxxxx   +++
           Rx_Low    │   xxxx│xxxxxx   ++
               ──────┼──►xxxx│xxxxxxx   +
                     │       │
                    ─┼───────┼───────────►  Tx_DLL
                    0│       │          127
                             │
                        Search Tx_Low
     */

    searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
    searchPoint.rxDLL = rxLow.rxDLL+(rxHigh.rxDLL-rxLow.rxDLL)/4U;
    searchPoint.txDLL = phyTuneWindowParams->txLowSearchStart;
    searchResult = OSPI_phyFindTxLow(&searchPoint, &txLow, arr);

    while(!searchResult)
    {
        searchPoint.rdDelay++;
        searchResult = OSPI_phyFindTxLow(&searchPoint, &txLow, arr);

        if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
        {
            status = OSPI_PHY_OLD_TUNING_FAILURE;
            return status;
        }
    }

    /***************************** GOLDEN Tx_High Search *********************/
    /*
     * Find txDLL Max
     * Start the rdDelay (Read delay) from maximum and decrement it.
     */
    /*
                Rx_DLL
               127 ▲
                   │   +++++++++++++++++
        Rx_High    │     +++++++++++++++
            ───────┼──►x   +++++++++++++
                   │   xx   ++++++++++++
                   │   xxx   +++++++++++
                   │   xxxx   ++++++++++
                   │   xxxxx   +++++++++
     Fix Rx_DLL    │   xxxxxx   ++++++++
    1/4 between    │   xxxxxxx   +++++++
 Rx_High and Rx_Low│   xxxxxxxx   ++++++
             ──────┼─► xxxxxxxxx   ◄─┬─►
                   │   xxxxxxxxxx   +│++
         Rx_Low    │   xxxxxxxxxxx   │++
             ──────┼──►xxxxxxxxxxxx  │++
                   │                 │
                  ─┼─────────────────┼─►  Tx_DLL
                  0│                 │127
                                  Search Tx_Max
    */

    searchPoint.txDLL = phyTuneWindowParams->txHighSearchEnd;
    searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;
    searchResult = OSPI_phyFindTxHigh( &searchPoint, &txHigh, arr);

    while(!searchResult)
    {
        searchPoint.rdDelay--;
        searchResult = OSPI_phyFindTxHigh(&searchPoint, &txHigh, arr);

        if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
        {
            status = OSPI_PHY_OLD_TUNING_FAILURE;
            return status;
        }
    }

    /*
     * Check a different point if the txLow and txHigh are on the same rdDelay.
     * This avoids mistaking the metastability gap for a txDLL boundary
     */
    if(txLow.rdDelay == txHigh.rdDelay)
    {

        /***************************** BACKUP Tx_Low Search *********************/
        /* Look for txDLL boundaries at 3/4 of rxDLL window */
        /* Find txDLL Min */
        /*
             Rx_DLL
            127 ▲
                │
       Rx_High──┼──►+++++++++++++++++
                │   +++++++++++++++++
    Fix Rx_DLL  │   +++++++++++++++++
3/4 of Rx_High ─┼─► ◄───┬───►++++++++
    and Rx_Low  │     ++│++++++++++++
                │      +│++++++++++++
                │   x   │++++++++++++
                │   xx  │++++++++++++
                │   xxx │ +++++++++++
                │   xxxx│  ++++++++++
                │   xxxx│   +++++++++
                │   xxxx│x   ++++++++
                │   xxxx│xx   +++++++
        Rx_Low──┼──►xxxx│xxx   ++++++
                │       │
               ─┼───────┼────────────► Tx_DLL
               0│       │               127
                   Search Tx_Min
        */

        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMin;
        searchPoint.rxDLL = rxLow.rxDLL + 3U*(rxHigh.rxDLL-rxLow.rxDLL)/4U;
        searchPoint.txDLL = phyTuneWindowParams->txLowSearchStart;
        searchResult = OSPI_phyFindTxLow(&searchPoint, &backupPoint, arr);
        while(!searchResult)
        {
            searchPoint.rdDelay++;
            searchResult = OSPI_phyFindTxLow(&searchPoint, &backupPoint, arr);
            if(searchPoint.rdDelay > phyTuneWindowParams->rdDelayMax)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                return status;
            }
        }
        if(backupPoint.txDLL < txLow.txDLL)
        {
            txLow = backupPoint;
        }

        /***************************** BACKUP Tx_High Search *********************/
        /*
         * Find txDLL Max
         * Start the rdDelay (Read delay) from maximum and decrement it.
         */
        /*
         Rx_DLL
          127
            ▲
            │
   Rx_High──┼──►+++++++++++++++++
            │   +++++++++++++++++
 Fix Rx_DLL │   +++++++++++++++++
 3/4 of ────┼─► +++++++◄────┬───►
  Rx_High   │     ++++++++++│++++
   and      │      +++++++++│++++
  Rx_Low    │   x   ++++++++│++++
            │   xx   +++++++│++++
            │   xxx   ++++++│++++
            │   xxxx   +++++│++++
            │   xxxxx   ++++│++++
            │   xxxxxx   +++│++++
            │   xxxxxxx   ++│++++
    Rx_Low──┼──►xxxxxxxx   +│++++
            │               │
           ─┼───────────────┼────► Tx_DLL
           0│               │       127
                         Search Tx_Max
        */

        searchPoint.txDLL = phyTuneWindowParams->txHighSearchEnd;
        searchPoint.rdDelay = phyTuneWindowParams->rdDelayMax;
        searchResult = OSPI_phyFindTxHigh(&searchPoint, &backupPoint, arr);
        while(!searchResult)
        {
            searchPoint.rdDelay--;
            searchResult = OSPI_phyFindTxHigh(&searchPoint, &backupPoint, arr);
            if(searchPoint.rdDelay < phyTuneWindowParams->rdDelayMin)
            {
                status = OSPI_PHY_OLD_TUNING_FAILURE;
                return status;
            }
        }
        if(backupPoint.txDLL > txHigh.txDLL)
        {
            txHigh = backupPoint;
        }
    }

    /*
     * Set bottom left and top right right corners.  These are theoretical corners. They may not actually be "good" points.
     * But the longest diagonal of the shmoo will be between these corners.
     */

    /* Bottom Left */
    bottomLeft.txDLL = txLow.txDLL;
    bottomLeft.rxDLL = rxLow.rxDLL;

    if(txLow.rdDelay <= rxLow.rdDelay)
    {
        bottomLeft.rdDelay = txLow.rdDelay;
    }
    else
    {
        bottomLeft.rdDelay = rxLow.rdDelay;
    }

    backupCornerPoint = bottomLeft;
    backupCornerPoint.txDLL += 4U;
    backupCornerPoint.rxDLL += 4U;


    status = OSPI_phyIsPassingPoint(&backupCornerPoint, arr);

    if(status == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        backupCornerPoint.rdDelay--;
        status = OSPI_phyIsPassingPoint(&backupCornerPoint, arr);
    }

    if(status == OSPI_PHY_OLD_TUNING_SUCCESS)
    {
        bottomLeft.rdDelay = backupCornerPoint.rdDelay;
    }

    topRight.txDLL = txHigh.txDLL;
    topRight.rxDLL = rxHigh.rxDLL;

    if(txHigh.rdDelay > rxHigh.rdDelay)
    {
        topRight.rdDelay = txHigh.rdDelay;
    }
    else
    {
        topRight.rdDelay = rxHigh.rdDelay;
    }

    backupCornerPoint = topRight;
    backupCornerPoint.txDLL -= 4U;
    backupCornerPoint.rxDLL -= 4U;

    status = OSPI_phyIsPassingPoint(&backupCornerPoint, arr);

    if(status == OSPI_PHY_OLD_TUNING_FAILURE)
    {
        backupCornerPoint.rdDelay++;
        status = OSPI_phyIsPassingPoint(&backupCornerPoint, arr);
    }

    if(status == OSPI_PHY_OLD_TUNING_SUCCESS)
    {
        topRight.rdDelay = backupCornerPoint.rdDelay;
    }

    /* Find the equation of diagonal between topRight and bottomLeft */

    /* Slope and Intercept*/
    slope = ((float)topRight.rxDLL-(float)bottomLeft.rxDLL)/((float)topRight.txDLL-(float)bottomLeft.txDLL);
    /* Binary Search */
    OSPI_PhyConfig left, right;
    /* Search along the diagonal between corners */
    left = bottomLeft;
    right = topRight;
    searchPoint.txDLL = left.txDLL + ((right.txDLL - left.txDLL) / 2);
    searchPoint.rxDLL = left.rxDLL + ((right.rxDLL - left.rxDLL) / 2);
    searchPoint.rdDelay = left.rdDelay;

    do
    {
        status = OSPI_phyIsPassingPoint(&searchPoint, arr);
        if(status == OSPI_PHY_OLD_TUNING_FAILURE)
        {
            /*
            * As the read failed, we go to the lower half for finding the gap low
            */
            right.txDLL = searchPoint.txDLL;
            right.rxDLL = searchPoint.rxDLL;

            searchPoint.txDLL = left.txDLL + ((searchPoint.txDLL - left.txDLL)/2);
            searchPoint.rxDLL = left.rxDLL + ((searchPoint.rxDLL - left.rxDLL)/2);
        } else
        {
            /*
            * As the read is a success we go to the upper half for finding the gap low
            */
            left.txDLL = searchPoint.txDLL;
            left.rxDLL = searchPoint.rxDLL;

            searchPoint.txDLL = searchPoint.txDLL + ((right.txDLL - searchPoint.txDLL)/2);
            searchPoint.rxDLL = searchPoint.rxDLL + ((right.rxDLL - searchPoint.rxDLL)/2);
        }
    /* Break the loop if the window has closed. */
    } while ((right.txDLL - left.txDLL >= 2) && (right.rxDLL - left.rxDLL >= 2));

    gapLow = searchPoint;

    /* If there's only one segment, put tuning point in the middle and adjust for temperature */
    if(bottomLeft.rdDelay == topRight.rdDelay)
    {
        /* Start of the metastability gap is a good approximation for the topRight */
        topRight = gapLow;
        searchPoint.rdDelay = bottomLeft.rdDelay;
        searchPoint.txDLL = (bottomLeft.txDLL+topRight.txDLL)/2U;
        searchPoint.rxDLL = (bottomLeft.rxDLL+topRight.rxDLL)/2U;

    }
    else
    {
        /* If there are two segments, find the start and end of the second one */
        left = bottomLeft;
        right = topRight;
        searchPoint.txDLL = left.txDLL + ((right.txDLL - left.txDLL) / 2);
        searchPoint.rxDLL = left.rxDLL + ((right.rxDLL - left.rxDLL) / 2);
        searchPoint.rdDelay = right.rdDelay;
        do{
            status = OSPI_phyIsPassingPoint(&searchPoint, arr);
            if(status == OSPI_PHY_OLD_TUNING_FAILURE)
            {
                /*
                * As the read failed, we go to the upper half for finding the gap high
                */
                left.txDLL = searchPoint.txDLL;
                left.rxDLL = searchPoint.rxDLL;

                searchPoint.txDLL = searchPoint.txDLL + ((right.txDLL - searchPoint.txDLL)/2);
                searchPoint.rxDLL = searchPoint.rxDLL + ((right.rxDLL - searchPoint.rxDLL)/2);
            }
            else
            {
                /*
                * As the read is a success we go to the lower half for finding the gap high
                */
                right.txDLL = searchPoint.txDLL;
                right.rxDLL = searchPoint.rxDLL;

                searchPoint.txDLL = left.txDLL + ((searchPoint.txDLL - left.txDLL)/2);
                searchPoint.rxDLL = left.rxDLL + ((searchPoint.rxDLL - left.rxDLL)/2);
            }
            /* Break the loop if the window has closed. */
        } while ((right.txDLL - left.txDLL >= 2) && (right.rxDLL - left.rxDLL >= 2));
        gapHigh = searchPoint;

        /* Place the final tuning point of the PHY in the corner furthest from the gap */
        int len1 = abs(gapLow.txDLL-bottomLeft.txDLL) + abs(gapLow.rxDLL-bottomLeft.rxDLL);
        int len2 = abs(gapHigh.txDLL-topRight.txDLL) + abs(gapHigh.rxDLL-topRight.rxDLL);

        if(len2 > len1)
        {
            searchPoint = topRight;
            searchPoint.txDLL -= 16;
            searchPoint.rxDLL -= (int)((float)16*slope);
        }
        else
        {
            searchPoint = bottomLeft;
            searchPoint.txDLL += 16;
            searchPoint.rxDLL += (int)((float)16*slope);
        }
    }

    status = OSPI_phyIsPassingPoint(&searchPoint, arr);

    if(status == OSPI_PHY_OLD_TUNING_SUCCESS)
    {
        otp->rxDLL = searchPoint.rxDLL;
        otp->txDLL = searchPoint.txDLL;
        otp->rdDelay = searchPoint.rdDelay;
        if(check_circle_area(arr, otp->rxDLL, otp->txDLL, radius,  otp->rxDLL+radius, otp->txDLL+radius, otp->rdDelay) == false)
        {
            otp->rdDelay = READ_DELAY_INVALID;
        }
    }
    else
    {
        otp->rxDLL = 0;
        otp->txDLL = 0;
        otp->rdDelay = READ_DELAY_INVALID;
    }

    return status;
}
