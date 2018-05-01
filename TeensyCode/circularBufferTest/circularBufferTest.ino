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
#include "circ_buff_util.h"
#include "global.h"

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

float win[FFT_LEN];
unsigned long time = 0;
bool end = true;
const char filenameOut[] = {"buffer_out.txt"};

void setup() {
  set_periph();
  create_window(win, FFT_LEN);
  delay(2000);
  queueIn.begin();
  time=millis();
  audio_SD.play("piano.WAV");
  delay(50);
}

void loop() {
  float arrayIn[FFT_LEN/2];
  float frame[FFT_LEN];
  int i=0;
  
  //if(audio_SD.isPlaying()) {
    // copy input signal in arrayIn in block of FFT_LEN/2
    read_array_form_queue(arrayIn, FFT_LEN/2, &queueIn);

    // copy elements in buffer
    for(i=0; i<FFT_LEN/2; i++) {
      buffIn.push(arrayIn[i]);
    }

    // process the frame
    if(buffIn.size() > FFT_LEN) {      
      read_frame_from_buffer(frame, FFT_LEN);
            
      // window the frame
      for(i=0; i<FFT_LEN; i++) {
        frame[i] *= win[i];
      }
    }
    
    // do overlapp and add ine the buffer
    if(buffOut.size() < FFT_LEN/2){      // fist time
      for(i=0; i<FFT_LEN; i++) {
        buffOut.push(frame[i]);
      }
    }
    else {
      overlap_add(frame, FFT_LEN);
    }

    // output the buffer
    int16_t* pOut;
    if(buffOut.size() < FFT_LEN + QUEUE_LEN) {
      pOut = queueOut.getBuffer();
      // take the QUEUE_LEN first sample (from head)
      for(i=0; i<QUEUE_LEN; i++) {
        pOut[i] = (int16_t)buffOut.shift();
      }
      queueOut.playBuffer();
    }
  
    if(millis()-time>=250) {
      digitalWrite(13,HIGH);
    }
    if(millis()-time>=500) {
      digitalWrite(13,LOW);
      time=millis();
    }
  //}
//  else if(end) {
//    queueIn.end();
//    delay(2000);
//    Serial.println("done");
////    AudioNoInterrupts();
////    write_array_to_txt_line((int*)signal, NULL, 11813, filenameOut);
////    AudioInterrupts();
//    end=false;
//  }
}

/* =================================================================================================== */
// copy elements of type flaot from an array to another
void copy_array_float(float dst[], float src[], int length) {
  int i=0;
  
  for(i=0; i<length; i++) {
    dst[i] = src[i];
  }
}

// creates a normalized hann window
void create_window(float win[], int win_l){
  int i=0;
  q15_t     max     = 0;
  uint32_t  max_idx = 0;
  
  arm_max_q15((q15_t*)AudioWindowHanning256, win_l, &max, &max_idx);
  for(i=0; i<win_l; i++) {
    win[i] = (float)AudioWindowHanning256[i] / max;
  }
}

// writes ONE array (int OR float) on one line of filename.txt file. Set the other pointer to NULL
void write_array_to_txt_line(int* arrayInt, float* arrayFloat, int arraySize, const char* filename) {
  int i = 0;
  File myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    if (arrayInt) {
      for (i = 0; i < arraySize; i++) {
        myFile.print(arrayInt[i]);
        myFile.print(" ");
      }
      myFile.println();
    }
    else if (arrayFloat) {
      for (i = 0; i < arraySize; i++) {
        myFile.print(arrayFloat[i], 9);
        myFile.print(" ");
      }
      myFile.println();
    }
    else {
      Serial.println(F("error: NULL array to write"));
    }
    // close the file:
    myFile.close();
  }
  else {
    Serial.println(F("error opening .txt file"));
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
