#pragma once

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "lvgl_helpers.h"

#include "adapters/lvgl_adapter.h"

#include "res/fonts/user_font.h"
#include "res/fonts/user_font_symbols.h"
#include "res/fonts/user_font_utils.h"

/***********************************************************************************************************************
 * Force type casting to eliminate warning: bitwise operation between different enumeration types '<unnamed enum>' and 
 * '<unnamed enum>' is deprecated [-Wdeprecated-enum-enum-conversion] in C++20
***********************************************************************************************************************/
#define LV_SELECTOR(A, B) ((lv_style_selector_t)A | (lv_style_selector_t)B)

#define DEFAULT_COLOR_BG_SCREEN             LV_COLOR_MAKE(0x00, 0x00, 0x00)
#define DEFAULT_COLOR_BG_PANEL              LV_COLOR_MAKE(0x30, 0x30, 0x00)
#define DEFAULT_COLOR_BG_LABEL              LV_COLOR_MAKE(0x00, 0x30, 0x30)
#define DEFAULT_COLOR_PANEL_DESCRIPTION     LV_COLOR_MAKE(0x7F, 0x7F, 0x7F)
#define DEFAULT_COLOR_PANEL_VALUE           LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)

#define DEFAULT_OPACITY_BG_SCREEN           LV_OPA_COVER
#define DEFAULT_OPACITY_BG_PANEL            LV_OPA_80
#define DEFAULT_OPACITY_BG_LABEL            LV_OPA_TRANSP

#define DEFAULT_CORNER_RADIUS_PANEL         (2)

#define DEFAULT_PADDING_LABEL               (0)
#define DEFAULT_PADDING_PANEL               (2)

#define DEFAULT_DISTANCE_PANEL_LABEL        (4)

class LvglLabel {
public:
    lv_obj_t *m_label = NULL;

public:
    LvglLabel(lv_obj_t *parent, lv_obj_t *ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t text_align, lv_color_t color, const lv_font_t *font, const char *text);
    ~LvglLabel();

public:
    operator lv_obj_t* () { return m_label; }

public:
    void SetText(const char *text);
    void SetColor(lv_color_t color);
    void SetVisible(bool visible);
};

class LvglPanel {
public:
    lv_obj_t *m_contianer = NULL;
    lv_obj_t *m_description_label = NULL;
    lv_obj_t *m_value_label = NULL;

public:
    LvglPanel(lv_obj_t *parent, lv_obj_t *ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t description_align, const lv_font_t *description_font, const lv_color_t description_color, const char * description_string, lv_text_align_t value_align, const lv_font_t *value_font, const lv_color_t value_color, const char *value_string);
    ~LvglPanel();

public:
    operator lv_obj_t* () { return m_contianer; }

public:
    void SetValue(const char * value_string);
    void SetColor(lv_color_t color);
    void SetVisible(bool visible);
};

class LvglVarioMeter {
public:
    lv_obj_t *m_meter = NULL;
    lv_meter_indicator_t *m_sink_arc = NULL;
    lv_meter_indicator_t *m_rise_arc = NULL;
    LvglLabel *m_digit_value = NULL;
    LvglLabel *m_description = NULL;
    int32_t m_last_int_value = 0;

public:
    LvglVarioMeter(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
    ~LvglVarioMeter();

public:
    void SetValue(float value);

private:
    lv_meter_indicator_t * add_arc(lv_obj_t *meter, lv_meter_scale_t *scale, int16_t width, lv_color_t color, int16_t r_mod, int32_t start_value, int32_t end_value);
};

class BluethroatUi {
public:
    lv_obj_t *m_flying_screen = NULL;

    lv_obj_t *m_flying_tabview = NULL;
    lv_obj_t *m_flying_dashboard_tab = NULL;
    lv_obj_t *m_flying_map_tab = NULL;
    lv_obj_t *m_flying_chart_tab = NULL;

    LvglLabel *m_clock_label = NULL;
    LvglLabel *m_bettery_icon = NULL;
    LvglLabel *m_charge_icon = NULL;
    LvglLabel *m_bluetooth_icon = NULL;
    LvglLabel *m_gnss_icon = NULL;
    LvglLabel *m_volumn_icon = NULL;
    LvglLabel *m_sdcard_icon = NULL;
    LvglLabel *m_lock_icon = NULL;

    LvglPanel *m_speed_panel = NULL;
    LvglPanel *m_airtime_panel = NULL;
    LvglPanel *m_autitude_panel = NULL;
    LvglPanel *m_agl_panel = NULL;

    lv_obj_t *config_screen = NULL;
        lv_obj_t *config_tabview = NULL;

    LvglVarioMeter *m_vario_meter = NULL;
    LvglLabel *m_vario_label = NULL;

public:
	void Init(void);
	void SetClock(const char * clock_string);
};

extern BluethroatUi * g_p_BluethroatUi;