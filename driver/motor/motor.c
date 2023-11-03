/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define PWM_PIN1         0
#define PWM_PIN2         1
#define LEFT_WHEEL_PIN1  10
#define LEFT_WHEEL_PIN2  11
#define RIGHT_WHEEL_PIN1 21
#define RIGHT_WHEEL_PIN2 20

uint     slice_num_left;
uint     slice_num_right;
uint16_t wrap;
uint16_t slow_speed;
uint16_t normal_speed;

void motor_init() {
    // Initialize GPIO Pins
    gpio_init(LEFT_WHEEL_PIN1);
    gpio_init(LEFT_WHEEL_PIN2);
    gpio_init(RIGHT_WHEEL_PIN1);
    gpio_init(RIGHT_WHEEL_PIN2);

    // Set the GPIO pins as outputs
    gpio_set_dir(LEFT_WHEEL_PIN1, GPIO_OUT);
    gpio_set_dir(LEFT_WHEEL_PIN2, GPIO_OUT);
    gpio_set_dir(RIGHT_WHEEL_PIN1, GPIO_OUT);
    gpio_set_dir(RIGHT_WHEEL_PIN2, GPIO_OUT);
}

void move_forward() {
    // Set the pins to high and low
    gpio_put(LEFT_WHEEL_PIN1, 1);
    gpio_put(LEFT_WHEEL_PIN2, 0);
    gpio_put(RIGHT_WHEEL_PIN1, 1);
    gpio_put(RIGHT_WHEEL_PIN2, 0);
}

void move_backward() {
    // Set the pins to high and low
    gpio_put(LEFT_WHEEL_PIN1, 0);
    gpio_put(LEFT_WHEEL_PIN2, 1);
    gpio_put(RIGHT_WHEEL_PIN1, 0);
    gpio_put(RIGHT_WHEEL_PIN2, 1);
}

void move_stop() {
    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);
}

void turn_right() {

    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, normal_speed);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, slow_speed);
}

void turn_left() {

    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, slow_speed);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);
}

void pwm_control() { 
    // Tell predefined GPIO pin that they are allocated to the PWM
    gpio_set_function(PWM_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(PWM_PIN2, GPIO_FUNC_PWM);
    // aka speed control
    // Find out which PWM slice is connected to GPIO 0 and 1
    slice_num_left  = pwm_gpio_to_slice_num(PWM_PIN1);
    slice_num_right = pwm_gpio_to_slice_num(PWM_PIN2);

    // Slice the the main clock into 100 div
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_clkdiv(slice_num_right, 100);

    uint16_t sample_size = 12500;
    uint16_t default_hz  = 100;
    uint16_t target_hz   = 20;

    wrap         = sample_size * (default_hz / target_hz);
    normal_speed = wrap / 2;
    slow_speed   = wrap / 6;

    // Set period of 12500 cycles
    pwm_set_wrap(slice_num_left, wrap);
    pwm_set_wrap(slice_num_right, wrap);

    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, normal_speed);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);

    // Set the PWM running
    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);
}

int main() {
    // Initialize the Raspberry Pi Pico SDK
    stdio_init_all();

    motor_init();

    move_forward();

    pwm_control();
    for (;;)
    {
        sleep_ms(10000);
        turn_left();
        sleep_ms(10000);
        turn_right();
    }
}