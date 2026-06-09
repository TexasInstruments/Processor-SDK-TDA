#include "c_test_main.h"
#include "../ospi_tuning_algo/algo_v0/ospi_phy_tuning.h"
#include "../ospi_tuning_algo/algo_v1/ospi_phy_tuning.h"

#define MIN_DISTANCE_FROM_SLOPE 10

# ifndef MIN
# define MIN(x,y) \
    ({typeof (x) _x = (x);\
      typeof (y) _y = (y);\
      _x < _y ? _x : _y; \
    })
# endif
# ifndef MAX
# define MAX(x,y) \
    ({typeof (x) _x = (x); \
      typeof (y) _y = (y); \
      _x > _y ? _x : _y; \
    })
# endif

OSPI_phyParams PhyParams = {
    .radius             = 10,
    .rxTxDllMin         = 0,
    .rxTxDllMax         = 127,
    .minReadDelay       = 0,
    .maxReadDelay       = 4,
    .minPassSize        = 100,
    .diagonalShift      = 10,
    .maxDiagonalShift   = 70,
    .numConsecutiveFail = 5,
    .numConsecutivePass = 10,
    .rdDelaySearchStep  = 16,
};

OSPI_phyParams* gPhyParams = &PhyParams;

double linear_probability(double a, double max)
{
    return (a/max);
}
extern int OSPI_phyFindOTP1(short int (*arr)[128][128], int32_t radius, OSPI_PhyConfig *otp);

int32_t OSPI_phySetAndRead(void* handle, uint32_t offset, OSPI_PhyConfig *result)
{
    if(result->rxDLL < 0 || result->rxDLL > 127 || result->txDLL < 0 || result->txDLL > 127)
    {
        return OSPI_PHY_TUNING_FAILURE;
    }

    usleep(30);

    if((*((short int (*)[128][128])(handle)))[result->rxDLL][result->txDLL] == result->rdDelay)
    {
        return OSPI_PHY_TUNING_SUCCESS;
    }

    return OSPI_PHY_TUNING_FAILURE;
}

int check_circle_area(short int (*arr)[128][128], int row, int col, int radius, int rows, int cols, int checkValue) {
    // Check if row and column are within the matrix boundaries
    if (row < 0 || row >= rows || col < 0 || col >= cols || rows > 128 || cols > 128) {
        return false;
    }

    int i,j;
    // Check if all elements in the circle area are equal to 1
    for (i = row-radius; i < rows && i < 128;  i++) {
        for (j = col-radius; j < cols && j < 128; j++) {
            // Calculate the distance between the current point and the center of the circle
            double distance = sqrt(pow(i - row, 2) + pow(j - col, 2));
            // Check if the point is within the circle
            if (distance <= radius) {
                // Check if the point is within the matrix boundaries
                if (i >= 0 && i < rows && j >= 0 && j < cols) {
                    if ((*arr)[i][j] != checkValue) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

// Function to calculate the perpendicular distance
double calculate_distance(double a, double b, double c, double x1, double y1) {
    if (a == 0 && b == 0) {
        printf("The line equation is invalid\n");
        return -1; // Return an error value
    }
    double numerator = fabs(a*x1 + b*y1 - c);
    double denominator = sqrt(a*a + b*b);
    return numerator / denominator;
}

void printArr(short int (*arr)[128][128])
{
    int i,j;

    for(i = 127; i >= 0 ; i--)
    {
        printf("row no: %d \t", i);
        for(j = 0; j < 128; j++ )
        {
            if((*arr)[i][j] == READ_DELAY_INVALID)
            {
                printf("*");
            }
            else
            {
                printf("%d", (*arr)[i][j]);
            }
        }
        printf("\n");
    }
}

int generate_random()
{
    int max = 10;
    int rnd_num = rand() % max;
    return rnd_num;
}

int generate_random_integer(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void shuffleArray(int *arr, int size)
{
    int i;

    for ( i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

int main(int argc, char *argv[])
{
    srand(time(0));
    int iteration = atoi(argv[1]);
    short int testArr[128][128];
    int passingPoints = 0;
    int i, j, k, l;

    for(i = 0; i <128 ; i++)
    {
        for(j = 0; j < 128; j++)
        {
           testArr[i][j] = FIRST_PASS_READ_DELAY;
        }
    }

    int testcase_num = iteration;

    int tx_setup_fail_ol, rx_hold_fail_ot, tx_setup_fail_or,rx_hold_fail_ob, cdcOffset;
    int tx_setup_mix_pass_fail_il, rx_hold_mix_pass_fail_it, tx_hold_mix_pass_fail_ir, rx_setup_mix_pass_fail_ib;
    int mix_fail_width, cdc_fail_width;
    float total_time = 0;
    OSPI_PhyConfig otp1 = { 0, 0, READ_DELAY_INVALID}, otp2 = { 0, 0, READ_DELAY_INVALID};

    switch (testcase_num)
    {
        case 1:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 8;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 30;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 2:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 8;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 180;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 3:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 8;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 4:
            tx_setup_fail_ol = 6;
            rx_hold_fail_ot = 14;
            tx_setup_fail_or = 8;
            rx_hold_fail_ob = 10;
            cdcOffset = 40;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 5:
            tx_setup_fail_ol = 8;
            rx_hold_fail_ot = 12;
            tx_setup_fail_or = 14;
            rx_hold_fail_ob = 8;
            cdcOffset = 160;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 6:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 8;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 80;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 7:
            tx_setup_fail_ol = 15;
            rx_hold_fail_ot = 14;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 70;
            tx_setup_mix_pass_fail_il = 12;
            rx_hold_mix_pass_fail_it = 12;
            tx_hold_mix_pass_fail_ir = 12;
            rx_setup_mix_pass_fail_ib = 12;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 8:
            tx_setup_fail_ol = 20;
            rx_hold_fail_ot = 20;
            tx_setup_fail_or = 20;
            rx_hold_fail_ob = 20;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 12;
            rx_hold_mix_pass_fail_it = 12;
            tx_hold_mix_pass_fail_ir = 12;
            rx_setup_mix_pass_fail_ib = 12;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 9:
            tx_setup_fail_ol = 15;
            rx_hold_fail_ot = 14;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 12;
            rx_hold_mix_pass_fail_it = 12;
            tx_hold_mix_pass_fail_ir = 12;
            rx_setup_mix_pass_fail_ib = 12;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 10:
            tx_setup_fail_ol = 25;
            rx_hold_fail_ot = 14;
            tx_setup_fail_or = 25;
            rx_hold_fail_ob = 14;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 10;
            rx_setup_mix_pass_fail_ib = 10;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 11:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 10;
            tx_setup_fail_or = 10;
            rx_hold_fail_ob = 10;
            cdcOffset = 30;
            tx_setup_mix_pass_fail_il = 15;
            rx_hold_mix_pass_fail_it = 15;
            tx_hold_mix_pass_fail_ir = 14;
            rx_setup_mix_pass_fail_ib = 15;
            mix_fail_width = 15;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 12:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 10;
            tx_setup_fail_or = 10;
            rx_hold_fail_ob = 10;
            cdcOffset = 30;
            tx_setup_mix_pass_fail_il = 15;
            rx_hold_mix_pass_fail_it = 15;
            tx_hold_mix_pass_fail_ir = 14;
            rx_setup_mix_pass_fail_ib = 15;
            mix_fail_width = 15;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 13:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 60;
            tx_setup_fail_or = 20;
            rx_hold_fail_ob = 20;
            cdcOffset = 40;
            tx_setup_mix_pass_fail_il = 20;
            rx_hold_mix_pass_fail_it = 20;
            tx_hold_mix_pass_fail_ir = 20;
            rx_setup_mix_pass_fail_ib = 20;
            mix_fail_width = 20;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 14:
            tx_setup_fail_ol = 40;
            rx_hold_fail_ot = 10;
            tx_setup_fail_or = 20;
            rx_hold_fail_ob = 60;
            cdcOffset = 160;
            tx_setup_mix_pass_fail_il = 10;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 10;
            rx_setup_mix_pass_fail_ib = 40;
            mix_fail_width = 20;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 15:
            tx_setup_fail_ol = 25;
            rx_hold_fail_ot = 25;
            tx_setup_fail_or = 25;
            rx_hold_fail_ob = 25;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 10;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 10;
            rx_setup_mix_pass_fail_ib = 10;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 16:
            tx_setup_fail_ol = 25;
            rx_hold_fail_ot = 25;
            tx_setup_fail_or = 25;
            rx_hold_fail_ob = 25;
            cdcOffset = 127;
            tx_setup_mix_pass_fail_il = 10;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 10;
            rx_setup_mix_pass_fail_ib = 10;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 17:
            tx_setup_fail_ol = 30;
            rx_hold_fail_ot = 30;
            tx_setup_fail_or = 30;
            rx_hold_fail_ob = 30;
            cdcOffset = 30;
            tx_setup_mix_pass_fail_il = 15;
            rx_hold_mix_pass_fail_it = 15;
            tx_hold_mix_pass_fail_ir = 14;
            rx_setup_mix_pass_fail_ib = 15;
            mix_fail_width = 15;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 18:
            tx_setup_fail_ol = 35;
            rx_hold_fail_ot = 35;
            tx_setup_fail_or = 35;
            rx_hold_fail_ob = 35;
            cdcOffset = 160;
            tx_setup_mix_pass_fail_il = 10;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 10;
            rx_setup_mix_pass_fail_ib = 10;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 19: // Tesla test case
            tx_setup_fail_ol = 8;
            rx_hold_fail_ot = 15;
            tx_setup_fail_or = 0;
            rx_hold_fail_ob = 0;
            cdcOffset = 50;
            tx_setup_mix_pass_fail_il = 2;
            rx_hold_mix_pass_fail_it = 25;
            tx_hold_mix_pass_fail_ir = 3;
            rx_setup_mix_pass_fail_ib = 0;
            mix_fail_width = 5;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 20:
            tx_setup_fail_ol = 50;
            rx_hold_fail_ot = 20;
            tx_setup_fail_or = 20;
            rx_hold_fail_ob = 50;
            cdcOffset = 160;
            tx_setup_mix_pass_fail_il = 20;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 20;
            rx_setup_mix_pass_fail_ib = 20;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 21:
            tx_setup_fail_ol = 20;
            rx_hold_fail_ot = 50;
            tx_setup_fail_or = 50;
            rx_hold_fail_ob = 20;
            cdcOffset = 70;
            tx_setup_mix_pass_fail_il = 20;
            rx_hold_mix_pass_fail_it = 10;
            tx_hold_mix_pass_fail_ir = 20;
            rx_setup_mix_pass_fail_ib = 20;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            cdc_fail_width = 10;
            break;
        case 22:
            tx_setup_fail_ol = generate_random_integer(0,50);
            rx_hold_fail_ot = generate_random_integer(0,50);
            tx_setup_fail_or = generate_random_integer(0,50);
            rx_hold_fail_ob = generate_random_integer(0,50);
            cdcOffset = generate_random_integer(0,180);
            tx_setup_mix_pass_fail_il = generate_random_integer(0,30);
            rx_hold_mix_pass_fail_it = generate_random_integer(0,30);
            tx_hold_mix_pass_fail_ir = generate_random_integer(0,30);
            rx_setup_mix_pass_fail_ib = generate_random_integer(0,30);
            mix_fail_width = generate_random_integer(0,20);
            gPhyParams->radius = 10;
            cdc_fail_width = generate_random_integer(5,20);
            break;
        case 23: // MCU TEST CASE 1
            tx_setup_fail_ol = 2;
            rx_hold_fail_ot = 79;
            tx_setup_fail_or = 66;
            rx_hold_fail_ob = 6;
            cdcOffset = 20;
            tx_setup_mix_pass_fail_il = 1;
            rx_hold_mix_pass_fail_it = 3;
            tx_hold_mix_pass_fail_ir = 1;
            rx_setup_mix_pass_fail_ib = 1;
            mix_fail_width = 2;
            gPhyParams->radius = 8;
            cdc_fail_width = 6;
            break;
        case 24: // MCU TEST CASE 2
            tx_setup_fail_ol = 3;
            rx_hold_fail_ot = 38;
            tx_setup_fail_or = 10;
            rx_hold_fail_ob = 9;
            cdcOffset = 22;
            tx_setup_mix_pass_fail_il = 4;
            rx_hold_mix_pass_fail_it = 40;
            tx_hold_mix_pass_fail_ir = 55;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 7;
            gPhyParams->radius = 8;
            cdc_fail_width = 3;
            break;
        case 25: // MCU TEST CASE 3
            tx_setup_fail_ol = 0;
            rx_hold_fail_ot = 33;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 27;
            cdcOffset = 35;
            tx_setup_mix_pass_fail_il = 0;
            rx_hold_mix_pass_fail_it = 7;
            tx_hold_mix_pass_fail_ir = 6;
            rx_setup_mix_pass_fail_ib = 12;
            mix_fail_width = 4;
            gPhyParams->radius = 8;
            cdc_fail_width = 30;
            break;
        default:
            tx_setup_fail_ol = 10;
            rx_hold_fail_ot = 8;
            tx_setup_fail_or = 12;
            rx_hold_fail_ob = 14;
            cdcOffset = 30;
            tx_setup_mix_pass_fail_il = 8;
            rx_hold_mix_pass_fail_it = 6;
            tx_hold_mix_pass_fail_ir = 9;
            rx_setup_mix_pass_fail_ib = 4;
            mix_fail_width = 10;
            gPhyParams->radius = 10;
            break;
    }

    //cdc offset slope
    if(cdcOffset < 128)
    {
        for( j = 0; j <= cdcOffset; j++)
        {
            if(((cdcOffset - j) >= 0) && ((cdcOffset - j) < 128))
            {
                testArr[cdcOffset-j][j] = READ_DELAY_INVALID;
            }
        }

        // All points on right side should be 2
        for( i = rx_hold_fail_ob; i < 128 - rx_hold_fail_ot; i++)
        {
            if(cdcOffset - i >= tx_setup_fail_ol)
            {
                k = cdcOffset - i + 1;
            }
            else
            {
                k = tx_setup_fail_ol;
            }
            for( j = k; j < 128 - tx_setup_fail_or; j++)
            {
                if((i < 128) && ( j < 128))
                {
                    testArr[i][j] = SECOND_PASS_READ_DELAY;
                }
            }
        }

        // fail region on left side of slope
        for( i = (cdc_fail_width/2) - 1; i > 0; i--)
        {
            k = cdcOffset-i;
            for( j = 0; j <= k; j++)
            {
                if(((k-j) >= 0) && ((k-j) < 128))
                {
                    testArr[k-j][j] = READ_DELAY_INVALID;
                }
            }
        }

        // mix fail region on left side of slope
        for( i = mix_fail_width; i > 0; i--)
        {
            k = cdcOffset-(cdc_fail_width/2) + 1 - i;
            j = tx_setup_fail_ol;
            while(j <= k)
            {
                int rnd_val = generate_random();
                if(rnd_val > 5)
                {
                    j += rnd_val/2;
                }
                else
                {
                    for( l = 0; l < rnd_val; l++ )
                    {
                        if(((k-j-l) >= 0) && ((k-j-l) < 128) && ((j+l) < 128))
                        {
                            testArr[k-j-l][j+l] = READ_DELAY_INVALID;
                        }
                    }
                    j += rnd_val;
                }
            }
        }

        // fail region on right side of slope
        for( i = 1; i < (cdc_fail_width/2) + 1 ; i++)
        {
            k = cdcOffset + i;
            for( j = 0; j <= k; j++)
            {
                if(((k-j) >= 0) && ((k-j) < 128))
                {
                    testArr[k-j][j] = READ_DELAY_INVALID;
                }
            }
        }

        // mix fail region on right side of slope
        for( i = mix_fail_width; i > 0; i--)
        {
            k = cdcOffset+(cdc_fail_width/2)+i;
            j = tx_setup_fail_ol;
            while(j <= k)
            {
                int rnd_val = generate_random();
                if(rnd_val > 5)
                {
                    j += rnd_val/2;
                }
                else
                {
                    for( l = 0; l < rnd_val; l++ )
                    {
                        // Check if first dimension index is valid
                        if(((k-j-l) < 0) || (k-j-l) > 127) // Assuming 127 is also the max for first dimension
                        {
                            break;
                        }

                        testArr[k-j-l][j+l] = READ_DELAY_INVALID;

                        if(j+l > 127)
                        {
                            break;
                        }

                    }
                    j += rnd_val;
                }
                if(j > 127)
                {
                    break;
                }
            }
        }
    }
    else
    {
        for( j = 127 - tx_setup_fail_or; ((j >= 0) && ((cdcOffset-j) < 128) && ((cdcOffset-j) >= 0)); j--)
        {
            testArr[cdcOffset-j][j] = READ_DELAY_INVALID;
        }

        // All points on right side of slope should be 2
        for( i = cdcOffset - 127 - rx_hold_fail_ob; i <= 127 - rx_hold_fail_ot;  i++)
        {
            for( j = cdcOffset - i + 1; j <= 127 - tx_setup_fail_or; j++)
            {
                if((j >= 0) && (j < 128))
                {
                    testArr[i][j] = SECOND_PASS_READ_DELAY;
                }
            }
        }

        // fail region on right side of slope
        for( i = 1 ; i <= (cdc_fail_width/2); i++)
        {
            for( j = 127 - tx_setup_fail_or; cdcOffset-j < 128; j--)
            {
                if(((cdcOffset + i - j) >= 0) && ((cdcOffset + i - j) < 128))
                {
                    testArr[cdcOffset+ i - j][j] = READ_DELAY_INVALID;
                }
            }
        }

        // mix fail region on right side of slope
        for( i = 1; i <= mix_fail_width; i++)
        {
            k = cdcOffset + (cdc_fail_width/2) + i;
            j = 127 - tx_setup_fail_or;
            while((j > (k - 128)) && (j >= 0))
            {
                int rnd_val = generate_random();
                if(rnd_val > 5)
                {
                    j -= (rnd_val/2 > 0) ? rnd_val/2 : 1;
                }
                else
                {
                    for( l = 0; l < rnd_val ; l++ )
                    {
                        if(((k-j+l) >= 0) && ((k-j+l) < 128) && ((j-l) >= 0) && ((j-l) < 128))
                        {
                            testArr[k-j+l][j-l] = READ_DELAY_INVALID;
                        }
                    }
                    j -= rnd_val;
                }
                if(j < 0)
                {
                    break;
                }
            }
        }

        // fail region on left side of slope
        for( i = 1 ; i <= cdc_fail_width/2 - 1; i++)
        {
            for( j = 127 - tx_setup_fail_or; (((cdcOffset-j) < 128) && (j >= 0)); j--)
            {
                if(((cdcOffset - i - j) >= 0) && ((cdcOffset - i - j) < 128))
                {
                    testArr[cdcOffset - i - j][j] = READ_DELAY_INVALID;
                }
            }
        }

        //mix fail region on left side of slope
        for( i = 1; i <= mix_fail_width; i++)
        {
            k = cdcOffset-(cdc_fail_width/2) + 1 -i;
            j = 127 - tx_setup_fail_or;
            while(j > k - 128)
            {
                int rnd_val = generate_random();
                rnd_val = (rnd_val > 0) ? rnd_val : 1;
                if(rnd_val > 5)
                {
                    j -= rnd_val/2;
                }
                else
                {
                    for( l = 0; l < rnd_val ; l++ )
                    {
                        if(((k-j+l) >= 0) && ((k-j+l) < 128) && ((j-l) >= 0) && ((j-l) < 128))
                        {
                            testArr[k-j+l][j-l] = READ_DELAY_INVALID;
                        }
                    }
                    j -= rnd_val;
                }
            }
            if(j < 0)
            {
                break;
            }
        }
    }

    // failure region
    // Tx setup fail ol

    for( i = 0; i < tx_setup_fail_ol; i++)
    {
        for( j = 0 ; j < 128 ; j++)
        {
            testArr[j][i] = READ_DELAY_INVALID;
        }
    }

    // Tx setup fail or

    for( i = 127; i > 127 - tx_setup_fail_or; i--)
    {
        for( j = rx_hold_fail_ob; j < 128; j++)
        {
            testArr[j][i] = READ_DELAY_INVALID;
        }
    }

    // rx hold fail ob

    for( i = 0; i < rx_hold_fail_ob; i++)
    {
        for( j = tx_setup_fail_ol; j < 128; j++)
        {
            testArr[i][j] = READ_DELAY_INVALID;
        }
    }

    // rx hold fail ot

    for( i = 127; i > 127 - rx_hold_fail_ot; i--)
    {
        for( j = tx_setup_fail_ol; j < 128 - tx_setup_fail_or; j++)
        {
            testArr[i][j] = READ_DELAY_INVALID;
        }
    }

    int num_passing_points = 128 - tx_setup_fail_or - tx_setup_fail_ol;
    int *arr = (int *)malloc(num_passing_points * sizeof(int));
    for( i = 0; i < num_passing_points ; i++)
    {
        arr[i] = i;
    }

    double *prob_arr = (double *)malloc(rx_hold_mix_pass_fail_it * sizeof(double));
    for( i = 0; i < rx_hold_mix_pass_fail_it; i++)
    {
        prob_arr[i] = linear_probability(rx_hold_mix_pass_fail_it - 1 - i, rx_hold_mix_pass_fail_it);
    }

    shuffleArray(arr, num_passing_points);

    // rx_hold_mix_pass_fail_it
    int steps = 0;
    for( i = 127 - rx_hold_fail_ot; i > (127 - rx_hold_mix_pass_fail_it - rx_hold_fail_ot); i--)
    {
        j = tx_setup_fail_ol;
        int num_zeroes = num_passing_points*prob_arr[steps];
        int rnd_indx = generate_random_integer(0, num_passing_points - 1);

        for( k = 0; k <= num_zeroes ; k++)
        {
            if(k  + rnd_indx >= num_passing_points - 1)
            {
                testArr[i][tx_setup_fail_ol + arr[k + 1 + rnd_indx - num_passing_points]] = READ_DELAY_INVALID;
            }
            else
            {
                testArr[i][tx_setup_fail_ol + arr[k + rnd_indx]] = READ_DELAY_INVALID;
            }
        }

        steps++;
    }

    // tx_setup_mix_pass_fail_il
    for( i = tx_setup_fail_ol; i < tx_setup_fail_ol + tx_setup_mix_pass_fail_il; i++)
    {
        j = rx_hold_fail_ob;
        while(j < 128-rx_hold_fail_ot)
        {
            int rnd_val = generate_random();
            if(rnd_val > 3)
            {
                j += rnd_val;
            }
            else
            {
                for( l = 0; l < rnd_val; l++)
                {
                    testArr[j+l][i] = READ_DELAY_INVALID;
                }
                j += rnd_val;
            }
        }
    }

    // rx_setup_mix_pass_fail_ib
    for( i = rx_hold_fail_ob; i < rx_hold_fail_ob + rx_setup_mix_pass_fail_ib; i++)
    {
        j = tx_setup_fail_ol + tx_setup_mix_pass_fail_il;
        while( j < 128 - tx_setup_fail_or)
        {
            int rnd_val = generate_random();
            if(rnd_val > 3)
            {

                j += rnd_val;
            }
            else
            {
                for( l = 0; l < rnd_val; l++)
                {
                    testArr[i][j+l] = READ_DELAY_INVALID;
                }
                j += rnd_val;
            }
        }
    }

    // tx_hold_mix_pass_fail_ir
    for( i = 127 - tx_setup_fail_or; i > (127- tx_setup_fail_or - tx_hold_mix_pass_fail_ir); i--)
    {
        j = rx_hold_fail_ob;
        while(j < 128 - rx_hold_fail_ot )
        {
            int rnd_val = generate_random();
            if(rnd_val > 3)
            {
                j += rnd_val;
            }
            else
            {
                for( l = 0; l < rnd_val; l++)
                {
                    testArr[j+l][i] = READ_DELAY_INVALID;
                }
                j += rnd_val;
            }
        }
    }

    if(testcase_num == 22)
    {
        FILE *ff = fopen("testParams.txt", "a");
        if (ff == NULL)
        {
            printf("Error opening file\n");
        }
        else
        {
            fprintf(ff, "Iteration: %d, tx_setup_fail_ol = %d, rx_hold_fail_ot = %d, tx_setup_fail_or = %d, rx_hold_fail_ob = %d, cdcOffset = %d, tx_setup_mix_pass_fail_il = %d, rx_hold_mix_pass_fail_it = %d, tx_hold_mix_pass_fail_ir = %d, rx_setup_mix_pass_fail_ib = %d, mix_fail_width = %d, gPhyParams->radius = %d, cdc_fail_width = %d\n", iteration,tx_setup_fail_ol,rx_hold_fail_ot,tx_setup_fail_or,rx_hold_fail_ob,cdcOffset,tx_setup_mix_pass_fail_il,rx_hold_mix_pass_fail_it,tx_hold_mix_pass_fail_ir,rx_setup_mix_pass_fail_ib,mix_fail_width,gPhyParams->radius, cdc_fail_width);
            fclose(ff);
        }
    }

    FILE* fptr;
    char pBinFileName[100];
    sprintf(pBinFileName, "binaries/test_case_%d.bin", iteration);
    fptr = fopen(pBinFileName,"wb");
    if(fptr != NULL)
    {
        fwrite((void*)testArr, sizeof(short int), 128*128, fptr);
        fclose(fptr);
    }
    else
    {
        printf("Error opening file\n");
    }

    printf("Iteration: %3d ", iteration);

    int count = 0;
    int *goldenPoints;
    int count_1 = 0, count_2 = 0;
    int passingValue = 0;
    for( i = rx_hold_fail_ob+rx_setup_mix_pass_fail_ib+gPhyParams->radius; i < 128 - rx_hold_fail_ot - rx_hold_mix_pass_fail_it-gPhyParams->radius ; i++)
    {
        for( j = tx_setup_fail_ol+tx_setup_mix_pass_fail_il+gPhyParams->radius; j < 128 - tx_setup_fail_or - tx_hold_mix_pass_fail_ir-gPhyParams->radius; j++)
        {
            if(testArr[i][j] == FIRST_PASS_READ_DELAY)
            {
                if(true == check_circle_area(&testArr, i,j, gPhyParams->radius, i+gPhyParams->radius, j+gPhyParams->radius, testArr[i][j]) && (calculate_distance(1, 1, cdcOffset, j, i) >= MIN_DISTANCE_FROM_SLOPE))
                {
                    count_1++;
                }
            }
            else if(testArr[i][j] == SECOND_PASS_READ_DELAY)
            {
                if(true == check_circle_area(&testArr,i,j, gPhyParams->radius, i+gPhyParams->radius, j+gPhyParams->radius, testArr[i][j]) && (calculate_distance(1, 1, cdcOffset, j, i) >= MIN_DISTANCE_FROM_SLOPE))
                {
                    count_2++;
                }
            }
        }
    }

    if (abs(count_1 - count_2) > 50)
    {
        if (count_1 > count_2)
        {
            passingValue = FIRST_PASS_READ_DELAY;
        }
        else
        {
            passingValue = SECOND_PASS_READ_DELAY;
        }

        count = count_1 + count_2;

        if (count > 0)
        {
            int* circlePoints = (int*) malloc(count * 2 * sizeof(int));

            if (circlePoints != NULL)
            {
                k = 0;
                for( i = rx_hold_fail_ob+rx_setup_mix_pass_fail_ib + gPhyParams->radius; i < 128 - rx_hold_fail_ot - rx_hold_mix_pass_fail_it - gPhyParams->radius; i++)
                {
                    for( j = tx_setup_fail_ol+tx_setup_mix_pass_fail_il + gPhyParams->radius; j < 128 - tx_setup_fail_or - tx_hold_mix_pass_fail_ir - gPhyParams->radius; j++)
                    {
                        if(testArr[i][j] == passingValue)
                        {
                            if(true == check_circle_area(&testArr,i,j, gPhyParams->radius, i+gPhyParams->radius, j+gPhyParams->radius, testArr[i][j]) && (calculate_distance(1, 1, cdcOffset, j, i) >= MIN_DISTANCE_FROM_SLOPE))
                            {
                                circlePoints[k++] = i;
                                circlePoints[k++] = j;
                            }
                        }
                    }
                }

                goldenPoints = (int*) malloc(count * 2 * sizeof(int));

                if (goldenPoints != NULL)
                {
                    memcpy(goldenPoints, circlePoints, count * 2 * sizeof(int));
                }
                else
                {
                    printf("malloc failed\n");
                }

                free(circlePoints);
            }
        }
    }
    else
    {
        count = count_1 + count_2;
        if (count > 0)
        {
            int* circlePoints = (int*) malloc(count * 2 * sizeof(int));

            if (circlePoints != NULL)
            {
                k = 0;
                for( i = rx_hold_fail_ob+rx_setup_mix_pass_fail_ib + gPhyParams->radius; i < 128 - rx_hold_fail_ot - rx_hold_mix_pass_fail_it - gPhyParams->radius; i++)
                {
                    for( j = tx_setup_fail_ol+tx_setup_mix_pass_fail_il + gPhyParams->radius; j < 128 - tx_setup_fail_or - tx_hold_mix_pass_fail_ir - gPhyParams->radius; j++)
                    {
                        if(testArr[i][j] == FIRST_PASS_READ_DELAY)
                        {
                            if(true == check_circle_area(&testArr,i,j, gPhyParams->radius, i+gPhyParams->radius, j+gPhyParams->radius, testArr[i][j]) && (calculate_distance(1, 1, cdcOffset, j, i) >= MIN_DISTANCE_FROM_SLOPE))
                            {
                                circlePoints[k++] = i;
                                circlePoints[k++] = j;
                            }
                        }

                        if(testArr[i][j] == SECOND_PASS_READ_DELAY)
                        {
                            if(true == check_circle_area(&testArr,i,j, gPhyParams->radius, i+gPhyParams->radius, j+gPhyParams->radius, testArr[i][j]) && (calculate_distance(1, 1, cdcOffset, j, i) >= MIN_DISTANCE_FROM_SLOPE))
                            {
                                circlePoints[k++] = i;
                                circlePoints[k++] = j;
                            }

                        }
                    }
                }

                goldenPoints = (int*) malloc(count * 2 * sizeof(int));

                if (goldenPoints != NULL)
                {
                    memcpy(goldenPoints, circlePoints, count * 2 * sizeof(int));
                }
                else
                {
                    printf("malloc failed\n");
                }

                free(circlePoints);
            }
        }
    }

    char fileName[50];
    sprintf(fileName, "binaries/golden_points%d.bin", iteration);
    FILE *fp = fopen(fileName, "wb");
    if (fp != NULL)
    {
        fwrite(goldenPoints, sizeof(int), count*2, fp);
        fclose(fp);
    }
    else
    {
        printf("Error opening file\n");
    }

    if(count > 10)
    {
        printf("TestCase: Valid ");

        clock_t startTime1 = clock();
        OSPI_phyFindOTP1(&testArr, gPhyParams->radius, &otp1);
        clock_t endTime1 = clock();
        total_time = ((float)(endTime1 - startTime1)/CLOCKS_PER_SEC)*1000000;

        printf("otp1 rx = %3d, tx = %3d, rd = %d time: %5dus ", otp1.rxDLL, otp1.txDLL, otp1.rdDelay, (int)total_time);

        OSPI_phyOps phyOps;
        phyOps.ops = OSPI_phySetAndRead;
        phyOps.phyParams = gPhyParams;

        clock_t startTime2 = clock();
        OSPI_phyFindOTP4((void*)(&testArr), 0, &phyOps, 5, &otp2);
        clock_t endTime2 = clock();
        total_time = ((float)(endTime2 - startTime2)/CLOCKS_PER_SEC)*1000000;

        printf("otp2 rx = %3d, tx = %3d, rd = %d, search_radius: 5, time: %5dus, ", otp2.rxDLL, otp2.txDLL, otp2.rdDelay, (int)total_time);

        clock_t startTime3 = clock();
        OSPI_phyFindOTP4((void*)(&testArr), 0, &phyOps, gPhyParams->radius, &otp2);
        clock_t endTime3 = clock();
        total_time = ((float)(endTime3 - startTime3)/CLOCKS_PER_SEC)*1000000;

        printf("search_radius: %d, time: %5dus ", gPhyParams->radius, (int)total_time);

        if(otp1.rdDelay == READ_DELAY_INVALID)
        {
            printf("OTP1 result: FAIL  ");
        }
        else
        {
            printf("OTP1 result: PASS  ");
        }

        if(otp2.rdDelay == READ_DELAY_INVALID)
        {
            printf("OTP2 result: FAIL\n");
        }
        else
        {
            printf("OTP2 result: PASS\n");
        }
    }
    else
    {
        printf("TestCase: Invalid ");
        printf("otp1 rx = %3d, tx = %3d, rd = %d time: %5dus ", otp1.rxDLL, otp1.txDLL, otp1.rdDelay, (int)total_time);
        printf("otp2 rx = %3d, tx = %3d, rd = %d, search_radius: 5, time: %5dus, search_radius: 10, time: %5dus ", otp2.rxDLL, otp2.txDLL, otp2.rdDelay, (int)total_time, (int)total_time);
        printf("OTP1 result: FAIL  ");
        printf("OTP2 result: FAIL\n");
    }

    return 0;
}
