#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "layout.h"

#ifdef USE_UDP
#include "udp.h"
#else
#include "i2c.h"
#endif

#include "ssd1306.h"

bool ssd1306_init() {
    // Initialize the display
    ssd1306_set_display(SSD1306_OPTION_DISPLAY_OFF);
    ssd1306_set_display_clock_div_ratio(0x80);
    ssd1306_set_multiplex(0x1F);                                                    // 0x3F for 128x64
    ssd1306_set_display_offset(0x00);                                               // no offset
    ssd1306_set_start_line(0x00);                                                   // line #0
    ssd1306_charge_pump(0x1);                                                       // Enable charge pump
    ssd1306_set_segment_remap(SSD1306_OPTION_SEGMENT_REMAP_SEG0_TO_0);              // 0 - LTR, 1 - RTL
    ssd1306_set_com_output_scan_dir(SSD1306_OPTION_COM_SCAN_DIR_NORMAL);            // 0xC8 - top to bottom, 0xC0 - bottom to top
    ssd1306_set_com_pins(0x00);                                                     // 0x12 - alternative, 0x02 - sequential ??
    ssd1306_set_contrast(0x8F);
    ssd1306_set_precharge_period(0xF1);
    ssd1306_set_vcom_deselect_level(0x4);
    ssd1306_set_display(SSD1306_OPTION_DISPLAY_ALLON_RESUME);
    ssd1306_set_display(SSD1306_OPTION_DISPLAY_NORMAL);
    ssd1306_set_display(SSD1306_OPTION_DISPLAY_ON);
    return true;
}

int main() {
    // Initialize the display
    ssd1306_init();

#ifdef USE_UDP
    udp_init();
    LayoutPtr layout = layout_create(udp_send);
#else
    i2c_init();
    LayoutPtr layout = layout_create(ssd1306_send_data);
#endif

    int8_t tile;

    tile = layout_add_tile(layout, &(Point){0, 0}, &(Point){0, 32 * 3 - 1});
    tile = layout_add_tile(layout, &(Point){1, 0}, &(Point){1, 32 * 3 - 1});
    tile = layout_add_tile(layout, &(Point){0, 32 * 3 + 8}, &(Point){0, N_COLUMNS - 1});
    tile = layout_add_tile(layout, &(Point){1, 32 * 3 + 8}, &(Point){1, N_COLUMNS - 1});
    tile = layout_add_tile(layout, &(Point){2, 0}, &(Point){3, N_COLUMNS - 1});
    tile = layout_add_tile(layout, &(Point){0, 32 * 3}, &(Point){0, 32 * 3 + 7});
    tile = layout_add_tile(layout, &(Point){1, 32 * 3}, &(Point){1, 32 * 3 + 7});

    if (tile != 6) {
        printf("Failed to add one of the tiles\n");
    } else {
        printf("6 tiles added successfully\n");
    }

    // // Try adding an overlapping tile
    // int8_t tile = layout_add_tile(layout, &(Point){0, 0}, &(Point){0x3, 0x3F});
    // if (tile < 0) {
    //     printf("Failed to add overlapping tile\n");
    // } else {
    //     printf("Overlapping tile added: %d\n", tile);
    // }

    for (uint8_t tile = 0; tile < 7; tile++) {
        char text[2];
        snprintf(text, sizeof(text), "%d", tile);
        layout_print(layout, tile, (uint8_t *)text, (uint8_t)strlen(text), tile == 4 ? FONT_16x8 : FONT_8x9);
    }

    layout_flush(layout);
    printf("Layout flushed\n");

    layout_free(layout);
    printf("Layout freed\n");

#ifdef USE_UDP
    udp_close();
#else
    i2c_close();
#endif
    return 0;
}



// #include <stdio.h> // printf
// #include <wchar.h> // wchar_t

// #include <hidapi/hidapi.h>

// #pragma comment(lib, "ws2_32.lib")
// #pragma comment(lib, "hidapi.lib")

// #define MAX_STR 255

// int main(int argc, char* argv[])
// {
// 	int res;
// 	unsigned char buf[65];
// 	wchar_t wstr[MAX_STR];
// 	hid_device *handle;
// 	int i;

//     printf("HIDAPI Example\n");

// 	// Initialize the hidapi library
// 	res = hid_init();

//     printf("hid_init: %d\n", res);
//     if (res < 0) {
//         printf("hid_init failed\n");
//         return 1;
//     }

// 	// Open the device using the VID, PID,
// 	// and optionally the Serial number.
// 	handle = hid_open(0x4d8, 0x3f, NULL);
// 	if (!handle) {
// 		printf("Unable to open device\n");
// 		hid_exit();
//  		return 1;
// 	}

// 	// Read the Manufacturer String
// 	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
// 	printf("Manufacturer String: %ls\n", wstr);

// 	// Read the Product String
// 	res = hid_get_product_string(handle, wstr, MAX_STR);
// 	printf("Product String: %ls\n", wstr);

// 	// Read the Serial Number String
// 	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
// 	printf("Serial Number String: (%d) %ls\n", wstr[0], wstr);

// 	// Read Indexed String 1
// 	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
// 	printf("Indexed String 1: %ls\n", wstr);

// 	// Toggle LED (cmd 0x80). The first byte is the report number (0x0).
// 	buf[0] = 0x0;
// 	buf[1] = 0x80;
// 	res = hid_write(handle, buf, 65);

// 	// Request state (cmd 0x81). The first byte is the report number (0x0).
// 	buf[0] = 0x0;
// 	buf[1] = 0x81;
// 	res = hid_write(handle, buf, 65);

// 	// Read requested state
// 	res = hid_read(handle, buf, 65);

// 	// Print out the returned buffer.
// 	for (i = 0; i < 4; i++)
// 		printf("buf[%d]: %d\n", i, buf[i]);

// 	// Close the device
// 	hid_close(handle);

// 	// Finalize the hidapi library
// 	res = hid_exit();

// 	return 0;
// }