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
#include "radio_utils.h"

//used for access in the main.c file 
static SpeedCommand received_cmd;
static uint8_t received_button_states[4];
static bool auto_mode = false;
static uint8_t ultrosonic_distance;

void radio_init(MicrobitRadio* self, uint8_t* src_pan_id, uint8_t* dst_pan_id, uint8_t* src_extended_addr, uint8_t* dst_extended_addr, uint8_t channel) {
    memcpy(self->config.src_pan_id, src_pan_id, 2);
    memcpy(self->config.dst_pan_id, dst_pan_id, 2);
    memcpy(self->config.src_extended_addr, src_extended_addr, 8);
    memcpy(self->config.dst_extended_addr, dst_extended_addr, 8);
    self->config.channel = channel;

    nrf_802154_init();
    nrf_802154_channel_set(channel);
    nrf_802154_pan_id_set(src_pan_id);
    nrf_802154_extended_address_set(src_extended_addr);
    nrf_802154_auto_ack_set(false); //disable automatic acknowledgment
    nrf_802154_promiscuous_set(false); //disable promiscuous mode
}

void radio_send(MicrobitRadio* self, SpeedCommand* command, uint8_t* button_states, bool auto_mode, uint8_t* ultrosonic_distance) {
    uint8_t pkt[PSDU_MAX_SIZE];
    pkt[0] = 32 + FCS_LENGTH;
    pkt[1] = 0x01;
    pkt[2] = 0xcc;
    pkt[3] = 0x00;
    memcpy(&pkt[4], self->config.dst_pan_id, 2); // Destination PAN ID
    memcpy(&pkt[6], self->config.dst_extended_addr, 8); // Destination Extended Address
    memcpy(&pkt[14], self->config.src_pan_id, 2); // Source PAN ID
    memcpy(&pkt[16], self->config.src_extended_addr, 8); // Source Extended Address
    pkt[24] = (uint8_t)command->x;
    pkt[25] = (uint8_t)command->y;
    pkt[26] = button_states[0];//button c
    pkt[27] = button_states[1];//button d
    pkt[28] = button_states[2];//button e
    pkt[29] = button_states[3];//button f
    pkt[30] = auto_mode;
    pkt[31] = ultrosonic_distance;
    if (!nrf_802154_transmit_raw(pkt, true)) {
        printf("Failure to send radio packet!\n");
    } else {
        printf("Sent a radio packet!\n");
    }
}

void nrf_802154_received_raw(uint8_t* p_data, int8_t power, uint8_t lqi) {
    // This function will be called by the callback when a packet is received
    int8_t x_speed = (int8_t)p_data[24];   // x_speed from byte 6
    int8_t y_speed = (int8_t)p_data[25];   // y_speed from byte 7
    received_cmd.x = x_speed;
    received_cmd.y = y_speed;
    // printf("Received packet: x_speed: %d, y_speed: %d\n", x_speed, y_speed);
    received_button_states[0] = p_data[26];
    received_button_states[1] = p_data[27];
    received_button_states[2] = p_data[28];
    received_button_states[3] = p_data[29];
    auto_mode = p_data[30];
    ultrosonic_distance = p_data[31];
    nrf_802154_buffer_free_raw(p_data);
}

//helper function to get the real-time speed command received
SpeedCommand radio_get_received_command() {
    return received_cmd;
}

//helper function to get the real-time button states received
uint8_t* radio_get_received_button_states() {
    return received_button_states;
}

//helper function to get the real-time auto mode status
bool radio_get_auto_mode() {
    return auto_mode;
}

//helper function to get the real-time ultrasonic distance received
uint8_t radio_get_received_ultrosonic_distance() {
    return ultrosonic_distance;
}