/*
    Separately stored Sign, Mantissa and Exponent FLOATing point number.
    This is a simple implementation of replacing floating point numbers with 32-bit integers. The sign, mantissa and exponent are 
    stored in 32-bit integers separately.
    When performing addition, subtraction, multiplication and division operations, rounding is not considered and all 
    overflow bits on the right side are discarded directly.
    During the calculation process, the data will be saved in a 64-bit integer. When the result is output, the displacement 
    and exponent are adjusted so that the result can be loaded into a 32-bit integer.
*/

#pragma once

#include <stdint.h>

#define POSITIVE    (0x00000001)
#define NEGATIVE    (0xffffffff)

class float32_t {
public:
    int32_t s;      //sign
    uint32_t m;     //mantissa
    int32_t e;      //exponent

public:
    static uint8_t get_msb_index_32(const uint32_t number) {
        uint32_t num_32 = number;
        uint8_t index = 0;

        if (num_32 & 0xffff0000) { index += 0x10; num_32 >>= 0x10; }
        if (num_32 & 0x0000ff00) { index += 0x08; num_32 >>= 0x08; }
        if (num_32 & 0x000000f0) { index += 0x04; num_32 >>= 0x04; }
        if (num_32 & 0x0000000c) { index += 0x02, num_32 >>= 0x02; }
        if (num_32 & 0x00000002) { index += 0x01; }

        return index;
    }

    static uint8_t get_msb_index_64(const uint64_t number) {
        uint64_t num_64 = number;
        uint8_t index = 0;

        if (num_64 & 0xffffffff00000000) { index += 0x20; num_64 >>= 0x20; }

        uint32_t num_32 =(uint32_t)num_64;
        if (num_32 & 0xffff0000) { index += 0x10; num_32 >>= 0x10; }
        if (num_32 & 0x0000ff00) { index += 0x08; num_32 >>= 0x08; }
        if (num_32 & 0x000000f0) { index += 0x04; num_32 >>= 0x04; }
        if (num_32 & 0x0000000c) { index += 0x02; num_32 >>= 0x02; }
        if (num_32 & 0x00000002) { index += 0x01; }
    
        return index;
    }

    static uint8_t get_lsb_index_32(const uint32_t number) {
        uint32_t num_32 = number & (0 - number);
        uint8_t index = 0;

        if (num_32 & 0xffff0000) { index += 0x10; num_32 >>= 0x10; }
        if (num_32 & 0x0000ff00) { index += 0x08; num_32 >>= 0x08; }
        if (num_32 & 0x000000f0) { index += 0x04; num_32 >>= 0x04; }
        if (num_32 & 0x0000000c) { index += 0x02, num_32 >>= 0x02; }
        if (num_32 & 0x00000002) { index += 0x01; }

        return index;
    }

    static uint8_t get_lsb_index_64(const uint64_t number) {
        uint64_t num_64 = number & (0 - number);
        uint8_t index = 0;

        if (num_64 & 0xffffffff00000000) { index += 0x20; num_64 >>= 0x20; }

        uint32_t num_32 = (uint32_t)num_64;
        if (num_32 & 0xffff0000) { index += 0x10; num_32 >>= 0x10; }
        if (num_32 & 0x0000ff00) { index += 0x08; num_32 >>= 0x08; }
        if (num_32 & 0x000000f0) { index += 0x04; num_32 >>= 0x04; }
        if (num_32 & 0x0000000c) { index += 0x02; num_32 >>= 0x02; }
        if (num_32 & 0x00000002) { index += 0x01; }

        return index;
    }

public:
    float32_t() : s(0), m(0), e(0) {}

    float32_t(int32_t mantissa, int32_t exponent) : m(mantissa), e(exponent) {
        if (m == 0) {
            e = 0;
        } else {
            uint8_t index = get_msb_index_32((uint32_t)m);
            if (index < 30) {
                m <<= (30 - index);
                e -= (30 - index);
            } else if (index > 30) {
                m >>= (index - 30);
                e += (index - 30);
            }
        }
    }

    float32_t operator+(const float32_t& other) const {
        int64_t mantissa;
        int32_t exponent;

        int32_t offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            mantissa = (int64_t)(this->m) + (other.m >> offset);
            exponent = this->e;
        } else if (offset < 0) {
            mantissa = (int64_t)(other.m) + (this->m >> (-offset));
            exponent = other.e;
        } else {
            mantissa = (int64_t)(this->m) + other.m;
            exponent = this->e;
        }

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            exponent -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            exponent += (index - 30);
        }

        return float32_t((int32_t)mantissa, exponent);
    }

    float32_t& operator+=(const float32_t& other) {
        int64_t mantissa;

        int32_t offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            mantissa = (int64_t)(this->m) + (other.m >> offset);
        } else if (offset < 0) {
            mantissa = (int64_t)(other.m) + (this->m >> (-offset));
            this->e = other.e;
        } else {
            mantissa = (int64_t)(this->m) + other.m;
        }

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            this->e -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            this->e += (index - 30);
        }

        this->m = (int32_t)mantissa;

        return *this;
    }

    float32_t operator-(const float32_t& other) const {
        int64_t mantissa;
        int32_t exponent;

        int32_t offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            mantissa = (int64_t)this->m - (other.m >> offset);
            exponent = this->e;
        } else if (offset < 0) {
            mantissa = (int64_t)other.m - (this->m >> (-offset));
            exponent = other.e;
        } else {
            mantissa = (int64_t)this->m - other.m;
            exponent = this->e;
        }

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            exponent -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            exponent += (index - 30);
        }

        return float32_t((int32_t)mantissa, exponent);
    }

    float32_t& operator-=(const float32_t& other) {
        int64_t mantissa;

        int32_t offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            mantissa = (int64_t)this->m - (other.m >> offset);
        } else if (offset < 0) {
            mantissa = (int64_t)other.m - (this->m >> (-offset));
            this->e = other.e;
        } else {
            mantissa = (int64_t)this->m - other.m;
        }

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            this->e -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            this->e += (index - 30);
        }

        this->m = (int32_t)mantissa;

        return *this;
    }

    float32_t operator*(const float32_t& other) const {
        int64_t mantissa = (int64_t)this->m * other.m;
        int32_t exponent = this->e + other.e;

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            exponent -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            exponent += (index - 30);
        }

        return float32_t((int32_t)mantissa, exponent);
    }

    float32_t& operator*=(const float32_t& other) {
        int64_t mantissa = (int64_t)this->m * other.m;
        this->e += other.e;

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            this->e -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            this->e += (index - 30);
        }

        this->m = (int32_t)mantissa;

        return *this;
    }

    float32_t operator/(const float32_t& other) const {
        int64_t mantissa = ((int64_t)this->m << 32) / other.m;
        int32_t exponent = this->e - 32 - other.e;

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            exponent -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            exponent += (index - 30);
        }

        return float32_t((int32_t)mantissa, exponent);
    }

    float32_t& operator/=(const float32_t& other) {
        int64_t mantissa = ((int64_t)this->m << 32) / other.m;
        this->e -= (32 + other.e);

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            this->e -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            this->e += (index - 30);
        }

        this->m = (int32_t)mantissa;

        return *this;
    }

    float32_t operator*(const int32_t& other) const {
        int64_t mantissa = (int64_t)this->m * other;
        int32_t exponent = this->e;

        uint8_t index = get_msb_index_64((uint64_t)mantissa);
        if (index < 30) {
            mantissa <<= (30 - index);
            exponent -= (30 - index);
        } else if (index > 30) {
            mantissa >>= (index - 30);
            exponent += (index - 30);
        }

        return float32_t((int32_t)mantissa, exponent);
    }


    float32_t& operator=(const float32_t& other) {
        this->m = other.m;
        this->e = other.e;
        return *this;
    }
};
