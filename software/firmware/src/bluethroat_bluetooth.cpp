#include <esp_err.h>
#include <esp_log.h>

#include "freertos/FreeRTOSConfig.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_uuid.h"
#include "host/ble_gap.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nimble/ble.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "bluethroat_message.h"

#include "bluethroat_bluetooth.h"

#define BLUETOOTH_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define BLUETOOTH_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define BLUETOOTH_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define BLUETOOTH_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define BLUETOOTH_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define BLUETOOTH_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define BLUETOOTH_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define BLUETOOTH_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define BLUETOOTH_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define BLUETOOTH_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define BLUETOOTH_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			BLUETOOTH_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define BLUETOOTH_ASSERT(condition, format, ...)
#endif

static const char *TAG = "BLUETOOTH";

#ifdef __cplusplus
extern "C" {
#endif

static int gatt_access_manufacturer_name(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_access_model_number(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_access_pressure(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_access_nordic_tx(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int bluetooth_gap_event(struct ble_gap_event *event, void *arg);
static void bluetooth_advertise(void);
static void bluetooth_on_sync(void);
static void bluetooth_on_reset(int reason);
static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
static void bluetooth_host_task(void *parameter);
static void bluetooth_report_state();

#ifdef __cplusplus
}
#endif



static QueueHandle_t bluethroat_queue_handle = NULL;

/**************************************************************************************************
    Supported BLE services & characteristics:
        Environmental Sensing Service       (0000181a-0000-1000-8000-00805f9b34fb)
            Pressure characteristic         (00002a6d-0000-1000-8000-00805f9b34fb)
        Battery Service                     (0000180f-0000-1000-8000-00805f9b34fb)
            Battery level characteristic    (00002a19-0000-1000-8000-00805f9b34fb)
        FBMini Airspeed Service             (00001819-0000-1000-8000-00805f9b34fb)
            TAS characteristic              (234337bf-f931-4d2d-a13c-07e2f06a0249)
        Nordic UART Service                 (6e400001-b5a3-f393-e0a9-e50e24dcca9e)
            TX characteristic               (6e400003-b5a3-f393-e0a9-e50e24dcca9e)
        LeBip service + characteristic
        Skydrop (1&2) service + characteristic
        RN4781 service + characteristic
        XCTracer service + characteristic
    XCTrack discovers services and subscribes for notifications for these characteristics. 
    See a log file (or the log widget) for details about connection happening.
**************************************************************************************************/
static const ble_uuid128_t gatt_device_information_service_uuid        = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x0a, 0x18, 0x00 ,0x00);
static const ble_uuid128_t gatt_manufacturer_name_characteristic_uuid  = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x29, 0x2a, 0x00 ,0x00);
static const ble_uuid128_t gatt_model_number_characteristic_uuid       = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x24, 0x2a, 0x00 ,0x00);
static const ble_uuid128_t gatt_environmental_sensing_service_uuid     = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x1a, 0x18, 0x00 ,0x00);
static const ble_uuid128_t gatt_pressure_characteristic_uuid           = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x6d, 0x2a, 0x00 ,0x00);
static const ble_uuid128_t gatt_battery_service_uuid                   = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x0f, 0x18, 0x00 ,0x00);
static const ble_uuid128_t gatt_battery_level_characteristic_uuid      = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x19, 0x2a, 0x00 ,0x00);
static const ble_uuid128_t gatt_fbmini_airspeed_service_uuid           = BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x19, 0x18, 0x00 ,0x00);
static const ble_uuid128_t gatt_fbmini_tas_characteristic_uuid         = BLE_UUID128_INIT(0x49, 0x02, 0x6a, 0xf0, 0xe2, 0x07, 0x3c, 0xa1, 0x2d, 0x4d, 0x31, 0xf9, 0xbf, 0x37, 0x43 ,0x23);
static const ble_uuid128_t gatt_nordic_uart_service_uuid               = BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40 ,0x6e);
static const ble_uuid128_t gatt_nordic_tx_characteristic_uuid          = BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40 ,0x6e);

static const char *device_name = "Bluethroat";
static const char *manufacturer_name = "SnailTrail.ORG";
static const char *model_number = "Bluethroat Variometer";

static uint8_t address_type;

static uint16_t connection_handle = BLE_HS_CONN_HANDLE_NONE;

static uint16_t pressure_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t battery_level_handle;
static uint16_t fbmini_tas_handle;
static uint16_t nordic_tx_handle;

static bool pressure_notify_state;
static bool battery_level_notify_state;
static bool fbmini_tas_notify_state;
static bool nordic_tx_notify_state;

static int gatt_access_manufacturer_name(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ble_uuid_cmp(ctxt->chr->uuid, &(gatt_manufacturer_name_characteristic_uuid.u)) == 0) {
        int result = os_mbuf_append(ctxt->om, manufacturer_name, strlen(manufacturer_name));
        return (result == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else {
        char buffer[BLE_UUID_STR_LEN];
        BLUETOOTH_LOGE("Receive access request of unknown characteristic: %s", ble_uuid_to_str(ctxt->chr->uuid, buffer));
        return BLE_ATT_ERR_UNLIKELY;
    }
}

static int gatt_access_model_number(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ble_uuid_cmp(ctxt->chr->uuid, &(gatt_model_number_characteristic_uuid.u)) == 0) {
        int result = os_mbuf_append(ctxt->om, model_number, strlen(model_number));
        return (result == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else {
        char buffer[BLE_UUID_STR_LEN];
        BLUETOOTH_LOGE("Receive access request of unknown characteristic: %s", ble_uuid_to_str(ctxt->chr->uuid, buffer));
        return BLE_ATT_ERR_UNLIKELY;
    }
}

static int gatt_access_pressure(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ble_uuid_cmp(ctxt->chr->uuid, &(gatt_pressure_characteristic_uuid.u)) == 0) {
        /* set default pressure is 101325 pascal, it is not a exact value, but no effect */
        static const char *pressure = "PRS 18BCD\n";
        int result = os_mbuf_append(ctxt->om, &pressure, sizeof(pressure));
        return (result == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else {
        char buffer[BLE_UUID_STR_LEN];
        BLUETOOTH_LOGE("Receive access request of unknown characteristic: %s", ble_uuid_to_str(ctxt->chr->uuid, buffer));
        return BLE_ATT_ERR_UNLIKELY;
    }
}

static int gatt_access_nordic_tx(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ble_uuid_cmp(ctxt->chr->uuid, &(gatt_nordic_tx_characteristic_uuid.u)) == 0) {
        static const char *gnss_nmea = "$GPGLL,3854.777,N,07702.464,W,171848.935,V*34\n";
        int result = os_mbuf_append(ctxt->om, gnss_nmea, strlen(gnss_nmea));
        return (result == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else {
        char buffer[BLE_UUID_STR_LEN];
        BLUETOOTH_LOGE("Receive access request of unknown characteristic: %s", ble_uuid_to_str(ctxt->chr->uuid, buffer));
        return BLE_ATT_ERR_UNLIKELY;
    }    
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const struct ble_gatt_svc_def gatt_svr_svcs[5] =
{
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &(gatt_device_information_service_uuid.u),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: * Manufacturer name */
                .uuid = &(gatt_manufacturer_name_characteristic_uuid.u),
                .access_cb = gatt_access_manufacturer_name,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                /* Characteristic: Model number string */
                .uuid = &(gatt_model_number_characteristic_uuid.u),
                .access_cb = gatt_access_model_number,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        /* Service: Environmental Sensing */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &(gatt_environmental_sensing_service_uuid.u),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: Pressure */
                .uuid = &(gatt_pressure_characteristic_uuid.u),
                .access_cb = gatt_access_pressure,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &pressure_handle,
            },
            {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        /* Service: Nordic UART */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &(gatt_nordic_uart_service_uuid.u),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: UART TX */
                .uuid = &(gatt_nordic_tx_characteristic_uuid.u),
                .access_cb = gatt_access_nordic_tx,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &nordic_tx_handle,
            },
            {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        .type = 0, /* No more services */
    },
};

#pragma GCC diagnostic pop

static void bluetooth_report_state() {
    if (bluethroat_queue_handle) {
        BluethroatMsg_t message;

        message.type = BLUETHROAT_MSG_TYPE_BLUETOOTH_STATE;

        if (connection_handle == BLE_HS_CONN_HANDLE_NONE) {
            message.bluetooth_state.environment_service_state = SERVICE_STATE_DISCONNECTED;
            message.bluetooth_state.nordic_uart_service_state = SERVICE_STATE_DISCONNECTED;
        } else {
            message.bluetooth_state.environment_service_state = pressure_notify_state ? SERVICE_STATE_CONNECTED : SERVICE_STATE_DISCONNECTED;
            message.bluetooth_state.nordic_uart_service_state = nordic_tx_notify_state ? SERVICE_STATE_CONNECTED : SERVICE_STATE_DISCONNECTED;
        }

        (void)xQueueSend(bluethroat_queue_handle, &message, 0);
    }
}

static int bluetooth_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        BLUETOOTH_LOGI("Connection %s; status=%d; handle=%d", (event->connect.status == 0) ? "established" : "failed", event->connect.status, event->connect.conn_handle);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            connection_handle = BLE_HS_CONN_HANDLE_NONE;

            pressure_notify_state = false;
            battery_level_notify_state = false;
            fbmini_tas_notify_state = false;
            nordic_tx_notify_state = false;

            bluetooth_advertise();
        } else {
            connection_handle = event->connect.conn_handle;
        }

        bluetooth_report_state();

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        BLUETOOTH_LOGI("Disconnect; reason=%d", event->disconnect.reason);

        /* Connection terminated; resume advertising */
        connection_handle = BLE_HS_CONN_HANDLE_NONE;

        pressure_notify_state = false;
        battery_level_notify_state = false;
        fbmini_tas_notify_state = false;
        nordic_tx_notify_state = false;

        bluetooth_advertise();
        bluetooth_report_state();

        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        BLUETOOTH_LOGI("Advertise complete, restart it.");

        bluetooth_advertise();
        
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        BLUETOOTH_LOGI("Subscribe event; cur_notify=%d, attr_handle=%d", event->subscribe.cur_notify, event->subscribe.attr_handle);

        if (event->subscribe.attr_handle == pressure_handle) {
            pressure_notify_state = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == battery_level_handle) {
            battery_level_notify_state = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == fbmini_tas_handle) {
            fbmini_tas_notify_state = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == nordic_tx_handle) {
            nordic_tx_notify_state = event->subscribe.cur_notify;
        }

        bluetooth_report_state();

        break;

    case BLE_GAP_EVENT_MTU:
        BLUETOOTH_LOGI("Mtu update event; conn_handle=%d mtu=%d", event->mtu.conn_handle, event->mtu.value);

        break;
    }

    return 0;
}

static void bluetooth_advertise(void) {
    struct ble_hs_adv_fields fields;
    struct ble_gap_adv_params adv_params;
    int result;

    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    result = ble_gap_adv_set_fields(&fields);
    if (result != 0) {
        BLUETOOTH_LOGE("Error setting advertisement data; result=%d", result);
        return;
    }

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    result = ble_gap_adv_start(address_type, NULL, BLE_HS_FOREVER, &adv_params, bluetooth_gap_event, NULL);
    if (result != 0) {
        BLUETOOTH_LOGE("Error enabling advertisement; result=%d", result);
        return;
    }
}

static void bluetooth_on_sync(void) {
    uint8_t addr[6] = {0};

    ESP_ERROR_CHECK(ble_hs_id_infer_auto(0, &address_type));
    ESP_ERROR_CHECK(ble_hs_id_copy_addr(address_type, addr, NULL));

    BLUETOOTH_LOGI("Device Address: %02x-%02x-%02x-%02x-%02x-%02x", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

    /* Begin advertising */
    bluetooth_advertise();
}

static void bluetooth_on_reset(int reason) {
    BLUETOOTH_LOGI("Resetting state, reason=%d", reason);
}


static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    char buffer[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        BLUETOOTH_LOGI("Registered service %s with handle=%d", ble_uuid_to_str(ctxt->svc.svc_def->uuid, buffer), ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        BLUETOOTH_LOGI("Registering characteristic %s with " "def_handle=%d val_handle=%d", ble_uuid_to_str(ctxt->chr.chr_def->uuid, buffer), ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        BLUETOOTH_LOGI("Registering descriptor %s with handle=%d", ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buffer), ctxt->dsc.handle);
        break;

    default:
        BLUETOOTH_LOGI("Registering unknown operation %d, uuid: %s", ctxt->op, ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buffer));
        break;
    }
}

void bluetooth_host_task(void *parameter) {
    BLUETOOTH_LOGI("BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void bluetooth_init(QueueHandle_t queue_handle) {
    /* Initialize controller and NimBLE host stack */
    ESP_ERROR_CHECK(nimble_port_init());

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.sync_cb = bluetooth_on_sync;
    ble_hs_cfg.reset_cb = bluetooth_on_reset;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;

    /* Initialize the GAP and GATT service. */
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ESP_ERROR_CHECK(ble_gatts_count_cfg(gatt_svr_svcs));
    ESP_ERROR_CHECK(ble_gatts_add_svcs(gatt_svr_svcs));

    /* Set the default device name */
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(device_name));

    /* Start the task */
    nimble_port_freertos_init(bluetooth_host_task);

    bluethroat_queue_handle = queue_handle;
    bluetooth_report_state();
}

void bluetooth_deinit() {
    /* Deinitialize controller and NimBLE host stack */
    ESP_ERROR_CHECK(nimble_port_deinit());
}

int BluetoothSendPressure(float pressure) {
    uint32_t pressure_data = (uint32_t)(pressure * 10);
    struct os_mbuf * om;

    if (pressure_notify_state) {
        om = ble_hs_mbuf_from_flat(&pressure_data, sizeof(pressure_data));
        return ble_gattc_notify_custom(connection_handle, pressure_handle, om);
    } else {
        return ESP_OK;
    }
}

int BluetoothSendGnssNmea(const char *nmea) {
    struct os_mbuf * om;

    if (nordic_tx_notify_state) {
        om = ble_hs_mbuf_from_flat(nmea, strlen(nmea));
        return ble_gattc_notify_custom(connection_handle, nordic_tx_handle, om);
    } else {
        return ESP_OK;
    }
}