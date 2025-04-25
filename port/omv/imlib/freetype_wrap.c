#include "py/obj.h"
#include "py/runtime.h"

#include "imlib.h"

#include "freetype_wrap.h"

void freetype_deinit(void) { freetype_wrap_deinit(); }

static void draw_bitmap(void* ctx, int color, int x, int y, FT_Bitmap* bitmap)
{
    FT_Int i, j, p, q;
    FT_Int x_max = x + bitmap->width;
    FT_Int y_max = y + bitmap->rows;

    image_t* img = ctx;

    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < 0 || j < 0 || i >= img->w || j >= img->h)
                continue;

            if (bitmap->buffer[q * bitmap->width + p]) {
                imlib_set_pixel(img, i, j, color);
            }
        }
    }
}

void imlib_draw_string_advance(image_t* img, int x_off, int y_off, int char_size, const char* str, int color,
                               const char* font_path)
{
    int error = freetype_wrap_draw_string(x_off, y_off, char_size, str, color, font_path, draw_bitmap, img);

    if (FREETYPE_WRAP_ERR_NONE != error) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("FreeType init failed, font %s errorno %d."), font_path,
                          error);
    }
}
