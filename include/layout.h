#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ssd1306-config.h"

typedef struct {
    uint8_t page;
    uint8_t column;
} Point;

typedef enum {
    ADDRESSING_MODE_HORIZONTAL = 0,
    ADDRESSING_MODE_VERTICAL = 1,
    ADDRESSING_MODE_PAGE = 2,
} AddressingMode;

typedef enum {
    FONT_8x9 = 0,
    FONT_16x8 = 1,
} FontType;

typedef void * LayoutPtr;
typedef bool (*write_f)(const uint8_t *data, size_t len);

LayoutPtr layout_create(write_f write);
void layout_free(LayoutPtr layout);

int8_t layout_add_tile(LayoutPtr layout, Point *start, Point *end);
int8_t layout_edit_tile(LayoutPtr layout, uint8_t tile, Point *tile_point, uint8_t *data, uint8_t len);

int8_t layout_print(LayoutPtr layout, uint8_t tile, uint8_t *text, uint8_t len, FontType font);
int8_t layout_flush(LayoutPtr layout);
int8_t layout_clear(LayoutPtr layout_, uint8_t fill);
