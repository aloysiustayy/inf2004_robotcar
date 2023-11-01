
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define ADC_PIN     26
#define DIGITAL_PIN 22

#define BLACK_THRESHOLD 1000
#define WHITE_THRESHOLD 400
#define BARCODE_BUF_SIZE 10
#define BARCODE_ARR_SIZE 9
#define ADC_DIFFERENCE_THRESHHOLD 20

uint8_t barcodeFirstChar  = 0;
uint8_t barcodeSecondChar = 0;
uint8_t barcodeThirdChar  = 0;

enum bartype
{
    THICK_BLACK, // 0
    THIN_BLACK,  // 1
    THICK_WHITE, // 2
    THIN_WHITE   // 3
};

// code 39 format of letter A to Z using enum bartype

// code 39 format of letter F using enum bartype
static char *A_ARRAY_MAP = "031312130";
static char *B_ARRAY_MAP = "130312130";
static char *C_ARRAY_MAP = "030312131";
static char *D_ARRAY_MAP = "131302130";
static char *E_ARRAY_MAP = "031302131";
static char *F_ARRAY_MAP = "130302131";
static char *G_ARRAY_MAP = "131312030";
static char *H_ARRAY_MAP = "031312031";
static char *I_ARRAY_MAP = "130312031";
static char *J_ARRAY_MAP = "131302031";
static char *K_ARRAY_MAP = "031313120";
static char *L_ARRAY_MAP = "130313120";
static char *M_ARRAY_MAP = "030313121";
static char *N_ARRAY_MAP = "131303120";
static char *O_ARRAY_MAP = "031303121";
static char *P_ARRAY_MAP = "130303121";
static char *Q_ARRAY_MAP = "131313020";
static char *R_ARRAY_MAP = "031313021";
static char *S_ARRAY_MAP = "130313021";
static char *T_ARRAY_MAP = "131303021";
static char *U_ARRAY_MAP = "021313130";
static char *V_ARRAY_MAP = "120313130";
static char *W_ARRAY_MAP = "020313131";
static char *X_ARRAY_MAP = "121303130";
static char *Y_ARRAY_MAP = "021303131";
static char *Z_ARRAY_MAP = "120303131";

static int waitingForStart = 1;

// code 39 format of asterisk using enum bartype
static char *ASTERISK_ARRAY_MAP = "121303031";

static int barcodeArr[BARCODE_ARR_SIZE];

static uint32_t res     = 0;
static uint16_t prevAvg = 0;

static int i                 = 0;
static int barcode_arr_index = 1;

char *outputBuffer;

static absolute_time_t blockStart;
static absolute_time_t blockEnd;

struct voltageClassification
{
    uint16_t voltage;
    // 0 - white
    // 1 - black
    int             blackWhite;
    absolute_time_t blockStart;
    int64_t         blockLength;
    enum bartype    type;
} voltageClassification;


// queue for voltageclassifications of length 9
static struct voltageClassification voltageClassifications[BARCODE_BUF_SIZE];

// queue for barcode read of length 3
static char barcodeRead[3];

// function to append to barcodeRead Queue
static void
appendToBarcodeRead(char barcodeChar)
{
    barcodeRead[0] = barcodeRead[1];
    barcodeRead[1] = barcodeRead[2];
    barcodeRead[2] = barcodeChar;
}

static int
isValidBarcode()
{
    if (barcodeRead[0] == '*' && barcodeRead[2] == '*')
    {
        if (barcodeRead[1] != 0)
            return 1;
    }

    return 0;
}

static int
isBarcodeFull()
{
    if (barcodeRead[0] != 0 && barcodeRead[1] != 0 && barcodeRead[2] != 0)
    {
        return 1;
    }

    return 0;
}

static void
clearBarcodeRead()
{
    barcodeRead[0] = 0;
    barcodeRead[1] = 0;
    barcodeRead[2] = 0;
}

// function to convert array of integer to string
static char *
intArrayToString(int *arr, int size)
{
    char *str = malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        str[i] = arr[i] + '0';
    }
    str[size] = '\0';
    return str;
}

static int *
thickThinClassification()
{
    // calculate average block length
    int64_t totalBarLength = 0;
    for (int i = 0; i < BARCODE_ARR_SIZE; i++)
    {
        totalBarLength += voltageClassifications[i].blockLength;
    }

    int *barsRead = malloc(BARCODE_ARR_SIZE * sizeof(int));

    int64_t avgBarLength = (totalBarLength / BARCODE_ARR_SIZE);
    // assign thick thin classification
    for (int i = 0; i < BARCODE_ARR_SIZE; i++)
    {
        // printf("%"PRId64", ",voltageClassifications[i].blockLength);

        if (voltageClassifications[i].blackWhite)
        {
            if (voltageClassifications[i].blockLength < avgBarLength)
            {
                voltageClassifications[i].type = THIN_BLACK;
            }
            else
            {
                voltageClassifications[i].type = THICK_BLACK;
            }
        }
        else
        {
            if (voltageClassifications[i].blockLength < avgBarLength)
            {
                voltageClassifications[i].type = THIN_WHITE;
            }
            else
            {
                voltageClassifications[i].type = THICK_WHITE;
            }
        }
        barsRead[i] = voltageClassifications[i].type;
    }
    // printf("\n\r");
    return barsRead;
}
// function to check if queue is full
static int
isVoltageClassificationFull()
{
    for (int i = 0; i < BARCODE_BUF_SIZE; i++)
    {
        if (voltageClassifications[i].blackWhite == -1)
        {
            return 0;
        }
    }
    return 1;
}

// function to flush queue
static void
flushVoltageClassification()
{
    barcode_arr_index = 1;
    blockStart        = get_absolute_time();

    struct voltageClassification lastReading
        = voltageClassifications[BARCODE_BUF_SIZE - 1];

    for (int i = 0; i < BARCODE_BUF_SIZE; i++)
    {
        voltageClassifications[i].voltage     = 0;
        voltageClassifications[i].blackWhite  = -1;
        voltageClassifications[i].blockLength = 0;
        voltageClassifications[i].type        = 0;
    }

    voltageClassifications[0] = lastReading;
}

// function to compare buffer and the barcodes
static char
compareTwoArray()
{
    int *barsRead = thickThinClassification();

    if (voltageClassifications[0].blackWhite == 0)
    {
        // printf("Start with white\n\r");
        return 0;
    }

    char *string = intArrayToString(barsRead, BARCODE_ARR_SIZE);
    free(barsRead);

    char *barcodes[]
        = { A_ARRAY_MAP, B_ARRAY_MAP, C_ARRAY_MAP,       D_ARRAY_MAP,
            E_ARRAY_MAP, F_ARRAY_MAP, G_ARRAY_MAP,       H_ARRAY_MAP,
            I_ARRAY_MAP, J_ARRAY_MAP, K_ARRAY_MAP,       L_ARRAY_MAP,
            M_ARRAY_MAP, N_ARRAY_MAP, O_ARRAY_MAP,       P_ARRAY_MAP,
            Q_ARRAY_MAP, R_ARRAY_MAP, S_ARRAY_MAP,       T_ARRAY_MAP,
            U_ARRAY_MAP, V_ARRAY_MAP, W_ARRAY_MAP,       X_ARRAY_MAP,
            Y_ARRAY_MAP, Z_ARRAY_MAP, ASTERISK_ARRAY_MAP };

    char characters[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                          'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                          'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '*' };

    for (int i = 0; i < 27; i++)
    {
        if (strncmp(barcodes[i], string, BARCODE_ARR_SIZE) == 0)
        {
            free(string);
            flushVoltageClassification();
            return characters[i];
        }
    }

    return 0;
    // return 1;
}

// function to append queue
static void
appendVoltageClassification(struct voltageClassification voltageClassification)
{
    voltageClassifications[0] = voltageClassifications[1];
    voltageClassifications[1] = voltageClassifications[2];
    voltageClassifications[2] = voltageClassifications[3];
    voltageClassifications[3] = voltageClassifications[4];
    voltageClassifications[4] = voltageClassifications[5];
    voltageClassifications[5] = voltageClassifications[6];
    voltageClassifications[6] = voltageClassifications[7];
    voltageClassifications[7] = voltageClassifications[8];
    voltageClassifications[8] = voltageClassifications[9];
    voltageClassifications[9] = voltageClassification;
    if (barcode_arr_index == BARCODE_BUF_SIZE)
    {
        char read = compareTwoArray();
        if (read != 0)
        {
            printf("%c\0", read);

            appendToBarcodeRead(read);
        }
    }
}

uint16_t classification_line[9];
// uint16_t temp_classification_line[5];

// void
// printClassification(uint16_t arr)
// {
//     printf("Classification:");
//     printf("%i, ", classification_line[0]);
//     printf("%i, ", classification_line[1]);
//     printf("%i, ", classification_line[2]);
//     printf("%i\n ", classification_line[3]);
// }

uint16_t
get_lowest_value(uint16_t *classification_line)
{
    uint16_t lowest_value = classification_line[4];

    for (int i = 3; i > -1; i--)
    {
        if (classification_line[i] < lowest_value)
        {
            lowest_value = classification_line[i];
        }
    }

    return lowest_value;
}

uint16_t
get_highest_value(uint16_t *classification_line)
{
    uint16_t highest_value = classification_line[8];

    for (int i = 7; i > -1; i--)
    {
        if (classification_line[i] > highest_value)
        {
            highest_value = classification_line[i];
        }
    }

    return highest_value;
}
void
printArray(uint16_t *array, int size)
{
    bool possible = false;
    for (int i = 0; i < size; i++)
    {

        printf("%d ", array[i]);
    }

    if (get_lowest_value(array) == array[0] && array[0] == array[1])
        possible = true;

    if (possible)
        printf(" (Potential?) ");
    printf("\n");
}

int
partition(uint16_t *array, int start, int end)
{
    int pivot = array[end];
    int i     = start - 1;
    for (int j = start; j < end; j++)
    {
        if (array[j] < pivot)
        {
            i++;
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    int temp     = array[i + 1];
    array[i + 1] = array[end];
    array[end]   = temp;
    return i + 1;
}

void
quicksort(uint16_t *array, int start, int end)
{
    if (start < end)
    {
        int pivot = partition(array, start, end);
        quicksort(array, start, pivot - 1);
        quicksort(array, pivot + 1, end);
    }
}
void
sortClassification()
{
}
void
assignClassification(uint16_t cur_value)
{
    struct voltageClassification voltageClassification;
    voltageClassification.voltage = cur_value;

    // int size = sizeof(classification_line) / sizeof(classification_line[0]);
    // uint16_t sortedArray[5];
    // memcpy(sortedArray, classification_line, size * sizeof(uint16_t));

    // // Sort the copy of the array
    // quicksort(sortedArray, 0, size - 1);

    // assign voltageClassification.blackWhite

    if (gpio_get(DIGITAL_PIN) == 1)
    {
        voltageClassification.blackWhite = 1;
    }
    else
    {
        voltageClassification.blackWhite = 0;
    }
    printArray(classification_line, 9);
    if (barcode_arr_index == BARCODE_BUF_SIZE)
    {

        // if((abs(voltageClassifications[BARCODE_ARR_SIZE -
        // 1].voltage
        // - curr)/ADC_DIFFERENCE_THRESHHOLD * 100 < 10)){
        //     return;
        // }
        if (voltageClassifications[BARCODE_BUF_SIZE - 1].blackWhite
            != voltageClassification.blackWhite)
        {
            blockEnd                         = get_absolute_time();
            voltageClassification.blockStart = blockEnd;
            // printf("white block length: %"PRIu64", %"PRIu64"\n\r",
            // blockEnd,blockStart);
            int64_t blockLength = absolute_time_diff_us(
                voltageClassifications[BARCODE_BUF_SIZE - 1].blockStart,
                blockEnd);

            voltageClassifications[BARCODE_BUF_SIZE - 1].blockLength
                = blockLength / 10000;
            appendVoltageClassification(voltageClassification);
        }
    }
    else
    {
        if (voltageClassifications[barcode_arr_index - 1].blackWhite
            != voltageClassification.blackWhite)
        {
            blockEnd                         = get_absolute_time();
            voltageClassification.blockStart = blockEnd;
            // printf("white block length: %"PRIu64", %"PRIu64"\n\r",
            // blockEnd,blockStart);
            if (barcode_arr_index == 0)
            {
                int64_t blockLength
                    = absolute_time_diff_us(blockStart, blockEnd);
                voltageClassification.blockLength = blockLength / 10000;
            }
            else
            {
                int64_t blockLength = absolute_time_diff_us(
                    voltageClassifications[barcode_arr_index - 1].blockStart,
                    blockEnd);
                voltageClassifications[barcode_arr_index - 1].blockLength
                    = blockLength / 10000;
            }

            voltageClassifications[barcode_arr_index] = voltageClassification;
            barcode_arr_index++;
        }
    }
    // uint16_t size
    // = sizeof(classification_line) / sizeof(classification_line[0]);
    // first is default (normal ground)
    // second is THIN BLACK (will happen twice but i read once) (inrange func)
    // third is THICK WHITE
    // fourth is THIN WHITE
    // fifth is THICK BLACK
}

bool
inRange(uint16_t newValue, uint16_t originalValue, uint16_t percentage)
{

    // Calculate the percentage range
    float valueRange = (originalValue * percentage) / 100;

    // Check if a is within the percentage range
    if (newValue <= originalValue + valueRange
        && newValue >= originalValue - valueRange)
    {
        return true;
    }
    return false;
}

int
get_classification_avg()
{
    int total = 0;
    for (int i = 8; i > 5; i--)
    {
        total += classification_line[i];
    }
    return total / 5;
}

void
appendClassification(uint16_t avg)
{

    // if (classification_line[8] != 0)
    // {
    if (inRange(classification_line[8], avg, 10))
        return;

    //     // float threshold_percentage = 0.1;

    //     float threshold_value = classification_line[8] *
    //     threshold_percentage; if (inRange(avg, classification_line[8],
    //     threshold_value))
    //         return;
    // }

    // AFTER AWHILE OF NOT DETECTING ANYTH, CLEAR IT? v confusing bro
    // maybe not bah if not v messy and more code... try to work w it
    // if (classification_line[0] != 0)
    // {

    //     if (avg > classification_line[8]
    //         && classification_line[8] ==
    //         get_highest_value(classification_line))
    //     {
    //         // printf("Highest Value is %d\n",
    //         //    get_highest_value(classification_line));
    //         return;
    //     }
    // }

    classification_line[0] = classification_line[1];
    classification_line[1] = classification_line[2];
    classification_line[2] = classification_line[3];
    classification_line[3] = classification_line[4];
    classification_line[4] = classification_line[5];
    classification_line[5] = classification_line[6];
    classification_line[6] = classification_line[7];
    classification_line[7] = classification_line[8];
    classification_line[8] = avg;

    assignClassification(avg);
}

static void
ADC_IRQ_FIFO_HANDLER()
{
    // read data from ADC FIFO
    if (!adc_fifo_is_empty())
    {
        uint16_t data = adc_fifo_get();
        res += data;
        if (i < 100)
        {
            i++;
        }
        else
        {
            // uint16_t avg = res/(i);
            uint16_t avg = res / (i);

            // printf("%d\0", gpio_get(DIGITAL_PIN));
            // printf("%"PRIu16"", data);
            // printf(",%"PRIu16"", avg);
            if (prevAvg == 0)
            {
                prevAvg = avg;
            }
            else
            {
                if (abs(prevAvg - avg) > ADC_DIFFERENCE_THRESHHOLD)
                {
                    prevAvg = avg;
                }
                else
                {
                    avg = prevAvg;
                }
            }
            // printf(",%"PRIu16"\n\r", avg);
            i   = 0;
            res = 0;

            // printf("avg: %i\n", avg);
            appendClassification(avg);

            // if (gpio_get(DIGITAL_PIN) == 1)
            // {
            //     voltageClassification.blackWhite = 1;
            // }
            // else
            // {
            //     voltageClassification.blackWhite = 0;
            // }

            // if (barcode_arr_index == BARCODE_BUF_SIZE)
            // {

            //     // if((abs(voltageClassifications[BARCODE_ARR_SIZE -
            //     1].voltage
            //     // - curr)/ADC_DIFFERENCE_THRESHHOLD * 100 < 10)){
            //     //     return;
            //     // }
            //     if (voltageClassifications[BARCODE_BUF_SIZE -
            //     1].blackWhite
            //         != voltageClassification.blackWhite)
            //     {
            //         blockEnd                         =
            //         get_absolute_time();
            //         voltageClassification.blockStart = blockEnd;
            //         // printf("white block length: %"PRIu64",
            //         %"PRIu64"\n\r",
            //         // blockEnd,blockStart);
            //         int64_t blockLength = absolute_time_diff_us(
            //             voltageClassifications[BARCODE_BUF_SIZE -
            //             1].blockStart, blockEnd);

            //         voltageClassifications[BARCODE_BUF_SIZE -
            //         1].blockLength
            //             = blockLength / 10000;
            //         appendVoltageClassification(voltageClassification);
            //     }
            // }
            // else
            // {
            //     if (voltageClassifications[barcode_arr_index -
            //     1].blackWhite
            //         != voltageClassification.blackWhite)
            //     {
            //         blockEnd                         =
            //         get_absolute_time();
            //         voltageClassification.blockStart = blockEnd;
            //         // printf("white block length: %"PRIu64",
            //         %"PRIu64"\n\r",
            //         // blockEnd,blockStart);
            //         if (barcode_arr_index == 0)
            //         {
            //             int64_t blockLength
            //                 = absolute_time_diff_us(blockStart,
            //                 blockEnd);
            //             voltageClassification.blockLength =
            //             blockLength / 10000;
            //         }
            //         else
            //         {
            //             int64_t blockLength = absolute_time_diff_us(
            //                 voltageClassifications[barcode_arr_index
            //                 - 1]
            //                     .blockStart,
            //                 blockEnd);
            //             voltageClassifications[barcode_arr_index - 1]
            //                 .blockLength
            //                 = blockLength / 10000;
            //         }

            //         voltageClassifications[barcode_arr_index]
            //             = voltageClassification;
            //         barcode_arr_index++;
            //     }
            // }
        }
    }
    irq_clear(ADC_IRQ_FIFO);
}

void
temp_detect(__unused void *params)
{
    while (true)
    {
        // i2c_write_byte('I');
        if (isValidBarcode())
        {
            printf("Valid Barcode\n\r");
            barcodeFirstChar  = barcodeRead[0];
            barcodeSecondChar = barcodeRead[1];
            barcodeThirdChar  = barcodeRead[2];
            printf("Barcode: %c%c%c\n\r",
                   barcodeFirstChar,
                   barcodeSecondChar,
                   barcodeThirdChar);
            // sendBarcodeVal(); To send barcode values to comms
            clearBarcodeRead();
            barcodeFirstChar  = 0;
            barcodeSecondChar = 0;
            barcodeThirdChar  = 0;
        }
    }
}
void
barcodeLaunch()
{

    adc_init();

    // init the queue
    flushVoltageClassification();

    adc_gpio_init(ADC_PIN);

    adc_select_input(0);

    adc_fifo_setup(true, false, 1, false, false);
    adc_set_clkdiv(0);
    adc_irq_set_enabled(true);

    irq_clear(ADC_IRQ_FIFO);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, ADC_IRQ_FIFO_HANDLER);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    adc_run(true);

    sleep_ms(2000);

    blockStart = get_absolute_time();

    TaskHandle_t temp_task;
    if (xTaskCreate(temp_detect,
                    "tempdetectTask",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    8,
                    &temp_task)
        != pdPASS)
    {
        printf("Failed to create tempdetectTask\n");
        return;
    }
    else
    {
        printf("Successfully created tempdetectTask\n");
    }
}