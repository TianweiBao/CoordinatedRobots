#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include "cutebot_utils.h"

//define the id of the devices
uint32_t joystick_id = 3693694357;//CE346-21
uint32_t microbot1_id = 3458891944;//CE346-09
uint32_t microbot2_id = 42216631;//CE346-28
uint32_t cutebot_id = 3540152952;//CE346-72
char *role = NULL;
//define PAN IDs and Addresses for the devices
uint8_t PAN_ID[] = {0xcd, 0xab};//keep the same for all devices
uint8_t channel = 11;//keep the same for all devices
uint8_t joystick_src_addr[] = {0xdc, 0xa9, 0x35, 0x7b, 0x73, 0x36, 0xce, 0xf4};
uint8_t joystick_dst_addr[] = {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};
uint8_t cutebot_src_addr[] =  {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};
uint8_t cutebot_dst_addr[] =   {0x60, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef};
uint8_t microbot1_src_addr[] = {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};
uint8_t microbot1_dst_addr[] = {0x60, 0xde, 0xfa, 0xce, 0xba, 0xbe, 0xca, 0xfe};
uint8_t microbot2_src_addr[] = {0x60, 0xde, 0xfa, 0xce, 0xba, 0xbe, 0xca, 0xfe};
uint8_t microbot2_dst_addr[] = {0xdc, 0xa9, 0x35, 0x7b, 0x73, 0x36, 0xce, 0xf4};
//variables for I2C
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

//helper function to get the unique device ID
uint32_t get_device_id(void) {
    uint32_t device_id[2];
    device_id[0] = NRF_FICR->DEVICEID[0];
    device_id[1] = NRF_FICR->DEVICEID[1];
    uint32_t unique_id = (device_id[0] << 16) | (device_id[1] & 0xFFFF);
    printf("Device ID: %lu\n", (unsigned long)unique_id);
    return unique_id;
}

int main(void) {
    printf("Board started!\n");
    // Initialize the ADC
    adc_init();
    // Get the current device ID
    uint32_t cur_device_id = get_device_id();
    if (cur_device_id == joystick_id) {
        role = "Joystick";
    } else if (cur_device_id == microbot1_id) {
        role = "Microbot1";
    } else if (cur_device_id == microbot2_id) {
        role = "Microbot2";
    } else if (cur_device_id == cutebot_id) {
        role = "Cutebot";
    } else {
        printf("Unknown device ID: %lu\n", (unsigned long)cur_device_id);
        exit(1);
    }
    printf("Current role is: %s\n", role);
    // Initialize the I2C peripheral and driver
    nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    i2c_config.scl = EDGE_P19;
    i2c_config.sda = EDGE_P20;
    i2c_config.frequency = NRF_TWI_FREQ_100K;
    i2c_config.interrupt_priority = 0;
    nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
    //if the role is a joystick
    if (strcmp(role, "Joystick") == 0) {
        joysticks_init();
        MicrobitRadio joystick_radio;
        radio_init(&joystick_radio, PAN_ID, PAN_ID, joystick_src_addr, joystick_dst_addr, channel);
        while (true) {
            // Read the joystick X and Y values
            float x_cmd = read_channel(ADC_JOYSTICK_X_CHANNEL);
            float y_cmd = read_channel(ADC_JOYSTICK_Y_CHANNEL);
            printf("X command: %f, Y command: %f\n", x_cmd, y_cmd);
            SpeedCommand command = {x_cmd, y_cmd};
            uint8_t button_states[4];
            check_all_buttons(button_states);
            printf("Button states: %d, %d, %d, %d\n", button_states[0], button_states[1], button_states[2], button_states[3]);
            uint8_t ultrosonic_distance = radio_get_received_ultrosonic_distance();
            printf("Ultrasonic distance: %dcm\n", ultrosonic_distance);
            //if the distance is less than 15cm, turn on the buzzer
            if (ultrosonic_distance < 15) {
                buzzer_on();
                nrf_delay_ms(50);
                buzzer_off();
            } 
            radio_send(&joystick_radio, &command, button_states, false, ultrosonic_distance);
            
            // nrf_delay_ms(1000/4);
        }
    //if the role is a microbot1
    } else if (strcmp(role, "Cutebot") == 0) {
        MicrobitRadio cutebot_radio;
        radio_init(&cutebot_radio, PAN_ID, PAN_ID, cutebot_src_addr, cutebot_dst_addr, channel);
        if (nrf_802154_receive()) {
            printf("Entered receive mode\n");
        } else {
            printf("Failed to enter receive mode\n");
            exit(1);
        }
        Cutebot cutebot;
        cutebot_init(&cutebot, &twi_mngr_instance);
        bool auto_mode = false;
        while (true) {
            SpeedCommand received_command = radio_get_received_command();
            printf("Received packet: x_cmd: %d, y_cmd: %d\n", received_command.x, received_command.y);
            // int8_t left_wheel_speed = received_command.y - received_command.x;
            // int8_t right_wheel_speed = received_command.y + received_command.x;
            int8_t left_wheel_speed = 0;
            int8_t right_wheel_speed = 0;
            uint8_t* button_states = radio_get_received_button_states();
            // printf("Button states: %d, %d, %d, %d\n", button_states[0], button_states[1], button_states[2], button_states[3]);
            if (button_states[0] == 1) {//if button C is pressed
                cutebot_set_head_light(&cutebot, "left", 255, 0, 0);
            } else {
                cutebot_set_head_light(&cutebot, "left", 0, 0, 0);
            }
            if (button_states[3] == 1) {//if button F is pressed
                cutebot_set_head_light(&cutebot, "right", 0, 255, 0);
            } else {
                cutebot_set_head_light(&cutebot, "right", 0, 0, 0);
            }
            

            if (button_states[1] == 1) {//if button D is pressed, enable auto mode
                printf("Auto mode enabled\n");
                auto_mode = true;
            } 
            if (button_states[2] == 1) {//if button E is pressed, disable auto mode
                printf("Auto mode disabled\n");
                auto_mode = false;
            }
            if (auto_mode) {
                WheelSpeeds speeds = cutebot_follow_black_road(&cutebot);
                left_wheel_speed = speeds.left_wheel_speed;
                right_wheel_speed = speeds.right_wheel_speed;
                // printf("AUTO: Left wheel speed: %d, Right wheel speed: %d\n", left_wheel_speed, right_wheel_speed);
                SpeedCommand auto_command = {left_wheel_speed + 10, right_wheel_speed + 10};
                // radio_send(&cutebot_radio, &auto_command, button_states, auto_mode);
            } else {
                left_wheel_speed = (int)((received_command.y - received_command.x)/3);
                right_wheel_speed = (int)((received_command.y + received_command.x)/3);
                printf("MANUAL: Left wheel speed: %d, Right wheel speed: %d\n", left_wheel_speed, right_wheel_speed);
                cutebot_set_motors_speed(&cutebot, left_wheel_speed, right_wheel_speed);
                printf("Sent packet: x_cmd: %d, y_cmd: %d\n", received_command.x, received_command.y);
                // radio_send(&cutebot_radio, &received_command, button_states, auto_mode);
            }

            // float distance = cutebot_get_distance(&cutebot, "cm");
            // printf("Distance: %f cm\n", distance);
            // uint8_t tracking_status = cutebot_get_tracking(&cutebot);
            // printf("Tracking status: %d\n", tracking_status);
            // nrf_delay_ms(1000/4);
        }
    } else if (strcmp(role, "Microbot1") == 0) {
        //initialize the radio
        MicrobitRadio microbot1_radio;
        radio_init(&microbot1_radio, PAN_ID, PAN_ID, microbot1_src_addr, microbot1_dst_addr, channel);
        if (nrf_802154_receive()) {
            printf("Entered receive mode\n");
        } else {
            printf("Failed to enter receive mode\n");
            exit(1);
        }
        //initiaze the motors and enable the microbot
        Microbot microbot1;
        microbot_init(&microbot1, &twi_mngr_instance, MICROBOT_I2C_ADDR);
        microbot_enable(&microbot1);
        //false true
        MicrobotMotor left_motor = microbot_left_motor(&microbot1, true);
        MicrobotMotor right_motor = microbot_right_motor(&microbot1, false);
        //receive the command and drive the motors
        while (true) {
            SpeedCommand received_command = radio_get_received_command();
            printf("Received packet: x_cmd: %d, y_cmd: %d\n", received_command.x, received_command.y);
            uint8_t distance = microbot_get_ultrasonic_distance(&microbot1, ULTRASONIC_IC2_ADDR, ULTRASONIC_REG_ADDR) / 10;
            printf("Distance: %d cm\n", distance);
            bool avoidance_mode = false;
            if (distance < 15) {
                avoidance_mode = true;
            } else {
                avoidance_mode = false;
            }
            int8_t left_wheel_speed;
            int8_t right_wheel_speed;
            if (avoidance_mode) {
                printf("Auto mode enabled\n");
                left_wheel_speed = -100;
                right_wheel_speed = -100;
            } else {
                printf("Auto mode disabled\n");
                left_wheel_speed = received_command.y + received_command.x;
                right_wheel_speed = received_command.y - received_command.x;
            }
            printf("Left wheel speed: %d, Right wheel speed: %d\n", left_wheel_speed, right_wheel_speed);
            uint8_t* button_states = radio_get_received_button_states();
            printf("Button states: %d, %d, %d, %d\n", button_states[0], button_states[1], button_states[2], button_states[3]);
            microbot_motor_drive(&left_motor, left_wheel_speed);
            microbot_motor_drive(&right_motor, right_wheel_speed);
            //relay the recived command and button states to microbot2
            radio_send(&microbot1_radio, &received_command, button_states, avoidance_mode, distance);
            // nrf_delay_ms(1000/4);
        }
    //if the role is a microbot2
    } else if (strcmp(role, "Microbot2") == 0) {
        MicrobitRadio microbot2_radio;
        radio_init(&microbot2_radio, PAN_ID, PAN_ID, microbot2_src_addr, microbot2_dst_addr, channel);
        if (nrf_802154_receive()) {
            printf("Entered receive mode\n");
        } else {
            printf("Failed to enter receive mode\n");
            exit(1);
        }
        Microbot microbot2;
        microbot_init(&microbot2, &twi_mngr_instance, MICROBOT_I2C_ADDR);
        microbot_enable(&microbot2);
        MicrobotMotor left_motor = microbot_left_motor(&microbot2, true);
        MicrobotMotor right_motor = microbot_right_motor(&microbot2, false);
        while (true) {
            SpeedCommand received_command = radio_get_received_command();
            bool avoidance_mode = radio_get_auto_mode();
            printf("Received packet: x_cmd: %d, y_cmd: %d\n", received_command.x, received_command.y);
            int8_t left_wheel_speed;
            int8_t right_wheel_speed;
            if (avoidance_mode) {
                left_wheel_speed = -100;
                right_wheel_speed = -100;
            } else {
                left_wheel_speed = received_command.y + received_command.x;
                right_wheel_speed = received_command.y - received_command.x;
            }
            // printf("Left wheel speed: %d, Right wheel speed: %d\n", left_wheel_speed, right_wheel_speed);
            uint8_t* button_states = radio_get_received_button_states();
            printf("Button states: %d, %d, %d, %d\n", button_states[0], button_states[1], button_states[2], button_states[3]);
            microbot_motor_drive(&left_motor, 0.9 * left_wheel_speed);
            microbot_motor_drive(&right_motor, 0.9 * right_wheel_speed);
            uint8_t distance = radio_get_received_ultrosonic_distance();
            printf("Distance: %d cm\n", distance);
            radio_send(&microbot2_radio, &received_command, button_states, avoidance_mode, distance);
            // nrf_delay_ms(1000/4);
        }
    } else {
        printf("Unknown role\n");
    }


}