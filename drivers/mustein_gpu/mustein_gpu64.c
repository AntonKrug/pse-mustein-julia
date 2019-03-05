/*
 * mustein_gpu64.c
 *
 *  Created on: 13 Nov 2018
 *      Author: akrug
 */

#include "mustein_gpu64.h"


void video_write_pixel(uint64_t base, uint64_t offset, uint32_t value) {
	*((uint64_t*)base + offset)  = value << 8;
}


void video_write_pixel_rgb(uint64_t base, uint64_t offset, uint8_t red, uint8_t green, uint8_t blue) {
	*((uint64_t*)base + offset)  = blue << 8 | green<<16 | red<<24;
}


void MUSTEIN_INLINE video_write_pixel_raw(uint64_t base, uint64_t offset, uint32_t bytecode) {
	*((uint64_t*)base + offset)  = bytecode;
}

void MUSTEIN_OPTIMISE video_write_pixel_buffer_fully_packed(uint64_t base, uint64_t *buffer, uint64_t count) {
  uint64_t* pointer = (uint64_t*)base;

  for (uint64_t index = 0; index < count; ++index) {
    *(pointer) = buffer[index];
    pointer++;
  }
}

void MUSTEIN_OPTIMISE video_write_pixel_buffer(uint64_t base, uint32_t *buffer, uint64_t count) {
  uint64_t* pointer = (uint64_t*)base;

  for (uint64_t index = 0; index < count; ++index) {
    *(pointer) = buffer[index];
    pointer++;
  }
}

void video_setup(uint64_t base, uint8_t controlBit, uint32_t width, uint32_t height, Colors colors, PixelPacking packing) {
	videoController *controller = (videoController*) (base | (1 << controlBit));
	controller->width  = width;
	controller->height = height;
	controller->parameters = colors | packing << 4;
}

