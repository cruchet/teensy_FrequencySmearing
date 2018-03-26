/* 
 *  Perform fft on .wav file from SD card, using "queue" blocks
 */


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioRecordQueue         queueIn;  
AudioPlayQueue           queueOut;         
AudioPlaySdWav           audioSD;     
AudioAnalyzeFFT1024      fft1024;            
AudioOutputI2S           audio_out;           
AudioConnection          patchCord1(queueOut, 0, audio_out, 0);
AudioConnection          patchCord2(queueOut, 0, audio_out, 1);
AudioConnection          patchCord3(queueOut, fft1024);
AudioConnection          patchCord4(audioSD, 0, queueIn, 0);
AudioControlSGTL5000     sgtl5000_1;
// GUItool: end automatically generated code

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define FRAME_SIZE       128
#define FFT_SIZE         1024

// global variables
bool  playing     = false;
bool  first       = true;
int   count       = 0;
char* filenameIn  = "piano.wav";
char* filenameOut = "spectrum.txt";

elapsedMicros elapsedTimeUs = 0;
elapsedMillis elapsedTimeMs = 0;
elapsedMillis oldTime       = 0;

void setup() {
  // set audio shield and SD card
  Serial.begin(9600);
  AudioMemory(100);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.45);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  pinMode(13, OUTPUT); // LED on pin 13
  delay(1000);

  // configure FFT
  fft1024.windowFunction(AudioWindowHanning1024);

  // start input queue
  queueIn.begin();

  // start playing audio file
  if(audioSD.isPlaying() == false && first) {
    digitalWrite(13,HIGH);
    audioSD.play(filenameIn);
    delay(20); // wait for library to parse WAV info
    playing = true;
    Serial.println("Start playing audio file");
  }
}

/* ================================================== */

void loop() {
  short int*   frameIn;
  short int*   frameOut;
  short int    buffOut[8*FRAME_SIZE];
  float spec[512];
  int   freqIdx[512];
  int   i=0;


  if(first) {
    first = 0;
  }
  
  // wait for frames to be available and read
  if(queueIn.available()) {
    Serial.print("read frameIn "); Serial.println(queueIn.available());
    frameIn = queueIn.readBuffer();
    if(frameIn == NULL) {
      Serial.println("No packet avialable for frameIn");
    }
    queueIn.freeBuffer();    
  }

  
 

  // process FFT
  if(fft1024.available()) {
    Serial.println("**** Read spectrum ****");
    for(i=0;i<512;i++) {
      spec[i]     = fft1024.read(i);
      freqIdx[i]  = i;
    }
        
   /*// write spectrum to .txt file (one line per frame)
   if(first) { 
      if(SD.exists(filenameOut)) {
        SD.remove(filenameOut);
      }
      write_array_to_txt_line(freqIdx, NULL, FFT_SIZE/2, filenameOut);
      first = false;
    }
    else {
      write_array_to_txt_line(NULL, spec, FFT_SIZE/2, filenameOut);
    }*/
  }

  if(queueIn.available()) {
     // put frame into queueOut
    frameOut = queueOut.getBuffer();
    if(frameOut == NULL) {
      Serial.println("Out of memory for queueOut");
    }
    else {
      for(i=0;i<FRAME_SIZE;i++) {
        frameOut[i] = frameIn[i];  
      }
      //memcpy(frameOut, frameIn, FRAME_SIZE);
      queueOut.playBuffer();  // send data to fft1024 and audioOut
      //Serial.println("write frameOut");
    }
  }

  // end of audio file
  if(audioSD.isPlaying() == false && playing == true) {
    Serial.println("end of audio file");
    digitalWrite(13,LOW);
    //queueIn.clear();
    queueIn.end();
    playing = false;
  }  

}

/* ================================================== */

// writes ONE array (int OR float) on one line of filename.txt file. Set the other pointer to NULL
void write_array_to_txt_line(int* arrayInt, float* arrayFloat, int arraySize, char* filename) {
  int i = 0;
  File myFile = SD.open(filename, FILE_WRITE);

  if(myFile) {
    if(arrayInt) {
      for(i=0;i<arraySize;i++) {
        myFile.print(arrayInt[i]);
        myFile.print(" ");
      }
      myFile.println();
    }
    else if(arrayFloat) {
      for(i=0;i<arraySize;i++) {
        myFile.print(arrayFloat[i],9);
        myFile.print(" "); 
      }
      myFile.println();
    }
    else {
      Serial.println("error: NULL array to write");
    }
    // close the file:
    myFile.close();
  }
  else {
    Serial.println("error opening .txt file");
  }
}

