/*
    Two simple first-order low-pass filters (FIR and IIR) are implemented. In order to reduce overhead and improve efficiency, 
    only support integer data processing and only support integer powers of 2 (such as 2, 4, 8, 16...) depth.
    It should be noted that the data type defined by SumType_t is used to store the cumulative sum of the data type defined by 
    DataType_t, so SumType_t must have enough bit width to accumulate the maximum value of the data defined by Datatype_t at 
    the specified depth without overflowing.
    Users must carefully consider the definitions of DataType_t and Sumype_t, and consider signed and unsigned situations to 
    ensure it work properly and not produce unexpected exceptions.
*/

#pragma once

#include <stdlib.h>
#include <esp_log.h>

#define LP_FILTER_LOGE(format, ...) 				ESP_LOGE(LP_TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGW(format, ...) 				ESP_LOGW(LP_TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGI(format, ...) 				ESP_LOGI(LP_TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGD(format, ...) 				ESP_LOGD(LP_TAG, format, ##__VA_ARGS__)
#define LP_FILTER_LOGV(format, ...) 				ESP_LOGV(LP_TAG, format, ##__VA_ARGS__)

#define LP_FILTER_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(LP_TAG, buffer, buff_len, ESP_LOG_ERROR)
#define LP_FILTER_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(LP_TAG, buffer, buff_len, ESP_LOG_WARN)
#define LP_FILTER_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(LP_TAG, buffer, buff_len, ESP_LOG_INFO)
#define LP_FILTER_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(LP_TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define LP_FILTER_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(LP_TAG, buffer, buff_len, ESP_LOG_VERBOSE)

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

static const char *LP_TAG = "LP_FILTER";

typedef enum {
    FILTER_DEPTH_POWER_02 = 1,
    FILTER_DEPTH_POWER_04 = 2,
    FILTER_DEPTH_POWER_08 = 3,
    FILTER_DEPTH_POWER_16 = 4,
    FILTER_DEPTH_POWER_32 = 5,
    FILTER_DEPTH_POWER_64 = 6,
} DepthPower_t;

template <typename DataType_t, typename SumType_t>
class IirFilter {
public:
    DepthPower_t m_depth_power; 
    SumType_t m_sum;
    DataType_t m_average;

public:
    IirFilter(DepthPower_t depth_power, DataType_t init_value) : m_depth_power(depth_power), m_average(init_value) {
        LP_FILTER_ASSERT(depth_power >= FILTER_DEPTH_POWER_02, "The depth of the filter must be greater than 2.");
        /* Sometimes it is necessary for users to control themselves to avoid overflow, rather than forcibly limiting data types.*/
        //LP_FILTER_ASSERT(sizeof(SumType_t) >= sizeof(DataType_t) + this->m_depth_power, "SumType_t has no enough bitwidth.");
        this->m_sum = init_value;
        this->m_sum <<= m_depth_power;
    }

    ~IirFilter() {

    }

    inline DataType_t ProcessData(DataType_t data){
        this->m_sum -= this->m_average;
        this->m_sum += data;
        this->m_average = (this->m_sum >> this->m_depth_power);

        return this->m_average;
    }

    inline DataType_t GetAverage() {
        return this->m_average;
    }
};

template <typename DataType_t, typename SumType_t>
class FirFilter : public IirFilter<DataType_t, SumType_t> {
    typedef struct _data_list{
        struct _data_list *next;
        DataType_t data;
    } DataList_t;

public:
    DataList_t *m_data_list_head;
    DataList_t *m_data_list_pos;

public:
    FirFilter(DepthPower_t depth_power, DataType_t init_value) : IirFilter<DataType_t, SumType_t>(depth_power, init_value), m_data_list_head(NULL), m_data_list_pos(NULL) {
        uint8_t depth = (1 << depth_power);
        this->m_data_list_head = (DataList_t *)malloc(depth * sizeof(DataList_t));

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

    ~FirFilter() {
        if (this->m_data_list_head != NULL) {
            free(this->m_data_list_head);
        }
    }

    inline DataType_t ProcessData(DataType_t data) {
        DataType_t last_data = this->m_data_list_pos->data;
        this->m_data_list_pos->data = data;
        this->m_data_list_pos = this->m_data_list_pos->next;

        this->m_sum -= last_data;
        this->m_sum += data;
        this->m_average = (this->m_sum >> this->m_depth_power);

        return this->m_average;
    }
};
