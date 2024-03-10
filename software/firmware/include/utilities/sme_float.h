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
    int32_t s;      //sign, 0 for positive, 1 for negative, 32-bit integer for future expansion
    uint32_t m;     //mantissa, 32-bit unsigned integer, the range is 0 to 4294967295
    int32_t e;      //exponent, 32-bit signed integer, the range is -2147483648 to 2147483647

public:
    // get the index of the most significant bit(1) of a 32-bit unsigned integer
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

    // get the index of the most significant bit(1) of a 64-bit unsigned integer
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

    // get the index of the least significant bit(1) of a 32-bit unsigned integer
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

    // get the index of the least significant bit(1) of a 64-bit unsigned integer
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

    float32_t(int32_t sign, uint32_t mantissa, int32_t exponent) : s(sign), m(mantissa), e(exponent) {
        if (this->m == 0) {
            this->s = POSITIVE;
            this->e = 0;
        } else {
            if (!(this->m & 0x80000000)) {
                uint8_t offset = 31 - get_msb_index_32(this->m);
                this->m <<= offset;
                this->e -= offset;
            }
        }
    }

    float32_t(int32_t number) : e(0) {
        if (number == 0) {
            this->s = POSITIVE;
            this->m = 0;
        } else {
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
                this->e -= offset;
            }
        }
    }

    float32_t(float number) {
        uint32_t num_32 = (uint32_t)(number);
        if ((num_32 & 0x7fffffff) == 0) {
            this->s = POSITIVE;
            this->m = 0;
            this->e = 0;
        } else if ((num_32 & 0x7f800000) == 0) {
            this->s = (num_32 >> 31);
            this->m = ((num_32 & 0x007fffff) | 0x00800000) << 8;
            this->e = (-126) - 23 - 8;
        } else if ((num_32 & 0x7fffffff) == 0x7f800000) {
            this->s = (num_32 >> 31);
            this->m = 0xffffffff;
            this->e = 0x7fffffff;
        } else if ((num_32 & 0x7f800000) == 0x7f800000) {
            this->s = POSITIVE;
            this->m = 0xffffffff / this->s; // generate a devide by zero exception instead of NaN exception
        } else {
            this->s = (num_32 >> 31);
            this->m = ((num_32 & 0x007fffff) | 0x00800000) << 8;
            this->e = ((num_32 & 0x7f800000) >> 23) - 127 - 23 - 8;
        }
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
        int32_t sign;
        int64_t mantissa;
        int32_t exponent;
        int32_t offset;

        offset = this->e - other.e;
        if (offset > 31) {
            sign = this->s;
            mantissa = this->m;
            sign = this->s;
            exponent = this->e;
        } else if (offset < -31) {
            sign = other.s;
            mantissa = other.m;
            exponent = other.e;
        } else {
            int64_t left, right;
            if (offset > 0) {
                left = this->m; if (this->s == NEGATIVE) { left = -left; }
                right = other.m >> offset; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
                exponent = this->e;
            } else if (offset < 0) {
                left = this->m >> (-offset); if (this->s == NEGATIVE) { left = -left; }
                right = other.m; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
                exponent = other.e;
            } else {
                left = this->m; if (this->s == NEGATIVE) { left = -left; }
                right = other.m; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
                exponent = this->e;
            }

            if (mantissa == 0) {
                sign = POSITIVE;
                exponent = 0;
            } else {
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
                    exponent -= offset;
                }
            }
        }

        return float32_t(sign, (uint32_t)mantissa, exponent);
    }

    float32_t& operator+=(const float32_t& other) {
        int64_t mantissa;

        int32_t offset = this->e - other.e;
        if (offset < -31) {
            this->s = other.s;
            this->m = other.m;
            this->e = other.e;
        } else if (offset <= 31) {
            int64_t left, right;
            if (offset > 0) {
                left = this->m; if (this->s == NEGATIVE) { left = -left; }
                right = other.m >> offset; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
            } else if (offset < 0) {
                left = this->m >> (-offset); if (this->s == NEGATIVE) { left = -left; }
                right = other.m; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
                this->e = other.e;
            } else {
                left = this->m; if (this->s == NEGATIVE) { left = -left; }
                right = other.m; if (other.s == NEGATIVE) { right = -right; }
                mantissa = left + right;
            }

            if (mantissa == 0) {
                this->s = POSITIVE;
                this->m = 0;
                this->e = 0;
            } else {
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
                    this->e -= offset;
                }

                this->m = (uint32_t)mantissa;
            }
        }

        return *this;
    }

    float32_t operator-(const float32_t& other) const {
        return (*this) + (-other);
    }

    float32_t& operator-=(const float32_t& other) {
        return (*this) += (-other);
    }

    float32_t operator*(const float32_t& other) const {
        uint32_t sign = this->s ^ other.s;
        uint64_t mantissa = (uint64_t)this->m * (uint64_t)other.m;
        int32_t exponent = this->e + other.e;

        if (mantissa == 0) {
            sign = POSITIVE;
            exponent = 0;
        } else {
            sign = this->s ^ other.s;
            int32_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
            if (offset > 0) {
                mantissa <<= offset;
                exponent -= offset;
            } else if (offset  < 0) {
                mantissa >>= (-offset);
                exponent -= offset;
            }
        }

        return float32_t(sign, (uint32_t)mantissa, exponent);
    }

    float32_t& operator*=(const float32_t& other) {
        uint64_t mantissa = (uint64_t)this->m * (uint64_t)other.m;
        this->e += other.e;

        if (mantissa == 0) {
            this->s = POSITIVE;
            this->m = 0;
            this->e = 0;
        } else {
            this->s ^= other.s;
            int32_t offset = 31 - get_msb_index_64((uint64_t)mantissa);
            if (offset > 0) {
                mantissa <<= offset;
                this->e -= offset;
            } else if (offset  < 0) {
                mantissa >>= (-offset);
                this->e -= offset;
            }

            this->m = (uint32_t)mantissa;
        }

        return *this;
    }

    float32_t operator/(const float32_t& other) const {
        int32_t sign;
        uint64_t mantissa;
        int32_t exponent;

        if (this->m == 0) {
            sign = POSITIVE;
            mantissa = 0;
            exponent = 0;
        } else if (other.m == 0) {  // division by zero
            mantissa = this->m / other.m; // generate a devide by zero exception
        } else {
            sign = this->s ^ other.s;
            mantissa = (uint64_t)this->m;
            exponent = this->e - other.e;

            mantissa <<= 32;
            exponent -= 32;

            int8_t offset = get_lsb_index_32(other.m);
            if (offset > 0) {
                mantissa /= (other.m >> offset);
                exponent -= offset;
            } else {
                mantissa /= other.m;
            }
            
            offset = 31 - get_msb_index_64((uint64_t)mantissa);
            if (offset > 0) {
                mantissa <<= offset;
                exponent -= offset;
            } else if (offset  < 0) {
                mantissa >>= (-offset);
                exponent -= offset;
            }
        }

        return float32_t(sign, (uint32_t)mantissa, exponent);
    }

    float32_t& operator/=(const float32_t& other) {
        if (this->m == 0) {
            this->s = POSITIVE;
            this->e = 0;
        } else if (other.m == 0) {  // division by zero
            this->m = this->m / other.m; // generate a devide by zero exception
        } else {
            this->s ^= other.s;
            uint64_t mantissa = (uint64_t)this->m;
            this->e -= other.e;

            mantissa <<= 32;
            this->e -= 32;

            int8_t offset = get_lsb_index_64(other.m);
            if (offset > 0) {
                mantissa /= (other.m >> offset);
                this->e -= offset;
            } else {
                mantissa /= other.m;
            }

            offset = 31 - get_msb_index_64((uint64_t)mantissa);
            if (offset > 0) {
                mantissa <<= offset;
                this->e -= offset;
            } else if (offset  < 0) {
                mantissa >>= (-offset);
                this->e -= offset;
            }

            this->m = (uint32_t)mantissa;
        }

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
        uint32_t num_32, sign, mantissa, exponent;

        if (this->m == 0) {
            num_32 = 0;
        } else {
            sign = this->s << 31;
            exponent = ((this->e +127 + 23 + 8) & 0x000000ff) << 23;
            mantissa = (this->m & 0x7fffffff) >> 8;
            num_32 = sign | exponent | mantissa;
        }

        return *((float*)&num_32);
    }
};
