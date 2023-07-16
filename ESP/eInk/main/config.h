#ifndef __CONFIG_H__ 
#define __CONFIG_H__

#include <stdint.h>
#include "sensitive.h"

//Panel
#define IT8951_PANEL_WIDTH  1872
#define IT8951_PANEL_HEIGHT 1404

#define TOUCH1 33
#define TOUCH2 32

#define ENABLE_5V 4
// APA102 Dotstar the RGB LED
#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12

//# SPI //VSPI
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_CLK 18
#define SPI_CS 5
#define SPI_RST 22
#define SPI_HRDY 21 // Wait for ready
// Bat
#define BAT_CHARGE 34
#define BAT_VOLTAGE 35

//Wifi
#define HOST_IP_ADDR "172.31.0.115"
#define PORT PORTNUM
#define EXAMPLE_ESP_WIFI_SSID      WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      WIFI_PASS
#define EXAMPLE_ESP_MAXIMUM_RETRY  6

#define PACKET_SIZE 1400
#define QUEUE_LENGTH 70

#define MY_WORD_SWAP(x) (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8)) //need this macro because esp is little endian


struct message{
    uint16_t size;
    uint8_t *packet;
};


#endif
