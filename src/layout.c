#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include <stdio.h>

#include "layout.h"
#include "ssd1306.h"

#define N_ROWS (N_PAGES * 8)

#define MAX_TILES 8

extern int8_t font_8x9_get_columns(uint8_t c, uint8_t *buf);
extern int8_t font_16x8_get_columns(uint8_t c, uint16_t *buf);

typedef struct {
    Point start;
    Point end;      // inclusive
    bool dirty;
} Tile;

typedef struct {
    uint8_t num_tiles;
    Tile tiles[MAX_TILES];
    uint8_t data[N_PAGES][N_COLUMNS];
    write_f write;
} Layout;

typedef enum {
    LAYOUT_OK = 0,
    LAYOUT_ERR_INVALID = -1,
    LAYOUT_ERR_FULL = -2,
    LAYOUT_ERR_OVERLAP = -3,
    LAYOUT_ERR_INVALID_TILE = -4,
    LAYOUT_ERR_INVALID_POINT = -5,
    LAYOUT_ERR_INVALID_DATA = -6,
    LAYOUT_ERR_FLUSH = -7,
} LayoutError;

static bool lt_set_position(Layout *layout, AddressingMode mode, Point *start, Point *end);
static bool lt_flush(Layout *layout, uint8_t *data, uint8_t len);

void tile_init(Tile *tile, Point start, Point end) {
    tile->start = start;
    tile->end = end;
    tile->dirty = false;
}

uint8_t tile_get_width(Tile *tile) {
    return tile->end.column - tile->start.column + 1;
}

uint8_t tile_get_height(Tile *tile) {
    return tile->end.page - tile->start.page + 1;
}

uint8_t tile_isdirty(Tile *tile) {
    return tile->dirty;
}

void tile_setdirty(Tile *tile, bool dirty) {
    tile->dirty = dirty;
}

bool tile_overlap(Tile *tile1, Tile *tile2) {
    return (tile1->start.page == tile2->start.page) &&
           (tile1->end.page == tile2->end.page) &&
           (tile1->start.column <= tile2->end.column) &&
           (tile1->end.column >= tile2->start.column);
}

LayoutPtr layout_create(write_f write) {
    Layout *layout = malloc(sizeof(Layout));
    if (layout == NULL) {
        errno = ENOMEM;
        perror("Failed to allocate memory for layout");
        return NULL; // Memory allocation failed
    }
    layout->num_tiles = 0;
    for (int i = 0; i < MAX_TILES; i++) {
        tile_init(&layout->tiles[i], (Point){0, 0}, (Point){0, 0});
    }
    for (int i = 0; i < N_PAGES; i++) {
        for (int j = 0; j < N_COLUMNS; j++) {
            layout->data[i][j] = 0;
        }
    }
    layout->write = write;
    return layout;
}

void layout_free(LayoutPtr layout) {
    if (layout != NULL) {
        free((Layout *)layout);
    }
}

int8_t layout_add_tile(LayoutPtr layout_, Point *start, Point *end) {
    Layout *layout = (Layout *)layout_;
    if (layout == NULL) {
        errno = EINVAL;
        perror("Layout is NULL");
        return LAYOUT_ERR_INVALID;
    }
    if (layout->num_tiles >= MAX_TILES) {
        errno = ENOMEM;
        perror("Layout is full");
        return LAYOUT_ERR_FULL;
    }
    if (start->page > end->page || start->column > end->column
        || end->page >= N_PAGES || end->column >= N_COLUMNS) {
        errno = EINVAL;
        perror("Invalid tile coordinates");
        return LAYOUT_ERR_INVALID_TILE;
    }
    for (int i = 0; i < layout->num_tiles; i++) {
        if (tile_overlap(&layout->tiles[i], &(Tile){*start, *end, false})) {
            errno = EEXIST;
            perror("Tile overlaps with existing tile");
            return LAYOUT_ERR_OVERLAP;
        }
    }
    layout->tiles[layout->num_tiles].start = *start;
    layout->tiles[layout->num_tiles].end = *end;
    layout->num_tiles++;
    return layout->num_tiles - 1; // Return the index of the new tile
}

int8_t layout_edit_tile(LayoutPtr layout_, uint8_t tile, Point *tile_point, uint8_t *data, uint8_t len) {
    Layout *layout = (Layout *)layout_;
    if (layout == NULL) {
        errno = EINVAL;
        perror("Layout is NULL");
        return LAYOUT_ERR_INVALID;
    }
    if (tile >= layout->num_tiles) {
        errno = EINVAL;
        perror("Invalid tile index");
        return -2; // Invalid tile index
    }
    Tile *t = &layout->tiles[tile];
    Point point = *tile_point;
    point.page += t->start.page;
    point.column += t->start.column;
    if (point.page < t->start.page || point.page > t->end.page ||
            point.column < t->start.column || point.column > t->end.column) {
        errno = EINVAL;
        perror("Invalid tile point");
        return -3; // Invalid tile point
    }
    if (point.column + len > t->end.column + 1) {
        errno = EMSGSIZE;
        perror("Data length exceeds tile bounds");
        return -4; // Data length exceeds tile bounds
    }
    for (int i = 0; i < len; i++) {
        layout->data[point.page][point.column + i] = data[i];
    }
    tile_setdirty(t, true);
    return 0;
}

int8_t layout_print(LayoutPtr layout_, uint8_t tile, uint8_t *text, uint8_t len, FontType font) {
    Layout *layout = (Layout *)layout_;
    if (layout == NULL) {
        errno = EINVAL;
        perror("Layout is NULL");
        return -1; // Invalid layout
    }
    if (tile >= layout->num_tiles) {
        errno = EINVAL;
        perror("Invalid tile index");
        return -2; // Invalid tile index
    }
    Tile *t = &layout->tiles[tile];
    uint8_t width = tile_get_width(t);
    uint8_t height = tile_get_height(t);

    if (font == FONT_8x9) {
        uint8_t page = 0;
        uint8_t column = 0;
        uint8_t columns[10] = {0};
        for (int i = 0; i < len; i++) {
            int8_t len = font_8x9_get_columns(text[i], columns);
            if (len < 0) {
                errno = EINVAL;
                perror("Invalid character");
                return -3; // Invalid character
            }
            for (int j = 0; j < len; j++) {
                if (page >= height) {
                    errno = ENOSPC;
                    perror("No space left in tile");
                    return -4; // No space left in tile
                }
                layout->data[t->start.page + page][t->start.column + column] = columns[j];
                column++;
                if (column >= width) {
                    column = 0;
                    page++;
                }
            }
        }
        while (page < height) {
            for (int j = column; j < width; j++) {
                layout->data[t->start.page + page][t->start.column + j] = 0;
            }
            page++;
        }
    } else if (font == FONT_16x8) {
        uint8_t page = 0;
        uint8_t column = 0;
        uint16_t columns[8] = {0};
        for (int i = 0; i < len; i++) {
            int8_t len = font_16x8_get_columns(text[i], columns);
            if (len < 0) {
                errno = EINVAL;
                perror("Invalid character");
                return -3; // Invalid character
            }
            for (int j = 0; j < len; j++) {
                if (page >= height) {
                    errno = ENOSPC;
                    perror("No space left in tile");
                    return -4; // No space left in tile
                }
                layout->data[t->start.page + page][t->start.column + column] = columns[j] & 0xFF;
                layout->data[t->start.page + page + 1][t->start.column + column] = (columns[j] >> 8) & 0xFF;
                column++;
                if (column >= width) {
                    column = 0;
                    page += 2;
                }
            }
        }
        while (page < height) {
            for (int j = column; j < width; j++) {
                layout->data[t->start.page + page][t->start.column + j] = 0;
                layout->data[t->start.page + page + 1][t->start.column + j] = 0;
            }
            page += 2;
        }
    } else {
        errno = EINVAL;
        perror("Invalid font type");
        return -5; // Invalid font type
    }

    tile_setdirty(t, true);
    return 0;
}

int8_t layout_flush(LayoutPtr layout_) {
    Layout *layout = (Layout *)layout_;
    if (layout == NULL) {
        errno = EINVAL;
        perror("Layout is NULL");
        return -1; // Invalid layout
    }
    for (int i = 0; i < layout->num_tiles; i++) {
        Tile *tile = &layout->tiles[i];
        if (tile_isdirty(tile)) {
            Point start = tile->start;
            Point end = tile->end;
            uint8_t width = tile_get_width(tile);
            uint8_t height = tile_get_height(tile);
            if (!lt_set_position(layout, ADDRESSING_MODE_HORIZONTAL, &start, &end)) {
                errno = EIO;
                perror("Failed to set position");
                return -2; // Failed to set position
            }
            for (int j = 0; j < height; j++) {
                if (!lt_flush(layout, layout->data[start.page + j] + start.column, width)) {
                    errno = EIO;
                    perror("Failed to print data");
                    return -3; // Failed to print data
                }
            }
            tile_setdirty(tile, false);
        }
    }
    return 0;
}

static bool lt_set_position(Layout *layout, AddressingMode mode, Point *start, Point *end) {
    switch (mode) {
        case ADDRESSING_MODE_HORIZONTAL:
            if (!ssd1306_set_memory_addressing_mode(SSD1306_OPTION_ADDRESSING_MODE_HORIZONTAL)) {
                perror("Failed to set addressing mode");
                return false;
            }
            if (!ssd1306_hava_mode_set_page_addr(start->page, end->page)) {
                perror("Failed to set page address");
                return false;
            }
            if (!ssd1306_hava_mode_set_column_addr(start->column, end->column)) {
                perror("Failed to set column address");
                return false;
            }
            break;
        case ADDRESSING_MODE_VERTICAL:
            if (!ssd1306_set_memory_addressing_mode(SSD1306_OPTION_ADDRESSING_MODE_VERTICAL)) {
                perror("Failed to set addressing mode");
                return false;
            }
            if (!ssd1306_hava_mode_set_page_addr(start->page, end->page)) {
                perror("Failed to set page address");
                return false;
            }
            if (!ssd1306_hava_mode_set_column_addr(start->column, end->column)) {
                perror("Failed to set column address");
                return false;
            }
            break;
        case ADDRESSING_MODE_PAGE:
            if (!ssd1306_set_memory_addressing_mode(SSD1306_OPTION_ADDRESSING_MODE_PAGE)) {
                perror("Failed to set addressing mode");
                return false;
            }
            if (!ssd1306_pa_mode_set_page_addr(start->page)) {
                perror("Failed to set page address");
                return false;
            }
            if (!ssd1306_pa_mode_set_column_addr_low(start->column & 0x0F)) {
                perror("Failed to set column address");
                return false;
            }
            if (!ssd1306_pa_mode_set_column_addr_high(start->column >> 4)) {
                perror("Failed to set column address");
                return false;
            }
            break;
        default:
            perror("Invalid addressing mode\n");
            return false;
    }
    return true;
}

static bool lt_flush(Layout *layout, uint8_t *data, uint8_t len) {
    if (!layout->write(data, len)) {
        perror("Failed to send message");
        return false;
    }
    return true;
}
