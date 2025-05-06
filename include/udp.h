#pragma once

#include <stdbool.h>

bool udp_init();
bool udp_send(const uint8_t *data, size_t len);
bool udp_close();
