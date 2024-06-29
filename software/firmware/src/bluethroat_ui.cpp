#include <math.h>
#include <esp_log.h>

#include "drivers/axp192_pmu.h"

#include "bluethroat_ui.h"

LvglLabel::LvglLabel(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t text_align, lv_color_t color, const lv_font_t * font, const char * text) {
    lv_obj_t * label = lv_label_create(parent);

	lv_obj_set_style_bg_color(label, DEFAULT_COLOR_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(label, DEFAULT_OPACITY_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_set_style_pad_all(label, DEFAULT_PADDING_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
	lv_obj_set_style_border_width(label, 1, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_align(label, text_align, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
    lv_obj_set_style_text_color(label, color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

    lv_obj_set_width(label, (w == 0 ? lvgl_get_symbol_width(font, text) : w));
	lv_obj_set_height(label, (h == 0 ? lvgl_get_symbol_height(font, text) : h));
    lv_obj_align_to(label, ref, ref_align, x, y);
	lv_obj_update_layout(label);

	m_label = label;
}

LvglLabel::~LvglLabel() {
	if (m_label) {
		lv_obj_del(m_label);
	}
}

void LvglLabel::SetText(const char * text) {
	lv_label_set_text(m_label, text);
}

void LvglLabel::SetColor(lv_color_t color) {
	lv_obj_set_style_text_color(m_label, color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
}

void LvglLabel::SetVisible(bool visible) {
	if (visible) {
		lv_obj_clear_flag(m_label, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(m_label, LV_OBJ_FLAG_HIDDEN);
	}
}

LvglPanel::LvglPanel(lv_obj_t *parent, lv_obj_t *ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t description_align, const lv_font_t *description_font, const lv_color_t description_color, const char * description_string, lv_text_align_t value_align, const lv_font_t *value_font, const lv_color_t value_color, const char *value_string) {
	lv_obj_t * container = lv_obj_create(parent);
	lv_obj_set_style_bg_color(container, DEFAULT_COLOR_BG_PANEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(container, DEFAULT_OPACITY_BG_PANEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_radius(container, DEFAULT_PADDING_PANEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_border_width(container, 1, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_size(container, w, h);
	lv_obj_align_to(container, ref, ref_align, x, y);

	lv_obj_t * description_label = lv_label_create(container);
	lv_obj_set_style_bg_opa(description_label, DEFAULT_OPACITY_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_color(description_label, DEFAULT_COLOR_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_pad_all(description_label, DEFAULT_PADDING_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_label_set_long_mode(description_label, LV_LABEL_LONG_CLIP);
	lv_label_set_text(description_label, description_string);
	lv_obj_set_style_text_font(description_label, description_font, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_text_align(description_label, description_align, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_text_color(description_label, description_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_width(description_label, w, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_align_to(description_label, container, LV_ALIGN_TOP_LEFT, 0, 0);

	lv_obj_t * value_label = lv_label_create(container);
	lv_obj_set_style_bg_opa(value_label, DEFAULT_OPACITY_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_color(value_label, DEFAULT_COLOR_BG_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_pad_all(value_label, DEFAULT_PADDING_LABEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_label_set_long_mode(value_label, LV_LABEL_LONG_CLIP);
	lv_label_set_text(value_label, value_string);
	lv_obj_set_style_text_font(value_label, value_font, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_text_align(value_label, value_align, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_text_color(value_label, value_color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_width(value_label, w, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_align_to(value_label, container, LV_ALIGN_BOTTOM_LEFT, 0, 0);

	m_contianer = container;
	m_description_label = description_label;
	m_value_label = value_label;
}

LvglPanel::~LvglPanel() {
	if (m_contianer) {
		lv_obj_del(m_contianer);
	}
}

void LvglPanel::SetValue(const char * value_string) {
	lv_label_set_text(m_value_label, value_string);
}

void LvglPanel::SetColor(lv_color_t color) {
	lv_obj_set_style_text_color(m_description_label, color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_text_color(m_value_label, color, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
}

void LvglPanel::SetVisible(bool visible) {
	if (visible) {
		lv_obj_clear_flag(m_contianer, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(m_contianer, LV_OBJ_FLAG_HIDDEN);
	}
}

LvglVarioMeter::LvglVarioMeter(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
	lv_obj_t *meter = lv_meter_create(parent);
	lv_obj_set_size(meter, w, h);
    lv_obj_align_to(meter, ref, ref_align, x, y);

	lv_meter_scale_t *scale = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale, 21, 1, 20, lv_color_hex(0x000000));
	lv_meter_set_scale_major_ticks(meter, scale, 2, 1, 20, lv_color_hex(0x000000), 15);
	lv_meter_set_scale_range(meter, scale, -10, 10, 270, 135);

	lv_obj_set_style_bg_opa(meter, 255, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_color(meter, lv_color_hex(0x000000), LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_radius(meter, 100, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_border_width(meter, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));

	lv_obj_set_style_text_color(meter, lv_color_hex(0x3affe7), LV_SELECTOR(LV_PART_TICKS, LV_STATE_DEFAULT));
	lv_obj_set_style_text_font(meter, &antonio_regular_14, LV_SELECTOR(LV_PART_TICKS, LV_STATE_DEFAULT));
	//lv_obj_set_style_text_opa(meter, 255, LV_PART_TICKS | LV_STATE_DEFAULT);

	//lv_obj_set_style_bg_opa(meter, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	//lv_obj_set_style_bg_color(meter, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	//lv_obj_set_style_bg_grad_dir(meter, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	add_arc(meter, scale, 20, lv_color_hex(0xff7f1f), 0, -10, -8);
	add_arc(meter, scale, 20, lv_color_hex(0xffcf1f), 0, -8, 0);
	add_arc(meter, scale, 20, lv_color_hex(0x1fff7f), 0, 0, 8);
	add_arc(meter, scale, 20, lv_color_hex(0x1fcfff), 0, 8, 10);

	m_sink_arc = add_arc(meter, scale, 20, lv_color_hex(0x7f7f7f), 0, -10, -10);
	m_rise_arc = add_arc(meter, scale, 20, lv_color_hex(0x7f7f7f), 0, 10, 10);

	m_meter = meter;
}

LvglVarioMeter::~LvglVarioMeter() {
	if (m_meter) {
		lv_obj_del(m_meter);
	}
	if (m_digit_value) {
		delete m_digit_value;
	}
	if (m_description) {
		delete m_description;
	}
}

lv_meter_indicator_t * LvglVarioMeter::add_arc(lv_obj_t *meter, lv_meter_scale_t *scale, int16_t width, lv_color_t color, int16_t r_mod, int32_t start_value, int32_t end_value) {
	lv_meter_indicator_t *arc = lv_meter_add_arc(meter, scale, width, color, r_mod);
	lv_meter_set_indicator_start_value(meter, arc, start_value);
	lv_meter_set_indicator_end_value(meter, arc, end_value);

	return arc;
}

void LvglVarioMeter::SetValue(float value) {
	if (m_digit_value) {
		char string[16];
		sprintf(string, "%.1f", value);
		m_digit_value->SetText(string);
	}

	if (m_meter) {
		int32_t int_value;

		if (value < 0.0) {
			int_value = (int32_t)(floor(value));
		} else {
			int_value = (int32_t)(ceil(value));
		}

		if (int_value != m_last_int_value) {
			if (int_value < 0) {
				lv_meter_set_indicator_end_value(m_meter, m_sink_arc, int_value);
				if (m_last_int_value > 0) lv_meter_set_indicator_start_value(m_meter, m_rise_arc, 0);
			} else {
				lv_meter_set_indicator_start_value(m_meter, m_rise_arc, int_value);
				if (m_last_int_value < 0) lv_meter_set_indicator_end_value(m_meter, m_sink_arc, 0);
			}
			m_last_int_value = int_value;
		}
	}
}

void BluethroatUi::Init(void) {
	m_flying_screen = lv_scr_act();
	lv_obj_set_style_bg_color(m_flying_screen, DEFAULT_COLOR_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(m_flying_screen, DEFAULT_OPACITY_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_clear_flag(m_flying_screen, LV_OBJ_FLAG_SCROLLABLE);

    m_flying_tabview = lv_tabview_create(m_flying_screen, LV_DIR_TOP, 0);
	lv_obj_set_style_bg_color(m_flying_tabview, DEFAULT_COLOR_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(m_flying_tabview, DEFAULT_OPACITY_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_size(m_flying_tabview, 320, 240);
	lv_obj_align_to(m_flying_tabview, m_flying_screen, LV_ALIGN_TOP_LEFT, 0, 0);

	m_flying_dashboard_tab = lv_tabview_add_tab(m_flying_tabview, "dashboard");
	lv_obj_set_style_bg_color(m_flying_dashboard_tab, DEFAULT_COLOR_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(m_flying_dashboard_tab, DEFAULT_OPACITY_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_clear_flag(m_flying_dashboard_tab, LV_OBJ_FLAG_SCROLLABLE);

	m_vario_meter = new LvglVarioMeter(m_flying_dashboard_tab, m_flying_dashboard_tab, LV_ALIGN_CENTER, 0, 20, 200, 200);

	m_flying_map_tab = lv_tabview_add_tab(m_flying_tabview, "map");
	lv_obj_set_style_bg_color(m_flying_map_tab, DEFAULT_COLOR_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(m_flying_map_tab, DEFAULT_OPACITY_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_clear_flag(m_flying_map_tab, LV_OBJ_FLAG_SCROLLABLE);

	m_flying_chart_tab = lv_tabview_add_tab(m_flying_tabview, "chart");
	lv_obj_set_style_bg_color(m_flying_chart_tab, DEFAULT_COLOR_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(m_flying_chart_tab, DEFAULT_OPACITY_BG_SCREEN, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_clear_flag(m_flying_chart_tab, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *status_bar = lv_obj_create(m_flying_screen);
	lv_obj_set_style_bg_color(status_bar, DEFAULT_COLOR_BG_PANEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_bg_opa(status_bar, DEFAULT_OPACITY_BG_PANEL, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_size(status_bar, 320, 20);
	lv_obj_set_style_pad_all(status_bar, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_radius(status_bar, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_set_style_border_width(status_bar, 0, LV_SELECTOR(LV_PART_MAIN, LV_STATE_DEFAULT));
	lv_obj_align_to(status_bar, m_flying_screen, LV_ALIGN_TOP_LEFT, 0, 0);

	m_clock_label		= new LvglLabel(status_bar, status_bar, LV_ALIGN_LEFT_MID, 0, 0, 60, 16, LV_TEXT_ALIGN_LEFT, lv_color_hex(0x3affe7), &antonio_regular_16, "23:59");
    m_bettery_icon		= new LvglLabel(status_bar, status_bar, LV_ALIGN_RIGHT_MID, 0, 0, 0, 18, LV_TEXT_ALIGN_CENTER, lv_color_hex(0x00ff00), &awesome6_16, LVGL_SYMBOL_BATTERY_THREE_QUARTERS);
    m_charge_icon		= new LvglLabel(status_bar, status_bar, LV_ALIGN_RIGHT_MID, 0, 0, 18, 18, LV_TEXT_ALIGN_CENTER, lv_color_hex(0xffffff), &awesome6_16, LVGL_SYMBOL_BOLT);
    m_bluetooth_icon	= new LvglLabel(status_bar, m_bettery_icon->m_label, LV_ALIGN_OUT_LEFT_MID, -4, 0, 0, 18, LV_TEXT_ALIGN_CENTER, lv_color_hex(0x0000ff), &awesome6_16, LVGL_SYMBOL_BLUETOOTH);

    m_gnss_icon			= NULL;
    m_volumn_icon		= NULL;
    m_sdcard_icon		= NULL;
    m_lock_icon			= NULL;

	//m_speed_panel		= new LvglPanel(m_flying_screen, m_flying_screen, LV_ALIGN_TOP_LEFT, 0, 36, 64, 60, LV_TEXT_ALIGN_LEFT, &antonio_regular_12, lv_color_hex(0x7f7f7f), "Speed(m/s)", LV_TEXT_ALIGN_LEFT, &antonio_regular_32, lv_color_hex(0x3affe7), "99.9");

	g_p_BluethroatUi = this;
}

void BluethroatUi::SetClock(const char * clock_string) {
	if (m_clock_label) {
		m_clock_label->SetText(clock_string);
	}
}

BluethroatUi * g_p_BluethroatUi = NULL;