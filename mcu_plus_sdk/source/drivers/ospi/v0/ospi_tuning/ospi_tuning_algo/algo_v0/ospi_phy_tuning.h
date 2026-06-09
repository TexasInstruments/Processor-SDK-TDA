#ifndef OSPI_OLD_TUNING_H
#define OSPI_OLD_TUNING_H

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define OSPI_PHY_OLD_TUNING_SUCCESS (1U)
#define OSPI_PHY_OLD_TUNING_FAILURE (0U)
#define READ_DELAY_INVALID -1

typedef struct
{
    int txDllLowWindowStart;
    /**< Tx Dll window lower value to search RxDLL low and high. This corresponds to the bottom left point serach.*/
    int txDllLowWindowEnd;
    /**< Tx Dll window higher value to search RxDLL low and high. This corresponds to the bottom left point search. */
    int txDllHighWindowStart;
    /**< Tx Dll window lower value to search RxDLL low and high. This corresponds to the top right point search.*/
    int txDllHighWindowEnd;
    /**< Tx Dll window higher value to search RxDLL low and high. This corresponds to the top right point search. */
    int rxLowSearchStart;
    /**< Rx Dll lower value for Rx Dll low search. The value of Rx dll will lie in this window bottom left point search. */
    int rxLowSearchEnd;
    /**< Rx Dll higher value for Rx Dll low search. The value of Rx dll will lie in this window bottom left point search. */
    int rxHighSearchStart;
    /**< Rx Dll lower value for Rx Dll high search. The value of Rx dll will lie in this window top right point search. */
    int rxHighSearchEnd;
    /**< Rx Dll higher value for Rx Dll high search. The value of Rx dll will lie in this window for top right point search. */
    int txLowSearchStart;
    /**< Tx Dll lower value for Tx Dll low search. The value of Tx dll will lie in this window. */
    int txLowSearchEnd;
    /**< Tx Dll higher value for Tx Dll low search. The value of Tx dll will lie in this window. */
    int txHighSearchStart;
    /**< Tx Dll lower value for Tx Dll high search. The value of Tx dll will lie in this window. */
    int txHighSearchEnd;
    /**< Tx Dll higher value for Tx Dll high search. The value of Tx dll will lie in this window. */
    int txDLLSearchOffset;
    /**< Tx Dll step increase for backup Rx Dll low and high search. */
    unsigned int rxTxDLLSearchStep;
    /**< Rx Dll and Tx DLL step increase for Rx Dll and Tx Dll low and high search. */
    unsigned int rdDelayMin;
    /**< Minimum value of Read delay for Read Delay Capture Register for tuning search. */
    unsigned int rdDelayMax;
    /**< Maximum value of Read delay for Read Delay Capture Register for tuning search. */
} OSPI_PhyTuneWindowParams;

int check_circle_area(short int (*arr)[128][128], int row, int col, int radius, int rows, int cols, int checkValue);

#endif