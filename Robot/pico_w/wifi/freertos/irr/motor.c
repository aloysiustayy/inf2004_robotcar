#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include "motor.h"
#include "encoder.h"
#include "ir.h"
#include "global_defined.h"

#define PWM_PIN1 0
#define PWM_PIN2 1
#define LEFT_WHEEL_PIN1 10
#define LEFT_WHEEL_PIN2 11
#define RIGHT_WHEEL_PIN1 21
#define RIGHT_WHEEL_PIN2 20
#define NOTCH_TO_DEGREE 15 // 1 notch is 15 degree

uint slice_num_left;
uint slice_num_right;
uint16_t wrap;
uint16_t slow_speed;
uint16_t normal_speed;
uint16_t left_speed;

char motor_command[100]; // receive from task_recv_msg_task

// Define PID controller constants
float Kp = 0.5;  // Proportional gain
float Ki = 0.2;  // Integral gain
float Kd = 0.02; // Derivative gain

// Define target speed
float target_speed = 100.0;
float proportional_factor = 221.43;

bool turning = false;
void setCarTurning(bool val)
{
    turning = val;
    printf("Car is turning");
}
bool isCarTurning()
{
    return turning;
}

// Function to compute control signal using PID control
float compute_pid(float setpoint, float current_value, float *integral, float *prev_error)
{
    float error = setpoint - current_value; // Calculate error by taking desired - current

    *integral += error; // Update integral term

    float derivative = error - *prev_error; // Calculate derivative term

    float control_signal = Kp * error + Ki * (*integral) + Kd * derivative; // Compute control signal

    *prev_error = error; // Update previous error in preparation for next iteration

    return control_signal; // Return computed control signal
}

uint32_t
degree_to_notch(uint8_t degree)
{
    if (degree >= 15 && degree <= 360)
        return degree / NOTCH_TO_DEGREE;

    return -1;
}
void motor_init()
{
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

void move_forward()
{
    // Set the pins to high and low
    gpio_put(LEFT_WHEEL_PIN1, 0);
    gpio_put(LEFT_WHEEL_PIN2, 1);
    gpio_put(RIGHT_WHEEL_PIN1, 0);
    gpio_put(RIGHT_WHEEL_PIN2, 1);
}

void move_backward()
{
    // Set the pins to high and low
    gpio_put(LEFT_WHEEL_PIN1, 1);
    gpio_put(LEFT_WHEEL_PIN2, 0);
    gpio_put(RIGHT_WHEEL_PIN1, 1);
    gpio_put(RIGHT_WHEEL_PIN2, 0);
}

void move_stop()
{
    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);
}

void move_straight() {

    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, left_speed);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);

}

void turn_right(uint8_t degree)
{
    printf("Turning right by %d degree now\n", degree);

    move_backward();
    vTaskDelay(500);
    move_forward();
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, left_speed);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, slow_speed);


    reset_notch(LEFT);

    // This var will contain the notch count needed to turn by 'degree'
    uint8_t notch_threshold = degree_to_notch(degree) * 3;

    while (true)
    {
        if (get_notch(LEFT) > 4)
        {
            break;
        }
    }

    move_stop();
    move_straight();
    printf("Moving straight now\n");
    vTaskDelay(500);
}

void turn_left(uint8_t degree)
{
    printf("Turning left by %d degree now\n", degree);

    move_backward();
    vTaskDelay(500);
    move_forward();
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, slow_speed);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);


    reset_notch(RIGHT);

    // This var will contain the notch count needed to turn by 'degree'
    uint8_t notch_threshold = degree_to_notch(degree) * 3;

    while (true)
    {
        if (get_notch(RIGHT) > 4)
        {
            break;
        }
    }

    move_stop();
    move_straight();
    printf("Moving straight now\n");
    vTaskDelay(500);
}

void u_turn()
{
    // printf("Notch for left should be more than right\n");
    printf("U-turning now!!");
    move_stop();
    sleep_ms(800);
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, normal_speed);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);

    gpio_put(RIGHT_WHEEL_PIN1, 1);
    gpio_put(RIGHT_WHEEL_PIN2, 0);

    // Reset LEFT notch because to turn right, left wheel encoder's notch
    // should be 0 to calculate how many notch it has travelled
    reset_notch(LEFT);

    // This var will contain the notch count needed to turn by 'degree'
    uint8_t notch_threshold = (degree_to_notch(180) * 2.2) + 1;

    while (true)
    {
        if (get_notch(LEFT) > notch_threshold)
        {
            break;
        }
    }

    gpio_put(RIGHT_WHEEL_PIN1, 0);
    gpio_put(RIGHT_WHEEL_PIN2, 1);
    printf("Moving straight now\n");
    sleep_ms(500);
    setCarTurning(false);
}

void pid_speed_left()
{
    float integral = 0.0;   // Integral term for PID control
    float prev_error = 0.0; // Previous error for PID control
    float current_speed = get_speed(LEFT);
    float pid_value_left = compute_pid(target_speed, current_speed, &integral, &prev_error);

    float new_duty_cycle = left_speed + (pid_value_left * proportional_factor);

    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, new_duty_cycle);
}

void pid_speed_right()
{
    float integral = 0.0;   // Integral term for PID control
    float prev_error = 0.0; // Previous error for PID control
    float current_speed = get_speed(RIGHT);
    float pid_value_right = compute_pid(target_speed, current_speed, &integral, &prev_error);

    float new_duty_cycle = (normal_speed + (pid_value_right * proportional_factor) * 1.08f);

    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, new_duty_cycle);
}

void pwm_control()
{
    // Tell predefined GPIO pin that they are allocated to the PWM
    gpio_set_function(PWM_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(PWM_PIN2, GPIO_FUNC_PWM);
    // aka speed control
    // Find out which PWM slice is connected to GPIO 0 and 1
    slice_num_left = pwm_gpio_to_slice_num(PWM_PIN1);
    slice_num_right = pwm_gpio_to_slice_num(PWM_PIN2);

    // Slice the the main clock into 100 div
    pwm_set_clkdiv(slice_num_left, 100);
    pwm_set_clkdiv(slice_num_right, 100);

    uint16_t sample_size = 12500;
    uint16_t default_hz = 100;
    uint16_t target_hz = 25;

    wrap = sample_size * (default_hz / target_hz);
    normal_speed = wrap * 0.6; // make it go slower
    slow_speed = 0;
    
    // Calibrate
    left_speed = wrap*0.48;

    // printf("Normal speed is %d\n", normal_speed);
    // Set period of 12500 cycles
    pwm_set_wrap(slice_num_left, wrap);
    pwm_set_wrap(slice_num_right, wrap);

    // Set channel A output high for one cycle before dropping (for GPIO 0)
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, left_speed);

    // Set channel B output high for one cycle before dropping (for GPIO 1)
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, normal_speed);

    // Set the PWM running
    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);
}

void set_motor_command(char *data)
{
    // Set motor_command
    snprintf(motor_command, sizeof(motor_command), "%s", data);
}



void react_to_commands(__unused void *params)
{
    while (true)
    {

        // probably need this so that other task can get some time for their
        // allocation
        vTaskDelay(100);
        if (turning == false)
        {
            // If command is "left=90"
            if (strcmp(motor_command, "left=90") == 0)
            {
                // turn_left(90);
                printf("Turning left now...");
                turning = true;
                // once detect white, turn 90 straight away
                turn_left(90);
                turning = false;
                // Reset motor_command
                snprintf(motor_command, sizeof(motor_command), "%s", "");
            }
            else if (strcmp(motor_command, "right=90") == 0)
            {
                printf("Turning right now...");
                turning = true;
                turn_right(90);

                turning = false;
                // Reset motor_command
                snprintf(motor_command, sizeof(motor_command), "%s", "");
            }
            else if (strcmp(motor_command, "U-turn=180") == 0)
            {
                printf("Too near to obstacles... Uturn now...");
                turning = true;
                u_turn();
                turning = false;
                snprintf(motor_command, sizeof(motor_command), "%s", "");
            }
            else if (strcmp(motor_command, "stop=1") == 0)
            {
                // turn_left(90);
                // printf("Turning left now...");
                turning = true;
                // once detect white, turn 90 straight away
                move_stop();
                turn_left(90);
                turning = false; // this line will make it move immediately on next loop (goes to else statement)

                // Reset motor_command
                snprintf(motor_command, sizeof(motor_command), "%s", "");
            }
            else
            {
                pid_speed_left();
                pid_speed_right();
                move_straight();
            }
        }
    }
}
void motor_main()
{
    motor_init();

    move_forward();

    pwm_control();

    TaskHandle_t motor_command_task;

    xTaskCreate(react_to_commands,
                "ReactCommand",
                configMINIMAL_STACK_SIZE,
                NULL,
                8,
                &motor_command_task);
}