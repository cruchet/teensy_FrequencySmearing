#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>
#include <CircularBuffer.h>

#include "Arduino.h"
#include "arm_math.h"
//#include "smearing.h"

#define FFT_LEN           256
#define QUEUE_LEN         128

// GUItool: begin automatically generated code
AudioPlayQueue           queueOut;         //xy=113.00000762939453,403.600040435791
AudioPlaySdWav           audio_SD;     //xy=115.00006103515625,340.6000328063965
AudioOutputI2S           audio_out;           //xy=360.0000228881836,463.00003242492676
AudioRecordQueue         queueIn;         //xy=364.00008392333984,341.6000556945801
AudioConnection          patchCord1(queueOut, 0, audio_out, 0);
AudioConnection          patchCord2(queueOut, 0, audio_out, 1);
AudioConnection          patchCord3(audio_SD, 0, queueIn, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=135.00000762939453,482.0000343322754
// GUItool: end automatically generated code

#define SDCARD_CS_PIN    BUILTIN_SDCARD  // teensy 3.6
//#define SDCARD_CS_PIN    10              // teensy 3.2
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

CircularBuffer<int16_t, 10*FFT_LEN> buffIn;
CircularBuffer<int16_t, 10*FFT_LEN> buffOut;

unsigned long time = 0;

void setup() {
  set_periph();
  delay(2000);
  queueIn.begin();
  time=millis();
  audio_SD.play("piano.WAV");
  delay(50);
}

void loop() {
  int16_t arrayIn[FFT_LEN/2];
  int16_t frame[FFT_LEN];
  int i=0;
  
  if(audio_SD.isPlaying()) {
    // copy input signal in arrayIn in block of FFT_LEN/2
    read_array_form_queue(arrayIn, FFT_LEN/2);

    // copy elements in buffer
    for(i=0; i<FFT_LEN/2; i++) {
      buffIn.push(arrayIn[i]);
    }

    Serial.println("copy elements in buffer");
    // process the frame
    q15_t max = 0;
    uint32_t max_idx = 0;
    
    //arm_max_q15((q15_t*)AudioWindowHanning256, FFT_LEN, &max, &max_idx);
    if(buffIn.size() > FFT_LEN) {
      for(i=0; i<FFT_LEN; i++) {
        frame[i] = buffIn.pop();
      }
    }
    Serial.println("process the frame");

    // do overlapp and add ine the buffer
    if(buffOut.size() < FFT_LEN/2){      // fist time
      for(i=0; i<FFT_LEN; i++) {
        buffOut.push(frame[i]);
      }
      Serial.println("first time");
    }
    else {
      overlap_add(frame, FFT_LEN);
      Serial.println("do overlapp and add ine the buffer");
    }

    // output the buffer
    int16_t* pOut;
    if(buffOut.size() < FFT_LEN + QUEUE_LEN) {
      pOut = queueOut.getBuffer();
      // take the QUEUE_LEN first sample (from head)
      for(i=0; i<QUEUE_LEN; i++) {
        pOut[i] = buffOut.shift();
      }
      queueOut.playBuffer();
    }
    Serial.println("output the buffer\n");

//    if(queueIn.available()) {
//      buffer.push(queueIn.readBuffer());
//      queueIn.freeBuffer();    
//    //}
//  
//      if(!(buffer.isEmpty())) {
//        frameOut = queueOut.getBuffer();
//        buffOut = buffer.pop();
//        for(i=0;i<QUEUE_LEN;i++) {
//          frameOut[i] = buffOut[i];
//        }
//        queueOut.playBuffer();
//      }
//    }
  
    if(millis()-time>=250) {
      digitalWrite(13,HIGH);
    }
    if(millis()-time>=500) {
      digitalWrite(13,LOW);
      time=millis();
    }
  }
}

/* =================================================================================================== */
// copy elements of type flaot from an array to another
void copy_array_float(float dst[], float src[], int length) {
  int i=0;
  
  for(i=0; i<length; i++) {
    dst[i] = src[i];
  }
}

// process overlapp and add in the output buffer
void overlap_add(int16_t frame[], int array_length) {
  int i=0;

  // overalp first half
  for(i=array_length/2-1; i>=0; i--) {
    frame[i] += buffOut.pop();
  }
  // push back the whole frame (to tail)
  for(i=0; i<array_length; i++) {
    buffOut.push(frame[i]);
  }
}

// copy input signal in arrayIn in block of FFT_LEN/2
void read_array_form_queue(int16_t arrayIn[], int array_length) {
  int i=0;
  int k=0;
  int16_t* temp;

  if(array_length%QUEUE_LEN==0) {
    while(queueIn.available()<=array_length/QUEUE_LEN) { 
    }     // wait to have enough samples
    
    for(i=0; i<array_length/QUEUE_LEN; i++) {
      temp = queueIn.readBuffer();
      queueIn.freeBuffer();
      for(k=0; k<QUEUE_LEN; k++) {
        arrayIn[k+i*QUEUE_LEN] = (int16_t)temp[k];
      }
    }
  }
  else {
    Serial.println(F("error in read_array_form_queue: array_length must be a multiple of QUEUE_LEN"));
  }  
}

// initialize SD card and other devices
void set_periph(void) {
  // set audio shield and SD card
  Serial.begin(9600);
  AudioMemory(30);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.45);
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
