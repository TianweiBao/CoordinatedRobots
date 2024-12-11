#ifndef CUTEBOT_UTILS_H
#define CUTEBOT_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "nrf_twi_mngr.h"

#define CUTEBOT_ADDR 0x10
#define LEFT_LIGHT_ADDR 0x04
#define RIGHT_LIGHT_ADDR 0x08
#define LEFT_WHEEL_ADDR 0x01
#define RIGHT_WHEEL_ADDR 0x02
#define FORWARD_FLAG 0x02
#define BACKWARD_FLAG 0x01

typedef struct {
    uint8_t i2c_addr;
    const nrf_twi_mngr_t* i2c_instance;
    uint8_t pin_echo;
    uint8_t pin_trigger;
    uint8_t pin_Ltrack;
    uint8_t pin_Rtrack;
} Cutebot;

typedef struct {
    int left_wheel_speed;
    int right_wheel_speed;
} WheelSpeeds;

void cutebot_init(Cutebot* self, const nrf_twi_mngr_t* i2c);
void cutebot_set_motors_speed(Cutebot* self, int left_wheel_speed, int right_wheel_speed);
void cutebot_set_head_light(Cutebot* self, const char* head_light, uint8_t r, uint8_t g, uint8_t b);
float cutebot_get_distance(Cutebot* self, const char* unit);
uint8_t cutebot_get_tracking(Cutebot* self);
WheelSpeeds cutebot_follow_black_road(Cutebot* self);
#endif // CUTEBOT_UTILS_H