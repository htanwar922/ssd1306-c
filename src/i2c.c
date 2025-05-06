#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"

bool i2c_init() {
    // Initialize I2C communication
    // This is a placeholder for actual I2C initialization code
    return true;
}

bool i2c_send(uint8_t bus, uint8_t payload_type, const uint8_t *data, size_t len) {
    // Send data over I2C
    // This is a placeholder for actual I2C send code
    if (len > 32) {
        for (size_t i = 0; i < len; i += 32) {
            size_t chunk_size = (len - i > 32) ? 32 : (len - i);
            bool result = i2c_send(bus, payload_type, data + i, chunk_size);
            if (!result) {
                return false;
            }
        }
        return true;
    }
    printf("[I2C] Sending %s: ", payload_type == 0x00 ? "command" : "data");
    printf("%02X ", bus);
    printf("%02X ", payload_type);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    return true;
}

bool i2c_close() {
    // Close I2C communication
    // This is a placeholder for actual I2C close code
    return true;
}