#pragma once

#include <esp_err.h>
#include <nvs_flash.h>

class BluethroatConfig {
public:
    BluethroatConfig();
    ~BluethroatConfig();

public:
    static esp_err_t SetString(const char *name_space, const char *key, const char *value);
    static esp_err_t GetString(const char *name_space, const char *key, char *value, size_t *length);
    static esp_err_t SetInteger(const char *name_space, const char *key, int32_t value);
    static esp_err_t GetInteger(const char *name_space, const char *key, int32_t *value);
};

extern BluethroatConfig *g_pBluethroatConfig;
