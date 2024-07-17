#include <math.h>
#include <esp_log.h>

#include "bluethroat_ui.h"

#define BLUETHROAT_UI_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_UI_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_UI_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_UI_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_UI_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define BLUETHROAT_UI_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define BLUETHROAT_UI_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define BLUETHROAT_UI_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define BLUETHROAT_UI_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define BLUETHROAT_UI_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define BLUETHROAT_UI_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			BLUETHROAT_UI_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define BLUETHROAT_UI_ASSERT(condition, format, ...)
#endif

static const char *TAG = "BLUETHROAT_UI";

lv_obj_t * bluethroat_draw_panel(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t radius, lv_coord_t border_width, lv_color_t border_color, lv_opa_t border_opacity, lv_coord_t padding) {
	lv_obj_t * panel = lv_obj_create(parent);

    if (w > 0 && h > 0) {
        lv_obj_set_size(panel, w, h);
    }
    if (ref == NULL) {
        lv_obj_set_pos(panel, x, y);
    } else {
        lv_obj_align_to(panel, ref, align, x, y);
    }

	lv_obj_set_style_bg_color(panel, bg_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(panel, bg_opacity, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_set_style_radius(panel, radius, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_border_width(panel, border_width, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    if (border_width > 0) {
        lv_obj_set_style_border_color(panel, border_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
        lv_obj_set_style_border_opa(panel, border_opacity, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    }

	lv_obj_set_style_pad_all(panel, padding, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    return panel;

}

lv_obj_t * bluethroat_draw_label(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t padding, lv_text_align_t text_align, lv_color_t text_color, const lv_font_t *font, const char *text) {
    lv_obj_t * label = lv_label_create(parent);

    if (w > 0) {
		lv_obj_set_width(label, w);
	}
	if (h > 0) {
		lv_obj_set_height(label, h);
	}
	if (ref == NULL) {
        lv_obj_set_pos(label, x, y);
    } else {
        lv_obj_align_to(label, ref, align, x, y);
    }

    lv_obj_set_style_bg_color(label, bg_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_bg_opa(label, bg_opacity, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_set_style_border_width(label, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_pad_all(label, padding, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_align(label, text_align, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_color(label, text_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(label, font, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_update_layout(label);

    return label;
}

lv_obj_t * bluethroat_draw_icon(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t padding, lv_text_align_t text_align, lv_color_t text_color, const lv_font_t *font, const char *text) {
    lv_obj_t * label = lv_label_create(parent);

   	lv_obj_set_size(label, ((w == 0) ? lvgl_get_symbol_width(font, text) : w), ((h == 0) ? lvgl_get_symbol_height(font, text) : h));
    if (ref == NULL) {
        lv_obj_set_pos(label, x, y);
    } else {
        lv_obj_align_to(label, ref, align, x, y);
    }

    lv_obj_set_style_bg_color(label, bg_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_bg_opa(label, bg_opacity, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_set_style_border_width(label, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_pad_all(label, padding, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_align(label, text_align, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_color(label, text_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(label, font, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_update_layout(label);

    return label;
}

static lv_meter_indicator_t * btui_meter_add_arc_set_range(lv_obj_t *meter, lv_meter_scale_t *scale, int16_t width, lv_color_t color, int16_t r_mod, int32_t start_value, int32_t end_value) {
    lv_meter_indicator_t *arc = lv_meter_add_arc(meter, scale, width, color, r_mod);
    lv_meter_set_indicator_start_value(meter, arc, start_value);
    lv_meter_set_indicator_end_value(meter, arc, end_value);

	return arc;
}

lv_obj_t * bluethroat_draw_vario_meter(lv_obj_t * parent, lv_obj_t * ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_meter_indicator_t **sink_arc, lv_meter_indicator_t **lift_arc) {
	lv_obj_t *meter = lv_meter_create(parent);
	lv_obj_set_size(meter, w, h);
    lv_obj_align_to(meter, ref, align, x, y);

	lv_meter_scale_t *scale = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale, 21, 1, 20, lv_color_hex(0x000000));
	lv_meter_set_scale_major_ticks(meter, scale, 2, 1, 20, lv_color_hex(0x000000), 15);
	lv_meter_set_scale_range(meter, scale, -10, 10, 270, 135);

	lv_obj_set_style_bg_opa(meter, 255, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_color(meter, lv_color_hex(0x000000), LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_border_width(meter, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_pad_all(meter, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_outline_width(meter, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_set_style_text_color(meter, lv_color_hex(0x3affe7), LV_SELECTOR(LV_PART_TICKS, LV_STATE_DEFAULT));
	lv_obj_set_style_text_font(meter, &antonio_regular_14, LV_SELECTOR(LV_PART_TICKS, LV_STATE_DEFAULT));

	btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0xff7f1f), 0, -10, -8);
	btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0xffcf1f), 0, -8, 0);
	btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0x1fff7f), 0, 0, 8);
	btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0x1fcfff), 0, 8, 10);

	*sink_arc = btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0x7f7f7f), 0, -10, -10);
	*lift_arc = btui_meter_add_arc_set_range(meter, scale, 20, lv_color_hex(0x7f7f7f), 0, 10, 10);

	return meter;
}

void BluethroatUi::Init(void) {
	if (pdTRUE == lvgl_acquire_token()) {
		m_flying_screen = lv_scr_act();
		lv_obj_set_style_bg_color(m_flying_screen, DEFAULT_SCREEN_BG_COLOR, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_style_bg_opa(m_flying_screen, DEFAULT_SCREEN_BG_OPACITY, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_clear_flag(m_flying_screen, LV_OBJ_FLAG_SCROLLABLE);

		m_flying_tabview = lv_tabview_create(m_flying_screen, LV_DIR_TOP, 0);
		lv_obj_set_style_bg_color(m_flying_tabview, DEFAULT_SCREEN_BG_COLOR, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_style_bg_opa(m_flying_tabview, DEFAULT_SCREEN_BG_OPACITY, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_size(m_flying_tabview, 320, 240);
		lv_obj_align_to(m_flying_tabview, m_flying_screen, LV_ALIGN_TOP_LEFT, 0, 0);

		m_flying_dashboard_tab = lv_tabview_add_tab(m_flying_tabview, "dashboard");
		lv_obj_set_style_bg_color(m_flying_dashboard_tab, DEFAULT_SCREEN_BG_COLOR, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_style_bg_opa(m_flying_dashboard_tab, DEFAULT_SCREEN_BG_OPACITY, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_size(m_flying_dashboard_tab, 320, 240);
		lv_obj_clear_flag(m_flying_dashboard_tab, LV_OBJ_FLAG_SCROLLABLE);

		m_vario_meter = bluethroat_draw_vario_meter(m_flying_dashboard_tab, m_flying_dashboard_tab, LV_ALIGN_TOP_LEFT, 4, 20, 180, 180, &m_sink_arc, &m_lift_arc);
		bluethroat_draw_label(m_flying_dashboard_tab, m_vario_meter, LV_ALIGN_CENTER, 0, 50, 80, 40, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "v-speed\n(m/s)");
		m_vario_label = bluethroat_draw_label(m_flying_dashboard_tab, m_vario_meter, LV_ALIGN_CENTER, 0, 0, 80, 40, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_40, "-99.9");

		m_flying_map_tab = lv_tabview_add_tab(m_flying_tabview, "map");
		lv_obj_set_style_bg_color(m_flying_map_tab, LV_COLOR_MAKE(0x3f, 0x00, 0x3f), LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_style_bg_opa(m_flying_map_tab, DEFAULT_SCREEN_BG_OPACITY, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_clear_flag(m_flying_map_tab, LV_OBJ_FLAG_SCROLLABLE);

		m_flying_chart_tab = lv_tabview_add_tab(m_flying_tabview, "chart");
		lv_obj_set_style_bg_color(m_flying_chart_tab, DEFAULT_SCREEN_BG_COLOR, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_set_style_bg_opa(m_flying_chart_tab, DEFAULT_SCREEN_BG_OPACITY, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
		lv_obj_clear_flag(m_flying_chart_tab, LV_OBJ_FLAG_SCROLLABLE);

		lv_obj_t *status_bar = bluethroat_draw_panel(m_flying_screen, m_flying_screen, LV_ALIGN_TOP_MID, 0, 0, 320, 24, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, 0, 0, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, 0);

		m_clock_label		= bluethroat_draw_label(status_bar, status_bar, LV_ALIGN_LEFT_MID, 4, 0, 64, 16, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_LABEL_CLOCK_COLOR, &antonio_regular_16, "23:59:59");
		m_battery_icon		= bluethroat_draw_icon(status_bar, status_bar, LV_ALIGN_RIGHT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_BATTERY_COLOR, &awesome6_16, LVGL_SYMBOL_BATTERY_THREE_QUARTERS);
	    m_charge_icon		= bluethroat_draw_icon(status_bar, m_battery_icon, LV_ALIGN_CENTER, 0, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_CHARGE_COLOR, &awesome6_16, LVGL_SYMBOL_BOLT);
		m_bluetooth_icon	= bluethroat_draw_icon(status_bar, m_battery_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_BT_DISABLED_COLOR, &awesome6_16, LVGL_SYMBOL_BLUETOOTH);
		m_gnss_icon			= bluethroat_draw_icon(status_bar, m_bluetooth_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_GNSS_DISABLED_COLOR, &awesome6_16, LVGL_SYMBOL_SATELLITE);
		m_sdcard_icon		= bluethroat_draw_icon(status_bar, m_gnss_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_SDCARD_DISABLED_COLOR, &awesome6_16, LVGL_SYMBOL_SD_CARD);
		m_volumn_icon		= bluethroat_draw_icon(status_bar, m_sdcard_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_VOLUME_COLOR, &awesome6_16, LVGL_SYMBOL_VOLUME_HIGH);
		m_lock_icon			= bluethroat_draw_icon(status_bar, m_volumn_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_CENTER, DEFAULT_ICON_LOCK_COLOR, &awesome6_16, LVGL_SYMBOL_LOCK);

		m_speed_panel		= bluethroat_draw_panel(m_flying_dashboard_tab, m_flying_screen, LV_ALIGN_TOP_RIGHT, 0, 32, 104, 64, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, DEFAULT_PANEL_RADIUS, DEFAULT_PANEL_BORDER_WIDTH, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, DEFAULT_PANEL_PADDING);
		bluethroat_draw_label(m_speed_panel, m_speed_panel, LV_ALIGN_TOP_LEFT, 0, 0, 0, 0, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "Speed(km/h)");
		m_speed_label		= bluethroat_draw_label(m_speed_panel, m_speed_panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0, 96, 40, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_RIGHT, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_40, "99.9");

		m_agl_panel			= bluethroat_draw_panel(m_flying_dashboard_tab, m_speed_panel, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 8, 104, 64, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, DEFAULT_PANEL_RADIUS, DEFAULT_PANEL_BORDER_WIDTH, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, DEFAULT_PANEL_PADDING);
		bluethroat_draw_label(m_agl_panel, m_agl_panel, LV_ALIGN_TOP_LEFT, 0, 0, 0, 0, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "Above Ground(m)");
		m_agl_label			= bluethroat_draw_label(m_agl_panel, m_agl_panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0, 96, 40, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_RIGHT, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_40, "9999");

		m_autitude_panel	= bluethroat_draw_panel(m_flying_dashboard_tab, m_agl_panel, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 8, 104, 64, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, DEFAULT_PANEL_RADIUS, DEFAULT_PANEL_BORDER_WIDTH, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, DEFAULT_PANEL_PADDING);
		bluethroat_draw_label(m_autitude_panel, m_autitude_panel, LV_ALIGN_TOP_LEFT, 0, 0, 0, 0, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "Altitude(m)");
		m_autitude_label	= bluethroat_draw_label(m_autitude_panel, m_autitude_panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0, 96, 40, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_RIGHT, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_40, "9999");

		m_distance_panel	= bluethroat_draw_panel(m_flying_dashboard_tab, m_flying_screen, LV_ALIGN_BOTTOM_LEFT, 0, 0, 100, 44, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, DEFAULT_PANEL_RADIUS, DEFAULT_PANEL_BORDER_WIDTH, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, DEFAULT_PANEL_PADDING);
		bluethroat_draw_label(m_distance_panel, m_distance_panel, LV_ALIGN_TOP_LEFT, 0, 0, 0, 0, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "Distance");
		m_distance_label	= bluethroat_draw_label(m_distance_panel, m_distance_panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0, 92, 20, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_RIGHT, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_20, "999.99 km");

		m_airtime_panel		= bluethroat_draw_panel(m_flying_dashboard_tab, m_distance_panel, LV_ALIGN_OUT_RIGHT_MID, 8, 0, 100, 44, DEFAULT_PANEL_BG_COLOR, DEFAULT_PANEL_BG_OPACITY, DEFAULT_PANEL_RADIUS, DEFAULT_PANEL_BORDER_WIDTH, DEFAULT_PANEL_BORDER_COLOR, DEFAULT_PANEL_BORDER_OPACITY, DEFAULT_PANEL_PADDING);
		bluethroat_draw_label(m_airtime_panel, m_airtime_panel, LV_ALIGN_TOP_LEFT, 0, 0, 0, 0, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_LEFT, DEFAULT_PANEL_DESCRIPTION_COLOR, &antonio_regular_12, "Airtime");
		m_airtime_label		= bluethroat_draw_label(m_airtime_panel, m_airtime_panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0, 92, 20, DEFAULT_LABEL_BG_COLOR, DEFAULT_LABEL_BG_OPACITY, DEFAULT_LABEL_PADDING, LV_TEXT_ALIGN_RIGHT, DEFAULT_PANEL_VALUE_COLOR, &antonio_regular_20, "99.99 h");

		lvgl_release_token();
	}

	g_p_BluethroatUi = this;
}

BluethroatUi * g_p_BluethroatUi = NULL;

void UiSetClock(const char * clock_string) {
	if (g_p_BluethroatUi && g_p_BluethroatUi->m_clock_label) {
		if (pdTRUE == lvgl_acquire_token()) {
			lv_label_set_text(g_p_BluethroatUi->m_clock_label, clock_string);
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetClock failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetClock failed, g_p_BluethroatUi=%p, m_clock_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_clock_label);
	}
}

void UiSetBatteryState(uint16_t battery_voltage, bool is_charging, bool is_activiting, bool is_undercurrent) {
	const char * battery_icon;
	lv_color_t battery_color = DEFAULT_ICON_BATTERY_COLOR;

	if (battery_voltage > 3950) {
		battery_icon = LVGL_SYMBOL_BATTERY_FULL;
	} else if (battery_voltage > 3820) {
		battery_icon = LVGL_SYMBOL_BATTERY_THREE_QUARTERS;
	} else if (battery_voltage > 3750) {
		battery_icon = LVGL_SYMBOL_BATTERY_HALF;
	} else if (battery_voltage > 3300) {
		battery_icon = LVGL_SYMBOL_BATTERY_QUARTER;
	} else {
		battery_icon = LVGL_SYMBOL_BATTERY_EMPTY;
		battery_color = DEFAULT_ICON_BATTERY_LOW_COLOR;
	}

	if (is_activiting || is_undercurrent) {
		battery_color = DEFAULT_ICON_BATTERY_SICK_COLOR;
	}

	if (g_p_BluethroatUi && g_p_BluethroatUi->m_battery_icon) {
		if (pdTRUE == lvgl_acquire_token()) {
			lv_label_set_text(g_p_BluethroatUi->m_battery_icon, battery_icon);
			lv_obj_set_style_text_color(g_p_BluethroatUi->m_battery_icon, battery_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetBetteryState failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetBetteryState failed, g_p_BluethroatUi=%p, m_battery_icon=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_battery_icon);
	}

	if (g_p_BluethroatUi && g_p_BluethroatUi->m_charge_icon) {
		if (pdTRUE == lvgl_acquire_token()) {
			lv_label_set_text(g_p_BluethroatUi->m_charge_icon, (is_charging ? LVGL_SYMBOL_BOLT : ""));
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetBetteryState failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetBetteryState failed, g_p_BluethroatUi=%p, m_charge_icon=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_charge_icon);
	}
}

void UiSetBluetoothState(UiBluetoothState_t state) {
	lv_color_t bluetooth_color = DEFAULT_ICON_BT_DISABLED_COLOR;

	switch (state) {
	case BLURTOOTH_STATE_DISCONNECTED:
		bluetooth_color = DEFAULT_ICON_BT_DISCONNECTED_COLOR;
		break;
	case BLURTOOTH_STATE_CONNECTED:
		bluetooth_color = DEFAULT_ICON_BT_CONNECTED_COLOR;
		break;
	default:
		break;
	}

	if (g_p_BluethroatUi && g_p_BluethroatUi->m_bluetooth_icon) {
		if (pdTRUE == lvgl_acquire_token()) {
			lv_obj_set_style_text_color(g_p_BluethroatUi->m_bluetooth_icon, bluetooth_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetBluetoothState failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetBluetoothState failed, g_p_BluethroatUi=%p, m_bluetooth_icon=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_bluetooth_icon);
	}
}

void UiSetGnssStatus(UiGnssStatus_t status) {
	lv_color_t gnss_color = DEFAULT_ICON_GNSS_DISABLED_COLOR;

	switch (status) {
	case GNSS_STATE_DISCONNECTED:
		gnss_color = DEFAULT_ICON_GNSS_DISCONNECTED_COLOR;
		break;
	case GNSS_STATE_CONNECTED:
		gnss_color = DEFAULT_ICON_GNSS_CONNECTED_COLOR;
		break;
	default:
		break;
	}

	if (g_p_BluethroatUi && g_p_BluethroatUi->m_gnss_icon) {
		if (pdTRUE == lvgl_acquire_token()) {
			lv_obj_set_style_text_color(g_p_BluethroatUi->m_gnss_icon, gnss_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetGnssStatus failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetGnssStatus failed, g_p_BluethroatUi=%p, m_gnss_icon=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_gnss_icon);
	}
}

void UiSetSpeed(float speed) {
	if (g_p_BluethroatUi && g_p_BluethroatUi->m_speed_label) {
		if (pdTRUE == lvgl_acquire_token()) {
			char speed_string[16];
			snprintf(speed_string, sizeof(speed_string), "%.2f", speed);
			lv_label_set_text(g_p_BluethroatUi->m_speed_label, speed_string);
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetSpeed failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetSpeed failed, g_p_BluethroatUi=%p, m_speed_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_speed_label);
	}
}

void UiSetAltitude(float altitude) {
	if (g_p_BluethroatUi && g_p_BluethroatUi->m_autitude_label) {
		if (pdTRUE == lvgl_acquire_token()) {
			char altitude_string[16];
			snprintf(altitude_string, sizeof(altitude_string), "%.0f", altitude);
			lv_label_set_text(g_p_BluethroatUi->m_autitude_label, altitude_string);
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetAltitude failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetAltitude failed, g_p_BluethroatUi=%p, m_autitude_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_agl_label);
	}
}

void UiSetAgl(float agl) {
	if (g_p_BluethroatUi && g_p_BluethroatUi->m_agl_label) {
		if (pdTRUE == lvgl_acquire_token()) {
			char agl_string[16];
			snprintf(agl_string, sizeof(agl_string), "%.0f", agl);
			lv_label_set_text(g_p_BluethroatUi->m_agl_label, agl_string);
			lvgl_release_token();
		} else {
			BLUETHROAT_UI_LOGE("UiSetAgl failed, lvgl_acquire_token failed");
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetAgl failed, g_p_BluethroatUi=%p, m_agl_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_agl_label);
	}
}

void UiSetVerticalSpeed(float vertical_speed) {
	if (g_p_BluethroatUi) {
		if (g_p_BluethroatUi->m_vario_label) {
			if (pdTRUE == lvgl_acquire_token()) {
				char vertical_speed_string[16];
				snprintf(vertical_speed_string, sizeof(vertical_speed_string), "%.1f", vertical_speed);
				lv_label_set_text(g_p_BluethroatUi->m_vario_label, vertical_speed_string);
				lvgl_release_token();
			} else {
				BLUETHROAT_UI_LOGE("UiSetVerticalSpeed failed, lvgl_acquire_token failed");
			}
		} else {
			BLUETHROAT_UI_LOGE("UiSetVerticalSpeed failed, g_p_BluethroatUi=%p, m_speed_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_speed_label);
		}

		if (g_p_BluethroatUi->m_sink_arc && g_p_BluethroatUi->m_lift_arc) {
			int8_t scale_value = (int8_t)round(vertical_speed);
			if (pdTRUE == lvgl_acquire_token()) {
				if (scale_value < 0) {
					lv_meter_set_indicator_end_value(g_p_BluethroatUi->m_vario_meter, g_p_BluethroatUi->m_sink_arc, scale_value);
					lv_meter_set_indicator_start_value(g_p_BluethroatUi->m_vario_meter, g_p_BluethroatUi->m_lift_arc, 0);
				} else {
					lv_meter_set_indicator_end_value(g_p_BluethroatUi->m_vario_meter, g_p_BluethroatUi->m_sink_arc, 0);
					lv_meter_set_indicator_start_value(g_p_BluethroatUi->m_vario_meter, g_p_BluethroatUi->m_lift_arc, scale_value);
				}
				lvgl_release_token();
			} else {
				BLUETHROAT_UI_LOGE("UiSetSpeed failed, lvgl_acquire_token failed");
			}
		} else {
			BLUETHROAT_UI_LOGE("UiSetSpeed failed, g_p_BluethroatUi=%p, m_sink_arc=%p, m_lift_arc=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_sink_arc, g_p_BluethroatUi->m_lift_arc);
		}
	} else {
		BLUETHROAT_UI_LOGE("UiSetSpeed failed, g_p_BluethroatUi=%p, m_speed_label=%p", g_p_BluethroatUi, g_p_BluethroatUi->m_speed_label);
	}

}