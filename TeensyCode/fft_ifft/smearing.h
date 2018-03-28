/* 
 *  Functions for frequency smearing
 */

 
#ifndef smearing_
#define smearing_

#include "arm_math.h"
#include "Arduino.h"

// Process frequency smearing on frame
void smearing(float32_t* frame, uint8_t b, int N, float32_t fs);

// reconstruct negative frequencies with complex conjugate [0Hz ... fmax, -fmax ... -delta_f]
void neg_freq(float32_t* frame, int N);

void test(void);

 #endif

