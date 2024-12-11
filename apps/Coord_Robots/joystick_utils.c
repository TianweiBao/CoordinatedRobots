#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "joystick_utils.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_twi_mngr.h"
#include "nrfx_pwm.h"
#include "microbit_v2.h"
#include "nrfx_saadc.h"

static void saadc_callback(nrfx_saadc_evt_t const * p_event){
    // ignore for now
}

void adc_init(void){
    // initialize the SAADC
    nrfx_saadc_config_t saadc_config = {
        .resolution = NRF_SAADC_RESOLUTION_12BIT,
        .oversample = NRF_SAADC_OVERSAMPLE_DISABLED,
        .interrupt_priority = 4,
        .low_power_mode = false
    };
    ret_code_t error_code = nrfx_saadc_init(&saadc_config, saadc_callback);
    APP_ERROR_CHECK(error_code);
    // initialize the ADC_JOYSTICK_X channel
    nrf_saadc_channel_config_t joystick_x_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(JOYSTICK_X_ANALOG);
    error_code = nrfx_saadc_channel_init(ADC_JOYSTICK_X_CHANNEL, &joystick_x_config);
    APP_ERROR_CHECK(error_code);

    // initialize the ADC_JOYSTICK_Y channel
    nrf_saadc_channel_config_t joystick_y_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(JOYSTICK_Y_ANALOG);
    error_code = nrfx_saadc_channel_init(ADC_JOYSTICK_Y_CHANNEL, &joystick_y_config);
    APP_ERROR_CHECK(error_code);

    printf("SAADC initialized!\n");
}

// Helper function to map the raw ADC value to a desired speed command
int map_value(int value, int from_low, int from_high, int to_low, int to_high) {
    return to_low + (value - from_low) * (to_high - to_low) / (from_high - from_low);
}

// Helper function to read the ADC channel and convert the raw ADC value to a speed command
float read_channel(uint8_t channel){
    // Read ADC counts (0-4095)
    int16_t adc_counts = 0;
    ret_code_t error_code = nrfx_saadc_sample_convert(channel, &adc_counts);
    APP_ERROR_CHECK(error_code);
    // Convert ADC counts to motor speed with range from -100 to 100
    float speed_cmd = map_value(adc_counts, 0, 3760, -100, 100);
    return speed_cmd;
}

// Initialize GPIO pins for buttons, buzzer, and vibration motor
void joysticks_init(void) {
    nrf_gpio_cfg_input(BTN_C_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_D_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_E_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_F_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);
    // nrf_gpio_cfg_output(VIBRATION_MOTOR_PIN);
    // nrf_gpio_pin_clear(VIBRATION_MOTOR_PIN);
}


bool is_button_c_pressed(void) {
    return !nrf_gpio_pin_read(BTN_C_PIN);
}

bool is_button_d_pressed(void) {
    return !nrf_gpio_pin_read(BTN_D_PIN);
}

bool is_button_e_pressed(void) {
    return !nrf_gpio_pin_read(BTN_E_PIN);
}

bool is_button_f_pressed(void) {
    return !nrf_gpio_pin_read(BTN_F_PIN);
}

void check_all_buttons(uint8_t button_states[4]) {
    button_states[0] = is_button_c_pressed() ? 1 : 0;//button C
    button_states[1] = is_button_d_pressed() ? 1 : 0;//button D
    button_states[2] = is_button_e_pressed() ? 1 : 0;//button E
    button_states[3] = is_button_f_pressed() ? 1 : 0;//button F
}


// Turn the Buzzer on
void buzzer_on(void) {
    nrf_gpio_pin_set(BUZZER_PIN);
}

// Turn the Buzzer off
void buzzer_off(void) {
    nrf_gpio_pin_clear(BUZZER_PIN);
}


// Turn the Vibration Motor on
void vibration_motor_on(void) {
    nrf_gpio_pin_set(VIBRATION_MOTOR_PIN);
    // nrf_gpio_pin_write(VIBRATION_MOTOR_PIN, 1);
}

// Turn the Vibration Motor off
void vibration_motor_off(void) {
    nrf_gpio_pin_clear(VIBRATION_MOTOR_PIN);
    // nrf_gpio_pin_write(VIBRATION_MOTOR_PIN, 0);
}