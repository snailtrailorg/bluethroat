#include "adapters/lvgl_adapter.h"

#include "bluethroat_ui.h"
#include  "bluethroat_clock.h"

extern "C" void app_main(void);

void app_main() {
    lvgl_init();
    bluethroat_ui_init();
    bluethroat_clock_init();
}

