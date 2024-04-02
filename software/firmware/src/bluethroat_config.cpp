#include "bluethroat_config.h"

BluethroatConfig::BluethroatConfig() {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
}

BluethroatConfig::~BluethroatConfig() {
    ESP_ERROR_CHECK(nvs_flash_deinit());
}

esp_err_t BluethroatConfig::SetString(const char *name_space, const char *key, const char *value) {
    nvs_handle_t handle;
    esp_err_t result = nvs_open(name_space, NVS_READWRITE, &handle);
    if (result != ESP_OK) {
        return result;
    }

    result = nvs_set_str(handle, key, value);
    if (result != ESP_OK) {
        nvs_close(handle);
        return result;
    }

    result = nvs_commit(handle);
    nvs_close(handle);
    return result;
}

esp_err_t BluethroatConfig::GetString(const char *name_space, const char *key, char *value, size_t *length) {
    nvs_handle_t handle;
    esp_err_t result = nvs_open(name_space, NVS_READONLY, &handle);
    if (result != ESP_OK) {
        return result;
    }

    result = nvs_get_str(handle, key, value, length);
    if (result != ESP_OK) {
        nvs_close(handle);
        return result;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t BluethroatConfig::SetInteger(const char *name_space, const char *key, int32_t value) {
    nvs_handle_t handle;
    esp_err_t result = nvs_open(name_space, NVS_READWRITE, &handle);
    if (result != ESP_OK) {
        return result;
    }

    result = nvs_set_i32(handle, key, value);
    if (result != ESP_OK) {
        nvs_close(handle);
        return result;
    }

    result = nvs_commit(handle);
    nvs_close(handle);
    return result;
}

esp_err_t BluethroatConfig::GetInteger(const char *name_space, const char *key, int32_t *value) {
    nvs_handle_t handle;
    esp_err_t result = nvs_open(name_space, NVS_READONLY, &handle);
    if (result != ESP_OK) {
        return result;
    }

    result = nvs_get_i32(handle, key, value);
    if (result != ESP_OK) {
        nvs_close(handle);
        return result;
    }

    nvs_close(handle);
    return ESP_OK;
}
