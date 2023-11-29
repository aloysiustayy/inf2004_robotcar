#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "motor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include <time.h>

#define NUM_OF_PINS 2
#define LEFT_VCC_PIN 14
#define LEFT_ANALOG_PIN 15
#define RIGHT_ANALOG_PIN 16
#define RIGHT_VCC_PIN 17
#define BLACK 1
#define WHITE 0
// static char event_str[128];

static QueueHandle_t ir_queue_handle;

// Init of global variable
static const uint8_t analog_pins[] = {LEFT_ANALOG_PIN, RIGHT_ANALOG_PIN};
uint8_t cur_pin;
int stop_signal[] = {1, 1};
alarm_id_t alarm[] = {-1, -1};
uint8_t sensor[] = {WHITE, WHITE};
uint8_t prev_sensor[] = {WHITE, WHITE};

void gpio_event_stringg(char *buf, uint32_t events);

#define DEBOUNCE_DELAY_MS 50 // Debouncing for IR sensor
uint32_t last_btn_press[] = {1, 1};

clock_t clock()
{
    return (clock_t)time_us_64() / 10000;
}

void set_black(int pin)
{
    for (int i = NUM_OF_PINS - 1; i > -1; i--)
    {
        if (pin == analog_pins[i])
        {
            sensor[i] = BLACK;
        }
    }
}

void set_white(int pin)
{
    for (int i = NUM_OF_PINS - 1; i > -1; i--)
    {
        if (pin == analog_pins[i])
        {
            sensor[i] = WHITE;
        }
    }
}

uint8_t
get_sensor(int pin)
{
    for (int i = NUM_OF_PINS - 1; i > -1; i--)
    {
        if (pin == analog_pins[i])
        {
            return sensor[i];
        }
    }
    return -1;
}

QueueHandle_t
IRMessageHandler()
{
    if (ir_queue_handle == NULL)
    {
        ir_queue_handle = xQueueCreate(5, sizeof(char) * 100);
    }

    return ir_queue_handle;
}

/*!
 * @brief Sends a message to the queue ("Text = Data\\n")
 * @param[in] type	Text
 * @param[in] data Data to print out
 * @return -
 */
void send_sensor_data(char *type, uint32_t data)
{

    char printf_message[100]; // Adjust the buffer size as needed
    snprintf(printf_message, sizeof(printf_message), "%s=%d", type, data);

    xQueueSend(ir_queue_handle, &printf_message, 0);
    printf("Sending %s\n", printf_message);
}

int64_t
alarm_callback(alarm_id_t id, void *user_data)
{

    printf("%d\n", cur_pin);

    if (gpio_get(cur_pin) == 0)
    {
        // this is white
        if (cur_pin == analog_pins[0])
        {
            send_sensor_data("left", 90);
        }
        else if (cur_pin == analog_pins[1])
        {
            send_sensor_data("right", 90);
        }
    }

    // Can return a value here in us to fire in the future
    return 0;
}

void detectLines(__unused void *params)
{

    gpio_init(LEFT_VCC_PIN);
    gpio_set_dir(LEFT_VCC_PIN, GPIO_OUT);
    gpio_put(LEFT_VCC_PIN, 1);

    gpio_init(RIGHT_VCC_PIN);
    gpio_set_dir(RIGHT_VCC_PIN, GPIO_OUT);
    gpio_put(RIGHT_VCC_PIN, 1);

    if (ir_queue_handle == NULL)
    {
        ir_queue_handle = xQueueCreate(5, sizeof(char) * 100);
    }

    printf("Starting IR!");
    while (true)
    {
        vTaskDelay(100);
        for (int i = NUM_OF_PINS - 1; i > -1; i--)
        {

            if (gpio_get(analog_pins[i]) == 1)
            {
                set_black(analog_pins[i]);
            }
            else
            {
                set_white(analog_pins[i]);
            }

            
            if (sensor[i] != prev_sensor[i])
            {
                if (sensor[0] == WHITE && sensor[1] == BLACK)
                {
                    send_sensor_data("left", 90);
                }
                if (sensor[0] == BLACK && sensor[1] == WHITE)
                {
                    send_sensor_data("right", 90);
                }

                prev_sensor[i] = sensor[i];
            }
        }
    }
}
