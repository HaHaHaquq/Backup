#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_

#include <stdint.h>

typedef typedef struct {
    uint8_t start_byte; // Start byte
    uint8_t length;     // Length of the data
    uint8_t command;    // Command byte
    uint8_t data[255];  // Data bytes
    uint8_t checksum;    // Checksum byte
} UART_Packet;

#endif


