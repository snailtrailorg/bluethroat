/*
    Separately stored Mantissa and Exponent Floatping point number.
    This is a simple implementation of replacing floating point numbers with 32-bit integers. The mantissa and exponent are 
    stored in two 32-bit integers separately. The mantissa contains 1 sign bit and 31 bits of data.
    When performing addition, subtraction, multiplication and division operations, rounding is not considered and all 
    overflow bits on the right side are discarded directly.
    During the calculation process, the data will be saved in a 64-bit integer. When the result is output, the displacement 
    and exponent are adjusted so that the result can be loaded into a 32-bit integer.
*/

#pragma once

#include <stdint.h>

class float32_t {
public:
    int32_t m; //mantissa
    int32_t e; //exponent

public:
    float32_t(int32_t mantissa, int32_t exponent) : m(mantissa), e(exponent) {
        uint8_t index = get_msb_index_32((uint32_t)this->m);
        if (index < 30) {
            m <<= (30 - index);
            e -= (30 - index);
        }
    }

    float32_t(int32_t mantissa, int32_t exponent, bool adjust) : m(mantissa), e(exponent) {
        if (adjust) {
            uint8_t index = get_msb_index_32((uint32_t)this->m);
            if (index < 30) {
                m <<= (30 - index);
                e -= (30 - index);
            }
        }
    }

    static uint8_t get_msb_index_32(uint32_t number) {
        number = (number & 0x80000000) ? ~number : number;
        uint8_t index = 0;

        if (number & 0xffff0000) { index += 0x10; number >>= 0x10; }
        if (number & 0x0000ff00) { index += 0x08; number >>= 0x08; }
        if (number & 0x000000f0) { index += 0x04; number >>= 0x04; }
        if (number & 0x0000000c) { index += 0x02, number >>= 0x02; }
        if (number & 0x00000002) { index += 0x01; }

        return index;
    }

    static uint8_t get_msb_index_64(uint64_t number) {
        uint8_t index = 0;
        number = (number & 0x8000000000000000) ? ~number : number;

        if (number & 0xffffffff00000000) { number >>= 0x20; index += 0x20; }

        uint32_t number32 =(uint32_t)number;
        if (number32 & 0xffff0000) { index += 0x10; number32 >>= 0x10; }
        if (number32 & 0x0000ff00) { index += 0x08; number32 >>= 0x08; }
        if (number32 & 0x000000f0) { index += 0x04; number32 >>= 0x04; }
        if (number32 & 0x0000000c) { index += 0x02; number32 >>= 0x02; }
        if (number32 & 0x00000002) { index += 0x01; }
        
        return index;
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
};
