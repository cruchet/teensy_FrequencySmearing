#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>

#include "Arduino.h"
#include "arm_math.h"
#include "RamMonitor.h"
//#include "smear_mat.h"
#include "smearing.h"

// GUItool: begin automatically generated code
AudioPlayQueue           queueOut;
AudioOutputI2S           audio_out;
AudioConnection          patchCord1(queueOut, 0, audio_out, 0);
AudioConnection          patchCord2(queueOut, 0, audio_out, 1);
AudioControlSGTL5000     sgtl5000_1;
// GUItool: end automatically generated code

//// windows.c
//extern "C" {
//  extern const int16_t AudioWindowHanning1024[];
//}

#define SDCARD_CS_PIN    BUILTIN_SDCARD  // teensy 3.6
//#define SDCARD_CS_PIN    10                // teensy 3.2
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define FFT_LEN           256
#define SIG_LEN          FFT_LEN*50
#define FFT_FLAG         0
#define IFFT_FLAG        1

// FFT variables
uint16_t  fftLen         = FFT_LEN;
uint8_t   fftFlag        = 0;
uint8_t   ifftFlag       = 1;
uint8_t   bitReverseFlag  = 1;
arm_cfft_radix4_instance_f32 fftInst;
arm_cfft_radix4_instance_f32 ifftInst;

// global variables
bool first = true;
const char filenameOut[] = {"fft_ifft.txt"};
RamMonitor ram;

void setup() {
  int i = 0;
  int nFrame = 0;
  float32_t tVec[SIG_LEN];      // time vector
  float32_t xVec[SIG_LEN];      // real intput signal
  float32_t yVec[SIG_LEN];      // real output signal
  float32_t frame[2 * FFT_LEN];   // complex frame
  float32_t win[FFT_LEN];       // window vector
  //float32_t spec_pow[FFT_LEN];  // spectrum
  float32_t fs = 16000;         // sampling frequency [Hz]
  float32_t ts = 1 / fs;        // sampling time [s]
  //float32_t T = SIG_LEN * ts;   // signal duration [s]
  float32_t sigFreq = 1000;     // signal frequency
  
  uint8_t   b       = 6;
  float32_t overlap = 0.5;
  q15_t     max     = 0;
  uint32_t  max_idx = 0;

  File myFile;

  arm_status fft_status = ARM_MATH_TEST_FAILURE;
  arm_status ifft_status = ARM_MATH_TEST_FAILURE;

  set_periph();
  delay(2000);

  // initialize FFT
  Serial.println(F("initialize FFT"));
  fft_status   = arm_cfft_radix4_init_f32(&fftInst, fftLen, fftFlag, bitReverseFlag);
  if (fft_status != ARM_MATH_SUCCESS) {
    Serial.println(F("error in initializing FFT"));
    if(fft_status == ARM_MATH_ARGUMENT_ERROR) {
      Serial.println(F("ARM_MATH_ARGUMENT_ERROR"));
    }
    else {
      Serial.println();
    }
  }
  ifft_status  = arm_cfft_radix4_init_f32(&ifftInst, fftLen, ifftFlag, bitReverseFlag);
  if (ifft_status != ARM_MATH_SUCCESS) {
    Serial.print(F("error in initializing IFFT:\t"));
    if(ifft_status == ARM_MATH_ARGUMENT_ERROR) {
      Serial.println(F("ARM_MATH_ARGUMENT_ERROR"));
    }
    else {
      Serial.println();
    }
  }
  Serial.print(F("FFT initialized\t")); Serial.print(fft_status); Serial.print("\t"); Serial.println(ifft_status);

  // synthetize input signal and initialize output vector
  Serial.println(F("synthetize input signal"));
  for (i = 0; i < SIG_LEN; i++) {
    tVec[i] = i*ts;
    xVec[i] = arm_sin_f32(2*PI*sigFreq*tVec[i]);// + 0.75*arm_sin_f32(2*PI*2*sigFreq*tVec[i]) + 0.5*arm_sin_f32(2*PI*4*sigFreq*tVec[i]);
    yVec[i] = 0;
  }
  // create window
  arm_max_q15((q15_t*)AudioWindowHanning256, FFT_LEN, &max, &max_idx);
  for(i=0; i<FFT_LEN; i++) {
    win[i] = sqrt((float)AudioWindowHanning256[i] / max);
  }
  
  // compute FFT / IFFT
  Serial.println(F("compute FFT / IFFT"));
  for (nFrame = 0; nFrame < SIG_LEN/FFT_LEN/overlap-1; nFrame++) {
    // copy frame
    Serial.print(F("nFrame=")); Serial.print(nFrame); Serial.println(F("\tcopy frame"));
    for (i = 0; i < FFT_LEN; i++) {
      frame[2*i] = xVec[i + nFrame * (int)(FFT_LEN*overlap)] * win[i]; // real part
      frame[2*i + 1] = 0;                     // imaginary part
    }
    Serial.println(F("\tFFT"));
    arm_cfft_radix4_f32(&fftInst, frame);
    
    smearing_comp(frame, b, FFT_LEN, fs);
    
    // IFFT
    Serial.println(F("\tIFFT"));
    arm_cfft_radix4_f32(&ifftInst, frame);
    for (i = 0; i < FFT_LEN; i++) {
      yVec[i + nFrame * (int)(FFT_LEN*overlap)] += (frame[2*i] * win[i]);
    }
  }

  // print xVec, yVec, and spec to .txt file
  // write spectrum into .txt file
  delay(1000);
  // open the file
  Serial.print(F("free RAM: "));  Serial.print((ram.free() + 512));  Serial.print(F(" bytes ("));
  Serial.print( (float)ram.free()/ram.total()*100);  Serial.println("%)");
  if (SD.exists(filenameOut)) {
    SD.remove(filenameOut);
  }
  Serial.print(F("write data to .txt file ..."));
  write_array_to_txt_line(NULL, xVec, SIG_LEN, filenameOut);
  write_array_to_txt_line(NULL, yVec, SIG_LEN, filenameOut);
  //write_array_to_txt_line(NULL, spec_pow, FFT_LEN, filenameOut);
  Serial.println("done");
}

/* ================================================== */

void loop() {
  if (first) {
    delay(1000);
    first = 0;
    Serial.println("in loop");
    digitalWrite(13, HIGH);
  }
  delay(3000);
  Serial.println("in loop");
  //digitalWrite(13, HIGH);
  delay(300);
  digitalWrite(13, LOW);
}


/* ================================================== */

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

// compute the sinus of a vector
void sin_vec_f32(float* vec_in, float* vec_out, int vec_length) {
  int i = 0;
  for (i = 0; i < vec_length; i++) {
    vec_out[i] = arm_sin_f32(vec_in[i]);
  }
}
