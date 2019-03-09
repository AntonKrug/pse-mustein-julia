/*
 *		Date: 2018/12/01
 *      Author: akrug
 */

#ifndef SRC_FRACTAL_CONFIGURATION_H_
#define SRC_FRACTAL_CONFIGURATION_H_

// 10 bits => 1024
// 9  bits => 512
// 8  bits => 256
// 7  bits => 128
// 6  bits => 64
// 5  bits => 32
// 4  bits => 16
// 3  bits => 8

#define WIDTH_BITS 7   // WIDTH  = 1 << WIDTH_BITS
#define HEIGHT_BITS 7  // HEIGHT = 1 << HEIGHT_BITS

#ifndef ANIMATION_SPEED
#define ANIMATION_SPEED 0.02f // How large steps are done between the frames
#endif


#define WIDTH ( 1 << (WIDTH_BITS))
#define HEIGHT ( 1 << (HEIGHT_BITS))

#endif /* SRC_FRACTAL_CONFIGURATION_H_ */
