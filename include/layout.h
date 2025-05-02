#pragma once

#include <stdint.h>

typedef struct {
    uint8_t page;
    uint8_t column;
} Point;

typedef enum {
    ADDRESSING_MODE_HORIZONTAL = 0,
    ADDRESSING_MODE_VERTICAL = 1,
    ADDRESSING_MODE_PAGE = 2,
} AddressingMode;

typedef bool (*set_position_f)(AddressingMode mode, Point *start, Point *end);
typedef bool (*print_f)(uint8_t *data, uint8_t len);

struct Layout;

Layout *layout_create(set_position_f set_position, print_f print);
void layout_free(Layout *layout);
int8_t layout_add_tile(Layout *layout, Point *start, Point *end);
void layout_edit_tile(Layout *layout, Point *edit);
bool layout_print(uint8_t tile, void *font, uint8_t *data, uint8_t len);
