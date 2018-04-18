#include <math.h>
#include "smearing.h"
#include "smear_mat.h"

/* void smearing_uncomp(float32_t* frame, uint8_t b, int N, float32_t fs) {
  float32_t spec_power[N/2];
  float32_t spec_phase[N/2];
  float32_t* row;
  float32_t tmp = 0;
  int i=0;
  if(N!=256 || fs!=16000) {
    Serial.println(F("ERROR: fs or N doesn't correspond to any registered smeared matrices"));
    return;
  }
  
  // extract magnitude and angle (only positive frequencies)
  arm_cmplx_mag_f32(frame, spec_power, N/2);
  for(i=0; i<N/2; i++) {
    spec_phase[i] = atan2(frame[2*i+1], frame[2*i]);
  }
  
  // process matrix multiplication
  switch(b) {
    case 1 :
      for(i=0; i<N/2; i++) {
        arm_dot_prod_f32 ((float32_t*)pgm_read_ptr_near(smear_mat_b1_N256_fs16k+i), spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
     break;
    case 3 :
      Serial.print("smear 3 ...");
      for(i=0; i<N/2; i++) {
        row = smear_mat_b3_N256_fs16k[i];
        arm_dot_prod_f32 (row, spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      Serial.println("done");
      break;
    case 6 :
      Serial.print("smear 6 ...");
      for(i=0; i<N/2; i++) {
        row = smear_mat_b6_N256_fs16k[i];
        arm_dot_prod_f32 (row, spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      Serial.println("done");
      break;
    default :
      Serial.println(F("ERROR: non-valid parameter for smearing()"));
  }
  neg_freq(frame, N);
}*/


void smearing_comp(float32_t* frame, uint8_t b, int N, float32_t fs) {
  float32_t spec_power[N/2];
  float32_t spec_power_smeared[N/2];
  float32_t spec_phase[N/2];
  int i=0;

    // extract magnitude and angle (only positive frequencies)
  arm_cmplx_mag_f32(frame, spec_power, N/2);
  for(i=0; i<N/2; i++) {
    spec_phase[i] = atan2(frame[2*i+1], frame[2*i]);
  }
  
  // process matrix multiplication
  switch(b) {
    /*case 1 :
      sprsax(sa_b1_fs16k_L256, ija_b1_fs16k_L256, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
      break;
    case 3 :
      Serial.print("smear 3 ...");
      sprsax(sa_b3_fs16k_L256, ija_b3_fs16k_L256, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
      Serial.println("done");
      break;*/
    case 6 :
      Serial.print("smear 6 ..., N="); Serial.print(N);
      sprsax(sa_b6_fs16k_L256, ija_b6_fs16k_L256, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
      Serial.println("done");
      break;
    default :
      Serial.println(F("ERROR: non-valid parameter for smearing()"));
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


// Multiply a matrix in row-index sparse storage arrays sa and ija by a vector x[1..n], giving
// a vector b[1..n].
void sprsax(float sa[], unsigned int ija[], float x[], float b[],unsigned int n) {
  unsigned int i,k;
  
  if (ija[0] != n+1) {
    Serial.println(F("sprsax: mismatched vector and matrix"));
    return;
  }
  for (i=0;i<n;i++) {
    b[i]=sa[i]*x[i];                      // Start with diagonal term.
    for (k=ija[i];k<ija[i+1]-1;k++) {     // Loop over off-diagonal terms.
      b[i] += sa[k]*x[ija[k]];
    }
  }
}
