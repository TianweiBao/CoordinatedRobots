#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
#include "microbot_utils.h"


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

void microbot_init(Microbot* self, const nrf_twi_mngr_t* i2c, uint8_t i2c_addr) {
    self->i2c_addr = i2c_addr;
    self->i2c_instance = i2c;
}

void microbot_enable(Microbot* self) {
    i2c_reg_write(self->i2c_instance, self->i2c_addr, CMD_ENABLE, 0x01);
}

void microbot_disable(Microbot* self) {
    i2c_reg_write(self->i2c_instance, self->i2c_addr, CMD_ENABLE, 0x00);
}

MicrobotMotor microbot_left_motor(Microbot* self, bool invert) {
    MicrobotMotor motor = {self->i2c_addr, self->i2c_instance, CMD_SPEED_LEFT, invert};
    return motor;
}

MicrobotMotor microbot_right_motor(Microbot* self, bool invert) {
    MicrobotMotor motor = {self->i2c_addr, self->i2c_instance, CMD_SPEED_RIGHT, invert};
    return motor;
}

void microbot_motor_drive(MicrobotMotor* motor, int speed) {
    uint8_t flags = 0;
    if (motor->invert) {
        speed = -speed;
    }
    if (speed >= 0) {
        flags |= FORWARD_FLAG;
    }
    speed = (int)(speed / 100.0 * 127);
    if (speed < -127) {
        speed = -127;
    }
    if (speed > 127) {
        speed = 127;
    }
    speed = (speed & 0x7f) | flags;
    i2c_reg_write(motor->i2c_instance, motor->i2c_addr, motor->cmd_speed, speed);
}

uint16_t microbot_get_ultrasonic_distance(Microbot* self, uint8_t i2c_addr, uint8_t reg_addr) {
    uint8_t rx_buf[2] = {0};
    nrf_twi_mngr_transfer_t const read_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 2, 0)
    };
    ret_code_t result = nrf_twi_mngr_perform(self->i2c_instance, NULL, read_transfer, 2, NULL);
    if (result != NRF_SUCCESS) {
        printf("Error reading from I2C device: %lX\n", result);
    }
    return (rx_buf[0] << 8) | rx_buf[1];
}