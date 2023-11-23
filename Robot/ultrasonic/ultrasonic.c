// Get readings from ultrasonic sensor

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

const uint32_t timeout_us = 26100; // Timeout in microseconds

absolute_time_t startTime, endTime;

int getCode()
{
    // printf("code is 456");
    return 2;
}
void setupUltrasonicPins(uint trigPin, uint echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
    // printf("Trig %d and Echo %d setup done!\n", trigPin, echoPin);
}

// void echoPinInterrupt(uint gpio, uint32_t events)
// {
//     // printf("i am echo 1\n");
//     // If Echo Pin is High
//     if (events & GPIO_IRQ_EDGE_RISE)
//     {
//         startTime = get_absolute_time(); // Get Start time
//         // printf("i am echo 2\n");
//     }

//     // If Echo Pin is Low
//     if (events & GPIO_IRQ_EDGE_FALL)
//     {
//         // printf("i am echo 3\n");
//         endTime = get_absolute_time(); // Get End time

//         // Calculate difference, and check if timed out
//         if (absolute_time_diff_us(startTime, endTime) >= timeout_us)
//         {
//             startTime = endTime;
//         }
//         // printf("i am echo 4\n");
//     }
// }

void echoPinInterrupt(uint gpio, uint32_t events)
{
    // If Echo Pin is High
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        startTime = get_absolute_time(); // Get Start time
        // printf("i am echo 2\n");
    }

    // If Echo Pin is Low
    if (events & GPIO_IRQ_EDGE_FALL)
    {
        // printf("i am echo 3\n");
        endTime = get_absolute_time(); // Get End time

        // Calculate difference, and check if timed out
        if (absolute_time_diff_us(startTime, endTime) >= timeout_us)
        {
            startTime = endTime;
        }
        // printf("i am echo 4\n");
    }
}
uint64_t getPulse(uint trigPin, uint echoPin)
{
    // printf("i am getPulse\n");

    gpio_put(trigPin, 1);
    // printf("part a) i am getPulse but %d\n", gpio_get(trigPin));
    sleep_us(10);
    gpio_put(trigPin, 0);
    // printf("part b) i am getPulse but %d\n", gpio_get(trigPin));

    return absolute_time_diff_us(startTime, endTime);
}

uint64_t getCm(uint trigPin, uint echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    // printf("Pulse len %d\n", pulseLength);
    return pulseLength / 29 / 2;
}

uint64_t getInch(uint trigPin, uint echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);

    return (long)pulseLength / 74.f / 2.f;
}