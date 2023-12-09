#include "esp_log.h"

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "lvgl_helpers.h"

#include "adapters/lvgl_adapter.h"
#include "res/fonts/user_font.h"

static struct {
    lv_obj_t * speed_meter;
    lv_obj_t * imu_meter;

    lv_obj_t * microphone_icon;
    lv_obj_t * speaker_icon;
    lv_obj_t * headphone_icon;
    lv_obj_t * xctrack_icon;
    lv_obj_t * charge_icon;
    lv_obj_t * battery_icon;

    lv_obj_t * clock_label;
    lv_obj_t * speed_label;
    lv_obj_t * pressure_label;
    lv_obj_t * temperature_label;
    lv_obj_t * humidity_label;
    lv_obj_t * altitude_label;
    lv_obj_t * compass_meter;
} bluethroat_ui;

static lv_obj_t * lvgl_draw_bar(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_color_t bar_grad_color, lv_color_t bar_color);
static lv_obj_t * lvgl_draw_label(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t text_align, lv_color_t color, const lv_font_t * font, const char * text);
static lv_obj_t * lvgl_draw_line(lv_obj_t * parent, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2, lv_coord_t width, lv_color_t color);
static lv_obj_t * lvgl_draw_icon(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, const lv_font_t * font, lv_color_t color, const char * symbol);
static lv_obj_t * lvg_draw_image(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_coord_t cx, lv_coord_t cy, const lv_img_dsc_t * image_src);

void bluethroat_ui_init(void) {
    lvgl_acquire_token();

    lv_obj_t * screen  = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    LV_IMG_DECLARE(compass_face);
    bluethroat_ui.compass_meter = lvg_draw_image(screen, screen, LV_ALIGN_TOP_LEFT, 143, 30, 75, 75, 37, 37, &compass_face);
    lv_img_set_angle(bluethroat_ui.compass_meter, 450);

    lvgl_draw_line(screen, 20, 28, 220, 28, 1, lv_color_make(127, 127, 127));
    lvgl_draw_line(screen, 20, 108, 220, 108, 1, lv_color_make(127, 127, 127));

    bluethroat_ui.speed_meter = lvgl_draw_bar(screen, screen, LV_ALIGN_TOP_LEFT, 4, 4, 12, 127, lv_color_make(63, 63, 63), lv_color_make(255, 0, 0), lv_color_make(0, 255, 0));
    bluethroat_ui.imu_meter = lvgl_draw_bar(screen, screen, LV_ALIGN_TOP_LEFT, 224, 4, 12, 127, lv_color_make(63, 63, 63), lv_color_make(255, 0, 0), lv_color_make(0, 255, 0));

    bluethroat_ui.battery_icon = lvgl_draw_icon(screen, screen, LV_ALIGN_TOP_RIGHT, -24, 8, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_BATTERY_HALF);
    bluethroat_ui.charge_icon = lvgl_draw_icon(screen, bluethroat_ui.battery_icon, LV_ALIGN_CENTER, 0, 0, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_BOLT);
    bluethroat_ui.xctrack_icon = lvgl_draw_icon(screen, bluethroat_ui.battery_icon, LV_ALIGN_OUT_LEFT_TOP, -8, 0, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_MOBILE_SCREEN_BUTTON);
    bluethroat_ui.headphone_icon = lvgl_draw_icon(screen, bluethroat_ui.xctrack_icon, LV_ALIGN_OUT_LEFT_TOP, -8, 0, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_HEADPHONES);
    bluethroat_ui.speaker_icon = lvgl_draw_icon(screen, bluethroat_ui.headphone_icon, LV_ALIGN_OUT_LEFT_TOP, -8, 0, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_VOLUME_HIGH);
    bluethroat_ui.microphone_icon = lvgl_draw_icon(screen, bluethroat_ui.speaker_icon, LV_ALIGN_OUT_LEFT_TOP, -8, 0, &awesome6_16, lv_palette_main(LV_PALETTE_GREY), LVGL_SYMBOL_MICROPHONE);

    bluethroat_ui.clock_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 24, 8, 52, 16, LV_TEXT_ALIGN_LEFT, lv_color_make(47, 218, 100), &antonio_regular_16, "23:59:59");
    bluethroat_ui.altitude_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 20, 35, 124, 68, LV_TEXT_ALIGN_CENTER, lv_color_make(192, 192, 192), &antonio_regular_68, "9999");
    bluethroat_ui.pressure_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 20, 112, 50, 16, LV_TEXT_ALIGN_CENTER, lv_color_make(47, 146, 218), &antonio_regular_16, "1013.25");
    bluethroat_ui.speed_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 70, 112, 50, 16, LV_TEXT_ALIGN_CENTER, lv_color_make(47, 202, 218), &antonio_regular_16, "+99.99");
    bluethroat_ui.temperature_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 120, 112, 50, 16, LV_TEXT_ALIGN_CENTER, lv_color_make(190, 175, 20), &antonio_regular_16, "+25.8");
    bluethroat_ui.humidity_label = lvgl_draw_label(screen, screen, LV_ALIGN_TOP_LEFT, 170, 112, 50, 16, LV_TEXT_ALIGN_CENTER, lv_color_make(38, 176, 140), &antonio_regular_16, "99.9%");

    lvgl_release_token();
}

static void set_temp(void * bar, int32_t temp)
{
    lv_bar_set_value(bar, temp, LV_ANIM_ON);
}

static lv_obj_t * lvgl_draw_bar(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t bg_color, lv_color_t bar_grad_color, lv_color_t bar_color) {
	lv_obj_t * bar = lv_bar_create(parent);

    lv_bar_set_range(bar, -100, 100);
    lv_bar_set_mode(bar, LV_BAR_MODE_SYMMETRICAL);

	lv_obj_align_to(bar, ref, ref_align, x, y);
	lv_obj_set_size(bar, w, h);

	lv_obj_set_style_bg_color(bar, bg_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(bar, bar_grad_color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar, bar_color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(bar, (w > h) ? LV_GRAD_DIR_HOR : LV_GRAD_DIR_VER, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_temp);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_playback_time(&a, 1000);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, -100, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    return bar;
}

static lv_obj_t * lvgl_draw_label(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_text_align_t text_align, lv_color_t color, const lv_font_t * font, const char * text) {
    lv_obj_t * label = lv_label_create(parent);

    lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, text_align, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_size(label, w, h);
    lv_obj_align_to(label, ref, ref_align, x, y);

    return label;
}

static lv_obj_t * lvgl_draw_line(lv_obj_t * parent, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2, lv_coord_t width, lv_color_t color) {
    lv_obj_t * line = lv_line_create(parent);

    lv_point_t * line_points = malloc(sizeof(lv_point_t) * 2);
    line_points[0].x = x1; line_points[0].y = y1;
    line_points[1].x = x2; line_points[1].y = y2;
    lv_line_set_points(line, line_points, 2);

    lv_obj_set_style_line_width(line, width, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(line, color, LV_PART_MAIN | LV_STATE_DEFAULT);

    return line;
}

static lv_obj_t * lvgl_draw_icon(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, const lv_font_t * font, lv_color_t color, const char * symbol) {
    lv_obj_t * icon = lv_label_create(parent);

    lv_obj_set_style_pad_all(icon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(icon, LV_LABEL_LONG_CLIP);

    lv_label_set_text(icon, symbol);
    lv_obj_set_style_text_font(icon, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(icon, color, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_width(icon, lvgl_get_symbol_width(font, lvgl_symbol_utf8_to_unicode(symbol)));
    lv_obj_align_to(icon, ref, ref_align, x, y);

    return icon;
}

static lv_obj_t * lvg_draw_image(lv_obj_t * parent, lv_obj_t * ref, lv_align_t ref_align, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_coord_t cx, lv_coord_t cy, const lv_img_dsc_t * image_src) {
    lv_obj_t * image = lv_img_create(parent);

    lv_img_set_src(image, image_src);

    lv_obj_set_size(image, w, h);
    lv_img_set_pivot(image, cx, cy);
    lv_obj_align_to(image, ref, ref_align, x, y);

    return image;
}

void bluethroat_ui_set_clock(const char * clock_string) {
    lvgl_acquire_token();
    lv_label_set_text(bluethroat_ui.clock_label,clock_string);
    lvgl_release_token();
}