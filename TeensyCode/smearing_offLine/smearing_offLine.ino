/**
 * smearing_offLine.ino
 * Process frequency smearing on a .wav file stored on the SD card, 
 * using block processing and circular buffer to achieve overlap-and-add.
 * 
 * Several combination of parameters  as b (smearing coefficient), frame lenght and 
 * sampling frequency are already coded in smear_mat.h but if other parameters are wanted,
 * a new smearing matrix must be calculated with MatLab script "generate_smear_matrix".
 * Changing paramters always induce recompiling.
 * The choice of parameters is limited. See setI2SFreq() for availble fs. The fft functions 
 * work only for frame length of 16, 64, 256, 1024 samples.
 * 
 * Author:
 *      Vassili Cruchet
 *      vassili.cruchet@gmail.com
 * Date:
 *      Spring 2018
 *      
 * Technical University of Denmark (DTU)
 *      
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <CircularBuffer.h>
#include <math.h>

#include "Arduino.h"
#include "arm_math.h"
#include "RamMonitor.h"
#include "smearing.h"
#include "circ_buff_util.h"
#include "utils.h"
#include "global.h"

/***************** GUItool: begin automatically generated code *****************/
AudioPlayQueue           queueOut;         
AudioPlaySdWav           audio_SD;
AudioOutputI2S           audio_out;
AudioRecordQueue         queueIn;
AudioConnection          patchCord1(queueOut, 0, audio_out, 0);
AudioConnection          patchCord2(queueOut, 0, audio_out, 1);
AudioConnection          patchCord3(audio_SD, 0, queueIn, 0);
AudioControlSGTL5000     sgtl5000_1;
/***************** GUItool: end automatically generated code *****************/

/***************** Global variables *****************/
float           win[FFT_LEN];     // vector for window function
unsigned long   time = 0;
unsigned int    count =0;
int             freq = 500;       // frequency for the waveform
// FFT variables
uint16_t  fftLen          = FFT_LEN;
uint8_t   fftFlag         = 0;
uint8_t   ifftFlag        = 1;
uint8_t   bitReverseFlag  = 1;
arm_cfft_radix2_instance_f32 fftInst;
arm_cfft_radix2_instance_f32 ifftInst;
/***************** end of global variables *****************/

void setup() {
  set_periph();
  fft_init(&fftInst, fftLen, fftFlag, bitReverseFlag);
  fft_init(&ifftInst, fftLen, ifftFlag, bitReverseFlag);
  create_sqrthann_window(win, FFT_LEN);
  delay(1000);
  queueIn.begin();
  audio_SD.play("piano.wav");  // choose an audio file from the SD card
  delay(50);
  time = micros();
}

void loop() {
  float arrayIn[QUEUE_LEN];
  float arrayOut[QUEUE_LEN];
  float frameR[FFT_LEN];      // real value frame
  float frameC[2*FFT_LEN];    // complex value frame {real(0],imag(0),real(1),imag(1),...}
  int i=0;
  
  if(audio_SD.isPlaying() && read_array_form_queue(arrayIn, QUEUE_LEN, &queueIn)) {
    // copy input signal in arrayIn in blocks
    // copy elements in buffer
    for(i=0; i<QUEUE_LEN; i++) {
      buffIn.push(arrayIn[i]);
    } 
  }

  // process the frame
  if(buffIn.size() > FFT_LEN) { 
    time = micros();     
    read_frame_from_buffer(frameR, FFT_LEN);
          
    // window the frame and create complex frame
    for(i=0; i<FFT_LEN; i++) {
      frameR[i] *= win[i];
      frameC[2*i] = frameR[i]; // real part
      frameC[2*i+1] = 0;       // imaginary part
    }

    // compute FFT
    //Serial.println(F("compute FFT"));
    arm_cfft_radix2_f32(&fftInst, frameC);
  
    // process smearing
    
//    smearing_comp(frameC, B, FFT_LEN, FS);
//    Serial.print("in "); Serial.print(millis()-time); Serial.println(" ms\n"); // display smearing time
    
    // compute IFFT
    arm_cfft_radix2_f32(&ifftInst, frameC);
  
    // take real part and window
    for(i=0; i<FFT_LEN; i++) {
      frameR[i] = frameC[2*i]*win[i];
    }
  
    // do overlapp and add ine the buffer
    //Serial.println(buffOut.size());
    if(buffOut.size() < FFT_LEN/2){      // first time
      for(i=0; i<FFT_LEN; i++) {
        buffOut.push(frameR[i]);
      }
    }
    else {
      overlap_add(frameR, FFT_LEN);
    }
    
    // output the buffer
    int16_t* pOut;
    if(buffOut.size() > FFT_LEN + QUEUE_LEN) {
      // take the QUEUE_LEN first sample (from head)
      for(i=0; i<QUEUE_LEN; i++) {
        arrayOut[i] = buffOut.shift();
      }
      pOut = queueOut.getBuffer();
      // convert back to int16_t and out
      arm_float_to_q15(arrayOut, pOut, QUEUE_LEN);    
      queueOut.playBuffer();
    }
//    Serial.print((micros()-time)); Serial.println(" us"); // display total loop time
  }
}


/***************** util functions *****************/
// initialize SD card and other devices
void set_periph(void) {
  // set audio shield and SD card
  Serial.begin(9600);
  AudioMemory(300);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.35);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println(F("Unable to access the SD card"));
      delay(500);
    }
  }
  pinMode(13, OUTPUT); // LED on pin 13
}
/***************** util functions *****************/

