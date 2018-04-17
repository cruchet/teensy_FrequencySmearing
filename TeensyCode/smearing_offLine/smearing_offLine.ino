/**
 * smearing_offLine.ino
 * Process frequency smearing on a .wav file stored on the SD card, 
 * using block processing and circular buffer to achieve overlap-and-add.
 * 
 * Changing parameters as b (smearing coefficient), frame lenght and smapling frequency
 * induce recalculating smearing matrix with MatLab script "generate_smear_matrix".
 * However, the audioSD block only support fs=44.1kHz and the fft function works only for
 * frame length of 16, 64, 256, 1024 samples.
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
float           win[FFT_LEN];               // vector for indow function
unsigned long   time = 0;
unsigned int    count =0;
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
  
  audio_SD.play("piano.wav");
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
    
    //smearing_comp(frameC, B, FFT_LEN, FS);
    //AudioNoInterrupts();
    //delayMicroseconds(1000);      // = QUEUE_LEN/fs
    //AudioInterrupts();
    //Serial.print("in "); Serial.print(millis()-time); Serial.println(" ms\n");
    
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
      Serial.println(buffOut.size());
      // take the QUEUE_LEN first sample (from head)
      for(i=0; i<QUEUE_LEN; i++) {
        arrayOut[i] = buffOut.shift();
      }
      pOut = queueOut.getBuffer();
      // convert back to int16_t and out
      arm_float_to_q15(arrayOut, pOut, QUEUE_LEN);    
      queueOut.playBuffer();
    }
    Serial.print((micros()-time)); Serial.println(" us");
  }
  
  /*if(millis()-time>=250) {
    digitalWrite(13,HIGH);
  }
  if(millis()-time>=500) {
    digitalWrite(13,LOW);
    time=millis();
  }*/
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

// function to change I2S sampling frequncy 
// from: https://forum.pjrc.com/threads/38753-Discussion-about-a-simple-way-to-change-the-sample-rate?p=121384&viewfull=1#post121384)
//void setI2SFreq(int freq) {
//  typedef struct {
//    uint8_t mult;
//    uint16_t div;
//  } __attribute__((__packed__)) tmclk;
//  const int numfreqs = 14;
//  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, 44117.64706 , 48000, 88200, 44117.64706 * 2, 96000, 176400, 44117.64706 * 4, 192000};
//
//#if (F_PLL==16000000)
//  const tmclk clkArr[numfreqs] = {{16, 125}, {148, 839}, {32, 125}, {145, 411}, {64, 125}, {151, 214}, {12, 17}, {96, 125}, {151, 107}, {24, 17}, {192, 125}, {127, 45}, {48, 17}, {255, 83} };
//#elif (F_PLL==72000000)
//  const tmclk clkArr[numfreqs] = {{32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {128, 1125}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375}, {249, 397}, {32, 51}, {185, 271} };
//#elif (F_PLL==96000000)
//  const tmclk clkArr[numfreqs] = {{8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {32, 375}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125}, {151, 321}, {8, 17}, {64, 125} };
//#elif (F_PLL==120000000)
//  const tmclk clkArr[numfreqs] = {{32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {128, 1875}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625}, {178, 473}, {32, 85}, {145, 354} };
//#elif (F_PLL==144000000)
//  const tmclk clkArr[numfreqs] = {{16, 1125}, {49, 2500}, {32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {4, 51}, {32, 375}, {98, 625}, {8, 51}, {64, 375}, {196, 625}, {16, 51}, {128, 375} };
//#elif (F_PLL==180000000)
//  const tmclk clkArr[numfreqs] = {{46, 4043}, {49, 3125}, {73, 3208}, {98, 3125}, {183, 4021}, {196, 3125}, {16, 255}, {128, 1875}, {107, 853}, {32, 255}, {219, 1604}, {214, 853}, {64, 255}, {219, 802} };
//#elif (F_PLL==192000000)
//  const tmclk clkArr[numfreqs] = {{4, 375}, {37, 2517}, {8, 375}, {73, 2483}, {16, 375}, {147, 2500}, {1, 17}, {8, 125}, {147, 1250}, {2, 17}, {16, 125}, {147, 625}, {4, 17}, {32, 125} };
//#elif (F_PLL==216000000)
//  const tmclk clkArr[numfreqs] = {{32, 3375}, {49, 3750}, {64, 3375}, {49, 1875}, {128, 3375}, {98, 1875}, {8, 153}, {64, 1125}, {196, 1875}, {16, 153}, {128, 1125}, {226, 1081}, {32, 153}, {147, 646} };
//#elif (F_PLL==240000000)
//  const tmclk clkArr[numfreqs] = {{16, 1875}, {29, 2466}, {32, 1875}, {89, 3784}, {64, 1875}, {147, 3125}, {4, 85}, {32, 625}, {205, 2179}, {8, 85}, {64, 625}, {89, 473}, {16, 85}, {128, 625} };
//#endif
//
//  for (int f = 0; f < numfreqs; f++) {
//    if ( freq == samplefreqs[f] ) {
//      while (I2S0_MCR & I2S_MCR_DUF) ;
//      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
//      return;
//    }
//  }
//}
/***************** util functions *****************/

