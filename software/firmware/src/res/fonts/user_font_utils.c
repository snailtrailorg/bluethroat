
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "esp_log.h"

#include "res/fonts/user_font_utils.h"

lv_coord_t lvgl_get_symbol_width(const lv_font_t * font, const char * utf8_symbol) {
    uint32_t unicode_symbol = lvgl_symbol_utf8_to_unicode(utf8_symbol);
    lv_font_glyph_dsc_t glyph_dsc;
    if (font->get_glyph_dsc(font, &glyph_dsc, unicode_symbol, 0)) {
        ESP_LOGE("symbol width", "%d", glyph_dsc.box_w);
        return glyph_dsc.box_w;
    } else {
        return font->line_height + font->base_line;
    }
}

lv_coord_t lvgl_get_symbol_height(const lv_font_t * font, const char * utf8_symbol) {
    uint32_t unicode_symbol = lvgl_symbol_utf8_to_unicode(utf8_symbol);
    lv_font_glyph_dsc_t glyph_dsc;
    if (font->get_glyph_dsc(font, &glyph_dsc, unicode_symbol, 0)) {
        ESP_LOGE("symbol height", "%d", glyph_dsc.box_w);
        return glyph_dsc.box_h;
    } else {
        return font->line_height + font->base_line;
    }
}

uint32_t lvgl_symbol_utf8_to_unicode(const char * symbol) {
    uint32_t unicode = 0;
    size_t length = strlen(symbol);

    switch (length) {
        case 4: unicode = (symbol[0] & 0x07); break;
        case 3: unicode = (symbol[0] & 0x0f); break;
        case 2: unicode = (symbol[0] & 0x1f); break;
        case 1: unicode = (symbol[0] & 0x7f); break;
        default: ;
    }

    for (int i=1; i<length; i++) {
        unicode <<= 6;
        unicode += (symbol[i] & 0x3f);
    }

    return unicode;
}