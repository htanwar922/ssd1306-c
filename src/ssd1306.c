#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ssd1306.h"

#include "config.h"
#include "i2c.h"

#define COMMAND_TYPE_COMMAND 0
#define COMMAND_TYPE_COMMAND_WITH_BITMASK 1
#define COMMAND_TYPE_COMMAND_WITH_ARGS 2
#define COMMAND_TYPE_COMMAND_WITH_BITMASK_AND_ARGS 3
#define COMMAND_TYPE_COMMAND_WITH_CALLABLE 4

typedef struct {
    uint8_t command;
} Command;

typedef struct {
    Command super;
    uint8_t bitmask;
    uint8_t options_count;
    uint8_t options[8];
} CommandWithBitmask;

typedef struct {
    Command super;
    uint8_t argc;
    uint8_t arg_bitmasks[8];
} CommandWithArgs;

typedef struct {
    CommandWithBitmask super;
    uint8_t argc;
    uint8_t arg_bitmasks[8];
} CommandWithBitmaskAndArgs;

typedef struct {
    CommandWithBitmaskAndArgs super;
    // uint8_t (*func)(uint8_t);
} CommandWithCallable;

typedef bool (*write_f)(uint8_t *data, size_t len);

bool ssd1306_write(uint8_t *cmd, size_t len) {
    return i2c_send(SSD1306_I2C_ADDRESS_WRITE, 0x00, cmd, len);
}

bool ssd1306_send_data(uint8_t *data, size_t len) {
    return i2c_send(SSD1306_I2C_ADDRESS_WRITE, 0x40, data, len);
}

bool ssd1306_send_simple_command(uint8_t command) {
    return ssd1306_write(&command, 1);
}

bool ssd1306_send_command_with_bitmask(CommandWithBitmask *cmd, uint8_t option) {
    if (cmd->options_count) {
        // Check if the option is valid
        bool valid_option = false;
        for (uint8_t i = 0; i < cmd->options_count; i++) {
            if (cmd->options[i] == option) {
                valid_option = true;
                break;
            }
        }
        if (!valid_option) {
            return false; // Invalid option
        }
    }
    uint8_t command = cmd->super.command | (option & cmd->bitmask);
    return ssd1306_write(&command, 1);
}

bool ssd1306_send_command_with_args(CommandWithArgs *cmd, uint8_t *args) {
    if (cmd->argc > 0) {
        // Check if the number of arguments is valid
        if (args == NULL) {
            return false; // Invalid arguments
        }
    }
    uint8_t command[8] = {0};
    command[0] = cmd->super.command;
    for (uint8_t i = 0; i < cmd->argc; i++) {
        command[i + 1] = args[i] & cmd->arg_bitmasks[i];
    }
    return ssd1306_write(command, 1 + cmd->argc);
}

bool ssd1306_send_command_with_bitmask_and_args(CommandWithBitmaskAndArgs *cmd, uint8_t option, uint8_t *args) {
    if (cmd->super.options_count) {
        // Check if the option is valid
        bool valid_option = false;
        for (uint8_t i = 0; i < cmd->super.options_count; i++) {
            if (cmd->super.options[i] == option) {
                valid_option = true;
                break;
            }
        }
        if (!valid_option) {
            return false; // Invalid option
        }
    }
    if (cmd->argc > 0) {
        // Check if the number of arguments is valid
        if (args == NULL) {
            return false; // Invalid arguments
        }
    }
    uint8_t command[8] = {0};
    command[0] = cmd->super.super.command | (option & cmd->super.bitmask);
    for (uint8_t i = 0; i < cmd->argc; i++) {
        command[i + 1] = args[i] & cmd->arg_bitmasks[i];
    }
    return ssd1306_write(command, 1 + cmd->argc);
}

// ---------------------------- Fundamental commands ---------------------------- //

CommandWithArgs cmd_set_contrast = {
    .super = {0x81},
    .argc = 1,
    .arg_bitmasks = {0xFF}
};
bool ssd1306_set_contrast(uint8_t contrast) {
    return ssd1306_send_command_with_args(&cmd_set_contrast, (uint8_t[]){contrast});
}

CommandWithBitmask cmd_set_display = {
    .super = {0xA4},
    .bitmask = 0xB,
    .options_count = 6,
    .options = {
        SSD1306_OPTION_DISPLAY_ALLON_RESUME,
        SSD1306_OPTION_DISPLAY_ALLON_CLEAR,
        SSD1306_OPTION_DISPLAY_NORMAL,
        SSD1306_OPTION_DISPLAY_INVERT,
        SSD1306_OPTION_DISPLAY_OFF,
        SSD1306_OPTION_DISPLAY_ON
    }
};
bool ssd1306_set_display(uint8_t option) {
    return ssd1306_send_command_with_bitmask(&cmd_set_display, option);
}

// ---------------------------- Scrolling commands ---------------------------- //

CommandWithBitmaskAndArgs cmd_scroll_horizontal = {
    .super = {
        .super = {0x26},
        .bitmask = 0x1,
        .options_count = 2,
        .options = {
            SSD1306_OPTION_HORIZONTAL_SCROLL_RIGHT,
            SSD1306_OPTION_HORIZONTAL_SCROLL_LEFT
        }
    },
    .argc = 6,
    .arg_bitmasks = {0x00, 0x07, 0x07, 0x07, 0x00, 0xFF}
};
bool ssd1306_set_scroll_horizontal(uint8_t option, uint8_t *args) {
    return ssd1306_send_command_with_bitmask_and_args(&cmd_scroll_horizontal, option, args);
}

CommandWithBitmaskAndArgs cmd_scroll_horizontal_vertical = {
    .super = {
        .super = {0x29},
        .bitmask = 0x1,
        .options_count = 2,
        .options = {
            SSD1306_OPTION_HORIZONTAL_SCROLL_RIGHT,
            SSD1306_OPTION_HORIZONTAL_SCROLL_LEFT
        }
    },
    .argc = 5,
    .arg_bitmasks = {0x00, 0x07, 0x07, 0x07, 0x3F}
};
bool ssd1306_set_scroll_horizontal_vertical(uint8_t option, uint8_t *args) {
    return ssd1306_send_command_with_bitmask_and_args(&cmd_scroll_horizontal_vertical, option, args);
}

Command cmd_scroll_deactivate = {0x2E};
bool ssd1306_deactivate_scroll() {
    return ssd1306_send_simple_command(cmd_scroll_deactivate.command);
}

Command cmd_scroll_activate = {0x2F};
bool ssd1306_activate_scroll() {
    return ssd1306_send_simple_command(cmd_scroll_activate.command);
}

CommandWithArgs cmd_set_vertical_scroll_area = {
    .super = {0xA3},
    .argc = 2,
    .arg_bitmasks = {0x3F, 0x7F}
};
bool ssd1306_set_vertical_scroll_area(uint8_t *args) {
    return ssd1306_send_command_with_args(&cmd_set_vertical_scroll_area, args);
}

// ---------------------------- Address setting commands ---------------------------- //

CommandWithArgs cmd_set_memory_addressing_mode = {
    .super = {0x20},
    .argc = 1,
    .arg_bitmasks = {0x3}
};
bool ssd1306_set_memory_addressing_mode(uint8_t mode) {
    return ssd1306_send_command_with_args(&cmd_set_memory_addressing_mode, (uint8_t[]){mode});
}

CommandWithBitmask cmd_pa_mode_set_page_addr = {
    .super = {0xB0},
    .bitmask = 0x07,
    .options_count = 0
};
bool ssd1306_pa_mode_set_page_addr(uint8_t page) {
    return ssd1306_send_command_with_bitmask(&cmd_pa_mode_set_page_addr, page);
}

CommandWithBitmask cmd_pa_mode_set_column_addr_low = {
    .super = {0x00},
    .bitmask = 0x0F,
    .options_count = 0
};
bool ssd1306_pa_mode_set_column_addr_low(uint8_t column) {
    return ssd1306_send_command_with_bitmask(&cmd_pa_mode_set_column_addr_low, column);
}

CommandWithBitmask cmd_pa_mode_set_column_addr_high = {
    .super = {0x10},
    .bitmask = 0x0F,
    .options_count = 0
};
bool ssd1306_pa_mode_set_column_addr_high(uint8_t column) {
    return ssd1306_send_command_with_bitmask(&cmd_pa_mode_set_column_addr_high, column);
}

CommandWithArgs cmd_hava_mode_set_page_addr = {
    .super = {0x22},
    .argc = 2,
    .arg_bitmasks = {0x03, 0x03}
};
bool ssd1306_hava_mode_set_page_addr(uint8_t start_page, uint8_t end_page) {
    return ssd1306_send_command_with_args(&cmd_hava_mode_set_page_addr, (uint8_t[]){start_page, end_page});
}

CommandWithArgs cmd_hava_mode_set_column_addr = {
    .super = {0x21},
    .argc = 2,
    .arg_bitmasks = {0x7F, 0x7F}
};
bool ssd1306_hava_mode_set_column_addr(uint8_t start_column, uint8_t end_column) {
    return ssd1306_send_command_with_args(&cmd_hava_mode_set_column_addr, (uint8_t[]){start_column, end_column});
}

// ---------------------------- Hardware configuration commands (panel resolution and layout related) ---------------------------- //

CommandWithBitmask cmd_start_line = {
    .super = {0x40},
    .bitmask = 0x3F,
    .options_count = 0
};
bool ssd1306_set_start_line(uint8_t line) {
    return ssd1306_send_command_with_bitmask(&cmd_start_line, line);
}

CommandWithBitmask cmd_segment_remap = {
    .super = {0xA0},
    .bitmask = 0x01,
    .options_count = 2,
    .options = {
        SSD1306_OPTION_SEGMENT_REMAP_SEG0_TO_0,
        SSD1306_OPTION_SEGMENT_REMAP_SEG0_TO_127
    }
};
bool ssd1306_set_segment_remap(uint8_t option) {
    return ssd1306_send_command_with_bitmask(&cmd_segment_remap, option);
}

CommandWithArgs cmd_set_multiplex = {
    .super = {0xA8},
    .argc = 1,
    .arg_bitmasks = {0x3F}
};
bool ssd1306_set_multiplex(uint8_t multiplex) {
    return ssd1306_send_command_with_args(&cmd_set_multiplex, (uint8_t[]){multiplex});
}

CommandWithBitmask cmd_com_output_scan_dir = {
    .super = {0xC0},
    .bitmask = 0x08,
    .options_count = 2,
    .options = {
        SSD1306_OPTION_COM_SCAN_DIR_NORMAL,
        SSD1306_OPTION_COM_SCAN_DIR_REVERSE
    }
};
bool ssd1306_set_com_output_scan_dir(uint8_t option) {
    return ssd1306_send_command_with_bitmask(&cmd_com_output_scan_dir, option);
}

CommandWithArgs cmd_set_display_offset = {
    .super = {0xD3},
    .argc = 1,
    .arg_bitmasks = {0x3F}
};
bool ssd1306_set_display_offset(uint8_t offset) {
    return ssd1306_send_command_with_args(&cmd_set_display_offset, (uint8_t[]){offset});
}

CommandWithCallable cmd_set_com_pins = {
    .super = {
        .super = {
            .super = {0xDA},
            .bitmask = 0xFF,
            .options_count = 0
        },
        .argc = 1,
        .arg_bitmasks = {0xFF},
    }
//     .func = [](uint8_t x){ return ((x & 0x03) << 4) | 0x02; }
};
bool ssd1306_set_com_pins(uint8_t pins) {
    pins = ((pins & 0x03) << 4) | 0x02;
    return ssd1306_send_command_with_bitmask_and_args(&cmd_set_com_pins.super, 0x00, (uint8_t[]){pins});
}

// ---------------------------- Timing and driving scheme commands ---------------------------- //

CommandWithArgs cmd_set_display_clock_div_ratio = {
    .super = {0xD5},
    .argc = 1,
    .arg_bitmasks = {0xFF}
};
bool ssd1306_set_display_clock_div_ratio(uint8_t ratio) {
    return ssd1306_send_command_with_args(&cmd_set_display_clock_div_ratio, (uint8_t[]){ratio});
}

CommandWithArgs cmd_set_precharge_period = {
    .super = {0xD9},
    .argc = 1,
    .arg_bitmasks = {0xFF}
};
bool ssd1306_set_precharge_period(uint8_t period) {
    return ssd1306_send_command_with_args(&cmd_set_precharge_period, (uint8_t[]){period});
}

CommandWithCallable cmd_set_vcom_deselect_level = {
    .super = {
        .super = {
            .super = {0xDB},
            .bitmask = 0xFF,
            .options_count = 0
        },
        .argc = 1,
        .arg_bitmasks = {0xFF},
    }
    // .func = [](uint8_t x){ return ((x & 0x07) << 4) | 0x00; }
};
bool ssd1306_set_vcom_deselect_level(uint8_t level) {
    level = ((level & 0x07) << 4) | 0x00;
    return ssd1306_send_command_with_bitmask_and_args(&cmd_set_vcom_deselect_level.super, 0x00, (uint8_t[]){level});
}

Command cmd_nop = {0xE3};
bool ssd1306_nop_command() {
    return ssd1306_send_simple_command(cmd_nop.command);
}

CommandWithCallable cmd_charge_pump = {
    .super = {
        .super = {
            .super = {0x8D},
            .bitmask = 0xFF,
            .options_count = 0
        },
        .argc = 1,
        .arg_bitmasks = {0xFF},
    }
    // .func = [](uint8_t x){ return ((x & 0x01) << 2) | 0x10; }
};
bool ssd1306_charge_pump(uint8_t pump) {
    pump = ((pump & 0x01) << 2) | 0x10;
    return ssd1306_send_command_with_bitmask_and_args(&cmd_charge_pump.super, 0x00, (uint8_t[]){pump});
}

// SSD1306_EXTERNAL_VCC = Command(0x1)     # Unsure
Command cmd_external_vcc = {0x1};
bool ssd1306_set_external_vcc() {
    return ssd1306_send_simple_command(cmd_external_vcc.command);
}

// SSD1306_SWITCH_CAP_VCC = Command(0x2)   # Unsure
Command cmd_switch_cap_vcc = {0x2};
bool ssd1306_set_switch_cap_vcc() {
    return ssd1306_send_simple_command(cmd_switch_cap_vcc.command);
}
