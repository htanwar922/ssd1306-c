#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"

#include "config.h"

#if USE_UDP
#include "udp.h"
#endif

__weak bool i2c_init() {
    bool ret = false;
#if USE_UDP
    ret = udp_init();
#else
    // Initialize I2C communication
    // This is a placeholder for actual I2C initialization code
    ret = true;
#endif
    return ret;
}

__weak bool i2c_send(uint8_t bus, uint8_t payload_type, const uint8_t *data, size_t len) {
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
#if USE_UDP
    uint8_t buffer[1024];
    buffer[0] = bus;
    buffer[1] = payload_type;
    for (size_t i = 0; i < len; i++) {
        buffer[i + 2] = data[i];
    }
    return udp_send(buffer, len + 2);
#else
    printf("[I2C] Sending %s: ", payload_type == 0x00 ? "command" : "data");
    printf("%02X ", bus);
    printf("%02X ", payload_type);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    return true;
#endif
}

__weak bool i2c_close() {
    bool ret = false;
#if USE_UDP
    ret= udp_close();
#else
    // Close I2C communication
    // This is a placeholder for actual I2C close code
    ret = true;
#endif
    return ret;
}