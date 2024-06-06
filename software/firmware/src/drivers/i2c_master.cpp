#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/i2c.h>

#include "drivers/i2c_master.h"

#if defined __has_include
#if __has_include("esp_idf_version.h")
#include "esp_idf_version.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
#define HAS_CLK_FLAGS
#endif
#endif
#endif

#define I2C_ADDR_10_BIT_FLAG 						((uint16_t)(0x8000))
#define I2C_REG_16_BIT_FLAG 						((uint32_t)(0x80000000))
#define I2C_NO_REG_FLAG 							((uint32_t)(0x40000000))

#define I2C_MASTER_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define I2C_MASTER_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define I2C_MASTER_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define I2C_MASTER_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define I2C_MASTER_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define I2C_MASTER_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define I2C_MASTER_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define I2C_MASTER_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define I2C_MASTER_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define I2C_MASTER_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define I2C_MASTER_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			I2C_MASTER_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define I2C_MASTER_ASSERT(condition, format, ...)
#endif

static const char *TAG = "I2C_MASTER";

I2cMaster * I2cMaster::m_instance[I2C_NUM_MAX] = {NULL};

I2cMaster::I2cMaster(i2c_port_t port, int sda_io_num, int scl_io_num, bool sda_pullup_en, bool scl_pullup_en, uint32_t clk_speed, uint16_t lock_timeout, uint16_t timeout) : m_mutex(NULL) {
    (void)this->init_controller(port, sda_io_num, scl_io_num, sda_pullup_en, scl_pullup_en, clk_speed, lock_timeout, timeout);
}

I2cMaster::~I2cMaster() {
	(void)this->deinit_controller();
}

esp_err_t I2cMaster::init_controller(i2c_port_t port, int sda_io_num, int scl_io_num, bool sda_pullup_en, bool scl_pullup_en, uint32_t clk_speed, uint16_t lock_timeout, uint16_t timeout) {
	I2C_MASTER_ASSERT(port >= 0 && port < I2C_MASTER_NUM_MAX,  "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_MASTER_NUM_MAX-1);
	I2C_MASTER_ASSERT(this->m_mutex == NULL, "I2C master instance has already initialized");
	I2C_MASTER_LOGI("Starting initialize I2C master instance.");

	if (I2cMaster::m_instance[port] == NULL) {
		I2cMaster::m_instance[port] = this;
		I2C_MASTER_LOGI("Binding I2C master instance to port %d.", port);
	} else {
		I2C_MASTER_LOGE("I2C port %d has already binded.", port);
		return ESP_FAIL;
	}

	this->m_port = port;
	this->m_lock_timeout = pdMS_TO_TICKS(lock_timeout);
	this->m_timeout = pdMS_TO_TICKS(timeout);

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda_io_num,
		.scl_io_num = scl_io_num,
		.sda_pullup_en = sda_pullup_en,
		.scl_pullup_en = scl_pullup_en,
		.master = {.clk_speed = clk_speed},
#ifdef HAS_CLK_FLAGS
		.clk_flags = 0,
#endif
	};

	esp_err_t result = i2c_param_config(this->m_port, &conf);
	
	if (result == ESP_OK) {
		I2C_MASTER_LOGI("Configure I2C master port %d parameters (SDA: %d, SCL: %d, speed: %lu Hz.).", this->m_port, sda_io_num, scl_io_num, clk_speed);
	} else {
		I2C_MASTER_LOGE("Failed to configureelse  I2C master port %d.", this->m_port);
		return result;
	}

	result = i2c_driver_install(this->m_port, conf.mode, 0, 0, 0);

	if (result == ESP_OK) {
		I2C_MASTER_LOGI("Install I2C driver for port %d.", this->m_port);
	} else {
		I2C_MASTER_LOGE("Failed to install I2C driver for port %d.", this->m_port);
		return result;
	}

	this->m_mutex = xSemaphoreCreateMutex();

	if (this->m_mutex != NULL) {
		I2C_MASTER_LOGI("Create I2C access mutex.");
	} else {
		I2C_MASTER_LOGE("Failed to reate I2C access mutex.");
		i2c_driver_delete(this->m_port);
		return ESP_FAIL;
	}

	return ESP_OK;
}

esp_err_t I2cMaster::deinit_controller() {
	I2C_MASTER_ASSERT(this->m_port >= 0 && this->m_port < I2C_NUM_MAX,  "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_MASTER_NUM_MAX-1);
	I2C_MASTER_ASSERT(I2cMaster::instance[this->m_port] == this, "I2C master instance was not properly initialized or I2C port %d was binded to other master instance.");
	I2C_MASTER_LOGI("Starting deinitialize I2C master instance.");

	I2cMaster::m_instance[this->m_port] = NULL;
	I2C_MASTER_LOGI("Unbinding I2C master instance from port %d.", this->m_port);

	esp_err_t result = ESP_OK;

	if (this->m_mutex != NULL) {
		I2C_MASTER_LOGI("Wait I2C access mutex for free.");
		xSemaphoreTake(this->m_mutex, portMAX_DELAY);
		I2C_MASTER_LOGI("Delete I2C access mutex.");
		vSemaphoreDelete(this->m_mutex);
		this->m_mutex = NULL;

		result = i2c_driver_delete(this->m_port);
		if (result == ESP_OK) {
			I2C_MASTER_LOGI("Uninstall I2C driver for port %d.", this->m_port);
		} else {
			I2C_MASTER_LOGE("Uninstall I2C driver for port %d failed.", this->m_port);
		}
	}

	return result;
}

esp_err_t I2cMaster::lock() {
	I2C_MASTER_ASSERT(this->m_mutex != NULL, "Invalid access mutex handle");
	I2C_MASTER_LOGV("Try to lock mutex for port %d.", this->m_port);
	
	if (xSemaphoreTake(this->m_mutex, this->m_lock_timeout) == pdTRUE) {
		I2C_MASTER_LOGV("Locking mutex for port %d succeeded.", this->m_port);
		return ESP_OK;
	} else {
		I2C_MASTER_LOGE("Locking mutex for port %d failed.", this->m_port);
		return ESP_FAIL;
	}
}

esp_err_t I2cMaster::unlock() {
	I2C_MASTER_ASSERT(this->m_mutex != NULL, "Invalid access mutex handle");
	I2C_MASTER_LOGV("Try to unlock mutex for port %d.", this->m_port);

	if (xSemaphoreGive(this->m_mutex) == pdTRUE) {
		I2C_MASTER_LOGV("Unlocking mutex for port %d succeeded.", this->m_port);
		return ESP_OK;
	} else {
		I2C_MASTER_LOGE("Unlocking mutex for port %d failed.", this->m_port);
		return ESP_FAIL;
	}
}

void I2cMaster::send_address(i2c_cmd_handle_t cmd, uint16_t device_addr, i2c_rw_t rw){
	if (device_addr & I2C_ADDR_10_BIT_FLAG)
	{
		i2c_master_write_byte(cmd, 0xF0 | ((device_addr & 0x3FF) >> 7) | rw, true);
		i2c_master_write_byte(cmd, device_addr & 0xFF, true);
	}
	else
	{
		i2c_master_write_byte(cmd, (device_addr << 1) | rw, true);
	}
}

void I2cMaster::send_register(i2c_cmd_handle_t cmd, uint32_t reg_addr) {
	if (reg_addr & I2C_REG_16_BIT_FLAG)
	{
		i2c_master_write_byte(cmd, (reg_addr & 0xFF00) >> 8, true);
	}
	i2c_master_write_byte(cmd, reg_addr & 0xFF, true);
}

esp_err_t I2cMaster::ProbeDevice(uint16_t device_addr) {
	I2C_MASTER_ASSERT(this->m_port >= 0 && this->m_port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_MASTER_NUM_MAX-1);
	I2C_MASTER_LOGI("Probe device on port %d, device_addr 0x%03x.", this->m_port, device_addr);

	esp_err_t result = ESP_OK;

	if (this->lock() == ESP_OK) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		i2c_master_start(cmd);
		this->send_address(cmd, device_addr, I2C_MASTER_WRITE);
		i2c_master_stop(cmd);

		result = i2c_master_cmd_begin(this->m_port, cmd, this->m_timeout);

		i2c_cmd_link_delete(cmd);

		this->unlock();

		if (result == ESP_OK) {
			I2C_MASTER_LOGI("Receive response from device_addr 0x%03x, port %d", device_addr, this->m_port);
		} else {
			I2C_MASTER_LOGI("No response from device_addr 0x%03x, port %d, error:%d", device_addr, this->m_port, result);
		}
	} else {
		I2C_MASTER_LOGE("Lock could not be obtained for port %d.", this->m_port);
		return ESP_ERR_TIMEOUT;
	}

	return result;
}

esp_err_t I2cMaster::ReadBuffer(uint16_t device_addr, uint32_t reg_addr, uint8_t *buffer, uint16_t size) {
	I2C_MASTER_ASSERT(this->m_port >= 0 && this->m_port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", port, I2C_MASTER_NUM_MAX-1);
	I2C_MASTER_LOGV("Reading port %d, device_addr 0x%03x, reg_addr 0x%04lx", this->m_port, device_addr, reg_addr);

	esp_err_t result = ESP_OK;

	if (this->lock() == ESP_OK) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		if (!(reg_addr & I2C_NO_REG_FLAG)) {
			/* When reading specific register set the device_addr pointer first. */
			i2c_master_start(cmd);
			this->send_address(cmd, device_addr, I2C_MASTER_WRITE);
			this->send_register(cmd, reg_addr);
		}

		/* Read size bytes from the current pointer. */
		i2c_master_start(cmd);
		this->send_address(cmd, device_addr, I2C_MASTER_READ);
		i2c_master_read(cmd, buffer, size, I2C_MASTER_LAST_NACK);
		i2c_master_stop(cmd);

		result = i2c_master_cmd_begin(this->m_port, cmd, this->m_timeout);

		i2c_cmd_link_delete(cmd);

		this->unlock();

		if (result != ESP_OK) {
			I2C_MASTER_LOGW("Reading port %d, device_addr 0x%03x, reg_addr 0x%04lx, size %d failed, error number: %d", this->m_port, device_addr, reg_addr, size, result);
		} else {
			I2C_MASTER_BUFFER_LOGD(buffer, size);
		}
	} else {
		I2C_MASTER_LOGE("Lock could not be obtained for port %d.", this->m_port);
		return ESP_ERR_TIMEOUT;
	}

	return result;
}

esp_err_t I2cMaster::WriteBuffer(uint16_t device_addr, uint32_t reg_addr, const uint8_t *buffer, uint16_t size) {
	I2C_MASTER_ASSERT(this->m_port >= 0 && this->m_port < I2C_NUM_MAX, "Invalid I2C port number: %d, Maximum valid port number is: %d.", this->m_port, I2C_MASTER_NUM_MAX-1);
	I2C_MASTER_LOGD("Writing port %d, device_addr 0x%03x, reg_addr 0x%04lx", this->m_port, device_addr, reg_addr);
	I2C_MASTER_BUFFER_LOGD(buffer, size);

	esp_err_t result = ESP_OK;

	if (this->lock() == ESP_OK)
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		i2c_master_start(cmd);
		this->send_address(cmd, device_addr, I2C_MASTER_WRITE);
		if (!(reg_addr & I2C_NO_REG_FLAG)) {
			this->send_register(cmd, reg_addr);
		}
		i2c_master_write(cmd, (uint8_t *)buffer, size, true);
		i2c_master_stop(cmd);
		result = i2c_master_cmd_begin(this->m_port, cmd, this->m_timeout);

		i2c_cmd_link_delete(cmd);

		this->unlock();

		if (result != ESP_OK) {
			I2C_MASTER_LOGW("Write port %d error: %d", this->m_port, result);
		} else {
			I2C_MASTER_BUFFER_LOGV(buffer, size);
		}
	} else {
		I2C_MASTER_LOGE("Lock could not be obtained for port %d.", this->m_port);
		return ESP_ERR_TIMEOUT;
	}

	return result;
}
esp_err_t I2cMaster::ReadByte(uint16_t device_addr, uint32_t reg_addr, uint8_t *p_byte) {
	return this->ReadBuffer(device_addr, reg_addr, p_byte, sizeof(uint8_t));
}

esp_err_t I2cMaster::WriteByte(uint16_t device_addr, uint32_t reg_addr, const uint8_t byte_value) {
	return this->WriteBuffer(device_addr, reg_addr, &byte_value, sizeof(uint8_t));
}
