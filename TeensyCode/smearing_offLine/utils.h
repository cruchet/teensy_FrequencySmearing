#ifndef utils_h_
#define utils_h_

#include "arm_math.h"

// creates a normalized hann window
void create_hann_window(float win[], int win_l);

// creates a normalized sqrt(hann) window
void create_sqrthann_window(float win[], int win_l);

// initialize fft and test for errors
void fft_init(arm_cfft_radix4_instance_f32* fftInst, uint16_t fftLen, uint8_t fftFlag, uint8_t bitReverseFlag);
#endif
