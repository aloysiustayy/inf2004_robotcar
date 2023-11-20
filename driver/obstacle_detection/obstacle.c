#include "pico/stdlib.h"
#include <stdio.h>
#include "ultrasonic.h"
#include "hardware/uart.h"

// For moving average
#define NUM_SAMPLES 4

uint trigPin = 0;
uint echoPin = 1;

bool obstacleDetected;
void echoPinInterrupt(uint gpio, uint32_t events);

// Kalman filter parameters
double x_est_last = 0;
double P_last = 1;
double Q = 0.022; // Process noise covariance
double R = 0.617; // Measurement noise covariance

// Kalman filter function
double kalmanFilter(double z_measured) {
    // Prediction
    double x_temp_est = x_est_last;
    double P_temp = P_last + Q;

    // Calculate Kalman gain
    double K = P_temp / (P_temp + R);

    // Update (Correction)
    double x_est = x_temp_est + K * (z_measured - x_temp_est);
    double P = (1 - K) * P_temp;

    // Update previous values
    x_est_last = x_est;
    P_last = P;

    return x_est;
}

// Function to calculate moving average
uint16_t calculateMovingAverage(uint16_t newReading, uint16_t readings[], int* index, uint32_t* sum, int* count) {
    // Subtract the oldest reading from the sum
    *sum -= readings[*index];

    // Store latest reading in array and add to sum
    readings[*index] = newReading;
    *sum += newReading;

    // Calculate moving average if we have at least NUM_SAMPLES readings
    uint16_t movingAverage = (*count >= NUM_SAMPLES) ? *sum / NUM_SAMPLES : newReading;

    // Update index and count for the next reading
    *index = (*index + 1) % NUM_SAMPLES;
    if (*count < NUM_SAMPLES) (*count)++;

    return movingAverage;
}

int main()
{
    stdio_init_all();
    setupUltrasonicPins(trigPin, echoPin);

    uint16_t readings[NUM_SAMPLES] = {0};
    int index = 0;
    uint32_t sum = 0;
    int count = 0;

    // Set up an interrupt handler for the echo pin
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echoPinInterrupt);

    while (1) { 

        uint64_t distReading = getCm(trigPin, echoPin);

        // Calculate the moving average
        uint16_t movingAverageDist = calculateMovingAverage(distReading, readings, &index, &sum, &count);

        // Apply Kalman filter to the sensor measurement
        // double kalmanFilteredDist = kalmanFilter(distReading);

        // Print filtered distance
        // printf("\nKalman Filtered Distance: %.0f cm", kalmanFilteredDist);
        // printf("Moving Average Distance: %d cm\n", movingAverageDist);

        // Check if obstacle detected
        if (movingAverageDist > 0 && movingAverageDist < 8) {
            obstacleDetected = true;
            printf("Obstacle detected\n");
        }
        // printf("\n %d cm", getCm(trigPin, echoPin)); 

        // Delay between each reading
        sleep_ms(100);
    }
}
