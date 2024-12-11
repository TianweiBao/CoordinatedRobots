#ifndef MICROBOT_UTILS_H
#define MICROBOT_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "nrf_twi_mngr.h"

#define MICROBOT_I2C_ADDR 0x59
#define CMD_ENABLE 0x70
#define CMD_SPEED_LEFT 0x21
#define CMD_SPEED_RIGHT 0x20
#define FORWARD_FLAG 0x80
#define ULTRASONIC_IC2_ADDR 0x00
#define ULTRASONIC_REG_ADDR 0x01

typedef struct {
    uint8_t i2c_addr;
    const nrf_twi_mngr_t* i2c_instance;
    uint8_t cmd_speed;
    bool invert;
} MicrobotMotor;

typedef struct {
    uint8_t i2c_addr;
    const nrf_twi_mngr_t* i2c_instance;
} Microbot;

// void microbot_system_init(const nrf_twi_mngr_t* i2c);
void microbot_init(Microbot* self, const nrf_twi_mngr_t* i2c, uint8_t i2c_addr);
void microbot_enable(Microbot* self);
void microbot_disable(Microbot* self);
MicrobotMotor microbot_left_motor(Microbot* self, bool invert);
MicrobotMotor microbot_right_motor(Microbot* self, bool invert);
void microbot_motor_drive(MicrobotMotor* motor, int speed);
uint16_t microbot_get_ultrasonic_distance(Microbot* self, uint8_t i2c_addr, uint8_t reg_addr);
#endif // MICROBOT_UTILS_H