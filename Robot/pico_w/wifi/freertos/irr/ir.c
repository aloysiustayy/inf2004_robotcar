#include <stdio.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#include "motor.h"
#include "global_defined.h"

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
// uint32_t last_btn_press_left = 0;
// uint32_t last_btn_press_right = 0;

int side_arr[3];
void set_side_arr(int side, int data)
{
    side_arr[side] = data;
}
int *get_side_arr()
{
    return side_arr;
}

// Clock Set Up for debouncing
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

// void gpio_callback_ir(uint gpio, uint32_t events)
// {
//     uint32_t curr_time = (uint32_t)clock();

//     // Put the GPIO event(s) that just happened into event_str
//     // so we can print it
//     gpio_event_stringg(event_str, events);
//     printf("Events!! %s\n", event_str);
//     // for (int i = NUM_OF_PINS - 1; i > -1; i--)
//     for (int i = 0; i < NUM_OF_PINS; i++)
//     {
//         // if (i == 0)
//         //     printf("GPIOOOO is %d analog pin is %d\n", gpio, analog_pins[i]);

//         // Allow changing of colour
//         if (curr_time - last_btn_press[i] >= DEBOUNCE_DELAY_MS)
//         {
//             // do things here
//             // idk how to differentiate the different IRs
//             if (events & GPIO_IRQ_EDGE_RISE)
//             {
//                 printf("PIN %d on white\n", gpio);
//                 set_white(analog_pins[i]);
//             }
//             else if (events & GPIO_IRQ_EDGE_FALL)
//             {
//                 printf("PIN %d on black\n", gpio);
//                 set_black(analog_pins[i]);
//             }
//         }

//         // If current color is not prev color (BLACK to WHITE, vice versa)
//         if (sensor[i] != prev_sensor[i])
//         {
//             char *line_colour = (sensor[i] == BLACK) ? "BLACK" : "WHITE";

//             if (sensor[i] == WHITE && prev_sensor[i] == BLACK)
//             {
//                 // printf("Turning right now");
//                 printf("Sensor %d detect %s\n", analog_pins[i], line_colour);
//                 if (analog_pins[i] == LEFT_ANALOG_PIN)
//                 {
//                     send_sensor_data("left", 90);
//                 }
//                 else if (analog_pins[i] == RIGHT_ANALOG_PIN)
//                 {
//                     send_sensor_data("right", 90);
//                 }
//             }

//             prev_sensor[i] = sensor[i];
//             // if (gpio == analog_pins[i])
//             // {
//             //     printf("Events is %d\n", events);
//             // if (events & GPIO_IRQ_EDGE_RISE)
//             // {
//             //     stop_signal[i] = 0;
//             // }
//             // else if (events & GPIO_IRQ_EDGE_FALL)
//             // {
//             //     stop_signal[i] = 1;

//             //         // Update time of the last button press
//             //         last_btn_press[i] = curr_time;
//             //     }
//             // }
//         }
//     }
// }

int64_t
alarm_callback(alarm_id_t id, void *user_data)
{

    printf("%d\n", cur_pin);

    // if (gpio_get(cur_pin) == 0)
    // {
    //     // this is white
    //     if (cur_pin == analog_pins[0])
    //     {
    //         send_sensor_data("left", 90);
    //     }
    //     else if (cur_pin == analog_pins[1])
    //     {
    //         send_sensor_data("right", 90);
    //     }
    // }
    bool leftSensor = gpio_get(analog_pins[0]);
    bool rightSensor = gpio_get(analog_pins[1]);
    if (leftSensor == BLACK &&
        rightSensor == BLACK)
    {
        // Still black, BOUNDARY
        printf("This is boundary");
        send_sensor_data("mapping-black", 1);
        }
    else if (leftSensor == WHITE &&
             rightSensor == WHITE)
    {
        // Barcode.
        printf("This is barcode");
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

    // printf("Detect line now");
    // for (int i = NUM_OF_PINS - 1; i > -1; i--)
    //     for (int i = 0; i < NUM_OF_PINS; i++)
    //     {
    //         gpio_set_irq_enabled_with_callback(analog_pins[i],
    //                                            GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
    //                                            true,
    //                                            &gpio_callback_ir);
    //     }

    printf("Starting IR!");
    while (true)
    {
        vTaskDelay(100);
        for (int i = NUM_OF_PINS - 1; i > -1; i--)
        {
            // if (isCarTurning() == false)
            // {
            if (gpio_get(analog_pins[i]) == 1)
            {
                set_black(analog_pins[i]);
            }
            else
            {
                set_white(analog_pins[i]);
            }
            // }

            // if (sensor[i] != prev_sensor[i])
            // {
            // printf("Left Floor %d\tRight Floor %d\tFront Floor %d\n", get_side_arr()[LEFT], get_side_arr()[RIGHT], get_side_arr()[FRONT]);
            // Check if both are black and not mapping right now
            // Start checking
            if (sensor[0] == BLACK && sensor[1] == BLACK)
            {
                // printf("both black\n");
                // send_sensor_data("left", 90);
                // send_sensor_data("stop", 1);
                // Call alarm in fking short time to check if its still black
                // If still black: BOUNDARY
                // If got become white: BARCODE
                // TODO: Play with timer on barcode see can detect barcode or not
                // if ((getState() != MAPPING_FRONT) ||
                //     (getState() != MAPPING_LEFT) ||
                //     (getState() != MAPPING_RIGHT))
                // {
                // }
                switch (getState())
                {
                case MAPPED_FRONT:
                    // now try to map left
                    if (get_side_arr()[LEFT] == WHITE)
                    {
                        // Sending this will make it u_turn()
                        send_sensor_data("mapping-black", 2);
                    }
                    break;
                case MAPPED_LEFT:
                    set_side_arr(LEFT, BLACK);
                    if (get_side_arr()[LEFT] == WHITE)
                    {
                        send_sensor_data("mapping-black", 3);
                    }
                    break;
                case MAPPED_RIGHT:
                    set_side_arr(RIGHT, BLACK);
                    break;
                default:
                    // If i detect black when moving forward
                    if (get_side_arr()[FRONT] == WHITE)
                    {
                        // Added alarm here to differentiate between barcode / boundary
                        add_alarm_in_ms(100, alarm_callback, NULL, false);
                    }
                    set_side_arr(FRONT, BLACK);
                    break;
                }
            }
            else if (sensor[i] == WHITE && sensor[i] == WHITE)
            {
                switch (getState())
                {
                case MAPPED_FRONT:
                    // If i am facing left right now, and i detect both white
                    send_sensor_data("mapping-white", 2);

                    // So i move in my current direction
                    // FRONT was my old FRONT (before turning), so i reset it
                    set_side_arr(FRONT, WHITE);
                    break;
                case MAPPED_LEFT:
                    set_side_arr(LEFT, WHITE);
                    send_sensor_data("mapping-white", 3);
                    // means can move straight
                    break;
                case MAPPED_RIGHT:
                    set_side_arr(RIGHT, WHITE);
                    break;
                default:
                    set_side_arr(FRONT, WHITE);
                    send_sensor_data("mapping-white", 1);
                    break;
                }
            }
            // if (sensor[0] == WHITE && prev_sensor[0] == BLACK &&
            //     sensor[1] == WHITE && prev_sensor[1] == BLACK)
            // {
            // }
            // char *line_colour = (sensor[i] == BLACK) ? "BLACK" : "WHITE";

            // if (sensor[i] == WHITE && prev_sensor[i] == BLACK)
            // {
            // printf("Turning right now");
            // printf("Sensor %d detect %s\n", analog_pins[i], line_colour);
            // if (analog_pins[i] == LEFT_ANALOG_PIN)
            // {
            //     send_sensor_data("left", 90);
            // }
            // else if (analog_pins[i] == RIGHT_ANALOG_PIN)
            // {
            //     send_sensor_data("right", 90);
            // }
            // cur_pin = analog_pins[i];
            // add_alarm_in_ms(600, alarm_callback, NULL, false);
            // if(sensor[0] == BLACK && sensor[1] == BLACK){
            //
            // }
            // }
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
            // }
        }
    }
}

// static const char *gpio_irq_str[] = {
//     "LEVEL_LOW",  // 0x1
//     "LEVEL_HIGH", // 0x2
//     "EDGE_FALL",  // 0x4
//     "EDGE_RISE"   // 0x8
// };

// void gpio_event_stringg(char *buf, uint32_t events)
// {
//     for (uint i = 0; i < 4; i++)
//     {
//         uint mask = (1 << i);
//         if (events & mask)
//         {
//             // Copy this event string into the user string
//             const char *event_str = gpio_irq_str[i];
//             while (*event_str != '\0')
//             {
//                 *buf++ = *event_str++;
//             }
//             events &= ~mask;

//             // If more events add ", "
//             if (events)
//             {
//                 *buf++ = ',';
//                 *buf++ = ' ';
//             }
//         }
//     }
//     *buf++ = '\0';
// }
/*** end of file ***/