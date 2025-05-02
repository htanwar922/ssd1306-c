#include <stdint.h>
#include <stdbool.h>

#include "layout.h"

#define N_PAGES 0x4
#define N_COLUMNS 0x40
#define N_ROWS (N_PAGES * 8)

#define MAX_TILES 8

typedef struct {
    Point start;
    Point end;      // inclusive
    bool dirty;
} Tile;

typedef struct {
    uint8_t num_tiles;
    Tile tiles[MAX_TILES];
    uint8_t data[N_PAGES][N_COLUMNS];
    set_position_f set_position;
    print_f print;
} Layout;

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

Layout *layout_create(set_position_f set_position, print_f print) {
    Layout *layout = malloc(sizeof(Layout));
    if (layout == NULL) {
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
    layout->set_position = set_position;
    layout->print = print;
    return layout;
}

void layout_free(Layout *layout) {
    if (layout != NULL) {
        free(layout);
    }
}

int8_t layout_add_tile(Layout *layout, Point *start, Point *end) {
    if (layout->num_tiles >= MAX_TILES) {
        return -1; // Layout is full
    }
    layout->tiles[layout->num_tiles].start = *start;
    layout->tiles[layout->num_tiles].end = *end;
    layout->num_tiles++;
    return 0;
}