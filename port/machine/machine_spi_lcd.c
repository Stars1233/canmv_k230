#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "imlib.h" // must include before misc.h

#include "misc.h"
#include "mphal.h"
#include "mpprint.h"
#include "py/obj.h"
#include "py/runtime.h"

#include "extmod/machine_spi.h"
#include "modmachine.h"

#include "py_image.h"
#include "py_helper.h"

/*
form machine import SPI_LCD

lcd = SPI_LCD(spi : SPI, dc : Pin, cs : Pin = None, rst : Pin = None, bl : Pin = None, type : int = SPI_LCD.ST7789)

lcd.configure(width, height, hmirror = False, vflip = False, bgr = False)

lcd.int(custom_command = False)

lcd.command(cmd, data)

lcd.deint()

lcd.width()
lcd.height()
lcd.hmirror()
lcd.vflip()
lcd.bgr()

lcd.direction()

lcd.light(value)

lcd.get(x,y)
lcd.pixel(x,y,color)

lcd.fill(color)

lcd.show(img = None)
lcd.show_image(img = None)

lcd.text(str,x,y,color)
lcd.write(str)

*/

enum {
    SPI_LCD_TYPE_ST7789 = 1,
};

typedef struct _machine_spi_lcd_obj_t {
    mp_obj_base_t base;

    int type;
    int lcd_width;
    int lcd_height;

    int direction;

    bool hmirror;
    bool vflip;
    bool bgr;

    mp_obj_t image;

    struct {
        mp_obj_t spi;       // machine_spi_type
        mp_obj_t pin_dc;    // machine_pin_type

        // optional
        mp_obj_t pin_cs;    // machine_pin_type
        mp_obj_t pin_rst;   // machine_pin_type
        mp_obj_t pin_bl;    // machine_pin_type
    } py_obj;
} machine_spi_lcd_obj_t;

const mp_obj_type_t machine_spi_lcd_type;

extern const mp_obj_type_t machine_spi_type;
extern const mp_machine_spi_p_t machine_hw_spi_p;
// APIs ///////////////////////////////////////////////////////////////////////
STATIC void machine_spi_lcd_spi_write(mp_obj_t spi_obj, const uint8_t *data, size_t size) {
    machine_hw_spi_p.transfer(spi_obj, size, data, NULL);
}

STATIC void machine_spi_lcd_gpio_write(mp_obj_t pin_obj, int state) {
    machine_pin_value_set(pin_obj, state);
}

STATIC int machine_spi_lcd_gpio_read(mp_obj_t pin_obj) {
    return machine_pin_value_get(pin_obj);
}

STATIC void machine_spi_lcd_write_command(mp_obj_t self_in, uint8_t cmd, const uint8_t *data, size_t len) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Set DC to 0 (command mode)
    machine_spi_lcd_gpio_write(self->py_obj.pin_dc, 0);

    // Select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 0); // CS low
    }

    // Send the command
    machine_spi_lcd_spi_write(self->py_obj.spi, &cmd, 1);

    // Set DC to 1 (data mode)
    machine_spi_lcd_gpio_write(self->py_obj.pin_dc, 1);

    if(len) {
        machine_spi_lcd_spi_write(self->py_obj.spi, data, len);
    }

    // De-select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 1); // CS high
    }
}

STATIC void machine_spi_lcd_write_data(mp_obj_t self_in, const uint8_t *data, size_t len) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(0x00 == len) {
        return;
    }

    // Set DC to 1 (data mode)
    machine_spi_lcd_gpio_write(self->py_obj.pin_dc, 1);

    // Select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 0); // CS low
    }

    machine_spi_lcd_spi_write(self->py_obj.spi, data, len);

    // De-select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 1); // CS high
    }
}

STATIC void machine_spi_lcd_send_cmd_seq(mp_obj_t self_in, const uint8_t *data, size_t size) {
    const uint8_t *p_data = data;
    const uint8_t *data_end = (data + size);

    while(p_data < data_end) {
        if((data_end - p_data - 2) < p_data[1]) {
            printf("command sequence format error.\n");
            break;
        }

        if(0x00 == p_data[0]) {
            mp_hal_delay_ms(p_data[1]);
            p_data += 2;
        } else {
            machine_spi_lcd_write_command(self_in, p_data[0], &p_data[2], p_data[1]);
            p_data += p_data[1] + 2;
        }
    }
}

// ST7789 Start ///////////////////////////////////////////////////////////////
STATIC void machine_spi_lcd_init_screen_st7789(mp_obj_t self_in) {
    // Define the command sequence for the LCD initialization
    const uint8_t lcd_init_sequence[] = {
        // Command sequence for initializing the LCD
        0x11, 0,              // Sleep Out
        0x00, 5,              // Delay
        0x11, 0,              // Sleep Out, send twice
        0x00, 30,             // Delay
        0x36, 1, 0x00,        // Memory Access Control
        0x3A, 1, 0x65,        // Interface Pixel Format
        0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33, // Porch Setting
        0xB7, 1, 0x75,        // Gate Control
        0xBB, 1, 0x1A,        // VCOM Setting
        0xC0, 1, 0x2C,        // LV0 Control
        0xC2, 1, 0x01,        // VGH/VGL Setting
        0xC3, 1, 0x13,        // VGH Setting
        0xC4, 1, 0x20,        // VGL Setting
        0xC6, 1, 0x0F,        // Display Control
        0xD0, 2, 0xA4, 0xA1,  // Power Control 1
        0xD6, 1, 0xA1,        // Power Control 2
        0xE0, 14, 0xD0, 0x0D, 0x14, 0x0D, 0x0D, 0x09, 0x38, 0x44, 0x4E, 0x3A, 0x17, 0x18, 0x2F, 0x30, // Positive Gamma Correction
        0xE1, 14, 0xD0, 0x09, 0x0F, 0x08, 0x07, 0x14, 0x37, 0x44, 0x4D, 0x38, 0x15, 0x16, 0x2C, 0x2E, // Negative Gamma Correction
        0x20, 0,               // Inversion Off
        0x29, 0,               // Display On
    };

    machine_spi_lcd_send_cmd_seq(self_in, lcd_init_sequence, sizeof(lcd_init_sequence) / sizeof(lcd_init_sequence[0]));
}

STATIC void machine_spi_lcd_set_area_st7789(mp_obj_t self_in, int x, int y, int width, int height) {
    uint8_t area_cmd_seq[14];

    // Column Address Set (0x2A)
    area_cmd_seq[0] = 0x2A; // Command to set column address
    area_cmd_seq[1] = 4;
    area_cmd_seq[2] = (x >> 8) & 0xFF; // X0 high byte
    area_cmd_seq[3] = x & 0xFF;        // X0 low byte
    area_cmd_seq[4] = ((x + width - 1) >> 8) & 0xFF; // X1 high byte
    area_cmd_seq[5] = (x + width - 1) & 0xFF;       // X1 low byte

    // Row Address Set (0x2B)
    area_cmd_seq[6] = 0x2B; // Command to set row address
    area_cmd_seq[7] = 4;
    area_cmd_seq[8] = (y >> 8) & 0xFF; // Y0 high byte
    area_cmd_seq[9] = y & 0xFF;        // Y0 low byte
    area_cmd_seq[10] = ((y + height - 1) >> 8) & 0xFF; // Y1 high byte
    area_cmd_seq[11] = (y + height - 1) & 0xFF;       // Y1 low byte

    area_cmd_seq[12] = 0x2C; // Write Memory Start
    area_cmd_seq[13] = 0x00;

    machine_spi_lcd_send_cmd_seq(self_in, area_cmd_seq, sizeof(area_cmd_seq) / sizeof(area_cmd_seq[0]));
}

STATIC mp_obj_t machine_spi_lcd_apply_configure_st7789(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Values of hmirror, vflip, bgr, width, and height are stored in self->hmirror, self->vflip, self->bgr, self->lcd_width, self->lcd_height
    uint8_t direction = 0;

    // Set D7 (MY) for vflip
    if (self->vflip) {
        direction |= (1 << 7);  // Set bit D7 (MY) to 1 for Bottom to Top
    }

    // Set D6 (MX) for hmirror
    if (self->hmirror) {
        direction |= (1 << 6);  // Set bit D6 (MX) to 1 for Right to Left
    }

    // Set D5 (MV) for Page/Column Order based on width and height
    if (self->lcd_width > self->lcd_height) {
        direction |= (1 << 5);  // Set bit D5 (MV) to 1 for rotating the display (landscape mode)
    }

    // Set D3 (RGB/BGR)
    if (self->bgr) {
        direction |= (1 << 3);  // Set bit D3 to 1 for BGR
    }

    uint8_t cmd[3];

    cmd[0] = 0x36;
    cmd[1] = 0x01;
    cmd[2] = direction;

    machine_spi_lcd_send_cmd_seq(self_in, cmd, sizeof(cmd) / sizeof(cmd[0]));

    self->direction = direction;

    return mp_obj_new_int(direction);
}
// ST7789 End /////////////////////////////////////////////////////////////////

STATIC void machine_spi_lcd_init_screen(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(SPI_LCD_TYPE_ST7789 == self->type) {
        machine_spi_lcd_init_screen_st7789(self_in);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("not support lcd type"));
    }
}

STATIC void machine_spi_lcd_set_area(mp_obj_t self_in, int x, int y, int width, int height) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(SPI_LCD_TYPE_ST7789 == self->type) {
        machine_spi_lcd_set_area_st7789(self_in, x, y, width, height);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("not support lcd type"));
    }
}

STATIC mp_obj_t machine_spi_lcd_apply_configure(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(SPI_LCD_TYPE_ST7789 == self->type) {
        return machine_spi_lcd_apply_configure_st7789(self_in);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("not support lcd type"));
    }
}

STATIC void machine_spi_lcd_show_image(mp_obj_t self_in, mp_obj_t img_obj, int x, int y) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    image_t *img = py_image_cobj(img_obj);

    int w = img->w;
    int h = img->h;

    if(((x + w) > self->lcd_width) || ((y + h) > self->lcd_height)) {
        mp_raise_ValueError(MP_ERROR_TEXT("image show position bigger than lcd resolution"));
    }

    // workaround, should be remove after
    {
        uint8_t tmp;
        uint8_t *pstart = img->data;
        uint8_t *pend = pstart + (w * h * 2);

        while (pstart < pend) {
            tmp = pstart[0];
            pstart[0] = pstart[1];
            pstart[1] = tmp;

            pstart += 2;
        }
    }

    machine_spi_lcd_set_area(self_in, x, y, img->w, img->h);

    machine_spi_lcd_write_data(self_in, img->data, image_size(img));
}

// Python Methods /////////////////////////////////////////////////////////////
STATIC mp_obj_t machine_spi_lcd_command(mp_obj_t self_in, mp_obj_t cmd_in, mp_obj_t data_in) {
    // Cast self_in to the lcd object
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Convert cmd to uint8_t
    uint8_t cmd = mp_obj_get_int(cmd_in);

    // Set DC to 0 (command mode)
    machine_spi_lcd_gpio_write(self->py_obj.pin_dc, 0);

    // Select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 0); // CS low
    }

    // Send the command
    machine_spi_lcd_spi_write(self->py_obj.spi, &cmd, 1);

    // Set DC to 1 (data mode)
    machine_spi_lcd_gpio_write(self->py_obj.pin_dc, 1);

    // Handle the data
    if (data_in != mp_const_none) {
        // Check if data is a bytes-like object or an integer (single byte)
        if (mp_obj_is_int(data_in)) {
            // Single byte (int)
            uint8_t data = mp_obj_get_int(data_in);
            machine_spi_lcd_spi_write(self->py_obj.spi, &data, 1);
        } else if (mp_obj_is_type(data_in, &mp_type_bytes) || mp_obj_is_type(data_in, &mp_type_bytearray)) {
            // Bytes or bytearray
            mp_buffer_info_t bufinfo;
            mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
            machine_spi_lcd_spi_write(self->py_obj.spi, bufinfo.buf, bufinfo.len);
        } else if (mp_obj_is_type(data_in, &mp_type_list) || mp_obj_is_type(data_in, &mp_type_tuple)) {
            // List or tuple of integers (bytes)
            size_t len;
            mp_obj_t *items;
            mp_obj_get_array(data_in, &len, &items);

            uint8_t data[len];
            for (size_t i = 0; i < len; i++) {
                data[i] = mp_obj_get_int(items[i]);
            }
            machine_spi_lcd_spi_write(self->py_obj.spi, data, len);
        } else {
            mp_raise_TypeError(MP_ERROR_TEXT("data must be int, bytes, bytearray, list or tuple"));
        }
    }

    // De-select the LCD (if CS is available)
    if (self->py_obj.pin_cs != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_cs, 1); // CS high
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_spi_lcd_command_obj, machine_spi_lcd_command);

// Define the configure function with two positional and three keyword arguments
STATIC mp_obj_t machine_spi_lcd_configure(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // Extract the positional arguments: width and height
    mp_int_t width = mp_obj_get_int(pos_args[1]);
    mp_int_t height = mp_obj_get_int(pos_args[2]);

    // Define the keyword argument names
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_hmirror, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },  // Default: False
        { MP_QSTR_vflip, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },    // Default: False
        { MP_QSTR_bgr, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },      // Default: False
    };

    // Parse the keyword arguments
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 3, pos_args + 3, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Set width and height
    self->lcd_width = width;
    self->lcd_height = height;

    // Set hmirror, vflip, and bgr from keyword arguments
    self->hmirror = args[0].u_bool;
    self->vflip = args[1].u_bool;
    self->bgr = args[2].u_bool;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_lcd_configure_obj, 3, machine_spi_lcd_configure);

STATIC mp_obj_t machine_spi_lcd_init(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    bool custom_command = false;

    if(0x02 == n_args) {
        custom_command = mp_obj_is_true(args[1]);
    }

    if(false == custom_command) {
        if (self->py_obj.pin_rst != mp_const_none) {
            machine_spi_lcd_gpio_write(self->py_obj.pin_rst, 0);
            mp_hal_delay_ms(100);
            machine_spi_lcd_gpio_write(self->py_obj.pin_rst, 1);
        }

        machine_spi_lcd_init_screen(MP_OBJ_FROM_PTR(self));
    }

    // set hmirro and vflip and bgr
    machine_spi_lcd_apply_configure(MP_OBJ_FROM_PTR(self));

    if (self->py_obj.pin_bl != mp_const_none) {
        machine_spi_lcd_gpio_write(self->py_obj.pin_bl, 1);
    }

    // Create a dictionary for keyword arguments
    mp_obj_dict_t *kwargs_dict = mp_obj_new_dict(3); // Allocate a dict with space for 3 items

    mp_map_t *map = &kwargs_dict->map;
    map->is_fixed = 0;  // Allow to modify the dict

    // Set alloc_type to ALLOC_MMZ
    mp_obj_t alloc_key = mp_obj_new_str("alloc", strlen("alloc"));
    mp_map_elem_t *alloc_elem = mp_map_lookup(map, alloc_key, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
    if (alloc_elem != NULL) {
        alloc_elem->value = mp_obj_new_int(ALLOC_MMZ); // Set alloc_type to ALLOC_MMZ
    }

    // Set cache to true
    mp_obj_t cache_key = mp_obj_new_str("cache", strlen("cache"));
    mp_map_elem_t *cache_elem = mp_map_lookup(map, cache_key, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
    if (cache_elem != NULL) {
        cache_elem->value = mp_obj_new_bool(true); // Set cache to true
    }

    image_t img = {};

    img.w = self->lcd_width;
    img.h = self->lcd_height;
    img.pixfmt = PIXFORMAT_RGB565;

    // Call the original image allocation function
    py_image_alloc(&img, &kwargs_dict->map);

    // clear image
    memset(img.data, 0, image_size(&img));

    self->image = py_image_from_struct(&img);

    return self->image;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_init_obj, 1, 2, machine_spi_lcd_init);

STATIC mp_obj_t machine_spi_lcd_deinit(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    image_t *img = py_image_cobj(self->image);

    py_image_free(img);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_lcd_deinit_obj, machine_spi_lcd_deinit);

STATIC mp_obj_t machine_spi_lcd_width(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(self->lcd_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_lcd_width_obj, machine_spi_lcd_width);

STATIC mp_obj_t machine_spi_lcd_height(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(self->lcd_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_lcd_height_obj, machine_spi_lcd_height);

STATIC mp_obj_t machine_spi_lcd_get_direction(mp_obj_t self_in) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(self->direction);   
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_lcd_get_direction_obj, machine_spi_lcd_get_direction);

STATIC mp_obj_t machine_spi_lcd_hmirror(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if(0x02 == n_args) {
        bool new = mp_obj_is_true(args[1]);
        if(new == self->hmirror) {
            return mp_const_true;
        } else {
            self->hmirror = new;
            return machine_spi_lcd_apply_configure(args[0]);
        }
    } else {
        return mp_obj_new_bool(self->hmirror);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_hmirror_obj, 1, 2, machine_spi_lcd_hmirror);

STATIC mp_obj_t machine_spi_lcd_vflip(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if(0x02 == n_args) {
        bool new = mp_obj_is_true(args[1]);
        if(new == self->vflip) {
            return mp_const_true;
        } else {
            self->vflip = new;
            return machine_spi_lcd_apply_configure(args[0]);
        }
    } else {
        return mp_obj_new_bool(self->vflip);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_set_vflip_obj, 1, 2, machine_spi_lcd_vflip);

STATIC mp_obj_t machine_spi_lcd_bgr(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if(0x02 == n_args) {
        bool new = mp_obj_is_true(args[1]);
        if(new == self->bgr) {
            return mp_const_true;
        } else {
            self->bgr = new;
            return machine_spi_lcd_apply_configure(args[0]);
        }
    } else {
        return mp_obj_new_bool(self->bgr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_bgr_obj, 1, 2, machine_spi_lcd_bgr);

STATIC mp_obj_t machine_spi_lcd_light(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    int val = -1;

    if(0x02 == n_args) {
        val = mp_obj_get_int(args[1]);

        if (self->py_obj.pin_bl != mp_const_none) {
            machine_spi_lcd_gpio_write(self->py_obj.pin_bl, val);
        }
    } else {
        if (self->py_obj.pin_bl != mp_const_none) {
            val = machine_spi_lcd_gpio_read(self->py_obj.pin_bl);
        }

        return mp_obj_new_int(val);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_light_obj, 1, 2, machine_spi_lcd_light);

STATIC mp_obj_t machine_spi_lcd_get(mp_obj_t self_in, mp_obj_t obj_x, mp_obj_t obj_y) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    image_t *img = py_image_cobj(self->image);

    int x = mp_obj_get_int(obj_x);
    int y = mp_obj_get_int(obj_y);

    return mp_obj_new_int(IMAGE_GET_RGB565_PIXEL(img, x, y));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_spi_lcd_get_obj, machine_spi_lcd_get);

STATIC mp_obj_t machine_spi_lcd_pixel(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    image_t *img = py_image_cobj(self->image);

    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);

    int arg_c =
        py_helper_keyword_color(img, n_args, args, 3, kw_args, -1); // White.

    if ((!IM_X_INSIDE(img, x)) || (!IM_Y_INSIDE(img, y))) {
        return args[0];
    }

    IMAGE_PUT_RGB565_PIXEL(img, x, y, arg_c);

    return args[0];
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_lcd_pixel_obj, 3, machine_spi_lcd_pixel);

STATIC mp_obj_t machine_spi_lcd_fill(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    image_t *img = py_image_cobj(self->image);

    int w = img->w;
    int h = img->h;
    uint16_t *img_data = (uint16_t *)img->data;

    uint16_t arg_c =
        py_helper_keyword_color(img, n_args, args, 1, kw_args, -1); // White.

    for(int index = 0; index < (w * h); index ++) {
        img_data[index] = arg_c;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_lcd_fill_obj, 1, machine_spi_lcd_fill);

STATIC mp_obj_t machine_spi_lcd_show(size_t n_args, const mp_obj_t *args) {
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    mp_obj_t img_obj = n_args >= 2 ? args[1] : self->image;
    int x = n_args >= 3 ? mp_obj_get_int(args[2]) : 0;
    int y = n_args >= 4 ? mp_obj_get_int(args[3]) : 0;

    image_t *img = py_image_cobj(img_obj);
    if(!IM_IS_RGB565(img)) {
        mp_raise_ValueError(MP_ERROR_TEXT("image is not rgb565"));
    }

    machine_spi_lcd_show_image(args[0], img_obj, x, y);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_lcd_show_obj, 1, 4, machine_spi_lcd_show);

STATIC void machine_spi_lcd_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // Convert the self_in object to the machine_spi_lcd_obj_t struct
    machine_spi_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Print the general info about the SPI_LCD object
    mp_printf(print, "<SPI_LCD (type=%d):\n", self->type);

    // Print the dimensions
    mp_printf(print, "  Width: %d, Height: %d\n", self->lcd_width, self->lcd_height);

    // Print the direction
    mp_printf(print, "  Direction: 0x%02X\n", self->direction);

    // Print the horizontal mirror and vertical flip statuses
    mp_printf(print, "  HMirror: %s, VFlip: %s\n",
              self->hmirror ? "True" : "False",
              self->vflip ? "True" : "False");

    // Print the RGB/BGR configuration
    mp_printf(print, "  BGR565: %s\n", self->bgr ? "True" : "False");

    // Print SPI object
    mp_printf(print, "  SPI: ");
    mp_obj_print_helper(print, self->py_obj.spi, PRINT_REPR);
    mp_printf(print, "\n");

    // Print DC pin object
    mp_printf(print, "  DC: ");
    mp_obj_print_helper(print, self->py_obj.pin_dc, PRINT_REPR);
    mp_printf(print, "\n");

    // Print optional pin objects (CS, RST, BL), check if they are not None
    if (self->py_obj.pin_cs != mp_const_none) {
        mp_printf(print, "  CS: ");
        mp_obj_print_helper(print, self->py_obj.pin_cs, PRINT_REPR);
        mp_printf(print, "\n");
    } else {
        mp_printf(print, "  CS: None\n");
    }

    if (self->py_obj.pin_rst != mp_const_none) {
        mp_printf(print, "  RST: ");
        mp_obj_print_helper(print, self->py_obj.pin_rst, PRINT_REPR);
        mp_printf(print, "\n");
    } else {
        mp_printf(print, "  RST: None\n");
    }

    if (self->py_obj.pin_bl != mp_const_none) {
        mp_printf(print, "  BL Pin: ");
        mp_obj_print_helper(print, self->py_obj.pin_bl, PRINT_REPR);
        mp_printf(print, "\n");
    } else {
        mp_printf(print, "  BL: None\n");
    }

    // Close the SPI_LCD tag
    mp_printf(print, ">");
}

STATIC void machine_spi_lcd_make_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Define the argument specifiers (width, height, spi, dc are positional, others are optional)
    enum { ARG_spi, ARG_dc, ARG_cs, ARG_rst, ARG_bl, ARG_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_spi, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dc, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_cs, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_rst, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_bl, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_type, MP_ARG_INT, {.u_int = SPI_LCD_TYPE_ST7789} },
    };

    // Parse arguments
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    // Validate types
    if (!mp_obj_is_type(parsed_args[ARG_spi].u_obj, &machine_spi_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("spi must be a SPI object"));
    }
    if (!mp_obj_is_type(parsed_args[ARG_dc].u_obj, &machine_pin_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("dc must be a Pin object"));
    }
    if (parsed_args[ARG_cs].u_obj != mp_const_none && !mp_obj_is_type(parsed_args[ARG_cs].u_obj, &machine_pin_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("cs must be a Pin object"));
    }
    if (parsed_args[ARG_rst].u_obj != mp_const_none && !mp_obj_is_type(parsed_args[ARG_rst].u_obj, &machine_pin_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("rst must be a Pin object"));
    }
    if (parsed_args[ARG_bl].u_obj != mp_const_none && !mp_obj_is_type(parsed_args[ARG_bl].u_obj, &machine_pin_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("bl must be a Pin object"));
    }

    machine_spi_lcd_obj_t *self = (machine_spi_lcd_obj_t *)self_in;

    // Set mandatory arguments
    self->lcd_width = 240;
    self->lcd_height = 320;
    self->hmirror = false;
    self->vflip = false;
    self->bgr = false;

    self->py_obj.spi = parsed_args[ARG_spi].u_obj;
    self->py_obj.pin_dc = parsed_args[ARG_dc].u_obj;

    // Set optional arguments (cs, rst, bl) if provided
    self->py_obj.pin_cs = parsed_args[ARG_cs].u_obj;
    self->py_obj.pin_rst = parsed_args[ARG_rst].u_obj;
    self->py_obj.pin_bl = parsed_args[ARG_bl].u_obj;

    self->type = parsed_args[ARG_type].u_int;
}

STATIC mp_obj_t machine_spi_lcd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_map_t kw_args;

    machine_spi_lcd_obj_t *self = m_new_obj_with_finaliser(machine_spi_lcd_obj_t);
    self->base.type = &machine_spi_lcd_type;

    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    machine_spi_lcd_make_init(&self->base, n_args, args, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: machine
//| class SPI_LCD:
//|     """machine.SPI_LCD object."""
//|     def __init__(self) -> None:
//|         """Create a machine.SPI_LCD object."""
//|     def bgr(self, new: bool = ..., /) -> Any:
//|         """Perform bgr for machine.SPI_LCD."""
//|     def command(self, cmd: Any, data: Any, /) -> Any:
//|         """Perform command for machine.SPI_LCD."""
//|     def configure(self, *, hmirror: bool = False, vflip: bool = False, bgr: bool = False) -> Any:
//|         """Configure for machine.SPI_LCD."""
//|     def deinit(self, /) -> None:
//|         """Release resources held by machine.SPI_LCD."""
//|     def fill(self, spi: Any, dc: Any, cs: Any = None, rst: Any = None, bl: Any = None, type: int = ..., *, color: Any = ...) -> Any:
//|         """Perform fill for machine.SPI_LCD."""
//|     def get(self, obj_x: Any, obj_y: Any, /) -> Any:
//|         """Perform get for machine.SPI_LCD."""
//|     def get_direction(self, /) -> Any:
//|         """Return direction for machine.SPI_LCD."""
//|     def height(self, /) -> Any:
//|         """Return height for machine.SPI_LCD."""
//|     def hmirror(self, new: bool = ..., /) -> Any:
//|         """Perform hmirror for machine.SPI_LCD."""
//|     def init(self, custom_command: bool = ..., /) -> None:
//|         """Initialize machine.SPI_LCD."""
//|     def light(self, val: int = ..., /) -> Any:
//|         """Perform light for machine.SPI_LCD."""
//|     def pixel(self, spi: Any, dc: Any, cs: Any = None, rst: Any = None, bl: Any = None, type: int = ..., *, color: Any = ...) -> Any:
//|         """Perform pixel for machine.SPI_LCD."""
//|     def set(self, spi: Any, dc: Any, cs: Any = None, rst: Any = None, bl: Any = None, type: int = ..., *, color: Any = ...) -> Any:
//|         """Perform set for machine.SPI_LCD."""
//|     def show(self, img: Any = ..., x: int = ..., y: int = ..., /) -> Any:
//|         """Perform show for machine.SPI_LCD."""
//|     def show_image(self, img: Any = ..., x: int = ..., y: int = ..., /) -> Any:
//|         """Perform show image for machine.SPI_LCD."""
//|     def vflip(self, new: bool = ..., /) -> Any:
//|         """Perform vflip for machine.SPI_LCD."""
//|     def width(self, /) -> Any:
//|         """Return width for machine.SPI_LCD."""


STATIC const mp_rom_map_elem_t machine_spi_lcd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_spi_lcd_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_configure), MP_ROM_PTR(&machine_spi_lcd_configure_obj) },

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_spi_lcd_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_spi_lcd_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_get_direction), MP_ROM_PTR(&machine_spi_lcd_get_direction_obj) },

    { MP_ROM_QSTR(MP_QSTR_hmirror), MP_ROM_PTR(&machine_spi_lcd_hmirror_obj) },
    { MP_ROM_QSTR(MP_QSTR_vflip), MP_ROM_PTR(&machine_spi_lcd_set_vflip_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgr), MP_ROM_PTR(&machine_spi_lcd_bgr_obj) },

    { MP_ROM_QSTR(MP_QSTR_command), MP_ROM_PTR(&machine_spi_lcd_command_obj) },

    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&machine_spi_lcd_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&machine_spi_lcd_height_obj) },

    { MP_ROM_QSTR(MP_QSTR_light), MP_ROM_PTR(&machine_spi_lcd_light_obj) },

    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&machine_spi_lcd_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&machine_spi_lcd_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&machine_spi_lcd_pixel_obj) },

    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&machine_spi_lcd_fill_obj) },

    { MP_ROM_QSTR(MP_QSTR_show), MP_ROM_PTR(&machine_spi_lcd_show_obj) },
    { MP_ROM_QSTR(MP_QSTR_show_image), MP_ROM_PTR(&machine_spi_lcd_show_obj) },

    // Const
    { MP_ROM_QSTR(MP_QSTR_TYPE_ST7789), MP_ROM_INT(SPI_LCD_TYPE_ST7789) },
};
MP_DEFINE_CONST_DICT(mp_machine_spi_lcd_locals_dict, machine_spi_lcd_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_spi_lcd_type,
    MP_QSTR_SPI_LCD,
    MP_TYPE_FLAG_NONE,
    make_new, machine_spi_lcd_make_new,
    print, machine_spi_lcd_print,
    locals_dict, &mp_machine_spi_lcd_locals_dict);
