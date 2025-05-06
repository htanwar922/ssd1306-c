#pragma once

#include <stdbool.h>

bool i2c_init();
bool i2c_send(uint8_t bus, uint8_t payload_type, const uint8_t *data, size_t len);
bool i2c_close();
