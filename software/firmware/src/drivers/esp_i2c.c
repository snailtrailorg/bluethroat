/*

SPDX-License-Identifier: MIT

MIT License

Copyright (c) 2021 Rop Gonggrijp. Based on esp_i2c_helper by Mika Tuupola.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/*
Because the usage scenarios are relatively simple, full multitasking support 
is not considered.

The initialization operation only needs to be done once, and then each task 
can read and write the I2C bus to access different or same device.

This mechanism leads to the same configuration on the same I2C port, e.g.:
frequency, timeout, pullup-resistors etc., no matter how many devices are 
connected to the port.

However, in a well-designed application system, it is more efficient to use 
the same configuration parameters for the same I2C port without frequent 
parameter adjustments.

michael@snailtrail.org
*/

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <driver/i2c.h>

#include "drivers/esp_i2c.h"

#if defined __has_include
	#if __has_include ("esp_idf_version.h")
		#include "esp_idf_version.h"
		#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
			#define HAS_CLK_FLAGS
		#endif
	#endif
#endif

#define ESP_I2C_LOGE(format, ...) ESP_LOGE(esp_i2c_tag, format, ##__VA_ARGS__)
#define ESP_I2C_LOGW(format, ...) ESP_LOGW(esp_i2c_tag, format, ##__VA_ARGS__)
#define ESP_I2C_LOGI(format, ...) ESP_LOGI(esp_i2c_tag, format, ##__VA_ARGS__)
#define ESP_I2C_LOGD(format, ...) ESP_LOGD(esp_i2c_tag, format, ##__VA_ARGS__)
#define ESP_I2C_LOGV(format, ...) ESP_LOGV(esp_i2c_tag, format, ##__VA_ARGS__)

#define ESP_I2C_BUFFER_LOGE(buffer, buff_len) ESP_LOG_BUFFER_HEX_LEVEL(esp_i2c_tag, buffer, buff_len, ESP_LOG_ERROR)
#define ESP_I2C_BUFFER_LOGW(buffer, buff_len) ESP_LOG_BUFFER_HEX_LEVEL(esp_i2c_tag, buffer, buff_len, ESP_LOG_WARN)
#define ESP_I2C_BUFFER_LOGI(buffer, buff_len) ESP_LOG_BUFFER_HEX_LEVEL(esp_i2c_tag, buffer, buff_len, ESP_LOG_INFO)
#define ESP_I2C_BUFFER_LOGD(buffer, buff_len) ESP_LOG_BUFFER_HEX_LEVEL(esp_i2c_tag, buffer, buff_len, ESP_LOG_DEBUG)
#define ESP_I2C_BUFFER_LOGV(buffer, buff_len) ESP_LOG_BUFFER_HEX_LEVEL(esp_i2c_tag, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define ESP_I2C_ASSERT(condition, format, ...) do { if (!(condition)) { ESP_I2C_LOGE(format, ##__VA_ARGS__); assert(0);} } while (0)
#else
#define ESP_I2C_ASSERT(condition, format, ...) void
#endif

static inline esp_err_t esp_i2c_lock(i2c_port_t port);
static inline esp_err_t esp_i2c_unlock(i2c_port_t port);
static inline void esp_i2c_send_address(i2c_cmd_handle_t cmd, uint16_t addr, i2c_rw_t rw);
static inline void esp_i2c_send_register(i2c_cmd_handle_t cmd, uint32_t reg);

static const char * esp_i2c_tag = "ESP_I2C";

static SemaphoreHandle_t esp_i2c_mutex[I2C_NUM_MAX];

esp_err_t esp_i2c_init(i2c_port_t port) {

	ESP_I2C_ASSERT(port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_NUM_MAX-1);
	ESP_I2C_ASSERT((port == I2C_NUM_0 && CONFIG_ESP_I2C_PORT_0_ENABLED) || (port == I2C_NUM_1 && CONFIG_ESP_I2C_PORT_1_ENABLED), "I2C port %d not enabled in config file.", port);
	ESP_I2C_ASSERT(esp_i2c_mutex[port] == NULL, "I2C port %d is already initialized.", port);

	esp_err_t result = ESP_OK;

	if (esp_i2c_mutex[port] == NULL) {

		ESP_I2C_LOGI("Starting I2C master %d.", port);

		esp_i2c_mutex[port] = xSemaphoreCreateMutex();

		i2c_config_t conf = {0};

		conf.mode = I2C_MODE_MASTER;

		if (port == I2C_NUM_0) {
			conf.sda_io_num = CONFIG_ESP_I2C_PORT_0_SDA;
			conf.scl_io_num = CONFIG_ESP_I2C_PORT_0_SCL;
			conf.master.clk_speed = CONFIG_ESP_I2C_PORT_0_FREQ_HZ;
			#ifdef CONFIG_ESP_I2C_PORT_0_PULLUPS
			conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
			#else
			conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
			#endif
			conf.scl_pullup_en = conf.sda_pullup_en;
		} else if (port == I2C_NUM_1) {
			conf.sda_io_num = CONFIG_ESP_I2C_PORT_1_SDA;
			conf.scl_io_num = CONFIG_ESP_I2C_PORT_1_SCL;
			conf.master.clk_speed = CONFIG_ESP_I2C_PORT_1_FREQ_HZ;
			#ifdef CONFIG_ESP_I2C_PORT_1_PULLUPS
			conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
			#else
			conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
			#endif
			conf.scl_pullup_en = conf.sda_pullup_en;
		}

		#ifdef HAS_CLK_FLAGS
			conf.clk_flags = 0;
		#endif

		result = i2c_param_config(port, &conf);
		result |= i2c_driver_install(port, conf.mode, 0, 0, 0);

		if (result != ESP_OK) {
			ESP_I2C_LOGE("Failed to initialise I2C port %d.", port);
		} else {
			ESP_I2C_LOGI("Initialised I2C port %d (SDA: %d, SCL: %d, speed: %lu Hz.)", port, conf.sda_io_num, conf.scl_io_num, conf.master.clk_speed);
		}
	}

    return result;
}

esp_err_t esp_i2c_deinit(i2c_port_t port) {
	ESP_I2C_ASSERT(port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_NUM_MAX-1);
	ESP_I2C_ASSERT((port == I2C_NUM_0 && CONFIG_ESP_I2C_PORT_0_ENABLED) || (port == I2C_NUM_1 && CONFIG_ESP_I2C_PORT_1_ENABLED), "I2C port %d not enabled in config file.", port);
	ESP_I2C_ASSERT(esp_i2c_mutex[port], "I2C port %d is not initialized.", port);

	/* 
	There is some risk here, if some other task locks the port and for some reason does not 
	release it in time, it will lead to unpredictable errors.
	Therefore, the caller must ensure that the task of reading and writing operations on 
	the port and the task of deinitialization cannot fall into a deadlock state of waiting 
	for each other, and the read and write operations should be as simple and efficient as 
	possible. 
	*/
    xSemaphoreTake(esp_i2c_mutex[port], portMAX_DELAY);
    vSemaphoreDelete(esp_i2c_mutex[port]);
    esp_i2c_mutex[port] = NULL;

    ESP_I2C_LOGI("Closing I2C master at port %d", port);

    return i2c_driver_delete(port);
}

esp_err_t esp_i2c_read(i2c_port_t port, uint16_t addr, uint32_t reg, uint8_t *buffer, uint16_t size) {

	ESP_I2C_ASSERT(port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_NUM_MAX-1);
	ESP_I2C_ASSERT((port == I2C_NUM_0 && CONFIG_ESP_I2C_PORT_0_ENABLED) || (port == I2C_NUM_1 && CONFIG_ESP_I2C_PORT_1_ENABLED), "I2C port %d not enabled in config file.", port);
	ESP_I2C_ASSERT(esp_i2c_mutex[port], "I2C port %d is not initialized.", port);

   	ESP_I2C_LOGV("Reading port %d, addr 0x%03x, reg 0x%04lx", port, addr, reg);

	TickType_t timeout = 0;

	if (port == I2C_NUM_0) {
		timeout = CONFIG_ESP_I2C_PORT_0_TIMEOUT;
	} else if (port == I2C_NUM_1) {
		timeout = CONFIG_ESP_I2C_PORT_1_TIMEOUT;
	}

	esp_err_t result;

	if (esp_i2c_lock(port) == ESP_OK) {

		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		if (!(reg & ESP_I2C_NO_REG_FLAG)) {
			/* When reading specific register set the addr pointer first. */
			i2c_master_start(cmd);
			esp_i2c_send_address(cmd, addr, I2C_MASTER_WRITE);
			esp_i2c_send_register(cmd, reg);
		}

		/* Read size bytes from the current pointer. */
		i2c_master_start(cmd);
		esp_i2c_send_address(cmd, addr, I2C_MASTER_READ);
		i2c_master_read(cmd, buffer, size, I2C_MASTER_LAST_NACK);
		i2c_master_stop(cmd);

		result = i2c_master_cmd_begin(port, cmd, timeout);

		i2c_cmd_link_delete(cmd);

		esp_i2c_unlock(port);

		if (result != ESP_OK) {
			ESP_I2C_LOGW("Read port %d error: %d", port, result);
		} else {
			ESP_I2C_BUFFER_LOGV(buffer, size);
		}
	} else {
		ESP_I2C_LOGE("Lock could not be obtained for port %d.", port);
		return ESP_ERR_TIMEOUT;
	}

    return result;
}

esp_err_t esp_i2c_write(i2c_port_t port, uint16_t addr, uint32_t reg, const uint8_t *buffer, uint16_t size) {

	ESP_I2C_ASSERT(port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_NUM_MAX-1);
	ESP_I2C_ASSERT((port == I2C_NUM_0 && CONFIG_ESP_I2C_PORT_0_ENABLED) || (port == I2C_NUM_1 && CONFIG_ESP_I2C_PORT_1_ENABLED), "I2C port %d not enabled in config file.", port);
	ESP_I2C_ASSERT(esp_i2c_mutex[port], "I2C port %d is not initialized.", port);

   	ESP_I2C_LOGV("Writing port %d, addr 0x%03x, reg 0x%04lx", port, addr, reg);

	TickType_t timeout = 0;

	if (port == I2C_NUM_0) {
		timeout = CONFIG_ESP_I2C_PORT_0_TIMEOUT;
	} else if (port == I2C_NUM_1) {
		timeout = CONFIG_ESP_I2C_PORT_1_TIMEOUT;
	}

	esp_err_t result;

	if (esp_i2c_lock(port) == ESP_OK) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		i2c_master_start(cmd);
		esp_i2c_send_address(cmd, addr, I2C_MASTER_WRITE);
		if (!(reg & ESP_I2C_NO_REG_FLAG)) {
			esp_i2c_send_register(cmd, reg);
		}
		i2c_master_write(cmd, (uint8_t *)buffer, size, true);
		i2c_master_stop(cmd);
		result = i2c_master_cmd_begin( port, cmd, timeout);

		i2c_cmd_link_delete(cmd);

		esp_i2c_unlock(port);

		if (result != ESP_OK) {
			ESP_I2C_LOGW("Write port %d error: %d", port, result);
		} else {
			ESP_I2C_BUFFER_LOGV(buffer, size);
		}
	} else {
		ESP_I2C_LOGE("Lock could not be obtained for port %d.", port);
		return ESP_ERR_TIMEOUT;
	}

    return result;
}

static inline esp_err_t esp_i2c_lock(i2c_port_t port) {
	ESP_I2C_LOGV("Try to lock mutex for port %d.", port);

	TickType_t timeout;
	if (port == I2C_NUM_0) {
		timeout = CONFIG_ESP_I2C_PORT_0_LOCK_TIMEOUT / portTICK_RATE_MS;
	} else if (port == I2C_NUM_1) {
		timeout = CONFIG_ESP_I2C_PORT_1_LOCK_TIMEOUT / portTICK_RATE_MS;
	}

	if (xSemaphoreTake(esp_i2c_mutex[port], timeout) == pdTRUE) {
		ESP_I2C_LOGV("Locking mutex for port %d succeeded.", port);
		return ESP_OK;
	} else {
		ESP_I2C_LOGE("Locking mutex for port %d failed.", port);
		return ESP_FAIL;
	}
}

static inline esp_err_t esp_i2c_unlock(i2c_port_t port) {
	ESP_I2C_LOGV("Try to unlock mutex for port %d.", port);

	if (xSemaphoreGive(esp_i2c_mutex[port]) == pdTRUE) {
		ESP_I2C_LOGV("Unlocking mutex for port %d succeeded.", port);
		return ESP_OK;
	} else {
		ESP_I2C_LOGE("Unlocking mutex for port %d failed.", port);
		return ESP_FAIL;
	}
}

static inline void esp_i2c_send_address(i2c_cmd_handle_t cmd, uint16_t addr, i2c_rw_t rw) {
	if (addr & ESP_I2C_ADDR_10_BIT_FLAG) {
		i2c_master_write_byte(cmd, 0xF0 | ((addr & 0x3FF) >> 7) | rw, true);
		i2c_master_write_byte(cmd, addr & 0xFF, true);
	} else {
		i2c_master_write_byte(cmd, (addr << 1) | rw, true);
	}
}

static inline void esp_i2c_send_register(i2c_cmd_handle_t cmd, uint32_t reg) {
	if (reg & ESP_I2C_REG_16_BIT_FLAG) {
	    i2c_master_write_byte(cmd, (reg & 0xFF00) >> 8, true);
	}
    i2c_master_write_byte(cmd, reg & 0xFF, true);
}
