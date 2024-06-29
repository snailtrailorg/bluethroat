#pragma once

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* LVGL get symbol width.
   The width and height of symbols and characters are various in some fonts, 
   so it is necessary to dynamically obtain the width during screen layout. */
lv_coord_t lvgl_get_symbol_width(const lv_font_t * font, const char * utf8_symbol);
lv_coord_t lvgl_get_symbol_height(const lv_font_t * font, const char * utf8_symbol);

uint32_t lvgl_symbol_utf8_to_unicode(const char * symbol);

#ifdef __cplusplus
}
#endif
