#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include "hardware/i2c.h"
#include "math.h"
#include "stdint.h"

#define I2C_PORT i2c0

// Accelerometer
// Accelerometer I2C Address
#define ACCEL_ADDR 0x19

// Control Registers
#define ACCEL_CTRL_REG1_A 0x20
#define ACCEL_CTRL_REG4_A 0x23

// Data Output registers
#define ACCEL_OUT_X_L_A 0x28
#define ACCEL_OUT_X_H_A 0x29
#define ACCEL_OUT_Y_L_A 0x2A
#define ACCEL_OUT_Y_H_A 0x2B
#define ACCEL_OUT_Z_L_A 0x2C
#define ACCEL_OUT_Z_H_A 0x2D

// Magnetometer
// Magnetometer I2C Address
#define MAG_ADDR 0x1E

// Mode Register
#define MAG_MR_REG_M 0x02

// Data Ouput registers
#define MAG_OUT_X_H_M 0x03
#define MAG_OUT_X_L_M 0x04
#define MAG_OUT_Z_H_M 0x05
#define MAG_OUT_Z_L_M 0x06
#define MAG_OUT_Y_H_M 0x07
#define MAG_OUT_Y_L_M 0x08

void init_i2c()
{
    i2c_init(I2C_PORT, 100000); // Initialize I2C with 100KHz
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
}

// Function to perform one-dimensional Kalman filtering
float kalman_filter(float measurement, float prev_estimate, float prev_error, float R, float Q)
{
    // Prediction step
    float predicted_estimate = prev_estimate;
    float predicted_error = prev_error + Q;

    // Update step
    float innovation = measurement - predicted_estimate;
    float innovation_covariance = predicted_error + R;
    float kalman_gain = predicted_error / innovation_covariance;

    float updated_estimate = predicted_estimate + kalman_gain * innovation;
    // float updated_error = (1 - kalman_gain) * predicted_error;

    return updated_estimate;
}

uint8_t read_reg(uint8_t addr, uint8_t reg)
{
    // Declare a variable to store the value of the register.
    uint8_t data;

    // Write register address.
    i2c_write_blocking(i2c0, addr, &reg, 1, true);

    // Read value of the register.
    i2c_read_blocking(i2c0, addr, &data, 1, false);

    // Return value.
    return data;
}

void write_reg(uint8_t addr, uint8_t reg, uint8_t value)
{
    // Array to store register to write to & value to write
    uint8_t data[2] = {reg, value};

    // writes data using i2c_write_blocking function
    i2c_write_blocking(i2c0, addr, data, 2, false);
}

void read_accel_data(int16_t *x, int16_t *y, int16_t *z)
{
    // Reads data from each axis
    *x = (int16_t)(read_reg(ACCEL_ADDR, ACCEL_OUT_X_L_A) | (read_reg(ACCEL_ADDR, ACCEL_OUT_X_H_A) << 8));
    *y = (int16_t)(read_reg(ACCEL_ADDR, ACCEL_OUT_Y_L_A) | (read_reg(ACCEL_ADDR, ACCEL_OUT_Y_H_A) << 8));
    *z = (int16_t)(read_reg(ACCEL_ADDR, ACCEL_OUT_Z_L_A) | (read_reg(ACCEL_ADDR, ACCEL_OUT_Z_H_A) << 8));
}

void read_mag_data(int16_t *x, int16_t *y, int16_t *z)
{
    // Reads data from each axis
    *x = (int16_t)(read_reg(MAG_ADDR, MAG_OUT_X_L_M) | (read_reg(MAG_ADDR, MAG_OUT_X_H_M) << 8));
    *y = (int16_t)(read_reg(MAG_ADDR, MAG_OUT_Y_L_M) | (read_reg(MAG_ADDR, MAG_OUT_Y_H_M) << 8));
    *z = (int16_t)(read_reg(MAG_ADDR, MAG_OUT_Z_L_M) | (read_reg(MAG_ADDR, MAG_OUT_Z_H_M) << 8));
}

float get_heading(int16_t x, int16_t y)
{
    // calculate angle of vector (y,x) in radians - gets direction of vector
    float heading = atan2(y, x);

    // convert heading to degrees
    heading *= 180.0 / M_PI;

    // If calculated value is -ve, convert to +ve
    if (heading < 0)
    {
        heading += 360;
    }

    return heading;
}

void read_coordinates(int16_t mx, int16_t my, int16_t mz, int16_t ax, int16_t ay, int16_t az, float coordinate[3])
{

    // Calculate the heading using the magnetometer data
    int16_t heading = get_heading(mx, my);

    // Calculate the roll, pitch, and yaw angles using the accelerometer data
    float roll = atan2f((float)az, (float)ay);
    float pitch = atan2f((float)az, (float)ax);
    float yaw = heading - roll;

    // Calculate the coordinates using the heading, roll, pitch, and yaw angles
    coordinate[0] = (sin(yaw) * cos(pitch));
    coordinate[1] = (cos(yaw) * cos(pitch));
    coordinate[2] = sin(pitch);
}

void get_readings(__unused void *params)
{
    float prev_estimate_ax = 0.0, prev_estimate_ay = 0.0, prev_estimate_az = 0.0;
    float prev_estimate_mx = 0.0, prev_estimate_my = 0.0, prev_estimate_mz = 0.0;

    while (true)
    {
        // initialize variable
        int16_t ax, ay, az, mx, my, mz;

        // read data
        read_accel_data(&ax, &ay, &az);
        read_mag_data(&mx, &my, &mz);

        // Apply Kalman filter to accelerometer and magnetometer data
        ax = kalman_filter((float)ax, prev_estimate_ax, 1.0, 0.1, 1);
        ay = kalman_filter((float)ay, prev_estimate_ay, 1.0, 0.1, 1);
        az = kalman_filter((float)az, prev_estimate_az, 1.0, 0.1, 1);

        mx = kalman_filter((float)mx, prev_estimate_mx, 1.0, 0.1, 1);
        my = kalman_filter((float)my, prev_estimate_my, 1.0, 0.1, 1);
        mz = kalman_filter((float)mz, prev_estimate_mz, 1.0, 0.1, 1);

        // get heading
        float heading = get_heading(mx, my);

        // Calculate the coordinates
        float coordinate[3] = {0.0f, 0.0f, 0.0f};
        read_coordinates(ax, ay, az, mx, my, mz, coordinate);

        // Print the coordinates
        // printf("Coordinates: X=%f, Y=%f, Z=%f\n", coordinate[0], coordinate[1], coordinate[2]);

        // print data
        // printf("Accelerometer: X=%d, Y=%d, Z=%d\n", ax, ay, az);
        // printf("Magnetometer: X=%d, Y=%d, Z=%d, Heading=%.2f degrees\n", mx, my, mz, heading);

        vTaskDelay(100);
    }
}

void magnetometer_main()
{
    // stdio_init_all();
    // Initialize I2C
    // init_i2c();

    // Initialize Accelerometer
    write_reg(ACCEL_ADDR, ACCEL_CTRL_REG1_A, 0x57);
    write_reg(ACCEL_ADDR, ACCEL_CTRL_REG4_A, 0x23);

    // Initialize Magnetometer
    // Sets Magnetometer
    write_reg(MAG_ADDR, MAG_MR_REG_M, 0x00);

    TaskHandle_t magnetometer_reading_task;
    xTaskCreate(get_readings,
                "GetReadingsThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                8,
                &magnetometer_reading_task);
}