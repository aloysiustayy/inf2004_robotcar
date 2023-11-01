#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
// #include "FreeRTOS.h"
// #include "message_buffer.h"
// #include "queue.h"

#define ANALOG_PIN 15
#define BLACK      1
#define WHITE      0

// static QueueHandle_t xQueueMessageHandle;
static char event_str[128];

// Init of global variable
int        stop_signal  = 1;
int        elapsed_time = 0;
alarm_id_t alarm        = -1;

uint8_t sensor      = WHITE;
uint8_t prev_sensor = WHITE;

void
start_timer()
{
    sensor = BLACK;
}

void
stop_timer()
{
    sensor = WHITE;
}

uint8_t
get_sensor()
{
    return sensor;
}

// QueueHandle_t
// getMessageHandler()
// {
//     return xQueueMessageHandle;
// }

/*!
 * @brief Sends a message to the queue
 * @param[in] type	What kind of temperature is being passed in? (Onboard /
 * Simple / Moving)
 * @param[in] temperature The temperature to print out
 * @return -
 */
void
send_sensor_data(char *type, uint32_t data)
{

    char printf_message[100]; // Adjust the buffer size as needed
    snprintf(printf_message, sizeof(printf_message), "%s = %d\n", type, data);

    // xQueueSend(xQueueMessageHandle, &printf_message, 0);
}
int64_t
debounce_callback(alarm_id_t id, void *user_data)
{
    uint32_t events = (uint32_t)user_data;

    // Check if pseudo button is high or low to trigger stop_signal
    //
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        stop_signal = 0;
    }
    else if (events & GPIO_IRQ_EDGE_FALL)
    {
        stop_signal = 1;
    }

    // Reset alarm so that alarm can be called again
    alarm = -1;
    return 0;
}

void gpio_event_string(char *buf, uint32_t events);
void
gpio_callback(uint gpio, uint32_t events)
{
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    if (alarm == -1)
    {
        // Call debounce_callback in 100ms
        alarm = add_alarm_in_ms(100, debounce_callback, (void *)events, false);
    }
}
void
detectLines(__unused void *params)
{
    gpio_set_irq_enabled_with_callback(ANALOG_PIN,
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    while (true)
    {

        // Start / Stop timer based on signaled event
        if (stop_signal == 0)
            start_timer();
        else
            stop_timer();

        if (sensor != prev_sensor)
        {
            char *line_colour = (sensor == BLACK) ? "BLACK" : "WHITE";
            printf("IR Sensor detected: %s\n", line_colour);
            prev_sensor = sensor;
        }
    }
}

static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4 PLUG OUT
    "EDGE_RISE"   // 0x8 PLUG IN
};

void
gpio_event_string(char *buf, uint32_t events)
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