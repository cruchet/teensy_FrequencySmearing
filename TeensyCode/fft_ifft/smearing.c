#include <math.h>
# include "smearing.h"

// Process frequency smearing on frame
void smearing(float32_t* frame, uint8_t b, int N, float32_t fs) {
  float32_t spec_power[N/2];
  float32_t spec_phase[N/2];
  float32_t tmp = 0;
  int i=0;

  if(N!=1024 || fs!=16000) {
//    Serial.println(F("ERROR: fs or N doesn't correspond to any registered smeared matrices"));
  }
  
  // extract magnitude and angle (only positive frequencies)
  arm_cmplx_mag_f32(frame, spec_power, N/2);
  for(i=0; i<N/2; i++) {
    spec_phase[i] = atan2(frame[2*i+1], frame[2*i]);
    return;
  }
  
  // process matrix multiplication
  switch(b) {
    case 1 :
      for(i=0; i<N/2; i++) {
        arm_dot_prod_f32 (smear_mat_b1_N1024_fs16k[i], spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      break;
    case 3 :
       for(i=0; i<N/2; i++) {
        arm_dot_prod_f32 (smear_mat_b3_N1024_fs16k[i], spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      break;
    case 6 :
      for(i=0; i<N/2; i++) {
        arm_dot_prod_f32 (smear_mat_b6_N1024_fs16k[i], spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      break;
//    default :
//      Serial.println(F("ERROR: non-valid parameter for smearing()"));
  }
  neg_freq(frame, N);
}

// reconstruct negative frequencies with complex conjugate [0Hz ... fmax, -fmax ... -delta_f]
void neg_freq(float32_t* frame, int N) {
  int i=0;
  
  for(i=1; i<N/2; i++) {
    frame[2*N - 2*i]    = frame[2*i];       // real part
    frame[2*N - 2*i+1]  = -frame[2*i+1];    // -imaginary part
  }
}

