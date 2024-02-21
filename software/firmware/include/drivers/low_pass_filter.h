#pragma once

/*
    Two simple first-order low-pass filters (FIR and IIR) are implemented. In order to reduce overhead and improve efficiency, 
    only support integer data processing and only support integer powers of 2 (such as 2, 4, 8, 16...) depth.
    It should be noted that the data type defined by SumType_t is used to store the cumulative sum of the data type defined by 
    DataType_t, so SumType_t must have enough bit width to accumulate the maximum value of the data defined by Datatype_t at 
    the specified depth without overflowing.
    Users must carefully consider the definitions of DataType_t and Sumype_t, and consider signed and unsigned situations to 
    ensure it work properly and not produce unexpected exceptions.
*/

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
    IirFilter(DepthPower_t depth_power, DataType_t init_value);
    ~IirFilter();
    inline DataType_t ProcessData(DataType_t data);
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
    FirFilter(DepthPower_t depth_power, DataType_t init_value);
    ~FirFilter();
    inline DataType_t ProcessData(DataType_t data);
};
