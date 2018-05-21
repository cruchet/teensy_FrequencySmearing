/* 
 *  Functions for frequency smearing
 */

 
#ifndef smearing_
#define smearing_

#include "arm_math.h"
#include "Arduino.h"

#ifndef COMPRESSED
// Process frequency smearing on frame (using full matrix)
void smearing_uncomp(float32_t* frame, int N);
#else
// Process frquency smearing on frame (using row-index sparse storage)
void smearing_comp(float32_t* frame, int N);
#endif

// Multiply a matrix in row-index sparse storage arrays sa and ija by a vector x[1..n], giving
// a vector b[1..n].
void sprsax(float sa[], unsigned int ija[], float x[], float b[], unsigned int n);

// reconstruct negative frequencies with complex conjugate [0Hz ... fmax, -fmax ... -delta_f]
void neg_freq(float32_t* frame, int N);

#endif

