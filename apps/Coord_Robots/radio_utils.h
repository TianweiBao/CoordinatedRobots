#ifndef MICROBIT_H
#define MICROBIT_H

#include <stdint.h>
#include <stdbool.h>

#define PSDU_MAX_SIZE (127)
#define FCS_LENGTH (2)

typedef struct {
    int8_t x;
    int8_t y;
} SpeedCommand;

typedef struct {
    uint8_t src_pan_id[2];
    uint8_t dst_pan_id[2];
    uint8_t src_extended_addr[8];
    uint8_t dst_extended_addr[8];
    uint8_t channel;
} MicrobitConfig;

typedef struct MicrobitRadio {
    MicrobitConfig config;
    void (*send)(struct MicrobitRadio* self, SpeedCommand* command);
    void (*receive)(struct MicrobitRadio* self, SpeedCommand* command);
} MicrobitRadio;

void radio_init(MicrobitRadio* self, uint8_t* src_pan_id, uint8_t* dst_pan_id, uint8_t* src_extended_addr, uint8_t* dst_extended_addr, uint8_t channel);
void radio_send(MicrobitRadio* self, SpeedCommand* command, uint8_t* button_states, bool auto_mode, uint8_t* ultrosonic_distance);
void nrf_802154_received_raw(uint8_t* p_data, int8_t power, uint8_t lqi);
SpeedCommand radio_get_received_command();
uint8_t* radio_get_received_button_states();
bool radio_get_auto_mode();
uint8_t radio_get_received_ultrosonic_distance();
#endif // MICROBIT_H