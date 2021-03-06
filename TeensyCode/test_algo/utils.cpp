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

  if(win_l == 256) {
    arm_max_q15((q15_t*)AudioWindowHanning256, win_l, &max, &max_idx);
    for(i=0; i<win_l; i++) {
      win[i] = sqrt((float)AudioWindowHanning256[i] / max);
    }
  }
  else {
    Serial.println(F("ERROR in create_sqrthann_window: unvalid window length"));
  }
}

// initialize fft and test for errors
void fft_init(arm_cfft_radix2_instance_f32* fftInst, uint16_t fftLen, uint8_t fftFlag, uint8_t bitReverseFlag) {
  if(fftFlag == 0) {
    Serial.print(F("initialize FFT...  "));
  }
  else if(fftFlag == 1){
    Serial.print(F("initialize IFFT...  "));
  }
  arm_status fft_status = ARM_MATH_TEST_FAILURE;
  
  fft_status   = arm_cfft_radix2_init_f32(fftInst, fftLen, fftFlag, bitReverseFlag);
  if (fft_status != ARM_MATH_SUCCESS) {
    Serial.print(F("error in initializing FFT"));
    if(fft_status == ARM_MATH_ARGUMENT_ERROR) {
      Serial.println(F("ARM_MATH_ARGUMENT_ERROR"));
    }
    else {
      Serial.println();
    }
  }
  Serial.println(F("done\t"));
}

// Allows to change the audio shield's I2S sampling frequency. Will not work with audio from the SD card.
// See discussion on: https://forum.pjrc.com/threads/38753-Discussion-about-a-simple-way-to-change-the-sample-rate
void setI2SFreq(int freq) {
  typedef struct {
    uint8_t mult;
    uint16_t div;
  } tmclk;

  const int numfreqs = 14;
  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, 44117 , 48000, 88200, 44117 * 2, 96000, 176400, 44117 * 4, 192000};

#if (F_PLL==16000000)
  const tmclk clkArr[numfreqs] = {{16, 125}, {148, 839}, {32, 125}, {145, 411}, {64, 125}, {151, 214}, {12, 17}, {96, 125}, {151, 107}, {24, 17}, {192, 125}, {127, 45}, {48, 17}, {255, 83} };
#elif (F_PLL==72000000)
  const tmclk clkArr[numfreqs] = {{32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {128, 1125}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375}, {249, 397}, {32, 51}, {185, 271} };
#elif (F_PLL==96000000)
  const tmclk clkArr[numfreqs] = {{8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {32, 375}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125}, {151, 321}, {8, 17}, {64, 125} };
#elif (F_PLL==120000000)
  const tmclk clkArr[numfreqs] = {{32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {128, 1875}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625}, {178, 473}, {32, 85}, {145, 354} };
#elif (F_PLL==144000000)
  const tmclk clkArr[numfreqs] = {{16, 1125}, {49, 2500}, {32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {4, 51}, {32, 375}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375} };
#elif (F_PLL==180000000)
  const tmclk clkArr[numfreqs] = {{46, 4043}, {49, 3125}, {73, 3208}, {98, 3125}, {183, 4021}, {196, 3125}, {16, 255}, {128, 1875}, {107, 853}, {32, 255}, {219, 1604}, {214, 853}, {64, 255}, {219, 802} };
#elif (F_PLL==192000000)
  const tmclk clkArr[numfreqs] = {{4, 375}, {37, 2517}, {8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {1, 17}, {8, 125}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125} };
#elif (F_PLL==216000000)
  const tmclk clkArr[numfreqs] = {{32, 3375}, {49, 3750}, {64, 3375}, {49, 1875}, {128, 3375}, {98, 1875}, {8, 153}, {64, 1125}, {196, 1875}, {16, 153}, {128, 1125}, {226, 1081}, {32, 153}, {147, 646} };
#elif (F_PLL==240000000)
  const tmclk clkArr[numfreqs] = {{16, 1875}, {29, 2466}, {32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {4, 85}, {32, 625}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625} };
#endif

  for (int f = 0; f < numfreqs; f++) {
    if ( freq == samplefreqs[f] ) {
      while (I2S0_MCR & I2S_MCR_DUF) ;
      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
      return;
    }
  }
  Serial.println(F("ERROR: unvalid sampling rate for setI2SFreq()"));
}

