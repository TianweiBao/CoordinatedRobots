#ifndef JOYSTICK_UTILS_H
#define JOYSTICK_UTILS_H

#include <stdint.h>
#include <stdbool.h>

//analog inputs and ADC channels for the joystick X-axis and Y-axis
#define JOYSTICK_X_ANALOG SAADC_CH_PSELP_PSELP_AnalogInput1 // P1 corresponds to AIN1
#define JOYSTICK_Y_ANALOG SAADC_CH_PSELP_PSELP_AnalogInput2 // P2 corresponds to AIN2
#define ADC_JOYSTICK_X_CHANNEL 0
#define ADC_JOYSTICK_Y_CHANNEL 1
//pin for buttons, buzzer, and vibration motor
#define BTN_C_PIN EDGE_P12
#define BTN_D_PIN EDGE_P13
#define BTN_E_PIN EDGE_P14
#define BTN_F_PIN EDGE_P15
#define BUZZER_PIN EDGE_P0
#define VIBRATION_MOTOR_PIN EDGE_P16


// Function declarations
void adc_init(void);
int map_value(int value, int from_low, int from_high, int to_low, int to_high);
float read_channel(uint8_t channel);
bool is_button_c_pressed(void);
bool is_button_d_pressed(void);
bool is_button_e_pressed(void);
bool is_button_f_pressed(void);
void check_all_buttons(uint8_t button_states[4]);
void buzzer_on(void);
void buzzer_off(void);
void vibration_motor_on(void);
void vibration_motor_off(void);
#endif // JOYSTICK_UTILS_H