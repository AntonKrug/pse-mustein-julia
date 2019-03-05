/*
 * mustein_gpu64.h
 *
 *  Created on: 13 Nov 2018
 *      Author: akrug
 */


#ifndef SRC_MUSTEIN_GPU64_H_
#define SRC_MUSTEIN_GPU64_H_

#include <stdint.h>

// Comment the following line to debug the driver
#define OPTIMISE_DEBUG_CONFIGURATION

#ifdef OPTIMISE_DEBUG_CONFIGURATION
#define MUSTEIN_INLINE __attribute__((always_inline)) inline
#define MUSTEIN_OPTIMISE __attribute__((optimize("O3")))
#else
#define MUSTEIN_INLINE
#define MUSTEIN_OPTIMISE
#endif


typedef struct {
  uint64_t width;
  uint64_t height;
  uint64_t parameters;
} videoController;


typedef enum {
	COLOR_LOW  = 0,
	COLOR_HIGH = 1,
	COLOR_TRUE = 2
} Colors;

typedef enum {
	PACKING_SINGLE_PIXEL_PER_WRITE = 0,
	PACKING_FULLY_32bit            = 1,
	PACKING_FULLY_64bit            = 2
} PixelPacking;

#define VIDEO_COLOR_RGBAX       0x3

void video_setup(uint64_t base, uint8_t controlBit, uint32_t width, uint32_t height, Colors colors, PixelPacking packing);

void video_write_pixel(uint64_t base, uint64_t offset, uint32_t value);
void video_write_pixel_rgb(uint64_t base, uint64_t offset, uint8_t red, uint8_t green, uint8_t blue);
void video_write_pixel_raw(uint64_t base, uint64_t offset, uint32_t bytecode);
void video_write_pixel_buffer_fully_packed(uint64_t base, uint64_t *buffer, uint64_t count);
void video_write_pixel_buffer(uint64_t base, uint32_t *buffer, uint64_t count);

#endif /* SRC_MUSTEIN_GPU64_H_ */
