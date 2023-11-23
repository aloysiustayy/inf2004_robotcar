/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "encoder.h"
#include "gpio_handler.h"

#define LEFT_VCC_PIN 13
#define RIGHT_VCC_PIN 18

#define NUM_OF_PINS 2
static char event_str[128];
static const uint32_t encoder_pins[] = {2, 3};
static const float wheel_circumference[] = {2.6, 2.6};
static uint32_t notch_count[] = {0, 0};
static float speed_count[] = {0.0, 0.0};
static uint32_t pulse_start_time[] = {0, 0};
static float distance[] = {0.0, 0.0};

void reset_notch(uint side)
{
    if (side >= 0 && side < NUM_OF_PINS)
    {
        notch_count[side] = 0;
    }
}
uint32_t
get_notch(uint side)
{
    if (side >= 0 && side < NUM_OF_PINS)
    {
        return notch_count[side];
    }
    return -1;
}

float get_speed(uint side)
{
    if (side >= 0 && side < NUM_OF_PINS)
    {
        return speed_count[side];
    }
    return -1;
}

void gpio_event_string(char *buf, uint32_t events);

void encoder_callback(uint gpio, uint32_t events)
{
    int encoder_index = -1; // Initialize to an invalid index
    for (int i = 0; i < 2; i++)
    {
        if (gpio == encoder_pins[i])
        {
            encoder_index = i;
            break;
        }
    }

    if (encoder_index == -1)
    {
        // This GPIO is not associated with any encoder
        return;
    }

    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);

    if (events & GPIO_IRQ_EDGE_RISE)
    {
        // Detected a rising edge, indicating a notch
        notch_count[encoder_index]++;
    }
    else if (events & GPIO_IRQ_EDGE_FALL)
    {
        // Detected a falling edge, indicating the end of a pulse
        uint32_t pulse_end_time = time_us_32();
        // Calculate pulse width in microseconds
        uint32_t pulse_width = pulse_end_time - pulse_start_time[encoder_index];
        // Calculate speed in centimeters per second
        // float speed_cps = wheel_circumference[encoder_index] / (pulse_width / 1000000.0);
        speed_count[encoder_index] = wheel_circumference[encoder_index] / (pulse_width / 1000000.0);

        // printf("Encoder #%d, speed: %f\n", encoder_index, speed_count[encoder_index]);

        // Update the distance traveled
        distance[encoder_index] += wheel_circumference[encoder_index];

        // printf(
        //     "Encoder %d - Notch Count: %d, Distance: %.2f cm, Speed: %.2f "
        //     "cm/s\n",
        //     encoder_index + 1,
        //     notch_count[encoder_index],
        //     distance[encoder_index],
        //     speed_count[encoder_index]);
    }

    pulse_start_time[encoder_index] = time_us_32();
}

void encoder_main(__unused void *params)
{
    // stdio_init_all();
    // printf("Starting main encoder");
    // Supply power
    gpio_init(LEFT_VCC_PIN);
    gpio_set_dir(LEFT_VCC_PIN, GPIO_OUT);
    gpio_put(LEFT_VCC_PIN, 1);
    gpio_init(RIGHT_VCC_PIN);
    gpio_set_dir(RIGHT_VCC_PIN, GPIO_OUT);
    gpio_put(RIGHT_VCC_PIN, 1);
    // printf("encoder main");
    // printf("Wheel Encoder\n");
    for (int i = 0; i < 2; i++)
    {
        // Set up interrupts and callbacks for both wheel encoders
        gpio_set_irq_enabled_with_callback(encoder_pins[i],
                                           GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                           true,
                                           &gpio_callback);
    }

    // printf("SUCCESS main encoder");
    // Wait forever
    while (1)
    {
        vTaskDelay(100);
    }
}

static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4
    "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events)
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