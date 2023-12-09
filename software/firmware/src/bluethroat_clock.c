#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/bm8563.h"

#include "bluethroat_ui.h"

static void bluethroat_clock_task(void * arg);

void bluethroat_clock_Init(void) {
    bm_8563_init();
    xTaskCreate(bluethroat_clock_task, "bluethroat_clock_task", 1024*2, NULL, 0, NULL);
}

static void bluethroat_clock_task(void * arg) {
    (void) arg;
    while (pdTRUE) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(100));
        struct tm t;
        char clock_string[16];

        bm_8563_read_time(&t);
        if (strftime(clock_string, sizeof(clock_string), "%H:%M:%S", &t) > 0) {
            bluethroat_ui_set_clock(clock_string);
        }
    }
}
