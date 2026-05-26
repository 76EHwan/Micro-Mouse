/*
 * fonts.h
 *
 *  Created on: Dec 20, 2025
 *      Author: kth59
 */

#ifndef BSP_ST7789_FONTS_H_
#define BSP_ST7789_FONTS_H_

#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
    const uint8_t width;
    const uint8_t height;
    const uint16_t *data;
} FontDef;

extern const unsigned char asc2_1608[95][16];


#endif

#endif /* BSP_ST7789_FONTS_H_ */
