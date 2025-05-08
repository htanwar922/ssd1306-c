#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ssd1306-config.h"

// # SSD1306_I2C_ADDRESS = 0x3C    # 011'110+SA0+RW - 0x3C or 0x3D
// OPTION_I2C_ADDRESS_WRITE = 0x0
// OPTION_I2C_ADDRESS_READ = 0x1
// SSD1306_I2C_ADDRESS = CommandWithBitmask(0x3C, 0x1, [OPTION_I2C_ADDRESS_WRITE, OPTION_I2C_ADDRESS_READ])
#define SSD1306_I2C_ADDRESS_WRITE   ((0x3C << 1) | 0x00)
#define SSD1306_I2C_ADDRESS_READ    ((0x3C << 1) | 0x01)

#define SSD1306_OPTION_DISPLAY_ALLON_RESUME         0x0
#define SSD1306_OPTION_DISPLAY_ALLON_CLEAR          0x1
#define SSD1306_OPTION_DISPLAY_NORMAL               0x2
#define SSD1306_OPTION_DISPLAY_INVERT               0x3
#define SSD1306_OPTION_DISPLAY_OFF                  0xA
#define SSD1306_OPTION_DISPLAY_ON                   0xB

#define SSD1306_OPTION_HORIZONTAL_SCROLL_RIGHT      0x0
#define SSD1306_OPTION_HORIZONTAL_SCROLL_LEFT       0x1

#define SSD1306_OPTION_ADDRESSING_MODE_HORIZONTAL   0x0
#define SSD1306_OPTION_ADDRESSING_MODE_VERTICAL     0x1
#define SSD1306_OPTION_ADDRESSING_MODE_PAGE         0x2

#define SSD1306_OPTION_SEGMENT_REMAP_SEG0_TO_0      0x0
#define SSD1306_OPTION_SEGMENT_REMAP_SEG0_TO_127    0x1

#define SSD1306_OPTION_COM_SCAN_DIR_NORMAL          0x0
#define SSD1306_OPTION_COM_SCAN_DIR_REVERSE         0x8

bool ssd1306_send_data(const uint8_t *data, size_t len);

// # Fundamental commands
bool ssd1306_set_contrast(uint8_t contrast);
bool ssd1306_set_display(uint8_t option);

// # Scrolling commands
bool ssd1306_set_scroll_horizontal(uint8_t option, uint8_t *args);
bool ssd1306_set_scroll_horizontal_vertical(uint8_t option, uint8_t *args);
bool ssd1306_deactivate_scroll();
bool ssd1306_activate_scroll();
bool ssd1306_set_vertical_scroll_area(uint8_t *args);

// # Address setting commands
bool ssd1306_set_memory_addressing_mode(uint8_t mode);
bool ssd1306_pa_mode_set_page_addr(uint8_t page);
bool ssd1306_pa_mode_set_column_addr_low(uint8_t column);
bool ssd1306_pa_mode_set_column_addr_high(uint8_t column);
bool ssd1306_hava_mode_set_page_addr(uint8_t start_page, uint8_t end_page);
bool ssd1306_hava_mode_set_column_addr(uint8_t start_column, uint8_t end_column);

// # Hardware configuration commands (panel resolution and layout related)
bool ssd1306_set_start_line(uint8_t line);
bool ssd1306_set_segment_remap(uint8_t option);
bool ssd1306_set_multiplex(uint8_t multiplex);
bool ssd1306_set_com_output_scan_dir(uint8_t option);
bool ssd1306_set_display_offset(uint8_t offset);
bool ssd1306_set_com_pins(uint8_t pins);

// # Timing and driving scheme commands
bool ssd1306_set_display_clock_div_ratio(uint8_t ratio);
bool ssd1306_set_precharge_period(uint8_t period);
bool ssd1306_set_vcom_deselect_level(uint8_t level);
bool ssd1306_nop_command();
bool ssd1306_charge_pump(uint8_t pump);
bool ssd1306_set_external_vcc();
bool ssd1306_set_switch_cap_vcc();
