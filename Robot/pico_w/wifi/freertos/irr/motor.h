#ifndef MOTOR_H
#define MOTOR_H

// PWM pin definitions
#define PWM_PIN1 0
#define PWM_PIN2 1

// Left and right wheel pin definitions
#define LEFT_WHEEL_PIN1 10
#define LEFT_WHEEL_PIN2 11
#define RIGHT_WHEEL_PIN1 21
#define RIGHT_WHEEL_PIN2 20

// Variable declaration
typedef enum
{
    MOVING,
    IDLE,
    TURN_LEFT,
    TURN_RIGHT,
    UTURN,
    MAPPED_FRONT,
    MAPPED_LEFT,
    MAPPED_RIGHT
} State;

// Function declarations
void setState(State newState);
State getState();
void setCarTurning(bool val);
bool isCarTurning();
void motor_init();
void move_forward();
void move_backward();
void move_stop();
void turn_right(uint8_t degree);
void turn_left(uint8_t degree);
void set_motor_command(char *data);
void pwm_control();
void motor_main();
void pid_speed_left();
void pid_speed_right();
float compute_pid(float setpoint, float current_value, float *integral, float *prev_error);
uint32_t degree_to_notch(uint8_t degree);

#endif // MOTOR_H