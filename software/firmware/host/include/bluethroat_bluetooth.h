#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <services/gatt/ble_svc_gatt.h>

void bluetooth_init(QueueHandle_t queue_handle);
void bluetooth_deinit(void);
int BluetoothSendPressure(float pressure);
int BluetoothSendGnssNmea(const char *nmea);
