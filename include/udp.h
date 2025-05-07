#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

#if USE_UDP

bool udp_init();
bool udp_send(const uint8_t *data, size_t len);
bool udp_close();

#endif