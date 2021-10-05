
#ifndef __LEDDISPLAYDRIVER__
#define __LEDDISPLAYDRIVER__

#include <arduino.h>
#include <print.h>
#include <inttypes.h>
#include <stdio.h>

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// This is version 1.18 from 2021-1-21
// By HKJ from lygte-info.dk

// Only include one of these
//#define _HC164_         // 2 pin connection, requires interrupt
//#define _HC595_         // 3 pin connection, requires interrupt, first HC595 is segments, second is digit
//#define _HC595R_        // 3 pin connection, requires interrupt, first HC595 is segments, second is digit in reverse sequence from above
//#define _HC595A_        // 3 pin connection, requires interrupt, first HC595 is digits, second is segments
//#define _HC595_STATIC_  // 3 pin connection
//#define _HC595A_STATIC_  // 3 pin connection, opposite polarity compare to the above static
//#define _HT1621_6D_     // 3 pin connection with 6 digit
//#define _HT1621_12D_    // 3 pin connection with 12 digit
//#define _HT16K33_4D_    // 2 pin connection with 4 digit and both colon and points
//#define _HT16K33_4DL_   // 2 pin connection with 4 large digit, colon and a couple of dots.
//#define _HT16K33_HKJ_4D_// 2 pin connection with 4 digits, indicators and buttons support
//#define _HT16K33_HKJ_6D_// 2 pin connection with 6 digits, indicators and buttons support
//#define _HT16K33_8D_    // 2 pin connection with 8 digits
//#define _HT16K33_14SEG_8D_// 2 pin connection with 8 alphanumeric 14 segment digits
//#define _HT16K33_14SEG_4DS_// 2 pin connection with 8 alphanumeric 14 segment digits, row/columns swapped
//#define _HT16K33_14SEG_4D_// 2 pin connection with 4 alphanumeric 14 segment digits
//#define _HT16K33_14SEG_HKJ_6D_// 2 pin connection with 6 alphanumeric 14 segment digits
//#define _HT16K33_14SEG_HKJ_6D8_// 2 pin connection with 6 alphanumeric 14 segment digits
//#define _HT16K33_16SEG_HKJ_4D_// 2 pin connection with 4 alphanumeric 16 segment digits, DP not supported
//#define _HT16K33_16SEG_HKJ_4DW_// 2 pin connection with 4 alphanumeric 16 segment digits and external DP control
//#define _MAX7219_       // 3 pin connection
#define _TM1637_        // 2 pin connection
//#define _TM1637_COLON_  // 2 pin connection
//#define _TM1637_6DX_      // 2 pin connection
//#define _TM1638_        // 3 pin connection
//#define _TM1638_QYF_    // 3 pin connection
//#define _TM1638_HWA11_  // 3 pin connection

// For displays that need interrupt (HC164 & HC595) enable one of these or make your own interrupt that calls: display.updateIntr();
//#define _USE_TIMER0_   // Supported on Mega4809(B0)
#define _USE_TIMER1_   // Supported on Mega328 & Mega2560 & Mega32U4 & Mega4809(B1)
//#define _USE_TIMER2_   // Supported on Mega328 & Mega2560 & Mega4809(B2)
//#define _USE_TIMER3_   // Supported on Mega2560 & Mega32U4
//#define _USE_TIMER4_   // Supported on Mega2560 & Mega32U4
//#define _USE_TIMER5_   // Supported on Mega2560

#define MAX_DIGITS 8    // Used to reserve frame buffer

// Limits how fast the pins change state
// Using 10 gives slow and stable signals, 1 is the fastest with any delay and 0 is crazy fast when _FAST_CLOCK_ is enabled
#define DELAY_TIME 10

// Fast clock will use direct IO and is faster when updating, especially when DELAY_TIME is zero
// This is probably only valid on MEGA processors
//#define _FAST_CLOCK_

// The code will be slightly longer when this is enabled, it enables support for long.
// It uses long to unsigned int and show floats, without this long and unsigned int are mapped to int
// It will work on both 4 and 6 digit displays without this, but maximum displayed number is 32767, this restricts float to use 4 digits
// The reasons to comment this out is to get maximum speed and/or if memory is really low
#define _ENABLE_6_DIGITS_

// Include a byte version of show, it is slight faster and smaller than the int version and great for 2 digit numbers (i.e. clocks)
// but may mean that both int and byte versions are included for more memory use
// When not included the int version will be used for bytes
#define _INCLUDE_BYTE_

// Include routines to handle scrolling text
// This uses two bytes of ram for the scrolling delay and some code to initialize the value
#define _INCLUDE_SCROLLING_TEXT_

//*****************************************************************************************************
//*****************************************************************************************************
// There is usual no reason to change anything below here.
//*****************************************************************************************************
//*****************************************************************************************************
// In the next section specific display defines some general options
// This section also includes defines for keys and indicators (leds).
//*****************************************************************************************************
//*****************************************************************************************************

// Display types that needs interrupt support
#if defined(_HC164_) || defined(_HC595_) || defined(_HC595A_) || defined(_HC595R_)
#define _USE_TIMER_
#endif


// Display types that needs a 3. pin, i.e. load
// The condition is reversed because most displays use 3 pins
#if !(defined(_TM1637_) || defined(_TM1637_6DX_) || defined(_TM1637_COLON_)|| defined(_HC164_) || defined(_HT16K33_8D_) || defined(_HT16K33_14SEG_8D_) || defined(_HT16K33_14SEG_4D_) || defined(_HT16K33_4D_))
#define _DATA_BIDIRECTIONAL_
#define _USE_LOAD_
#endif

#ifdef _HT16K33_8D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#endif

#ifdef _HT16K33_14SEG_8D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define SEGMENTS_14
#endif

#ifdef _HT16K33_14SEG_4D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define SEGMENTS_14
#endif

#ifdef _HT16K33_14SEG_4DS_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define SEGMENTS_14
// .NOPMLKHGFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<1)
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_H (1<<8)

#define SEG_K (1<<9)
#define SEG_L (1<<10)
#define SEG_M (1<<11)
#define SEG_P (1<<12)
#define SEG_O (1<<13)
#define SEG_N (1<<14)
#define SEG_DP (1<<15)
#endif


#ifdef _HT16K33_14SEG_HKJ_6D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8
#define SEGMENTS_14
//    .HNOPHMLKFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<1)
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<13)
#define SEG_H (1<<9)
#define SEG_K (1<<6)
#define SEG_L (1<<7)
#define SEG_M (1<<8)
#define SEG_N (1<<12)
#define SEG_O (1<<11)
#define SEG_P (1<<10)
#define SEG_DP (1<<14)

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<4)
#define LED_1G (1<<5)
#define LED_1B (1<<6)
#define LED_2R (1<<8)
#define LED_2G (1<<9)
#define LED_2B (1<<10)
#define LED_3R (1<<12)
#define LED_3G (1<<13)
#define LED_3B (1<<14)
#define LED_4R (1L<<16)
#define LED_4G (1L<<17)
#define LED_4B (1L<<18)
#define LED_5R (1L<<20)
#define LED_5G (1L<<21)
#define LED_5B (1L<<22)
#endif

#ifdef _HT16K33_14SEG_HKJ_6D8_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8
#define SEGMENTS_14
//    .HNOPHMLKFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<9)
#define SEG_C (1<<10)
#define SEG_D (1<<8)
#define SEG_E (1<<2)
#define SEG_F (1<<1)
#define SEG_G (1<<3)
#define SEG_H (1<<11)
#define SEG_K (1<<5)
#define SEG_L (1<<4)
#define SEG_M (1<<13)
#define SEG_N (1<<6)
#define SEG_O (1<<12)
#define SEG_P (1<<14)
#define SEG_DP (1<<15)

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<4)
#define LED_1G (1<<5)
#define LED_1B (1<<6)
#define LED_2R (1<<8)
#define LED_2G (1<<9)
#define LED_2B (1<<10)
#define LED_3R (1<<12)
#define LED_3G (1<<13)
#define LED_3B (1<<14)
#define LED_4R (1L<<16)
#define LED_4G (1L<<17)
#define LED_4B (1L<<18)
#define LED_5R (1L<<20)
#define LED_5G (1L<<21)
#define LED_5B (1L<<22)
#endif


#ifdef _HT16K33_16SEG_HKJ_4D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint16_t
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8
#define SEGMENTS_16
// SRPONMLKHGFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<1)
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_H (1<<7)
#define SEG_K (1<<8)
#define SEG_L (1<<9)
#define SEG_M (1<<10)
#define SEG_N (1<<15)
#define SEG_O (1<<11)
#define SEG_P (1<<14)
#define SEG_R (1<<12)
#define SEG_S (1<<13)
#define SEG_DP (0)

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<3)
#define LED_1G (1<<4)
#define LED_1B (1<<5)
#define LED_2R (1<<6)
#define LED_2G (1<<7)
#define LED_2B (1<<8)
#define LED_3R (1<<9)
#define LED_3G (1<<10)
#define LED_3B (1<<11)
#endif

#ifdef _HT16K33_16SEG_HKJ_4DW_
#define _DATA_BIDIRECTIONAL_
#define _EXTRA_PINS_
#define MAX_BRIGHTNESS 15
#define DIGIT_TYPE uint32_t
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8
#define SEGMENTS_16
// SRPONMLKHGFEDCBA
#define SEG_A (1U<<0)
#define SEG_B (1U<<1)
#define SEG_C (1U<<2)
#define SEG_D (1U<<3)
#define SEG_E (1U<<4)
#define SEG_F (1U<<5)
#define SEG_G (1U<<6)
#define SEG_H (1U<<7)
#define SEG_K (1U<<8)
#define SEG_L (1U<<9)
#define SEG_M (1U<<10)
#define SEG_N (1U<<15)
#define SEG_O (1U<<11)
#define SEG_P (1U<<14)
#define SEG_R (1U<<12)
#define SEG_S (1U<<13)
#define SEG_DP (1UL<<16)

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<3)
#define LED_1G (1<<4)
#define LED_1B (1<<5)
#define LED_2R (1<<6)
#define LED_2G (1<<7)
#define LED_2B (1<<8)
#define LED_3R (1<<9)
#define LED_3G (1<<10)
#define LED_3B (1<<11)
#endif




#ifdef _HT16K33_4D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define _USE_INDICATOR_
#define INDICATOR_TYPE byte
#define COLON_DISP_0 (1<<0)
#define COLON_DISP_1 (1<<1)
#define COLON_DISP_2 (1<<2)
#define COLON_DISP_3 (1<<3)
#define COLON_DISP_4 (1<<4)
#define COLON_DISP_5 (1<<5)
#define COLON_DISP_6 (1<<6)
#define COLON_DISP_7 (1<<7)
#endif

#ifdef _HT16K33_4DL_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define COLON_DISP_0 (1L<<0)
#define DOT_FT_DISP_0 (1L<<1)
#define DOT_FB_DISP_0 (1L<<2)
#define DOT_LT_DISP_0 (1L<<3)
#define COLON_DISP_1 (1L<<4)
#define DOT_FT_DISP_1 (1L<<5)
#define DOT_FB_DISP_1 (1L<<6)
#define DOT_LT_DISP_1 (1L<<7)
#define COLON_DISP_2 (1L<<8)
#define DOT_FT_DISP_2 (1L<9)
#define DOT_FB_DISP_2 (1L<<10)
#define DOT_LT_DISP_2 (1L<<11)
#define COLON_DISP_3 (1L<<12)
#define DOT_FT_DISP_3 (1L<<12)
#define DOT_FB_DISP_3 (1L<<14)
#define DOT_LT_DISP_3 (1L<<15)
#define COLON_DISP_4 (1L<<16)
#define DOT_FT_DISP_4 (1L<<17)
#define DOT_FB_DISP_4 (1L<<18)
#define DOT_LT_DISP_4 (1L<<19)
#define COLON_DISP_5 (1L<<20)
#define DOT_FT_DISP_5 (1L<<21)
#define DOT_FB_DISP_5 (1L<<22)
#define DOT_LT_DISP_5 (1L<<23)
#define COLON_DISP_6 (1L<<24)
#define DOT_FT_DISP_6 (1L<<25)
#define DOT_FB_DISP_6 (1L<<26)
#define DOT_LT_DISP_6 (1L<<27)
#define COLON_DISP_7 (1L<<28)
#define DOT_FT_DISP_7 (1L<<26)
#define DOT_FB_DISP_7 (1L<<30)
#define DOT_LT_DISP_7 (1L<<31)
#endif

#ifdef _HT16K33_HKJ_4D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint16_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<4)
#define LED_1G (1<<5)
#define LED_1B (1<<6)
#define LED_2R (1<<8)
#define LED_2G (1<<9)
#define LED_2B (1<<10)
#define LED_3R (1<<12)
#define LED_3G (1<<13)
#define LED_3B (1<<14)
#endif


#ifdef _HT16K33_HKJ_6D_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 15
#define _USE_INDICATOR_
#define INDICATOR_TYPE uint32_t
#define KEY_TYPE byte
#define _KEYS_
#define KEY_S1 1
#define KEY_S2 2
#define KEY_S3 3
#define KEY_S4 4
#define KEY_S5 5
#define KEY_S6 6
#define KEY_S7 7
#define KEY_S8 8

#define LED_0R (1<<0)
#define LED_0G (1<<1)
#define LED_0B (1<<2)
#define LED_1R (1<<4)
#define LED_1G (1<<5)
#define LED_1B (1<<6)
#define LED_2R (1<<8)
#define LED_2G (1<<9)
#define LED_2B (1<<10)
#define LED_3R (1<<12)
#define LED_3G (1<<13)
#define LED_3B (1<<14)
#define LED_4R (1L<<16)
#define LED_4G (1L<<17)
#define LED_4B (1L<<18)
#define LED_5R (1L<<20)
#define LED_5G (1L<<21)
#define LED_5B (1L<<22)
#endif



#ifdef _MAX7219_
#define MAX_BRIGHTNESS 15
#endif

// Some segments need direct access with showIndicators(xxx)
#ifdef _HT1621_6D_
#define _USE_INDICATOR_
#define INDICATOR_TYPE byte
#define BATT_LOW 0x04
#define BATT_MEDIUM 0x02
#define BATT_HIGH 0x01
#endif

#ifdef _HT1621_12D_
#define _USE_INDICATOR_
#define INDICATOR_TYPE byte
#if MAX_DIGITS<12
#undef MAX_DIGITS
#define MAX_DIGITS 12
#endif
#endif


#ifdef _TM1637_6DX_
#define _DATA_BIDIRECTIONAL_
#define MAX_BRIGHTNESS 7
#endif


#ifdef _TM1637_
#define _DATA_BIDIRECTIONAL_
#define KEY_TYPE unsigned int
#define _KEYS_
#define KEY_S1 6
#define KEY_S2 5
#define KEY_S3 4
#define KEY_S4 3
#define KEY_S5 2
#define KEY_S6 1
#define MAX_BRIGHTNESS 7
#endif

#ifdef _TM1637_COLON_
#define _DATA_BIDIRECTIONAL_
#define _USE_INDICATOR_
#define INDICATOR_TYPE byte
#define COLON_DISP_0 1
#define MAX_BRIGHTNESS 7
#endif


#ifdef _TM1638_
#define _DATA_BIDIRECTIONAL_
#define INDICATOR_TYPE uint16_t
#define _USE_INDICATOR_
#define KEY_TYPE unsigned long
#define _KEYS_
#define KEY_S1 32
#define KEY_S2 24
#define KEY_S3 16
#define KEY_S4 8
#define KEY_S5 28
#define KEY_S6 20
#define KEY_S7 12
#define KEY_S8 4
#define LED_1 (1<<0)
#define LED_2 (1<<2)
#define LED_3 (1<<4)
#define LED_4 (1<<6)
#define LED_5 (1<<8)
#define LED_6 (1<<10)
#define LED_7 (1<<12)
#define LED_8 (1<<14)
#define LED_1G (1<<0)
#define LED_1R (1<<1)
#define LED_2G (1<<2)
#define LED_2R (1<<3)
#define LED_3G (1<<4)
#define LED_3R (1<<5)
#define LED_4G (1<<6)
#define LED_4R (1<<7)
#define LED_5G (1<<8)
#define LED_5R (1<<9)
#define LED_6G (1<<10)
#define LED_6R (1<<11)
#define LED_7G (1<<12)
#define LED_7R (1<<13)
#define LED_8G (1<<14)
#define LED_8R (1<<15)
#define MAX_BRIGHTNESS 7
#endif


#ifdef _TM1638_QYF_
#define _DATA_BIDIRECTIONAL_
#define KEY_TYPE unsigned long
#define _KEYS_
#define KEY_S1 30
#define KEY_S2 14
#define KEY_S3 31
#define KEY_S4 15
#define KEY_S5 26
#define KEY_S6 10
#define KEY_S7 27
#define KEY_S8 11
#define KEY_S9 22
#define KEY_S10 6
#define KEY_S11 23
#define KEY_S12 7
#define KEY_S13 18
#define KEY_S14 2
#define KEY_S15 19
#define KEY_S16 3
#define MAX_BRIGHTNESS 7
#endif


#ifdef _TM1638_HWA11_
#define _DATA_BIDIRECTIONAL_
#define INDICATOR_TYPE uint16_t
#define _USE_INDICATOR_
#define LED_1 (1<<0)
#define LED_2 (1<<1)
#define KEY_K1 30
#define KEY_K2 31
#define KEY_K3 32
#define KEY_K4 26
#define KEY_K5 27
#define KEY_K6 28
#define KEY_K7 22
#define KEY_K8 23
#define KEY_K9 24
#define KEY_K10 18
#define KEY_K11 19
#define KEY_K12 20
#define KEY_K13 14
#define KEY_K14 15
#define KEY_K15 16
#define KEY_K16 10
#define KEY_K17 11
#define KEY_K18 12
#define KEY_K19 6
#define KEY_K20 7
#define KEY_K21 8
#define KEY_K22 2
#define KEY_K23 3
#define KEY_K24 4
#define KEY_TYPE unsigned long
#define _KEYS_
#define MAX_BRIGHTNESS 7
#endif

#ifndef DIGIT_TYPE
#define DIGIT_TYPE byte
#define SEGMENTS_7
#endif

#ifndef MAX_BRIGHTNESS
#define MAX_BRIGHTNESS 0
#endif

#ifndef _USE_TIMER_
// Do not implement a interrupt handle for displays that do not require it.
#undef _USE_TIMER0_
#undef _USE_TIMER1_
#undef _USE_TIMER2_
#undef _USE_TIMER3_
#undef _USE_TIMER4_
#undef _USE_TIMER5_
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// Defined segments for 7 segments display, when SEG_A..SEG_F and SEG_DP is defined all the digit will be defined
// using them
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef SEGMENTS_7
#if defined(_MAX7219_)
//                .ABCDEFG
#define SEG_A   0b01000000
#define SEG_B   0b00100000
#define SEG_C   0b00010000
#define SEG_D   0b00001000
#define SEG_E   0b00000100
#define SEG_F   0b00000010
#define SEG_G   0b00000001
#define SEG_DP  0b10000000
#endif
#if defined(_HT1621_6D_)
//                .CBADEGF
#define SEG_A   0b00010000
#define SEG_B   0b00100000
#define SEG_C   0b01000000
#define SEG_D   0b00001000
#define SEG_E   0b00000100
#define SEG_F   0b00000001
#define SEG_G   0b00000010
#define SEG_DP  0b10000000
#endif
#if defined(_HT1621_12D_)
//                FGEDABC.
#define SEG_A   0b00001000
#define SEG_B   0b00000100
#define SEG_C   0b00000010
#define SEG_D   0b00010000
#define SEG_E   0b00100000
#define SEG_F   0b10000000
#define SEG_G   0b01000000
#define SEG_DP  0b00000001
#endif

#ifndef SEG_A
// This is the most common segment definition, use it when nothing else is defined
//                .GFEDCBA
#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000
#define SEG_DP  0b10000000
#endif


const DIGIT_TYPE digit0 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digit1 = SEG_B | SEG_C;
const DIGIT_TYPE digit2 = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G;
const DIGIT_TYPE digit3 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G;
const DIGIT_TYPE digit4 = SEG_B | SEG_C | SEG_F | SEG_G;
const DIGIT_TYPE digit5 = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G;
const DIGIT_TYPE digit6 = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digit7 = SEG_A | SEG_B | SEG_C;
const DIGIT_TYPE digit8 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digit9 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;

// Ascii symbols
const DIGIT_TYPE digitSpace = 0;
const DIGIT_TYPE digitQuote2 =  SEG_B | SEG_F;
const DIGIT_TYPE digitQuote1 =  SEG_B;
const DIGIT_TYPE digitOpen = SEG_A | SEG_D | SEG_E | SEG_F;  // Opening bracket [
const DIGIT_TYPE digitClose = SEG_A | SEG_B | SEG_C | SEG_D;  // Closing bracket ]
const DIGIT_TYPE digitAsterix = 0;
const DIGIT_TYPE digitPlus = 0;
const DIGIT_TYPE digitDp = SEG_DP;              // Decimal point or colon
const DIGIT_TYPE digitMinus = SEG_G;
const DIGIT_TYPE digitSlash = digit1;
const DIGIT_TYPE digitLT = 0;
const DIGIT_TYPE digitEqual = SEG_D | SEG_G;
const DIGIT_TYPE digitGT = 0;
const DIGIT_TYPE digitBackslash = digit1;
const DIGIT_TYPE digitUnderscore = SEG_D;
const DIGIT_TYPE digitVerticalslash = digit1;



// Upper case letters, some of the letters are very bad.
const DIGIT_TYPE digitA = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitB = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitC = SEG_A | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitD = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
const DIGIT_TYPE digitE = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitF = SEG_A | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitG = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitH = SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitI = SEG_B | SEG_C;
const DIGIT_TYPE digitJ = SEG_B | SEG_C | SEG_D | SEG_E;
const DIGIT_TYPE digitK = SEG_A | SEG_C | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitL = SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitM = SEG_A | SEG_C | SEG_E;
const DIGIT_TYPE digitN = SEG_C | SEG_E | SEG_G;
const DIGIT_TYPE digitO = SEG_C | SEG_D | SEG_E | SEG_G;
const DIGIT_TYPE digitP = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitQ = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G;
const DIGIT_TYPE digitR = SEG_E | SEG_G;
const DIGIT_TYPE digitS = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G;
const DIGIT_TYPE digitT = SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitU = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitV = SEG_C | SEG_D | SEG_E;
const DIGIT_TYPE digitW = SEG_B | SEG_D | SEG_F;
const DIGIT_TYPE digitX = digitH;
const DIGIT_TYPE digitY = SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
const DIGIT_TYPE digitZ = digit2;

// Lower case letters, many are the same as upper case
const DIGIT_TYPE digita = digitA;
const DIGIT_TYPE digitb = digitB;
const DIGIT_TYPE digitc = SEG_D | SEG_E | SEG_G;
const DIGIT_TYPE digitd = digitD;
const DIGIT_TYPE digite = digitE;
const DIGIT_TYPE digitf = digitF;
const DIGIT_TYPE digitg = digitG;
const DIGIT_TYPE digith = SEG_C | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digiti = SEG_C;
const DIGIT_TYPE digitj = digitJ;
const DIGIT_TYPE digitk = digitK;
const DIGIT_TYPE digitl = digitL;
const DIGIT_TYPE digitm = digitM;
const DIGIT_TYPE digitn = digitN;
const DIGIT_TYPE digito = digitO;
const DIGIT_TYPE digitp = digitP;
const DIGIT_TYPE digitq = digitQ;
const DIGIT_TYPE digitr = digitR;
const DIGIT_TYPE digits = digitS;
const DIGIT_TYPE digitt = digitT;
const DIGIT_TYPE digitu = SEG_C | SEG_D | SEG_E;
const DIGIT_TYPE digitv = digitV;
const DIGIT_TYPE digitw = digitW;
const DIGIT_TYPE digitx = digitX;
const DIGIT_TYPE digity = digitY;
const DIGIT_TYPE digitz = digitZ;

// Other symbols, not in the standard ascii characters
const DIGIT_TYPE digitEmpty = 0;
const DIGIT_TYPE digitTop = SEG_A;              // Top segment
const DIGIT_TYPE digitBottom = SEG_D;           // Bottom segment
const DIGIT_TYPE digitRight = SEG_B | SEG_C;    // Right vertial bar (Same as 1)
const DIGIT_TYPE digitLeft = SEG_E | SEG_F;     // Left vertical bar
const DIGIT_TYPE digitDeg = SEG_A | SEG_B | SEG_F | SEG_G;  // Degree symbol
const DIGIT_TYPE digitAll = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP;

// Symbols for showing binary digits
const DIGIT_TYPE digitSmall0 = SEG_C | SEG_D | SEG_E | SEG_G;
const DIGIT_TYPE digitSmall1 = SEG_C;
const DIGIT_TYPE digitSmallTop0 = SEG_A | SEG_B | SEG_F | SEG_G;
const DIGIT_TYPE digitSmallTop1 = SEG_B;
const DIGIT_TYPE digitLeftBottom1 = SEG_E;
const DIGIT_TYPE digitRightBottom1 = SEG_C;
const DIGIT_TYPE digitLeftTop1 = SEG_F;
const DIGIT_TYPE digitRightTop1 = SEG_B;
const DIGIT_TYPE digitLeft1 = SEG_E | SEG_F;
const DIGIT_TYPE digitRight1 = SEG_C | SEG_B;
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// Defined segments for 14 segments alphanumeric display, when SEG_A..SEG_P and SEG_DP is defined all
// the digit will be defined using them
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef SEGMENTS_14

#ifndef SEG_A
// .PONMLKHGFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<1)
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_H (1<<7)
#define SEG_K (1<<8)
#define SEG_L (1<<9)
#define SEG_M (1<<10)
#define SEG_N (1<<11)
#define SEG_O (1<<12)
#define SEG_P (1<<13)
#define SEG_DP (1<<14)
#endif

// 14 segment definitions
// Numbers
const DIGIT_TYPE digit0 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digit1 = SEG_B | SEG_C | SEG_M;
const DIGIT_TYPE digit2 = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G | SEG_H;
const DIGIT_TYPE digit3 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_H;
const DIGIT_TYPE digit4 = SEG_B | SEG_C | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digit5 = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digit6 = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digit7 = SEG_A | SEG_M | SEG_O;
const DIGIT_TYPE digit8 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digit9 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G | SEG_H;

// Ascii symbols
const DIGIT_TYPE digitSpace = 0;
const DIGIT_TYPE digitQuote2 =  SEG_B | SEG_L;
const DIGIT_TYPE digitQuote1 =  SEG_L;
const DIGIT_TYPE digitOpen = SEG_A | SEG_D | SEG_E | SEG_F;  // Opening bracket [
const DIGIT_TYPE digitClose = SEG_A | SEG_B | SEG_C | SEG_D;  // Closing bracket ]
const DIGIT_TYPE digitAsterix = SEG_G | SEG_H | SEG_M | SEG_K | SEG_N | SEG_P;
const DIGIT_TYPE digitPlus = SEG_G | SEG_H | SEG_L | SEG_O;
const DIGIT_TYPE digitDp = SEG_DP;              // Decimal point or colon
const DIGIT_TYPE digitMinus = SEG_G | SEG_H;
const DIGIT_TYPE digitSlash = SEG_M | SEG_N;
const DIGIT_TYPE digitLT = SEG_M | SEG_P;
const DIGIT_TYPE digitEqual = SEG_D | SEG_G | SEG_H;
const DIGIT_TYPE digitGT = SEG_K | SEG_N;
const DIGIT_TYPE digitBackslash = SEG_K | SEG_P;
const DIGIT_TYPE digitUnderscore = SEG_D;
const DIGIT_TYPE digitVerticalslash = SEG_L | SEG_O;

// Upper case letters
const DIGIT_TYPE digitA = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitB = SEG_A | SEG_B | SEG_C | SEG_D | SEG_H | SEG_L | SEG_O;
const DIGIT_TYPE digitC = SEG_A | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitD = SEG_A | SEG_B | SEG_C | SEG_D | SEG_L | SEG_O;
const DIGIT_TYPE digitE = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitF = SEG_A | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitG = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F  | SEG_H;
const DIGIT_TYPE digitH = SEG_B | SEG_C | SEG_E | SEG_F | SEG_G  | SEG_H;
const DIGIT_TYPE digitI = SEG_A | SEG_D | SEG_L | SEG_O;
const DIGIT_TYPE digitJ = SEG_B | SEG_C | SEG_D | SEG_E;
const DIGIT_TYPE digitK = SEG_E | SEG_F | SEG_G | SEG_M | SEG_P;
const DIGIT_TYPE digitL = SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitM = SEG_B | SEG_C | SEG_E | SEG_F | SEG_K | SEG_M;
const DIGIT_TYPE digitN = SEG_B | SEG_C | SEG_E | SEG_F | SEG_K | SEG_P;
const DIGIT_TYPE digitO = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitP = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitQ = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_P;
const DIGIT_TYPE digitR = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G | SEG_H | SEG_P;
const DIGIT_TYPE digitS = SEG_A | SEG_C | SEG_D | SEG_H | SEG_K;
const DIGIT_TYPE digitT = SEG_A | SEG_L | SEG_O;
const DIGIT_TYPE digitU = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const DIGIT_TYPE digitV = SEG_E | SEG_F | SEG_M | SEG_N;
const DIGIT_TYPE digitW = SEG_B | SEG_C | SEG_E | SEG_F | SEG_N | SEG_P;
const DIGIT_TYPE digitX = SEG_K | SEG_M | SEG_N | SEG_P;
const DIGIT_TYPE digitY = SEG_B | SEG_F | SEG_G | SEG_H | SEG_O;
const DIGIT_TYPE digitZ = SEG_A | SEG_D | SEG_M | SEG_N;

// Lower case letters, a few are duplicates of upper case letters
const DIGIT_TYPE digita =  SEG_D | SEG_E | SEG_G | SEG_O;
const DIGIT_TYPE digitb =  SEG_D | SEG_E | SEG_F | SEG_G | SEG_P;
const DIGIT_TYPE digitc = SEG_D | SEG_E | SEG_G | SEG_H;
const DIGIT_TYPE digitd = SEG_B | SEG_C | SEG_D | SEG_H | SEG_N;
const DIGIT_TYPE digite = SEG_D | SEG_E | SEG_G | SEG_N;
const DIGIT_TYPE digitf = SEG_A | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitg = SEG_B | SEG_C | SEG_D | SEG_H | SEG_M;
const DIGIT_TYPE digith = SEG_E | SEG_F | SEG_G | SEG_O;
const DIGIT_TYPE digiti = SEG_O;
const DIGIT_TYPE digitj = SEG_B | SEG_C | SEG_D;
const DIGIT_TYPE digitk = SEG_L | SEG_M | SEG_O | SEG_P;
const DIGIT_TYPE digitl =  SEG_L | SEG_O;
const DIGIT_TYPE digitm =  SEG_C | SEG_E | SEG_G | SEG_H | SEG_O;
const DIGIT_TYPE digitn =  SEG_E | SEG_G | SEG_O;
const DIGIT_TYPE digito =  SEG_C | SEG_D | SEG_E | SEG_G | SEG_H;
const DIGIT_TYPE digitp =  SEG_A | SEG_E | SEG_F | SEG_G | SEG_M;
const DIGIT_TYPE digitq =  SEG_A | SEG_B | SEG_C | SEG_H | SEG_K;
const DIGIT_TYPE digitr =  SEG_E | SEG_G;
const DIGIT_TYPE digits =  SEG_D | SEG_H | SEG_P;
const DIGIT_TYPE digitt =  SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitu =  SEG_C | SEG_D | SEG_E;
const DIGIT_TYPE digitv =  SEG_E | SEG_N;
const DIGIT_TYPE digitw =  SEG_C | SEG_E | SEG_N | SEG_P;
const DIGIT_TYPE digitx = digitX;
const DIGIT_TYPE digity = SEG_B | SEG_C | SEG_D | SEG_H | SEG_L;
const DIGIT_TYPE digitz =  SEG_D | SEG_G | SEG_N;

// Other symbols, not in the standard ascii characters
const DIGIT_TYPE digitEmpty = 0;
const DIGIT_TYPE digitTop = SEG_A;              // Top segment
const DIGIT_TYPE digitBottom = SEG_D;           // Bottom segment
const DIGIT_TYPE digitRight = SEG_B | SEG_C;    // Right vertial bar (Same as 1)
const DIGIT_TYPE digitLeft = SEG_E | SEG_F;     // Left vertical bar
const DIGIT_TYPE digitDeg = SEG_F | SEG_K | SEG_G;  // Degree symbol
const DIGIT_TYPE digitAll = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G  | SEG_H | SEG_K | SEG_L | SEG_M | SEG_N | SEG_O | SEG_P | SEG_DP;

// Symbols for showing binary digits
const DIGIT_TYPE digitSmall0 = SEG_C | SEG_D | SEG_E | SEG_G | SEG_H;
const DIGIT_TYPE digitSmall1 = SEG_O;
const DIGIT_TYPE digitSmallTop0 = SEG_A | SEG_B | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitSmallTop1 = SEG_B;
const DIGIT_TYPE digitLeftBottom1 = SEG_E;
const DIGIT_TYPE digitRightBottom1 = SEG_C;
const DIGIT_TYPE digitLeftTop1 = SEG_F;
const DIGIT_TYPE digitRightTop1 = SEG_B;
const DIGIT_TYPE digitLeft1 = SEG_E | SEG_F;
const DIGIT_TYPE digitRight1 = SEG_C | SEG_B;

#endif

#ifdef SEGMENTS_16

#ifndef SEG_A
// .RSPONMLKHGFEDCBA
#define SEG_A (1<<0)
#define SEG_B (1<<1)
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_H (1<<7)

#define SEG_K (1<<8)
#define SEG_L (1<<9)
#define SEG_M (1<<10)
#define SEG_N (1<<11)
#define SEG_O (1<<12)
#define SEG_P (1<<13)
#define SEG_R (1<<15)
#define SEG_S (1<<14)
#define SEG_DP (0)    // Not valid with a 16 bit wide buffer
#endif

// 16 segment definitions
// Numbers
const DIGIT_TYPE digit0 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digit1 = SEG_C | SEG_D | SEG_M;
const DIGIT_TYPE digit2 = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G | SEG_N | SEG_O;
const DIGIT_TYPE digit3 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_O;
const DIGIT_TYPE digit4 = SEG_C | SEG_D | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digit5 = SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digit6 = SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digit7 = SEG_A | SEG_B | SEG_M | SEG_R;
const DIGIT_TYPE digit8 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digit9 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_H | SEG_N | SEG_O;

// Ascii symbols
const DIGIT_TYPE digitSpace = 0;
const DIGIT_TYPE digitQuote2 =  SEG_C | SEG_L;
const DIGIT_TYPE digitQuote1 =  SEG_L;
const DIGIT_TYPE digitOpen = SEG_B | SEG_E | SEG_L | SEG_R;  // Opening bracket
const DIGIT_TYPE digitClose = SEG_A | SEG_F | SEG_L | SEG_R;  // Closing bracket
const DIGIT_TYPE digitAsterix = SEG_K | SEG_M | SEG_N | SEG_O | SEG_P | SEG_S;
const DIGIT_TYPE digitPlus = SEG_L | SEG_N | SEG_O | SEG_R;
const DIGIT_TYPE digitDp = SEG_DP;              // Decimal point or colon
const DIGIT_TYPE digitMinus = SEG_N | SEG_O;
const DIGIT_TYPE digitSlash = SEG_M | SEG_P;
const DIGIT_TYPE digitLT = SEG_M | SEG_S;
const DIGIT_TYPE digitEqual = SEG_F | SEG_E | SEG_N | SEG_O;
const DIGIT_TYPE digitGT = SEG_K | SEG_P;
const DIGIT_TYPE digitBackslash = SEG_K | SEG_S;
const DIGIT_TYPE digitUnderscore = SEG_F | SEG_E;
const DIGIT_TYPE digitVerticalslash = SEG_L | SEG_R;

// Upper case letters
const DIGIT_TYPE digitA = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digitB = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_L | SEG_O | SEG_R;
const DIGIT_TYPE digitC = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitD = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_L | SEG_R;
const DIGIT_TYPE digitE = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G | SEG_H | SEG_N;
const DIGIT_TYPE digitF = SEG_A | SEG_B | SEG_G | SEG_H | SEG_N;
const DIGIT_TYPE digitG = SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H | SEG_O;
const DIGIT_TYPE digitH = SEG_C | SEG_D | SEG_G | SEG_H | SEG_N | SEG_O;
const DIGIT_TYPE digitI = SEG_A | SEG_B | SEG_E | SEG_F | SEG_L | SEG_R;
const DIGIT_TYPE digitJ = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const DIGIT_TYPE digitK = SEG_G | SEG_H | SEG_M | SEG_N | SEG_S;
const DIGIT_TYPE digitL = SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitM = SEG_C | SEG_D | SEG_G | SEG_H | SEG_K | SEG_M;
const DIGIT_TYPE digitN = SEG_C | SEG_D | SEG_G | SEG_H | SEG_K | SEG_S;
const DIGIT_TYPE digitO = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitP = SEG_A | SEG_B | SEG_G | SEG_H | SEG_M | SEG_N;
const DIGIT_TYPE digitQ = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H | SEG_S;
const DIGIT_TYPE digitR = SEG_A | SEG_B | SEG_C | SEG_G | SEG_H | SEG_N | SEG_O | SEG_S;
const DIGIT_TYPE digitS = SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_K | SEG_O;
const DIGIT_TYPE digitT = SEG_A | SEG_B | SEG_L | SEG_R;
const DIGIT_TYPE digitU = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_H;
const DIGIT_TYPE digitV = SEG_G | SEG_H | SEG_M | SEG_P;
const DIGIT_TYPE digitW = SEG_C | SEG_D | SEG_G | SEG_H | SEG_P | SEG_R;
const DIGIT_TYPE digitX = SEG_K | SEG_M | SEG_P | SEG_S;
const DIGIT_TYPE digitY = SEG_C | SEG_H | SEG_N | SEG_O | SEG_R;
const DIGIT_TYPE digitZ = SEG_A | SEG_B | SEG_F | SEG_E | SEG_M | SEG_P;

// Lower case letters, a few are duplicates of upper case letters
const DIGIT_TYPE digita = SEG_E | SEG_F | SEG_G | SEG_N | SEG_R;
const DIGIT_TYPE digitb = SEG_F | SEG_G | SEG_H | SEG_N | SEG_R;
const DIGIT_TYPE digitc = SEG_F | SEG_G | SEG_N;
const DIGIT_TYPE digitd = SEG_C | SEG_D | SEG_E | SEG_O | SEG_R;
const DIGIT_TYPE digite = SEG_F | SEG_G | SEG_N | SEG_P;
const DIGIT_TYPE digitf = SEG_B | SEG_L | SEG_N | SEG_O | SEG_R;
const DIGIT_TYPE digitg = SEG_C | SEG_D | SEG_E | SEG_M | SEG_O;
const DIGIT_TYPE digith = SEG_G | SEG_H | SEG_N | SEG_R;
const DIGIT_TYPE digiti = SEG_R;
const DIGIT_TYPE digitj = SEG_F | SEG_G | SEG_L | SEG_R;
const DIGIT_TYPE digitk = SEG_L | SEG_M | SEG_R | SEG_S;
const DIGIT_TYPE digitl = SEG_L | SEG_R;
const DIGIT_TYPE digitm = SEG_D | SEG_G | SEG_N | SEG_O | SEG_R;
const DIGIT_TYPE digitn = SEG_G | SEG_N | SEG_R;
const DIGIT_TYPE digito = SEG_F | SEG_G | SEG_N | SEG_R;
const DIGIT_TYPE digitp = SEG_A | SEG_G | SEG_H | SEG_L | SEG_N;
const DIGIT_TYPE digitq = SEG_A | SEG_H | SEG_L | SEG_N | SEG_R;
const DIGIT_TYPE digitr = SEG_G | SEG_N;
const DIGIT_TYPE digits = SEG_E | SEG_O | SEG_S;
const DIGIT_TYPE digitt = SEG_F | SEG_G | SEG_H | SEG_N;
const DIGIT_TYPE digitu = SEG_F | SEG_G | SEG_R;
const DIGIT_TYPE digitv = SEG_G | SEG_P;
const DIGIT_TYPE digitw = SEG_D | SEG_G | SEG_P | SEG_S;
const DIGIT_TYPE digitx = digitX;
const DIGIT_TYPE digity = SEG_C | SEG_D | SEG_E | SEG_L | SEG_O;
const DIGIT_TYPE digitz = SEG_F | SEG_N | SEG_P;

// Other symbols, not in the standard ascii characters
const DIGIT_TYPE digitEmpty = 0;
const DIGIT_TYPE digitTop = SEG_A | SEG_B;      // Top segment
const DIGIT_TYPE digitBottom = SEG_E | SEG_F;   // Bottom segment
const DIGIT_TYPE digitRight = SEG_C | SEG_D;    // Right vertial bar (Same as 1)
const DIGIT_TYPE digitLeft = SEG_H | SEG_G;     // Left vertical bar
const DIGIT_TYPE digitDeg = SEG_H | SEG_K | SEG_N; // Degree symbol
const DIGIT_TYPE digitAll = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G  | SEG_H | SEG_K | SEG_L | SEG_M | SEG_N | SEG_O | SEG_P | SEG_R | SEG_S | SEG_DP;

// Symbols for showing binary digits
const DIGIT_TYPE digitSmall0 = SEG_F | SEG_G | SEG_N | SEG_R;
const DIGIT_TYPE digitSmall1 = SEG_R;
const DIGIT_TYPE digitSmallTop0 = SEG_A | SEG_H | SEG_L | SEG_N;
const DIGIT_TYPE digitSmallTop1 = SEG_L;
const DIGIT_TYPE digitLeftBottom1 = SEG_G;
const DIGIT_TYPE digitRightBottom1 = SEG_D;
const DIGIT_TYPE digitLeftTop1 = SEG_H;
const DIGIT_TYPE digitRightTop1 = SEG_C;
const DIGIT_TYPE digitLeft1 = SEG_G | SEG_H;
const DIGIT_TYPE digitRight1 = SEG_C | SEG_D;
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// LEDDisplayDriver
//*****************************************************************************************************
//*****************************************************************************************************

// All patters for 0-F in a array
extern const DIGIT_TYPE segmentPatterns[];

class LEDDisplayDriver {
    friend class LEDLCDAPIDriver;
  private:

    //*****************************************************************************************************
    //*****************************************************************************************************
    // Routines and variables to handle IO, they exist in a fast version with direct IO and standard Arduino version
    // Bidirectional and load pin code and data will be included when needed.
    //*****************************************************************************************************
    //*****************************************************************************************************

#ifdef _FAST_CLOCK_
    byte clockBitMask;
    volatile byte *clockPort;
    byte dataBitMask;
    volatile byte *dataPort;
#ifdef _DATA_BIDIRECTIONAL_
    volatile byte *dataDDPort;
    volatile byte *dataInPort;
#endif
#ifdef _USE_LOAD_
    byte loadBitMask;
    volatile byte *loadPort;
#endif
    void inline clockHigh() {
      (*clockPort) |= clockBitMask;
    }

    void inline clockLow() {
      (*clockPort) &= ~clockBitMask;
    }

    void inline dataHigh() {
      (*dataPort) |= dataBitMask;
    }

    void inline dataLow() {
      (*dataPort) &= ~dataBitMask;
    }
#ifdef _DATA_BIDIRECTIONAL_
    byte inline dataRead() {
      return ((*dataInPort) & dataBitMask) ? HIGH : LOW;
    }

    void inline dataInput() {
      (*dataDDPort) &= ~dataBitMask;
    }
    void inline dataOutput() {
      (*dataDDPort) |= dataBitMask;
    }

#endif
#ifdef _USE_LOAD_
    void inline loadHigh() {
      (*loadPort) |= loadBitMask;
    }

    void inline loadLow() {
      (*loadPort) &= ~loadBitMask;
    }
#endif

    //*****************************************************************************************************
#else // Standard Arduino version

    byte dataPin;
    byte clockPin;
#ifdef _USE_LOAD_
    byte loadPin;
#endif

    void inline clockHigh() {
      digitalWrite(clockPin, HIGH);
    }

    void inline clockLow() {
      digitalWrite(clockPin, LOW);
    }

    void inline dataHigh() {
      digitalWrite(dataPin, HIGH);
    }

    void inline dataLow() {
      digitalWrite(dataPin, LOW);
    }
#ifdef _DATA_BIDIRECTIONAL_
    byte inline dataRead() {
      return digitalRead(dataPin);
    }

    void inline dataInput() {
      pinMode(dataPin, INPUT);
    }

    void inline dataOutput() {
      pinMode(dataPin, OUTPUT);
    }
#endif
#ifdef _USE_LOAD_
    void inline loadLow() {
      digitalWrite(loadPin, LOW);
    }

    void inline loadHigh() {
      digitalWrite(loadPin, HIGH);
    }
#endif
#endif



    //*****************************************************************************************************
    //*****************************************************************************************************
    // Internal variables and support routines
    //*****************************************************************************************************
    //*****************************************************************************************************

#ifdef _EXTRA_PINS_
    byte *extraPins;
#endif

#ifdef _USE_TIMER_
    byte currentDigit = 0; // Used only by the interrupt routine to update the display
#endif
    void initInterrupt();

    byte digits;      // Digits on display, usual from 2 to 8
    DIGIT_TYPE data[MAX_DIGITS];     // Data for the digits, this is always allocated for the maximum number of digits
    byte brightness; // Brightness level, 0-7 or 0-15
    byte dpDigit;   // Digit with a point or colon, any value higher than number of digits will turn it off
    boolean autoUpdate: 1; // Call update after each show, clear, brightness and setDp call
    boolean ack: 1; // Display is present, this is updated when update is called
    boolean on: 1; // Display is on or off

#ifdef _USE_INDICATOR_
    INDICATOR_TYPE indicator = 0;
#endif

#ifdef _KEYS_
    byte lastKeyIndex = 0;
#endif

#ifdef _INCLUDE_SCROLLING_TEXT_
    unsigned int scrollDelay = 300;
#endif

    void inline clockDelay() {
      if (DELAY_TIME > 0) delayMicroseconds(DELAY_TIME);
    }

    void inline shiftOutMsbFirst(byte val) {
      for (byte i = 0; i < 8; i++)  {
        if (val & 0x80) dataHigh();
        else dataLow();
        clockHigh();
        val <<= 1;
        clockDelay();
        clockLow();
      }
    }

    void inline shiftOutLsbFirst(byte val) {
      for (byte i = 0; i < 8; i++)  {
        if (val & 0x01) dataHigh();
        else dataLow();
        clockHigh();
        val >>= 1;
        clockDelay();
        clockLow();
      }
    }

    //*****************************************************************************************************
    //*****************************************************************************************************
    // Routines for each display
    //*****************************************************************************************************
    //*****************************************************************************************************
    void init();
    void updateSetup();   // Update brightness and on/off

    // Any support routines used when writing to the display must be declared here.
    // Actual writing is done from update and update(byte digit) that is declared below and must be implemented in the cpp file

#ifdef _TM1637_
    // Support routines to communicate with TM1637 display
    void writeByte(byte b);
    void stop();
    void start();
#endif
#ifdef _TM1637_6DX_
    // Support routines to communicate with TM1637 display
    void writeByte(byte b);
    void stop();
    void start();
#endif
#ifdef _TM1637_COLON_
    // Support routines to communicate with TM1637 display with colon
    void writeByte(byte b);
    void stop();
    void start();
#endif
#ifdef _TM1638_
    void writeByte(byte b);
    byte readByte();
#endif
#ifdef _TM1638_QYF_
    void writeByte(byte b);
    byte readByte();
#endif
#ifdef _TM1638_HWA11_
    void writeByte(byte b);
    byte readByte();
#endif
#ifdef _MAX7219_
    // Support routines to communicate with _MAX7219_ display
    void write(byte commandAndAddress, byte data);
#endif

#ifdef _HT1621_6D_
    void write(unsigned int data, byte bits);
    void writeCmd(byte cmd);
    byte getDigitData(byte digit);
#endif

#ifdef _HT1621_12D_
    void write(unsigned int data, byte bits);
    void writeCmd(byte cmd);
#endif
#ifdef _HT16K33_8D_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_4D_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_4DL_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_HKJ_4D_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_HKJ_6D_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_14SEG_8D_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_14SEG_4D_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_14SEG_4DS_
    void writeByte(byte b);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_14SEG_HKJ_6D_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_14SEG_HKJ_6D8_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_16SEG_HKJ_4D_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif
#ifdef _HT16K33_16SEG_HKJ_4DW_
    void writeByte(byte b);
    byte readByte(boolean last);
    void writeCmd(byte addr, byte cmd);
#endif

  // Rounding value used
    inline double roundValue(double v) {
      if (v<0) return -0.5;
      return 0.5;
    }
    
    void showCharInternal(byte digit, byte c); // Do not check auto update

  public:

    //*****************************************************************************************************
    //*****************************************************************************************************
    // Public part, many routines are shorthand routines and implemented inline
    //*****************************************************************************************************
    //*****************************************************************************************************

#ifndef _USE_LOAD_
    // Only want the 2 pin versions available for displays that support 2 pin mode.
    // A 2 pin display can be initialized with a 3 pin constructor, the load parameter will be ignored.
    LEDDisplayDriver(byte data, byte clock, boolean autoUpdate, byte digits): LEDDisplayDriver(data, clock, 255, autoUpdate, digits) {};
    LEDDisplayDriver(byte data, byte clock): LEDDisplayDriver(data, clock, 255, true, 4) {};
#endif
    LEDDisplayDriver(byte data, byte clock, byte load): LEDDisplayDriver(data, clock, load, true, 4) {};

#ifdef _EXTRA_PINS_
    LEDDisplayDriver(byte data, byte clock, byte load, byte *extraPins, boolean autoUpdate, byte digits);
    LEDDisplayDriver(byte data, byte clock, byte load, boolean autoUpdate, byte digits):
      LEDDisplayDriver(data, clock, load, NULL, autoUpdate, digits) {};
#else
    LEDDisplayDriver(byte data, byte clock, byte load, boolean autoUpdate, byte digits);
#endif

    inline void begin() {
      init();
    }

    byte inline numberOfDigits() {
      return digits;
    }

    void setBrightness(byte brightness) {
      this->brightness = brightness > MAX_BRIGHTNESS ? MAX_BRIGHTNESS : brightness;
      updateSetup();
    }

    inline byte getBrightess() {
      return this->brightness;
    }

    void setOn(boolean on)  {
      this->on = on;
      updateSetup();
    }

    void setAutoUpdate(boolean autoUpdate) {
      this->autoUpdate = autoUpdate;
    }

    // Show decimal point or colon on this digit, this segment is added during update and will not affect data
    // This function is not supported on all displays
    // Any point or colon enabled with this function must be removed with removeDp()
    void setDp(byte digit) {
      dpDigit = digit;
      if (autoUpdate) update();
    }

    inline void removeDp() {
      setDp(0xff);
    }

    // Show 7segment data directly
    void showDigits(const DIGIT_TYPE  data[], byte first, byte count);

    void showDigits(const DIGIT_TYPE data, byte first, byte count);

    // Set data for a single digit
    void showDigit(const DIGIT_TYPE data, byte digit) {
      this->data[digit] = data;
      if (autoUpdate) update(digit);
    }

    // Clear all digits and indicators on the display
    void clear() {
      showDigits(digitEmpty, 0, digits);
#ifdef _USE_INDICATOR_
      showIndicators(0);
#endif
    }

    inline void clear(byte digit) {
      showDigit(digitEmpty, digit);
    }

    void showMinus(byte first, byte count) {
      showDigits(digitMinus, first, count);
    }

    void showTest() {
      showDigits((DIGIT_TYPE)0xffffffffUL, 0, digits);
#ifdef _USE_INDICATOR_
      showIndicators((INDICATOR_TYPE)0xffffffffUL);
#endif
    }

    // Shortcut names for updating two digits at a time
    inline void showNum2Left(byte num) {
      showNum(num, 0, 2);
    }
    inline void showNum2Center(byte num) {
      showNum(num, digits / 2 - 1, 2);
    }
    inline void showNum2Right(byte num) {
      showNum(num, digits - 2, 2);
    }

    // Shortcut names for updating two digits at a time
    inline void showNum2LeftLZ(byte num) {
      showNum(num, 0, 2, true);
    }
    inline void showNum2CenterLZ(byte num) {
      showNum(num, digits / 2 - 1, 2, true);
    }
    inline void showNum2RightLZ(byte num) {
      showNum(num, digits - 2, 2, true);
    }

    inline void showHex2Left(byte num) {
      showHex(num, 0, 2);
    }
    inline void showHex2Center(byte num) {
      showHex(num, digits / 2 - 1, 2);
    }
    inline void showHex2Right(byte num) {
      showHex(num, digits - 2, 2);
    }

    // Shortcut names for updating four digits at a time
    inline void showNum4Left(int num) {
      showNum(num, 0, 4);
    }

    inline void showNum4Right(int num) {
      showNum(num, digits - 4, 4);
    }

    inline void showNum4LeftLZ(int num) {
      showNum(num, 0, 4, true);
    }

    inline void showNum4RightLZ(int num) {
      showNum(num, digits - 4, 4, true);
    }


    // Shortcut names for updating four digits at a time
    inline void showHex4Left(int num) {
      showHex(num, 0, 4);
    }

    inline void showHex4Right(int num) {
      showHex(num, digits - 4, 4);
    }

    void showBin(int num, byte format, byte first, byte count);

    inline void showBin(int num, byte format) {
      showBin(num, format, 0, digits);
    }

    inline void showBin(int num) {
      showBin(num, 0, 0, digits);
    }


    void showChar(byte digit, byte c);

    void showText(const String msg, byte first, byte count);

    void showText(const String msg) {
      showText(msg, 0, digits);
    }

    void showText(const char *msg, byte first, byte count);

    void showText(const char *msg) {
      showText(msg, 0, digits);
    }

#ifdef _INCLUDE_SCROLLING_TEXT_
    void showTextScroll(const String msg, byte first, byte count);

    void showTextScroll(const String msg) {
      showTextScroll(msg, 0, digits);
    }

    void showTextScroll(const char *msg, byte first, byte count);

    void showTextScroll(const char *msg) {
      showTextScroll(msg, 0, digits);
    }

    void setScrollSpeed(unsigned int delay) {
      scrollDelay = delay;
    }

#endif

    inline void showNum1decimal(double num) {
      showNumWithPoint((long) (num * 10 + roundValue(num)), 1);
    }
    inline void showNum2decimals(double num) {
      showNumWithPoint((long) (num * 100 +roundValue(num)), 2);
    }
    inline void showNum3decimals(double num) {
      showNumWithPoint((long) (num * 1000 + roundValue(num)), 3);
    }

    inline void showNum1decimal(double num, byte first, byte count) {
      showNumWithPoint((long) (num * 10 + roundValue(num)), 1, first, count);
    }
    inline void showNum2decimals(double num, byte first, byte count) {
      showNumWithPoint((long) (num * 100 + roundValue(num)), 2, first, count);
    }
    inline void showNum3decimals(double num, byte first, byte count) {
      showNumWithPoint((long) (num * 1000 + roundValue(num)), 3, first, count);
    }

    // Show a number with or without leading zero, can also handle negative numbers
    void showNum(int num, boolean showLeadingZeros = false) {
      showNum(num, 0, digits, showLeadingZeros);
    }

    // Same as above, but location on display can be specified
    void showNum(int num, byte first, byte count, boolean showLeadingZeros = false);

#ifdef _INCLUDE_BYTE_
    // Show a number with or without leading zero
    void showNum(byte num, boolean showLeadingZeros = false) {
      showNum(num, 0, digits, showLeadingZeros);
    }

    // Same as above, but location on display can be specified
    void showNum(byte num, byte first, byte count, boolean showLeadingZeros = false);

    // Show a 2 digit hex number
    void showHex(byte num) {
      showHex(num, 0, digits);
    }

    // Same as above, but location and length can be specified
    void showHex(byte num, byte first, byte count);
#endif


    // This includes long support and maps unsigned int to long
#ifdef _ENABLE_6_DIGITS_
    // For unsigned numbers, it is only need for the 6 digit version of the display
    void showNum(unsigned int num, boolean showLeadingZeros = false) {
      showNum((long) num, 0, digits, showLeadingZeros);
    }

    void showNum(unsigned int num, byte first, byte count, boolean showLeadingZeros = false) {
      showNum((long) num, first, count, showLeadingZeros);
    }

    // For long numbers, it is only need for the 6 digit version of the display
    void showNum(long num, boolean showLeadingZeros = false) {
      showNum(num, 0, digits, showLeadingZeros);
    }

    // Same as above, but location on display can be specified
    void showNum(long num, byte first, byte count, boolean showLeadingZeros = false);

    // Show a number with decimal point, zeros will be added as needed.
    // The dpPos is relative to the number: 0=3333. 1=333.3 2=33.33 3=3.333
    void showNumWithPoint(long num, int8_t dpPos) {
      showNumWithPoint(num, dpPos, 0, digits);
    }

    // Same as above, but location on display can be specified
    void showNumWithPoint(long num, int8_t dpPos, byte first, byte count);

    void showNumWithPrefix(DIGIT_TYPE prefix, unsigned int num, int8_t dpPos) {
      showNumWithPrefix(prefix, (long) num, dpPos, 0, digits);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, unsigned int num, int8_t dpPos, byte first, byte count) {
      showNumWithPrefix(prefix, (long) num, dpPos, first, count);
    }

    void showNumWithPrefix(DIGIT_TYPE prefix, long num, int8_t dpPos) {
      showNumWithPrefix(prefix, num, dpPos, 0, digits);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, long num, int8_t dpPos, byte first, byte count);

    void showHex(long num) {
      showHex(num, 0, digits);
    }

    // Same as above, but location and length can be specified
    void showHex(long num, byte first, byte count);

#else
    // For unsigned numbers use same handling as signed numbers
    void showNum(unsigned int num, boolean showLeadingZeros = false) {
      showNum((int) num, 0, digits, showLeadingZeros);
    }

    void showNum(unsigned int num, byte first, byte count, boolean showLeadingZeros = false) {
      showNum((int) num, first, count, showLeadingZeros);
    }

    // For long numbers use same handling as signed numbers
    void showNum(long num, boolean showLeadingZeros = false) {
      showNum((int) num, 0, digits, showLeadingZeros);
    }

    // Same as above, but location on display can be specified
    void showNum(long num, byte first, byte count, boolean showLeadingZeros = false) {
      showNum((int) num, first, count, showLeadingZeros);
    }

    void showNumWithPrefix(DIGIT_TYPE prefix, unsigned int num, int8_t dpPos) {
      showNumWithPrefix(prefix, (int) num, dpPos, 0, digits);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, unsigned int num, int8_t dpPos, byte first, byte count) {
      showNumWithPrefix(prefix, (int) num, dpPos, first, count);
    }

    void showNumWithPrefix(DIGIT_TYPE prefix, long num, int8_t dpPos) {
      showNumWithPrefix(prefix, (int) num, dpPos, 0, digits);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, long num, int8_t dpPos, byte first, byte count) {
      showNumWithPrefix(prefix, (int) num, dpPos, first, count);
    }

    void showHex(long num) {
      showHex(num, 0, digits);
    }

    // Same as above, but location and length can be specified
    void showHex(long num, byte first, byte count) {
      showHex((int) num, first, count);
    }

#endif

    // For float, it will get the best possible resolution from the display
    // A negative value takes priority over digits, i.e. on 4 digit you can show -999 or 9999, anything outside this will show overflow ----
    void showNum(double num) {
      showNum(num, digits, 0, digits);
    }

    // With this version the number of decimals can be limited
    void showNum(double num, byte maxDecimals) {
      showNum(num, maxDecimals, 0, digits);
    }

    // Same as above, but location on display can be specified
    void showNum(double num, byte first, byte count) {
      showNum(num, count, first, count);
    }

    // Same as above, but location on display can be specified
    void showNum(double num, byte maxDecimals, byte first, byte count);


    // This function reserves the first digit for status and ./-
    // With 4 digits it is possible to diplay from 0.001 to 999 together with a status digit
    // With negative numbers the sign is added to the prefix, i.e. avoid SEG_G for status when negative numbers are used.
    // It is a good idea to avoid status patterns that may look like a valid digit
    void showNumWithPrefix(DIGIT_TYPE prefix, double num) {
      showNumWithPrefix(prefix, num, digits, 0, digits);
    }

    // Same as above, but location on display can be specified
    void showNumWithPrefix(DIGIT_TYPE prefix, double num, byte first, byte count) {
      showNumWithPrefix(prefix, num, count, first, count);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, double num, byte maxDecimals, byte first, byte count);

    // Show a number with decimal point, zeros will be added as needed.
    // The dpPos is relative to the number: 0=3333. 1=333.3 2=33.33 3=3.333
    void showNumWithPoint(int num, int8_t dpPos) {
      showNumWithPoint(num, dpPos, 0, digits);
    }

    // Same as above, but location on display can be specified
    void showNumWithPoint(int num, int8_t dpPos, byte first, byte count);

    void showNumWithPrefix(DIGIT_TYPE prefix, int num, int8_t dpPos) {
      showNumWithPrefix(prefix, num, dpPos, 0, digits);
    }
    void showNumWithPrefix(DIGIT_TYPE prefix, int num, int8_t dpPos, byte first, byte count);

    // Show a hex number
    void showHex(int num) {
      showHex(num, 0, digits);
    }

    // Same as above, but location and length can be specified
    void showHex(int num, byte first, byte count);


    DIGIT_TYPE *getDigits() {
      return data;
    }

    // Transfer data to the display, with autoUpdate=true this is automatic done after each call
    void update();
    void update(byte digit);  // It is faster to update a single digit, but not much

    // Check if display is present, this is updated each time the display is updated
    inline boolean  isPresent() {
      return ack;
    }

#ifdef _USE_INDICATOR_
    void showIndicators(INDICATOR_TYPE indicators) {
      indicator = indicators;
      if (autoUpdate) update();
    }

    void addIndicators(INDICATOR_TYPE indicators) {
      indicator |= indicators;
      if (autoUpdate) update();
    }

    void subIndicators(INDICATOR_TYPE indicators) {
      indicator &= ~indicators;
      if (autoUpdate) update();
    }


    INDICATOR_TYPE getIndicators() {
      return indicator;
    }
#else
    void showIndicators(__attribute__((unused))long indicators) {}
    void addIndicators(__attribute__((unused))long indicators) {}
    void subIndicators(__attribute__((unused))long indicators) {}
    inline byte getIndicators() {
      return 0;
    }

#endif

#ifdef _KEYS_
    KEY_TYPE readKeys();
    byte readKeyIndex() {
      byte n = 0;
      KEY_TYPE k = readKeys();
      while (k != 0) {
        n++;
        k >>= 1;
      }
      return n;
    }
    byte readKeyIndexOnce() {
      byte v = readKeyIndex();
      if (v == lastKeyIndex) return 0;
      lastKeyIndex = v;
      return v;
    }
#else
    byte readKeys() {
      return 0;
    }
    byte readKeyIndex() {
      return 0;
    }
    byte readKeyIndexOnce() {
      return 0;
    }
#endif


#ifdef _USE_TIMER_
    void updateIntr();
#endif

};


class LEDLCDAPIDriver: private LEDDisplayDriver, public Print  {
  private:
    byte col = 0;

  public:

#ifndef _USE_LOAD_
    // Only want the 2 pin versions available for displays that support 2 pin mode.
    // A 2 pin display can be initialized with a 3 pin constructor, the load parameter will be ignored.
    LEDLCDAPIDriver(byte data, byte clock, boolean autoUpdate, byte digits): LEDDisplayDriver(data, clock, 255, autoUpdate, digits) {};
    LEDLCDAPIDriver(byte data, byte clock): LEDDisplayDriver(data, clock, 255, true, 4) {};
#endif
    LEDLCDAPIDriver(byte data, byte clock, byte load): LEDDisplayDriver(data, clock, load, true, 4) {};
    LEDLCDAPIDriver(byte data, byte clock, byte load, boolean autoUpdate, byte digits) : LEDDisplayDriver(data, clock, load, autoUpdate, digits) {};

    virtual size_t write(uint8_t c);
    inline void setCursor(__attribute__((unused))byte Row, byte Col) {
      this->col = Col;
    }
    inline void home() {
      col = 0;
    }
    void clear();
    inline void init() {
      begin();
    }
    inline void setBacklight(byte val) {
      setBrightness(val);
    }
    inline void on() {
      setOn(true);
    }
    inline void off() {
      setOn(false);
    }
    inline void update() {
      LEDDisplayDriver::update();
    }
    inline int keypad() {
      return readKeyIndex();
    }
#ifdef _INCLUDE_SCROLLING_TEXT_
    void setScrollSpeed(unsigned int delay) {
      scrollDelay = delay;
    }
#endif

#ifdef _USE_TIMER_
    inline void updateIntr() {
      LEDDisplayDriver::updateIntr();
    }
#endif
};



//******************************************************************************************************
//******************************************************************************************************
// Categorize the different Atmel processors, this is used for timer interrupt handling
//******************************************************************************************************
//******************************************************************************************************

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#define _uno_
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define _mega_
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
#define _U4_
#elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
#define _mega0_
#endif


//******************************************************************************************************
//******************************************************************************************************
// Define interrupt vector, this macro is empty when no interrupt is defined
//******************************************************************************************************
//******************************************************************************************************

#if defined(_USE_TIMER0_)
#if defined(_mega0_)
#define DISPLAY_INTR(display) ISR(TCB0_INT_vect) { TCB0.INTFLAGS=1;display.updateIntr();}
#endif
#endif

#if defined(_USE_TIMER1_)
#if defined(_mega0_)
#define DISPLAY_INTR(display) ISR(TCB1_INT_vect) { TCB1.INTFLAGS=1;display.updateIntr();}
#else
#define DISPLAY_INTR(display) ISR(TIMER1_COMPA_vect) { display.updateIntr();}
#endif
#endif

#if defined(_USE_TIMER2_)
#if defined(_mega0_)
#define DISPLAY_INTR(display) ISR(TCB2_INT_vect) { TCB2.INTFLAGS=1;display.updateIntr();}
#else
#define DISPLAY_INTR(display) ISR(TIMER2_COMPA_vect) { display.updateIntr();}
#endif
#endif

#if defined(_USE_TIMER3_)
#if defined(_mega0_)
#define DISPLAY_INTR(display) ISR(TCB3_INT_vect) { TCB3.INTFLAGS=1;display.updateIntr();}
#else
#define DISPLAY_INTR(display) ISR(TIMER3_COMPA_vect) { display.updateIntr();}
#endif
#endif

#if defined(_USE_TIMER4_)
#if defined(_U4_)
#define DISPLAY_INTR(display) ISR(TIMER4_OVF_vect) { display.updateIntr();}
#elif defined(_mega_)
#define DISPLAY_INTR(display) ISR(TIMER4_COMPA_vect) { display.updateIntr();}
#endif
#endif

#if defined(_USE_TIMER5_)
#if defined(_mega0_)
#define DISPLAY_INTR(display) ISR(TCA0_OVF_vect) { TCA0.SINGLE.INTFLAGS=1;display.updateIntr();}
#elif defined(_mega_)
#define DISPLAY_INTR(display) ISR(TIMER5_COMPA_vect) { display.updateIntr();}
#endif
#endif

#ifndef DISPLAY_INTR
#define DISPLAY_INTR(display)
#endif

#endif
