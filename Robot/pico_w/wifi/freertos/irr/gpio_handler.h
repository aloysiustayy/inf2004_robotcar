// gpio_handler.h
#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void gpio_callback(uint gpio, uint32_t events);

#endif // GPIO_HANDLER_H
