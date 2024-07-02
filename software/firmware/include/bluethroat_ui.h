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

#define DEFAULT_SCREEN_BG_COLOR             LV_COLOR_MAKE(0x00, 0x00, 0x00)
#define DEFAULT_SCREEN_BG_OPACITY           LV_OPA_COVER

#define DEFAULT_LABEL_BG_COLOR              LV_COLOR_MAKE(0x00, 0x00, 0x00)
#define DEFAULT_LABEL_BG_OPACITY            LV_OPA_TRANSP
#define DEFAULT_LABEL_PADDING               (0)
#define DEFAULT_LABEL_BORDER_WIDTH          (0)

#define DEFAULT_PANEL_BG_COLOR              LV_COLOR_MAKE(0x1F, 0x1F, 0x1F)
#define DEFAULT_PANEL_BG_OPACITY            LV_OPA_30
#define DEFAULT_PANEL_RADIUS                (3)
#define DEFAULT_PANEL_BORDER_WIDTH          (1)
#define DEFAULT_PANEL_BORDER_COLOR          LV_COLOR_MAKE(0x3F, 0x3F, 0x3F)
#define DEFAULT_PANEL_BORDER_OPACITY        LV_OPA_30
#define DEFAULT_PANEL_PADDING               (3)
#define DEFAULT_PANEL_DESCRIPTION_COLOR     LV_COLOR_MAKE(0x7F, 0x7F, 0x7F)
#define DEFAULT_PANEL_VALUE_COLOR           LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)

#define LV_SELECTOR(A, B) ((lv_style_selector_t)A | (lv_style_selector_t)B)

lv_obj_t * bluethroat_draw_panel(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t radius, lv_coord_t border_width, lv_color_t border_color, lv_opa_t border_opacity, lv_coord_t padding);
lv_obj_t * bluethroat_draw_label(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t padding, lv_text_align_t text_align, lv_color_t text_color, const lv_font_t *font, const char *text);
lv_obj_t * bluethroat_draw_icon(lv_obj_t *parent, lv_obj_t *ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_opa_t bg_opacity, lv_coord_t padding, lv_text_align_t text_align, lv_color_t text_color, const lv_font_t *font, const char *text);
lv_obj_t * bluethroat_draw_vario_meter(lv_obj_t * parent, lv_obj_t * ref, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_meter_indicator_t **sink_arc, lv_meter_indicator_t **lift_arc);

class BluethroatUi {
public:
    lv_obj_t *m_flying_screen = NULL;

    lv_obj_t *m_flying_tabview = NULL;
    lv_obj_t *m_flying_dashboard_tab = NULL;
    lv_obj_t *m_flying_map_tab = NULL;
    lv_obj_t *m_flying_chart_tab = NULL;

    lv_obj_t *m_clock_label = NULL;
    lv_obj_t *m_bettery_icon = NULL;
    lv_obj_t *m_charge_icon = NULL;
    lv_obj_t *m_bluetooth_icon = NULL;
    lv_obj_t *m_gnss_icon = NULL;
    lv_obj_t *m_volumn_icon = NULL;
    lv_obj_t *m_sdcard_icon = NULL;
    lv_obj_t *m_lock_icon = NULL;

    lv_obj_t *m_speed_panel = NULL;
    lv_obj_t *m_speed_label = NULL;
    lv_obj_t *m_agl_panel = NULL;
    lv_obj_t *m_agl_label = NULL;
    lv_obj_t *m_autitude_panel = NULL;
    lv_obj_t *m_autitude_label = NULL;
    lv_obj_t *m_airtime_panel = NULL;
    lv_obj_t *m_airtime_label = NULL;
    lv_obj_t *m_distance_panel = NULL;
    lv_obj_t *m_distance_label = NULL;

    lv_obj_t *m_vario_meter = NULL;
    lv_meter_indicator_t *m_sink_arc = NULL;
    lv_meter_indicator_t *m_lift_arc = NULL;
    lv_obj_t *m_vario_label = NULL;

    lv_obj_t *config_screen = NULL;
    lv_obj_t *config_tabview = NULL;

public:
	void Init(void);
	void SetClock(const char * clock_string);
};

extern BluethroatUi * g_p_BluethroatUi;