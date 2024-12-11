#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include <math.h>
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_twi_mngr.h"
#include "nrfx_pwm.h"
#include "microbit_v2.h"
#include "nrfx_saadc.h"
#include "nrf_802154.h"
#include "joystick_utils.h"
#include "radio_utils.h"
#include "cutebot_utils.h"


// Helper function to constrain a value within a range
static int constrain(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Helper function to perform a 1-byte I2C read of a given register
static uint8_t i2c_reg_read(const nrf_twi_mngr_t* i2c_instance, uint8_t i2c_addr, uint8_t reg_addr) {
    uint8_t rx_buf = 0;
    nrf_twi_mngr_transfer_t const read_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)
    };
    ret_code_t result = nrf_twi_mngr_perform(i2c_instance, NULL, read_transfer, 2, NULL);
    if (result != NRF_SUCCESS) {
        printf("Error reading from I2C device: %lX\n", result);
    }
    return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
static void i2c_reg_write(const nrf_twi_mngr_t* i2c_instance, uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
    uint8_t tx_buf[2] = {reg_addr, data};
    nrf_twi_mngr_transfer_t const write_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, tx_buf, 2, 0),
    };
    ret_code_t result = nrf_twi_mngr_perform(i2c_instance, NULL, write_transfer, 1, NULL);
    if (result != NRF_SUCCESS) {
        printf("Error writing to I2C device: %lX\n", result);
    }
}

// Initialize the Cutebot
void cutebot_init(Cutebot* self, const nrf_twi_mngr_t* i2c) {
    self->i2c_addr = CUTEBOT_ADDR;
    self->i2c_instance = i2c;
    self->pin_echo = EDGE_P12; // pin for echo
    self->pin_trigger = EDGE_P8;  // pin for trigger
    nrf_gpio_cfg_output(self->pin_trigger);
    nrf_gpio_cfg_input(self->pin_echo, NRF_GPIO_PIN_NOPULL);
    self->pin_Ltrack = EDGE_P13; // pin for left track sensor
    self->pin_Rtrack = EDGE_P14; // pin for right track sensor
    nrf_gpio_cfg_input(self->pin_Ltrack, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(self->pin_Rtrack, NRF_GPIO_PIN_NOPULL);
}

// Set the speed of the left and right wheels
void cutebot_set_motors_speed(Cutebot* self, int left_wheel_speed, int right_wheel_speed) {
    // Constrain the speed to the range -100 to 100
    left_wheel_speed = constrain(left_wheel_speed, -100, 100);
    right_wheel_speed = constrain(right_wheel_speed, -100, 100);
    printf("Left wheel speed: %d, Right wheel speed: %d\n", left_wheel_speed, right_wheel_speed);
    // Set the direction of the left and right wheels
    uint8_t left_dir = (left_wheel_speed > 0) ? FORWARD_FLAG : BACKWARD_FLAG;
    uint8_t right_dir = (right_wheel_speed > 0) ? FORWARD_FLAG : BACKWARD_FLAG;

    // Convert speed to absolute value
    left_wheel_speed = abs(left_wheel_speed);
    right_wheel_speed = abs(right_wheel_speed);

    // Send the speed and direction to the Cutebot
    uint8_t left_wheel_data[4] = {LEFT_WHEEL_ADDR, left_dir, left_wheel_speed, 0};
    uint8_t right_wheel_data[4] = {RIGHT_WHEEL_ADDR, right_dir, right_wheel_speed, 0};
    nrf_twi_mngr_transfer_t const write_transfer[] = {
        NRF_TWI_MNGR_WRITE(self->i2c_addr, left_wheel_data, 4, 0),
        NRF_TWI_MNGR_WRITE(self->i2c_addr, right_wheel_data, 4, 0)
    };
    ret_code_t result = nrf_twi_mngr_perform(self->i2c_instance, NULL, write_transfer, 2, NULL);
    if (result != NRF_SUCCESS) {
        printf("Error setting motor speed: %lX\n", result);
    }
}

// Set the color of the head light
void cutebot_set_head_light(Cutebot* self, const char* head_light, uint8_t r, uint8_t g, uint8_t b) {
    // Set the color of the head light
    uint8_t light_addr;
    if (strcmp(head_light, "left") == 0) {
        light_addr = LEFT_LIGHT_ADDR;
    } else if (strcmp(head_light, "right") == 0) {
        light_addr = RIGHT_LIGHT_ADDR;
    } else {
        printf("Invalid head light: %s\n", head_light);
        return;
    }
    uint8_t light_data[4] = {light_addr, r, g, b};
    nrf_twi_mngr_transfer_t const write_transfer[] = {
        NRF_TWI_MNGR_WRITE(self->i2c_addr, light_data, sizeof(light_data), 0)
    };
    ret_code_t result = nrf_twi_mngr_perform(self->i2c_instance, NULL, write_transfer, 1, NULL);
    if (result != NRF_SUCCESS) {
        printf("Error setting head light color: %lX\n", result);
    }
}

// Get the distance from the ultrasonic sensor
float cutebot_get_distance(Cutebot* self, const char* unit) {
    // Send a trigger signal to the ultrasonic sensor
    nrf_gpio_pin_clear(self->pin_trigger);
    nrf_delay_us(2);
    nrf_gpio_pin_set(self->pin_trigger);
    nrf_delay_us(10);
    nrf_gpio_pin_clear(self->pin_trigger);

    // Wait for the echo signal
    while (nrf_gpio_pin_read(self->pin_echo) == 0);
    uint32_t start_time = app_timer_cnt_get();
    printf("Start time: %lu\n", start_time);
    while (nrf_gpio_pin_read(self->pin_echo) == 1);
    printf("End time: %lu\n", app_timer_cnt_get());
    uint32_t end_time = app_timer_cnt_get();
    uint32_t time_diff = end_time - start_time;

    // Calculate the distance
    float distance = (time_diff * 340.0) / (2 * 1000000); // speed of sound is 340 m/s
    if (strcmp(unit, "cm") == 0) {
        distance *= 100;
    } else if (strcmp(unit, "mm") == 0) {
        distance *= 1000;
    }
    return distance;
}

// Get the tracking status
uint8_t cutebot_get_tracking(Cutebot* self) {
        // 00 -> all in white
        // 10 -> left in black right in white
        // 01 -> left in white right in black
        // 11 -> all in black
    uint8_t left = nrf_gpio_pin_read(self->pin_Ltrack);
    uint8_t right = nrf_gpio_pin_read(self->pin_Rtrack);

    if (left == 1 && right == 1) {
        return 0; // 00 -> all in white
    } else if (left == 0 && right == 1) {
        return 2; // 10 -> left in black, right in white
    } else if (left == 1 && right == 0) {
        return 1; // 01 -> left in white, right in black
    } else if (left == 0 && right == 0) {
        return 3; // 11 -> all in black
    } else {
        printf("Unknown ERROR\n");
        return 255; // Error code
    }
}

// Follow the black road
WheelSpeeds cutebot_follow_black_road(Cutebot* self) {
    uint8_t tracking_status = cutebot_get_tracking(self);
    printf("Tracking status: %d\n", tracking_status);
    WheelSpeeds speeds;

    switch (tracking_status) {
        case 0: // 00 -> all in white
            speeds.left_wheel_speed = 15;
            speeds.right_wheel_speed = 15;
            break;
        case 2: // 10 -> left in black, right in white
            speeds.left_wheel_speed = 0;
            speeds.right_wheel_speed = 15;
            break;
        case 1: // 01 -> left in white, right in black
            speeds.left_wheel_speed = 15;
            speeds.right_wheel_speed = 0;
            break;
        case 3: // 11 -> all in black
            speeds.left_wheel_speed = 20;
            speeds.right_wheel_speed = 20;
            break;
        default:
            printf("Unknown tracking status\n");
            speeds.left_wheel_speed = 0;
            speeds.right_wheel_speed = 0;
            break;
    }
    printf("AUTO: Left wheel speed: %d, Right wheel speed: %d\n", speeds.left_wheel_speed, speeds.right_wheel_speed);
    cutebot_set_motors_speed(self, speeds.left_wheel_speed, speeds.right_wheel_speed);
    return speeds;
}