
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "esp_log.h"

lv_coord_t lvgl_get_symbol_width(const lv_font_t * font, const uint32_t symbol) {
    lv_font_glyph_dsc_t glyph_dsc;
    if (font->get_glyph_dsc(font, &glyph_dsc, symbol, 0)) {
        ESP_LOGE("UNICODE", "%d", glyph_dsc.box_w);
        return glyph_dsc.box_w;
    } else {
        return font -> line_height;
    }
}

uint32_t lvgl_symbol_utf8_to_unicode(const char * utf8) {
    uint32_t unicode = 0;
    size_t length = strlen(utf8);

    switch (length) {
        case 4: unicode += (utf8[0] & 0x07); break;
        case 3: unicode += (utf8[0] & 0x0f); break;
        case 2: unicode += (utf8[0] & 0x3f); break;
        case 1: unicode += utf8[0]; break;
        default: ;
    }

    for (int i=1; i<length; i++) {
        unicode <<= 6;
        unicode += (utf8[i] & 0x3f);
    }

    return unicode;
}