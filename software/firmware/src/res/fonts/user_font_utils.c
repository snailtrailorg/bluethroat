
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "esp_log.h"

#include "res/fonts/user_font_utils.h"

#define FONT_UTILS_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define FONT_UTILS_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define FONT_UTILS_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define FONT_UTILS_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define FONT_UTILS_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define FONT_UTILS_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define FONT_UTILS_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define FONT_UTILS_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define FONT_UTILS_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define FONT_UTILS_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define FONT_UTILS_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			FONT_UTILS_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define FONT_UTILS_ASSERT(condition, format, ...)
#endif

static const char *TAG = "FONT_UTILS";

lv_coord_t lvgl_get_symbol_width(const lv_font_t * font, const char * utf8_symbol) {
    uint32_t unicode_symbol = lvgl_symbol_utf8_to_unicode(utf8_symbol);
    lv_font_glyph_dsc_t glyph_dsc;
    if (font->get_glyph_dsc(font, &glyph_dsc, unicode_symbol, 0)) {
        FONT_UTILS_LOGI("Get symbol 0x%lx(%ld) width from lv_font_glyph_dsc_t structure, result is %d", unicode_symbol, unicode_symbol, glyph_dsc.box_w);
        return glyph_dsc.box_w;
    } else {
        FONT_UTILS_LOGI("Get symbol 0x%lx(%ld) width from lv_font_t structure, result is %d", unicode_symbol, unicode_symbol, glyph_dsc.box_w);
        return font->line_height + font->base_line;
    }
}

lv_coord_t lvgl_get_symbol_height(const lv_font_t * font, const char * utf8_symbol) {
    uint32_t unicode_symbol = lvgl_symbol_utf8_to_unicode(utf8_symbol);
    lv_font_glyph_dsc_t glyph_dsc;
    if (font->get_glyph_dsc(font, &glyph_dsc, unicode_symbol, 0)) {
        FONT_UTILS_LOGI("Get symbol 0x%lx(%ld) height from lv_font_glyph_dsc_t structure, result is %d", unicode_symbol, unicode_symbol, glyph_dsc.box_h);
        return glyph_dsc.box_h;
    } else {
        FONT_UTILS_LOGI("Get symbol 0x%lx(%ld) height from lv_font_t structure, result is %d", unicode_symbol, unicode_symbol, glyph_dsc.box_h);
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