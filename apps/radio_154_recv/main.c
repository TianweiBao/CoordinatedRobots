// Radio 15.4 receive app
//
// Receives wireless packets via the 802.15.4 radio

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

// Pin configurations
#include "microbit_v2.h"

#include "nrf_802154.h"

#define PSDU_MAX_SIZE (127) // Max length of a packet
#define FCS_LENGTH (2) // Length of the Frame Control Sequence


// callback fn for successful rx
void nrf_802154_received_raw(uint8_t* p_data, int8_t power, uint8_t lqi) {
nrf_gpio_pin_toggle(20);
  printf("Packet: [ ");
  // for (int i=0; i<p_data[0]-2; i++) {
  //   printf("%02X ", p_data[i]);
  // }
  // cur_speed = p_data[24];
  int8_t x_speed = (int8_t)p_data[24];   // x_speed from byte 6
  int8_t y_speed = (int8_t)p_data[25];   // y_speed from byte 7
  printf("x_speed: %d, y_speed: %d", x_speed, y_speed);
  printf("]\n");
  nrf_802154_buffer_free_raw(p_data);
}


int main(void) {
  printf("Board started!\n");

  // Initialize.
  nrf_gpio_cfg_output(LED_MIC);

  // Configure 154 radio
  printf("About to init\n");
  nrf_802154_init();
  printf("Done with init\n");
  nrf_802154_channel_set(11);
  //If true: Auto-ACK is enabled. The device automatically sends an acknowledgment frame when a valid frame is received.
  //If false: Auto-ACK is disabled. The device does not send an acknowledgment frame when a valid frame is received.
  nrf_802154_auto_ack_set(false);
  //If true: Promiscuous mode is enabled, and the device receives all packets in its range.
  //If false: Promiscuous mode is disabled, and the device filters packets based on the PAN ID and address.
  nrf_802154_promiscuous_set(false);
  uint8_t src_pan_id[] = {0xcd, 0xab}; 
  nrf_802154_pan_id_set(src_pan_id);
  printf("Radio configured!\n");

  // Addresses (source and destination)
  uint8_t extended_addr[] = {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};
  nrf_802154_extended_address_set(extended_addr);


  if (nrf_802154_receive()) {
    printf("Entered receive mode\n");
  } else {
    printf("Could not enter receive mode\n");
  }

  while(true) {
  }
}

