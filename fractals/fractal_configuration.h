/*
 *		Date: 2018/12/01
 *      Author: akrug
 */

#ifndef SRC_FRACTAL_CONFIGURATION_H_
#define SRC_FRACTAL_CONFIGURATION_H_

// When you increase the resolution, double check if the platform is allocating
// enough memory for it:
// machine LoadPlatformDescriptionFromString "mustein: Video.MusteinGenericGPU @ sysbus 0x10100000 { frameBufferSize: 0x100000; registers64bitAligned: true }"
// Increasing the frameBufferSize might be necessary. When it's not specified it
// will use its default value which should be enough for 1024x1024 resolution
#define WIDTH_BITS  4  // WIDTH  = 1 << WIDTH_BITS
#define HEIGHT_BITS 4  // HEIGHT = 1 << HEIGHT_BITS

// 10 bits means 1024 pixels
// 9  bits means 512  pixels
// 8  bits means 256  pixels
// 7  bits means 128  pixels
// 6  bits means 64   pixels
// 5  bits means 32   pixels
// 4  bits means 16   pixels
// 3  bits means 8    pixels


#ifndef ANIMATION_SPEED
#define ANIMATION_SPEED 0.0005f // How large steps are done between the frames
#endif


// Do not edit these lines below, to edit the resolution edit the defines above
#define WIDTH  ( 1 << (WIDTH_BITS))
#define HEIGHT ( 1 << (HEIGHT_BITS))

#endif /* SRC_FRACTAL_CONFIGURATION_H_ */
