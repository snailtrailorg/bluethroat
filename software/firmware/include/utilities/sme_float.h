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

#define POSITIVE    (0x00000000)
#define NEGATIVE    (0x00000001)

class float32_t {
public:
    uint32_t s;      //sign
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
    float32_t() : s(POSITIVE), m(0), e(0) {}

    float32_t(uint32_t sign, uint32_t mantissa, uint32_t exponent) : s(sign), m(mantissa), e(exponent) {
        if (!(this->m & 0x80000000)) {
            uint8_t offset = 31 - get_msb_index_32(this->m);
            this->m <<= offset;
            this->e += offset;
        }
    }

    float32_t(int32_t number) : e(0) {
        if (number < 0) {
            this->s = NEGATIVE;
            this->m = (-number);
        } else {
            this->s = POSITIVE;
            this->m = number;
        }

        if (!(this->m & 0x80000000)) {
            uint8_t offset = 31 - get_msb_index_32(this->m);
            this->m <<= offset;
            this->e += offset;
        }
    }

    float32_t(float number) {
        uint32_t num_32 = (uint32_t)(number);
        this->s = (num_32 & 0x80000000) >> 31;
        this->m = (num_32 & 0x007fffff) | 0x00800000;
        this->e = ((num_32 & 0x7f800000) >> 23) - 127;
    }

    float32_t& operator=(const float32_t& other) {
        this->s = other.s;
        this->m = other.m;
        this->e = other.e;
        return *this;
    }

    float32_t operator-() const {
        return float32_t(this->s ^ 0x00000001, this->m, this->e);
    }

    float32_t operator+(const float32_t& other) const {
        int32_t offset;
        int64_t left, right, mantissa;
        int32_t exponent;
        int32_t sign;

        offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            left = this->m; if (this->s == POSITIVE) { left = -left; }
            right = other.m >> offset; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
            exponent = this->e;
        } else if (offset < 0) {
            left = this->m >> (-offset); if (this->s == POSITIVE) { left = -left; }
            right = other.m; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
            exponent = other.e;
        } else {
            left = this->m; if (this->s == POSITIVE) { left = -left; }
            right = other.m; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
            exponent = this->e;
        }

        if (mantissa < 0) {
            mantissa = -mantissa;
            sign = NEGATIVE;
        } else {
            sign = POSITIVE;
        }

        offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            exponent -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            exponent += offset;
        }

        return float32_t((uint32_t)mantissa, exponent, sign);
    }

    float32_t& operator+=(const float32_t& other) {
        int64_t left, right, mantissa;

        int32_t offset = this->e - other.e;
        offset = (offset > 31) ? 31 : ((offset < -31) ? -31 : offset);

        if (offset > 0) {
            left = this->m; if (this->s == POSITIVE) { left = -left; }
            right = other.m >> offset; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
        } else if (offset < 0) {
            left = this->m >> (-offset); if (this->s == POSITIVE) { left = -left; }
            right = other.m; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
            this->e = other.e;
        } else {
            left = this->m; if (this->s == POSITIVE) { left = -left; }
            right = other.m; if (other.s == POSITIVE) { right = -right; }
            mantissa = left + right;
        }

        if (mantissa < 0) {
            mantissa = -mantissa;
            this->s = NEGATIVE;
        } else {
            this->s = POSITIVE;
        }

        offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            this->e -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            this->e += offset;
        }

        this->m = (uint32_t)mantissa;

        return *this;
    }

    float32_t operator-(const float32_t& other) const {
        return (*this) + (-other);
    }

    float32_t& operator-=(const float32_t& other) {
        return (*this) += (-other);
    }

    float32_t operator*(const float32_t& other) const {
        uint64_t mantissa = (uint64_t)this->m * other.m;
        int32_t exponent = this->e + other.e;

        int32_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            exponent -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            exponent += offset;
        }

        return float32_t((uint32_t)mantissa, exponent, this->s ^ other.s);
    }

    float32_t& operator*=(const float32_t& other) {
        uint64_t mantissa = (uint64_t)this->m * other.m;
        this->e += other.e;

        int32_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            this->e -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            this->e += offset;
        }

        this->m = (int32_t)mantissa;
        this->s ^= other.s;

        return *this;
    }

    float32_t operator/(const float32_t& other) const {
        int32_t exponent = this->e - other.e;

        uint64_t mantissa = (uint64_t)this->m;
        int8_t offset_left = 63 - get_msb_index_64((uint64_t)this->m);
        mantissa <<= offset_left;
        exponent -= offset_left;

        int8_t offset_right = get_lsb_index_64(other.m);
        mantissa /= other.m >> offset_right;
        exponent -= offset_right;

        int8_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            exponent -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            exponent += offset;
        }

        return float32_t((uint32_t)mantissa, exponent, this->s ^ other.s);
    }

    float32_t& operator/=(const float32_t& other) {
        this->e -= other.e;

        uint64_t mantissa = (uint64_t)this->m;
        int8_t offset_left = 63 - get_msb_index_64((uint64_t)this->m);
        mantissa <<= offset_left;
        this->e -= offset_left;

        int8_t offset_right = get_lsb_index_64(other.m);
        mantissa /= other.m >> offset_right;
        this->e -= offset_right;

        int8_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
        if (offset > 0) {
            mantissa <<= offset;
            this->e -= offset;
        } else if (offset  < 0) {
            mantissa >>= (-offset);
            this->e += offset;
        }

        this->m = (int32_t)mantissa;
        this->s ^= other.s;

        return *this;
    }

    float32_t operator+(const int32_t& other) const {
        return (*this) + float32_t(other);
    }

    float32_t& operator+=(const int32_t& other) {
        return (*this) += float32_t(other);
    }

    float32_t operator-(const int32_t& other) const {
        return (*this) - float32_t(other);
    }

    float32_t& operator-=(const int32_t& other) {
        return (*this) -= float32_t(other);
    }

    float32_t operator*(const int32_t& other) const {
        return (*this) * float32_t(other);
    }

    float32_t& operator*=(const int32_t& other) {
        return (*this) *= float32_t(other);
    }

    float32_t operator/(const int32_t& other) const {
        return (*this) / float32_t(other);
    }

    float32_t& operator/=(const int32_t& other) {
        return (*this) /= float32_t(other);
    }

    operator float() const {
        uint32_t num_32 = (this->s << 31) | ((this->e + 127) << 23) | (this->m & 0x007fffff);
        return *((float*)&num_32);
    }
};
