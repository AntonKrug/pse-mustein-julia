/*
 *		Date: 2018/12/01
 *      Author: akrug
 */

#ifndef SRC_COMMON_MACROS_H_
#define SRC_COMMON_MACROS_H_

#include <stdlib.h>

// https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))
#define FORCE_INLINE __attribute__((always_inline)) inline
#define FORCE_DEBUG __attribute__((optimize("O0")))
#define FORCE_O3 __attribute__((optimize("O3")))

#endif /* SRC_COMMON_MACROS_H_ */
