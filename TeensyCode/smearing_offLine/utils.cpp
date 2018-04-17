#include "utils.h"

#include <Audio.h>
#include <math.h>
#include "arm_math.h"
#include "global.h"


// creates a normalized hann window
void create_hann_window(float win[], int win_l){
  int i=0;
  q15_t     max     = 0;
  uint32_t  max_idx = 0;
  
  arm_max_q15((q15_t*)AudioWindowHanning256, win_l, &max, &max_idx);
  for(i=0; i<win_l; i++) {
    win[i] = (float)AudioWindowHanning256[i] / max;
  }
}

// creates a normalized sqrt(hann) window
void create_sqrthann_window(float win[], int win_l){
  int i=0;
  q15_t     max     = 0;
  uint32_t  max_idx = 0;
  
  arm_max_q15((q15_t*)AudioWindowHanning256, win_l, &max, &max_idx);
  for(i=0; i<win_l; i++) {
    win[i] = sqrt((float)AudioWindowHanning256[i] / max);
  }
}

// initialize fft and test for errors
void fft_init(arm_cfft_radix2_instance_f32* fftInst, uint16_t fftLen, uint8_t fftFlag, uint8_t bitReverseFlag) {
  Serial.print(F("initialize FFT...  "));
  arm_status fft_status = ARM_MATH_TEST_FAILURE;
  
  fft_status   = arm_cfft_radix2_init_f32(fftInst, fftLen, fftFlag, bitReverseFlag);
  if (fft_status != ARM_MATH_SUCCESS) {
    Serial.println(F("error in initializing FFT"));
    if(fft_status == ARM_MATH_ARGUMENT_ERROR) {
      Serial.println(F("ARM_MATH_ARGUMENT_ERROR"));
    }
    else {
      Serial.println();
    }
  }
  Serial.print(F("done\t")); Serial.println(fft_status);
}

