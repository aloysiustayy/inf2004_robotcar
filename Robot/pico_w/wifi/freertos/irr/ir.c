#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "motor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#define NUM_OF_PINS 2
#define LEFT_VCC_PIN 14
#define LEFT_ANALOG_PIN 15
#define RIGHT_ANALOG_PIN 16
#define RIGHT_VCC_PIN 17
#define BLACK 1
#define WHITE 0
static char event_str[128];

static QueueHandle_t ir_queue_handle;

// Init of global variable
static const uint8_t analog_pins[] = {LEFT_ANALOG_PIN, RIGHT_ANALOG_PIN};
int stop_signal[] = {1, 1};
alarm_id_t alarm[] = {-1, -1};
uint8_t sensor[] = {WHITE, WHITE};
uint8_t prev_sensor[] = {WHITE, WHITE};

void gpio_event_stringg(char *buf, uint32_t events);

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
}

void gpio_callback_ir(uint gpio, uint32_t events)
{

    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_stringg(event_str, events);

    // for (int i = NUM_OF_PINS - 1; i > -1; i--)
    for (int i = 0; i < NUM_OF_PINS; i++)
    {
        // if (i == 0)
        //     printf("GPIOOOO is %d analog pin is %d\n", gpio, analog_pins[i]);

        if (gpio == analog_pins[i])
        {
            printf("Events is %d\n", events);
            if (events & GPIO_IRQ_EDGE_RISE)
            {
                stop_signal[i] = 0;
            }
            else if (events & GPIO_IRQ_EDGE_FALL)
            {
                stop_signal[i] = 1;
            }
        }
    }
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

    // printf("Detect line now");
    // for (int i = NUM_OF_PINS - 1; i > -1; i--)
    for (int i = 0; i < NUM_OF_PINS; i++)
    {
        gpio_set_irq_enabled_with_callback(analog_pins[i],
                                           GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                           true,
                                           &gpio_callback_ir);
    }

    while (true)
    {

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
                char *line_colour = (sensor[i] == BLACK) ? "BLACK" : "WHITE";

                if (sensor[i] == WHITE && prev_sensor[i] == BLACK)
                {
                    // printf("Turning right now");
                    printf("Sensor %d detect %s\n", analog_pins[i], line_colour);
                    if (analog_pins[i] == LEFT_ANALOG_PIN)
                    {
                        send_sensor_data("left", 90);
                    }
                    else if (analog_pins[i] == RIGHT_ANALOG_PIN)
                    {
                        send_sensor_data("right", 90);
                    }
                }
                // if (analog_pins[i] == LEFT_ANALOG_PIN)
                // {
                //     printf("Left IR Sensor detected: %s\n", line_colour);
                // if (sensor[i] == WHITE && prev_sensor[i] == BLACK)
                // {
                //     printf("Turning left right now");
                //     // turn_left(90);
                //     send_sensor_data("left", 90);
                // }
                // }
                // else if (analog_pins[i] == RIGHT_ANALOG_PIN)
                // {
                //     // printf("Right IR Sensor detected: %s\n", line_colour);
                //     // if (sensor[i] == WHITE && prev_sensor[i] == BLACK)
                //     // {
                //     //     printf("Turning right right now");

                //     // }
                // }

                prev_sensor[i] = sensor[i];
            }
        }
    }
}

static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4
    "EDGE_RISE"   // 0x8
};

void gpio_event_stringg(char *buf, uint32_t events)
{
    for (uint i = 0; i < 4; i++)
    {
        uint mask = (1 << i);
        if (events & mask)
        {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0')
            {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events)
            {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}
/*** end of file ***/