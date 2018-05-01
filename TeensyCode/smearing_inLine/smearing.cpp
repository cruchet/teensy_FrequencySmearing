/*
 * smearing.cpp
 * Contains all the smearing functions.
 * Only necessary parts are compiled, acording to the parameters set in global.h.  
 */

#include <math.h>
#include "smearing.h"
#include "smear_mat.h"

#ifndef COMPRESSED
void smearing_uncomp(float32_t* frame, uint8_t b, int N, float32_t fs) {
  float32_t spec_power[N/2];
  float32_t spec_phase[N/2];
  float32_t* row;
  float32_t tmp = 0;
  int i=0;
  
  // extract magnitude and angle (only positive frequencies)
  arm_cmplx_mag_f32(frame, spec_power, N/2);
  for(i=0; i<N/2; i++) {
    spec_phase[i] = atan2(frame[2*i+1], frame[2*i]);
  }
  
  // process matrix multiplication
#if B == 3
      //Serial.print("smear 3 ...");
      for(i=0; i<N/2; i++) {
        row = smear_mat_b3[i];
        arm_dot_prod_f32 (row, spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      //Serial.println("done");
#elif B == 6
      //Serial.print("smear 6 ...");
      for(i=0; i<N/2; i++) {
        row = smear_mat_b6[i];
        arm_dot_prod_f32 (row, spec_power, N/2, &tmp);
        frame[2*i]    = tmp * cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = tmp * sin(spec_phase[i]);          // imaginary part
      }
      //Serial.println("done");
#else
      Serial.println(F("ERROR: non-valid parameter for smearing()"));
#endif // B
  neg_freq(frame, N);
}

#else // COMPRESSED defined
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
#if B == 1
      sprsax(sa_b1, ija_b1, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
#elif B == 3
      sprsax(sa_b3, ija_b3, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
#elif B == 6
      sprsax(sa_b6, ija_b6, spec_power, spec_power_smeared, N/2);
      for(i=0; i<N/2; i++) {
        frame[2*i]    = spec_power_smeared[i]*cos(spec_phase[i]);          // real part 
        frame[2*i+1]  = spec_power_smeared[i]*sin(spec_phase[i]);          // imaginary part
      }
#else
      Serial.println(F("ERROR: non-valid parameter for smearing()"));
#endif // B
#endif // COMPRESSED
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
