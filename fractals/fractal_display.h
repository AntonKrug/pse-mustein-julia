/*
 *		Date: 2018/12/01
 *      Author: akrug
 */

#ifndef SRC_FRACTAL_DISPLAY_H_
#define SRC_FRACTAL_DISPLAY_H_

#include <stdbool.h>
#include <stdint.h>

void fractalLoop(uint64_t base, uint32_t *buffer);

void juliaMain(uint64_t base);

#endif /* SRC_FRACTAL_DISPLAY_H_ */
