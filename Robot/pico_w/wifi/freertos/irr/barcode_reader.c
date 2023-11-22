
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
#include "message_buffer.h"
#include "queue.h"

#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define ADC_PIN 26
#define DIGITAL_PIN 22

#define BLACK_THRESHOLD 3000

#define WHITE_THRESHOLD 400

#define BARCODE_BUF_SIZE 10

#define BARCODE_ARR_SIZE 9

#define ADC_DIFFERENCE_THRESHHOLD 50

#define SAMPLE_SIZE 10000

// Read only 3 barcodes
uint8_t firstBarcode = 0;
uint8_t secondBarcode = 0;
uint8_t thirdBarcode = 0;

static QueueHandle_t barcode_queue_handle;

// Determine whether bars are white / black, thick / thin (4 classifications)
enum bar
{
    THICK_WHITE, // 0
    THIN_WHITE,  // 1
    THICK_BLACK, // 2
    THIN_BLACK   // 3
};
// 303121213
// Alphabets in Code 39 format (each barcode uses 9 bartype)
static char *A_ARRAY_MAP = "213130312";
static char *B_ARRAY_MAP = "312130312";
static char *C_ARRAY_MAP = "212130313";
static char *D_ARRAY_MAP = "313120312";
static char *E_ARRAY_MAP = "213120313";
static char *F_ARRAY_MAP = "312120313";
static char *G_ARRAY_MAP = "313130212";
static char *H_ARRAY_MAP = "213130213";
static char *I_ARRAY_MAP = "312130213";
static char *J_ARRAY_MAP = "313120213";
static char *K_ARRAY_MAP = "213131302";
static char *L_ARRAY_MAP = "312131302";
static char *M_ARRAY_MAP = "212131303";
static char *N_ARRAY_MAP = "313121302";
static char *O_ARRAY_MAP = "213121303";
static char *P_ARRAY_MAP = "312121303";
static char *Q_ARRAY_MAP = "313131202";
static char *R_ARRAY_MAP = "213131203";
static char *S_ARRAY_MAP = "312131203";
static char *T_ARRAY_MAP = "313121203";
static char *U_ARRAY_MAP = "203131312";
static char *V_ARRAY_MAP = "302131312";
static char *W_ARRAY_MAP = "202131313";
static char *X_ARRAY_MAP = "303121312";
static char *Y_ARRAY_MAP = "203121313";
static char *Z_ARRAY_MAP = "302121313";

// '*' in Code 39 format
static char *ASTERISK_ARRAY_MAP = "303121213";
// static char *BACK_ASTERISK_ARRAY_MAP = "312121303";

static uint32_t res = 0;
static uint16_t prevAvg = 0;

static int i = 0;
static int barcode_arr_index = 1;

char *outputBuffer;

static absolute_time_t blockStart;
static absolute_time_t blockEnd;

struct barcode
{
    uint16_t analog_value;
    // 0 - white
    // 1 - black
    int blackWhite;
    absolute_time_t blockStart;
    int64_t blockLength;
    enum bar type;
} barcode;

// queue for barcodes of length 9
static struct barcode barcodes[BARCODE_BUF_SIZE];

// queue for barcode read of length 3
static char barcodeRead[3];

char *reverseString(const char *str)
{
    int length = strlen(str);
    char *reversed = (char *)malloc((length + 1) * sizeof(char)); // +1 for the null terminator

    if (reversed == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    int i, j;
    for (i = length - 1, j = 0; i >= 0; i--, j++)
    {
        reversed[j] = str[i];
    }

    reversed[length] = '\0'; // Add null terminator at the end

    return reversed;
}

static void
clearBarcodeRead()
{
    barcodeRead[0] = 0;
    barcodeRead[1] = 0;
    barcodeRead[2] = 0;
}

// function to append to barcodeRead Queue
static void
appendToBarcodeRead(char barcodeChar)
{
    barcodeRead[0] = barcodeRead[1];
    barcodeRead[1] = barcodeRead[2];
    barcodeRead[2] = barcodeChar;

    printf("Current Reading: ");
    printf("%c %c %c\n", barcodeRead[0], barcodeRead[1], barcodeRead[2]);
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

// function to convert array of integer to string
static char *
intArrayToString(int *arr, int size)
{
    char *tempStr = malloc(size + 1);

    for (int i = 0; i < size; i++)
    {
        tempStr[i] = arr[i] + '0';
    }
    tempStr[size] = '\0';
    return tempStr;
}

static int *
sortBarType()
{
    // calculate average block length
    int64_t totalBarLength = 0;
    for (int i = 0; i < BARCODE_ARR_SIZE; i++)
    {
        totalBarLength += barcodes[i].blockLength;
    }

    int *barsRead = malloc(BARCODE_ARR_SIZE * sizeof(int));

    int64_t avgBarLength = (totalBarLength / BARCODE_ARR_SIZE);

    // Assign thick / thin
    for (int i = 0; i < BARCODE_ARR_SIZE; i++)
    {
        if (barcodes[i].blackWhite)
        {
            if (barcodes[i].blockLength < avgBarLength)
            {
                barcodes[i].type = THIN_BLACK;
            }
            else
            {
                barcodes[i].type = THICK_BLACK;
            }
        }
        else
        {
            if (barcodes[i].blockLength < avgBarLength)
            {
                barcodes[i].type = THIN_WHITE;
            }
            else
            {
                barcodes[i].type = THICK_WHITE;
            }
        }
        barsRead[i] = barcodes[i].type;
    }

    return barsRead;
}

// function to flush queue
static void
resetBarcodes()
{
    barcode_arr_index = 1;
    blockStart = get_absolute_time();

    struct barcode lastReading = barcodes[BARCODE_BUF_SIZE - 1];

    for (int i = 0; i < BARCODE_BUF_SIZE; i++)
    {
        barcodes[i].analog_value = 0;
        barcodes[i].blackWhite = -1;
        barcodes[i].blockLength = 0;
        barcodes[i].type = 0;
    }

    barcodes[0] = lastReading;
}

// function to compare buffer and the barcodes
static char
compareTwoArray()
{
    int *barsRead = sortBarType();

    if (barcodes[0].blackWhite == 0)
    {
        // printf("Start with white\n\r");
        return 0;
    }

    char *string = intArrayToString(barsRead, BARCODE_ARR_SIZE);

    free(barsRead);

    char *barcodes[] = {A_ARRAY_MAP, B_ARRAY_MAP, C_ARRAY_MAP, D_ARRAY_MAP,
                        E_ARRAY_MAP, F_ARRAY_MAP, G_ARRAY_MAP, H_ARRAY_MAP,
                        I_ARRAY_MAP, J_ARRAY_MAP, K_ARRAY_MAP, L_ARRAY_MAP,
                        M_ARRAY_MAP, N_ARRAY_MAP, O_ARRAY_MAP, P_ARRAY_MAP,
                        Q_ARRAY_MAP, R_ARRAY_MAP, S_ARRAY_MAP, T_ARRAY_MAP,
                        U_ARRAY_MAP, V_ARRAY_MAP, W_ARRAY_MAP, X_ARRAY_MAP,
                        Y_ARRAY_MAP, Z_ARRAY_MAP, ASTERISK_ARRAY_MAP};

    char characters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                         'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                         'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '*'};

    for (int i = 0; i < 27; i++)
    {
        // left to right
        if (strncmp(barcodes[i], string, BARCODE_ARR_SIZE) == 0)
        {
            free(string);
            resetBarcodes();
            return characters[i];
        }
        // right to left
        if (strncmp(reverseString(barcodes[i]), string, BARCODE_ARR_SIZE) == 0)
        {
            free(string);
            resetBarcodes();
            return characters[i];
        }
    }

    return 0;
}

// function to append queue
static void
addBarcode(struct barcode barcode)
{
    barcodes[0] = barcodes[1];
    barcodes[1] = barcodes[2];
    barcodes[2] = barcodes[3];
    barcodes[3] = barcodes[4];
    barcodes[4] = barcodes[5];
    barcodes[5] = barcodes[6];
    barcodes[6] = barcodes[7];
    barcodes[7] = barcodes[8];
    barcodes[8] = barcodes[9];
    barcodes[9] = barcode;

    if (barcode_arr_index == BARCODE_BUF_SIZE)
    {
        char read = compareTwoArray();
        if (read != 0)
        {
            printf("%c\n", read);

            appendToBarcodeRead(read);
        }
    }
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
            i = 0;
            res = 0;
            // printf("Avg is %d\n", avg);
            struct barcode barcode;
            barcode.analog_value = avg;

            if (avg > BLACK_THRESHOLD || gpio_get(DIGITAL_PIN) == 1)
            {
                barcode.blackWhite = 1;
            }
            else
            {
                barcode.blackWhite = 0;
            }

            // If the barcode array index is at the end of the array,
            // check if the black/white flag of the current barcode is different
            // from the black/white flag of the previous barcode. If it is, then
            // calculate the block length and add the barcode to the barcode
            // array.
            if (barcode_arr_index == BARCODE_BUF_SIZE)
            {
                if (barcodes[BARCODE_BUF_SIZE - 1].blackWhite != barcode.blackWhite)
                {
                    blockEnd = get_absolute_time();
                    barcode.blockStart = blockEnd;

                    int64_t blockLength = absolute_time_diff_us(
                        barcodes[BARCODE_BUF_SIZE - 1].blockStart, blockEnd);
                    barcodes[BARCODE_BUF_SIZE - 1].blockLength = blockLength / 10000;
                    addBarcode(barcode);
                }
            }
            else
            {
                // Check if the black/white flag of the current barcode is
                // different from the black/white flag of the previous barcode.
                // If it is, then calculate the block length and add the barcode
                // to the barcode array.
                if (barcodes[barcode_arr_index - 1].blackWhite != barcode.blackWhite)
                {
                    blockEnd = get_absolute_time();
                    barcode.blockStart = blockEnd;

                    if (barcode_arr_index == 0)
                    {
                        int64_t blockLength = absolute_time_diff_us(blockStart, blockEnd);

                        // Convert microseconds to milliseconds, which is used
                        // as length in this case
                        barcode.blockLength = blockLength / 10000;
                    }
                    else
                    {
                        int64_t blockLength = absolute_time_diff_us(
                            barcodes[barcode_arr_index - 1].blockStart,
                            blockEnd);

                        barcodes[barcode_arr_index - 1].blockLength = blockLength / 10000;
                    }

                    barcodes[barcode_arr_index] = barcode;
                    barcode_arr_index++;
                }
            }
        }
    }

    // Clear the ADC FIFO interrupt.
    irq_clear(ADC_IRQ_FIFO);
}

void temp_detect(__unused void *params)
{
    while (true)
    {
        // i2c_write_byte('I');
        if (isValidBarcode())
        {
            printf("Valid Barcode\n\r");
            firstBarcode = barcodeRead[0];
            secondBarcode = barcodeRead[1];
            thirdBarcode = barcodeRead[2];
            printf("Barcode: %c%c%c\n\r",
                   firstBarcode,
                   secondBarcode,
                   thirdBarcode);
            // sendBarcodeVal(); To send barcode values to comms
            clearBarcodeRead();
            firstBarcode = 0;
            secondBarcode = 0;
            thirdBarcode = 0;
        }
        vTaskDelay(100);
        // printf("running barcode task");
    }
}

QueueHandle_t
BarcodeMessageHandler()
{
    if (barcode_queue_handle == NULL)
    {
        barcode_queue_handle = xQueueCreate(5, sizeof(char) * 100);
    }

    return barcode_queue_handle;
}

/*!
 * @brief Sends a message to the queue ("Text = Data\\n")
 * @param[in] type	Text
 * @param[in] data Data to print out
 * @return -
 */
void send_barcode_data()
{

    char printf_message[100]; // Adjust the buffer size as needed
    snprintf(printf_message,
             sizeof(printf_message),
             "%c%c%c",
             barcodeRead[0],
             barcodeRead[1],
             barcodeRead[2]);

    xQueueSend(barcode_queue_handle, &printf_message, 0);
}

void barcodeLaunch(__unused void *params)
{
    gpio_init(19);
    gpio_set_dir(19, GPIO_OUT);
    gpio_put(19, 1);

    // adc_init();

    // init the queue
    resetBarcodes();

    adc_gpio_init(ADC_PIN);

    // adc_select_input(0);

    adc_fifo_setup(true, false, 1, false, false);
    adc_set_clkdiv(0);
    adc_irq_set_enabled(true);

    irq_clear(ADC_IRQ_FIFO);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, ADC_IRQ_FIFO_HANDLER);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    adc_run(true);

    sleep_ms(2000);

    blockStart = get_absolute_time();
    printf("Barcode reading");

    if (barcode_queue_handle == NULL)
    {
        barcode_queue_handle = xQueueCreate(5, sizeof(char) * 100);
    }

    while (true)
    {
        vTaskDelay(100);
        adc_select_input(0);
        // i2c_write_byte('I');
        if (isValidBarcode())
        {
            printf("Valid Barcode\n\r");
            firstBarcode = barcodeRead[0];
            secondBarcode = barcodeRead[1];
            thirdBarcode = barcodeRead[2];

            printf("Barcode: %c%c%c\n\r",
                   firstBarcode,
                   secondBarcode,
                   thirdBarcode);

            send_barcode_data();
            clearBarcodeRead();
            firstBarcode = 0;
            secondBarcode = 0;
            thirdBarcode = 0;
        }

        // printf("running barcode task");
    }
    // TaskHandle_t temp_task;
    // if (xTaskCreate(temp_detect,
    //                 "tempdetectTask",
    //                 configMINIMAL_STACK_SIZE,
    //                 NULL,
    //                 8,
    //                 &temp_task)
    //     != pdPASS)
    // {
    //     printf("Failed to create tempdetectTask\n");
    //     return;
    // }
    // else
    // {
    //     printf("Successfully created tempdetectTask\n");
    // }
}