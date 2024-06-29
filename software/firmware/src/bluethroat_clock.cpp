#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bluethroat_ui.h"

#include "bluethroat_clock.h"

static void bluethroat_clock_task(void *arg);

void bluethroat_clock_init(void) {
    xTaskCreate(bluethroat_clock_task, "bluethroat_clock_task", 1024*2, NULL, 0, NULL);
}

static void bluethroat_clock_task(void *arg) {
    (void) arg;
    while (pdTRUE) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        time_t now = time(NULL);
        char clock_string[16];

        if (strftime(clock_string, sizeof(clock_string), "%H:%M", localtime(&now)) > 0) {
//            bluethroat_ui_set_clock(clock_string);
        }
    }
}
