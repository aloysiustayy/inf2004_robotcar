#ifndef ENCODER_H
#define ENCODER_H

// Encoder pin definitions
#define LEFT_VCC_PIN 13
#define ENCODER_PINS \
    {                \
        2, 3         \
    }

// Wheel circumference in centimeters
#define WHEEL_CIRCUMFERENCE \
    {                       \
        2.6, 2.6            \
    }

// Function declarations
void reset_notch(uint side);
uint32_t get_notch(uint side);
float get_speed(uint side);
void gpio_callback(uint gpio, uint32_t events);
void gpio_event_string(char *buf, uint32_t events);
void encoder_main();

#endif // ENCODER_H