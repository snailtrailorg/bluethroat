#include "adapters/lvgl_adapter.h"
#include "bluethroat_ui.h"
#include "bluethroat_msg_proc.h"
#include  "bluethroat_clock.h"

#include "drivers/bm8563_rtc.h"
#include "drivers/dps310_barometer.h"

extern "C" void app_main(void);

void app_main() {
    lvgl_init();
    bluethroat_ui_init();
    BluethroatMsgProc * g_pBluethroatMsgProc = new BluethroatMsgProc("MSG_PROC", 4096, tskIDLE_PRIORITY, tskNO_AFFINITY, pdMS_TO_TICKS(10));
    I2cMaster * g_pI2cMaster = new I2cMaster(I2C_NUM_1, 0, 0, true, true, 400000, 10, 10);
    Bm8563Rtc * g_pBm8563Rtc = new Bm8563Rtc(g_pI2cMaster, 10, "BM8563_RTC", 2048, tskIDLE_PRIORITY, tskNO_AFFINITY, pdMS_TO_TICKS(1000*60*15), g_pBluethroatMsgProc->m_queue_handle);
    g_pBm8563Rtc->Start();
    bluethroat_clock_init();
}

