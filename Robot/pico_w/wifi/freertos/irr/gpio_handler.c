#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "gpio_handler.h"

#include "encoder.h"
#include "ultrasonic.h"

#define LEFT_ENCODER 2
#define RIGHT_ENCODER 3
#define ULTRASONIC_ECHO 7

void encoder_callback(uint gpio, uint32_t events);
void echoPinInterrupt(uint gpio, uint32_t events);

void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == LEFT_ENCODER || gpio == RIGHT_ENCODER)
    {
        encoder_callback(gpio, events);
    }
    else if (gpio == ULTRASONIC_ECHO)
    {
        // Handle events for encoder pin 2
        echoPinInterrupt(gpio, events);
    }
    // Add more conditions for additional pins if needed
}
