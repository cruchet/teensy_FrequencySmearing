#ifndef utils_h_
#define utils_h_

#include "arm_math.h"

// creates a normalized hann window
void create_hann_window(float win[], int win_l);

// creates a normalized sqrt(hann) window 
// /!\ the audio library has defined windows only for 256 samples!
void create_sqrthann_window(float win[], int win_l);

// initialize fft and test for errors
void fft_init(arm_cfft_radix2_instance_f32* fftInst, uint16_t fftLen, uint8_t fftFlag, uint8_t bitReverseFlag);

// Allows to change the audio shield's I2S sampling frequency. Will not work with audio from the SD card.
// See discussion on: https://forum.pjrc.com/threads/38753-Discussion-about-a-simple-way-to-change-the-sample-rate
void setI2SFreq(int freq);
#endif
