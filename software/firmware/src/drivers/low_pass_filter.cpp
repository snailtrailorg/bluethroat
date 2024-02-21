#include <stdlib.h>
#include <esp_log.h>

#include "drivers/low_pass_filter.h"

#define LP_FILTER_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define LP_FILTER_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define LP_FILTER_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define LP_FILTER_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define LP_FILTER_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define LP_FILTER_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define LP_FILTER_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			LP_FILTER_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define LP_FILTER_ASSERT(condition, format, ...)
#endif

static const char *TAG = "LP_FILTER";

template <typename DataType_t, typename SumType_t>
IirFilter<DataType_t, SumType_t>::IirFilter(DepthPower_t depth_power, DataType_t init_value) : m_depth_power(depth_power), m_average(init_value) {
    LP_FILTER_ASSERT(depth_power >= FILTER_DEPTH_POWER_02, "The depth of the filter must be greater than 2.");
    LP_FILTER_ASSERT(sizeof(SumType_t) >= sizeof(DataType_t) + this->m_depth_power, "SumType_t has no enough bitwidth.");
    this->m_sum = init_value;
    this->m_sum <<= m_depth_power;
}

template <typename DataType_t, typename SumType_t>
IirFilter<DataType_t, SumType_t>::~IirFilter() {

}

template <typename DataType_t, typename SumType_t>
DataType_t IirFilter<DataType_t, SumType_t>::ProcessData(DataType_t data) {
    this->m_sum -= this->m_average;
    this->m_sum += data;
    this->m_average = (this->m_sum >> this->m_depth_power);

    return this->m_average;
}

template <typename DataType_t, typename SumType_t> 
FirFilter<DataType_t, SumType_t>::FirFilter(DepthPower_t depth_power, DataType_t init_value) : IirFilter<DataType_t, SumType_t>(depth_power, init_value), m_data_list_head(NULL), m_data_list_pos(NULL) {
    uint8_t depth = (1 << depth_power);
    this->m_data_list_head = malloc(depth * sizeof(DataList_t));

    if (this->m_data_list_head != NULL) {
        for (uint8_t i=0; i<(depth-1); i++) {
            this->m_data_list_head[i].next = &(this->m_data_list_head[i+1]);
            this->m_data_list_head[i].data = init_value;
        }

        this->m_data_list_head[depth-1].next = this->m_data_list_head;
        this->m_data_list_head[depth-1].data = init_value;

        this->m_data_list_pos = this->m_data_list_head;
    } else {
        LP_FILTER_LOGE("No enough memory, init FIR low-pass filter failed.");
    }
}

template <typename DataType_t, typename SumType_t>
FirFilter<DataType_t, SumType_t>::~FirFilter() {
    if (this->m_data_list_head != NULL) {
        free(this->m_data_list_head);
    }
}

template <typename DataType_t, typename SumType_t> 
DataType_t FirFilter<DataType_t, SumType_t>::ProcessData(DataType_t data) {
    DataType_t last_data = this->m_data_list_pos->data;
    this->m_data_list_pos->data = data;
    this->m_data_list_pos = this->m_data_list_pos->next;

    this->m_sum -= last_data;
    this->m_sum += data;
    this->m_average = (this->m_sum >> this->m_depth_power);

    return this->m_average;
}
