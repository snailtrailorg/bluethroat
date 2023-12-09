#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "user_font_symbols.h"
#include "user_font_utils.h"

#ifndef ANTONIO_REGULAR_10
    #ifdef CONFIG_ANTONIO_REGULAR_10
        #define ANTONIO_REGULAR_10 CONFIG_ANTONIO_REGULAR_10
    #else
        #define ANTONIO_REGULAR_10 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_12
    #ifdef CONFIG_ANTONIO_REGULAR_12
        #define ANTONIO_REGULAR_12 CONFIG_ANTONIO_REGULAR_12
    #else
        #define ANTONIO_REGULAR_12 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_14
    #ifdef CONFIG_ANTONIO_REGULAR_14
        #define ANTONIO_REGULAR_14 CONFIG_ANTONIO_REGULAR_14
    #else
        #define ANTONIO_REGULAR_14 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_16
    #ifdef CONFIG_ANTONIO_REGULAR_16
        #define ANTONIO_REGULAR_16 CONFIG_ANTONIO_REGULAR_16
    #else
        #define ANTONIO_REGULAR_16 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_18
    #ifdef CONFIG_ANTONIO_REGULAR_18
        #define ANTONIO_REGULAR_18 CONFIG_ANTONIO_REGULAR_18
    #else
        #define ANTONIO_REGULAR_18 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_20
    #ifdef CONFIG_ANTONIO_REGULAR_20
        #define ANTONIO_REGULAR_20 CONFIG_ANTONIO_REGULAR_20
    #else
        #define ANTONIO_REGULAR_20 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_24
    #ifdef CONFIG_ANTONIO_REGULAR_24
        #define ANTONIO_REGULAR_24 CONFIG_ANTONIO_REGULAR_24
    #else
        #define ANTONIO_REGULAR_24 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_36
    #ifdef CONFIG_ANTONIO_REGULAR_36
        #define ANTONIO_REGULAR_36 CONFIG_ANTONIO_REGULAR_36
    #else
        #define ANTONIO_REGULAR_36 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_48
    #ifdef CONFIG_ANTONIO_REGULAR_48
        #define ANTONIO_REGULAR_48 CONFIG_ANTONIO_REGULAR_48
    #else
        #define ANTONIO_REGULAR_48 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_60
    #ifdef CONFIG_ANTONIO_REGULAR_60
        #define ANTONIO_REGULAR_60 CONFIG_ANTONIO_REGULAR_60
    #else
        #define ANTONIO_REGULAR_60 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_64
    #ifdef CONFIG_ANTONIO_REGULAR_64
        #define ANTONIO_REGULAR_64 CONFIG_ANTONIO_REGULAR_64
    #else
        #define ANTONIO_REGULAR_64 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_68
    #ifdef CONFIG_ANTONIO_REGULAR_68
        #define ANTONIO_REGULAR_68 CONFIG_ANTONIO_REGULAR_68
    #else
        #define ANTONIO_REGULAR_68 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_72
    #ifdef CONFIG_ANTONIO_REGULAR_72
        #define ANTONIO_REGULAR_72 CONFIG_ANTONIO_REGULAR_72
    #else
        #define ANTONIO_REGULAR_72 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_80
    #ifdef CONFIG_ANTONIO_REGULAR_80
        #define ANTONIO_REGULAR_80 CONFIG_ANTONIO_REGULAR_80
    #else
        #define ANTONIO_REGULAR_80 0
    #endif
#endif
#ifndef ANTONIO_REGULAR_96
    #ifdef CONFIG_ANTONIO_REGULAR_96
        #define ANTONIO_REGULAR_96 CONFIG_ANTONIO_REGULAR_96
    #else
        #define ANTONIO_REGULAR_96 0
    #endif
#endif

#ifndef AWESOME6_12
    #ifdef CONFIG_AWESOME6_12
        #define AWESOME6_12 CONFIG_AWESOME6_12
    #else
        #define AWESOME6_12 0
    #endif
#endif
#ifndef AWESOME6_14
    #ifdef CONFIG_AWESOME6_14
        #define AWESOME6_14 CONFIG_AWESOME6_14
    #else
        #define AWESOME6_14 0
    #endif
#endif
#ifndef AWESOME6_16
    #ifdef CONFIG_AWESOME6_16
        #define AWESOME6_16 CONFIG_AWESOME6_16
    #else
        #define AWESOME6_16 0
    #endif
#endif

#if ANTONIO_REGULAR_10
LV_FONT_DECLARE(antonio_regular_10)
#endif
#if ANTONIO_REGULAR_12
LV_FONT_DECLARE(antonio_regular_12)
#endif
#if ANTONIO_REGULAR_14
LV_FONT_DECLARE(antonio_regular_14)
#endif
#if ANTONIO_REGULAR_16
LV_FONT_DECLARE(antonio_regular_16)
#endif
#if ANTONIO_REGULAR_18
LV_FONT_DECLARE(antonio_regular_18)
#endif
#if ANTONIO_REGULAR_20
LV_FONT_DECLARE(antonio_regular_20)
#endif
#if ANTONIO_REGULAR_24
LV_FONT_DECLARE(antonio_regular_24)
#endif
#if ANTONIO_REGULAR_36
LV_FONT_DECLARE(antonio_regular_36)
#endif
#if ANTONIO_REGULAR_48
LV_FONT_DECLARE(antonio_regular_48)
#endif
#if ANTONIO_REGULAR_60
LV_FONT_DECLARE(antonio_regular_60)
#endif
#if ANTONIO_REGULAR_64
LV_FONT_DECLARE(antonio_regular_64)
#endif
#if ANTONIO_REGULAR_68
LV_FONT_DECLARE(antonio_regular_68)
#endif
#if ANTONIO_REGULAR_72
LV_FONT_DECLARE(antonio_regular_72)
#endif
#if ANTONIO_REGULAR_80
LV_FONT_DECLARE(antonio_regular_80)
#endif
#if ANTONIO_REGULAR_96
LV_FONT_DECLARE(antonio_regular_96)
#endif

#if AWESOME6_12
LV_FONT_DECLARE(awesome6_12)
#endif
#if AWESOME6_14
LV_FONT_DECLARE(awesome6_14)
#endif
#if AWESOME6_16
LV_FONT_DECLARE(awesome6_16)
#endif
