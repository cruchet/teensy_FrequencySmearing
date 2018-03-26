/* 
 * process a 1024pts FFT to a .wav audio file stored in SD card and write the result in a .txt file 
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav;    //
AudioOutputI2S           audio_out;    //
AudioAnalyzeFFT1024      fft1024;      //
AudioSynthWaveformSine   sinewave;

AudioConnection          patchCord1(playSdWav, 0, fft1024, 0);
AudioConnection          patchCord2(playSdWav, 0, audio_out, 0);
AudioConnection          patchCord3(playSdWav, 1, audio_out, 1);
/*AudioConnection patchCord1(sinewave, 0, fft1024, 0);
AudioConnection patchCord2(sinewave, 0, audio_out, 0);
AudioConnection patchCord3(sinewave, 0, audio_out, 1);*/
AudioControlSGTL5000     sgtl5000_1;     //xy=146.0056915283203,394.0056838989258
// GUItool: end automatically generated code


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

char* filenameIn  = "piano.wav";
char* filenameOut = "spectrum.txt";
int   first = 1;
int   last = 0;
int   frames = 0;

void setup() {
  Serial.begin(9600);
  AudioMemory(30);
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


  // read audio file
  if (playSdWav.isPlaying() == false) {
    Serial.println("Read audio file");
    digitalWrite(13,HIGH);
    playSdWav.play(filenameIn);
    delay(10); // wait for library to parse WAV info
    digitalWrite(13, LOW);
  }
  sinewave.amplitude(0.8);
  sinewave.frequency(1034.007);
  
 delay(1000);
  // read spectrum
  float spec[512];
  int   freqIdx[512];
  int   i=0;
  
  Serial.println("Waiting for FFT");
  while(!fft1024.available()) {
  }
  Serial.println("Read spectrum");
  for(i=0;i<512;i++) {
    spec[i]     = fft1024.read(i);
    freqIdx[i]  = i;
  }
 
  Serial.print("\n\ncpu usage:\t");
  Serial.println(AudioProcessorUsageMax());

  // write spectrum into .txt file
  File myFile;
  // open the file
  myFile = SD.open(filenameOut, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");

    for(i=0;i<512;i++) {
      myFile.print(freqIdx[i]);
      myFile.print(" ");
      myFile.print(i);
      //myFile.print("\n");
    }
    
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  digitalWrite(13,HIGH);
  delay(200);
  digitalWrite(13,LOW);
  delay(200);
  if(first) {
    Serial.print("Loop\n");
    first=0;
  }
  float spec[512];
  int   freqIdx[512];
  int   i=0;
  
  
  //Serial.println("Waiting for FFT");
  if(fft1024.available() && frames<1) {
    frames++;
    //Serial.println("Read spectrum");
    for(i=0;i<512;i++) {
      spec[i]     = fft1024.read(i);
      freqIdx[i]  = i;
      //Serial.println(freqIdx[i]); //Serial.print("\t"); 
      Serial.print(spec[i]);      Serial.print(" ");
    }
    Serial.println();
    last = 1;
  }
  if(frames==1 && last) { //stop playing and write .txt file
    AudioNoInterrupts();
    
    // write spectrum into .txt file
    File myFile;
    // open the file
    if(SD.exists(filenameOut)) {
      SD.remove(filenameOut);
    }
    Serial.println(myFile);
    myFile = SD.open(filenameOut, FILE_WRITE);
    Serial.println(myFile);
    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Writing to test.txt...");
  
      for(i=0;i<512;i++) {
        myFile.print(freqIdx[i]);
        myFile.print("\t");
        myFile.print(spec[i],9);    // 2nd argument specify the number of digits
        myFile.println();
      }
      
      // close the file:
      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
    last = 0;
    AudioInterrupts();
  }
  
  /*float n;
    
  if (fft1024.available()) {
    // each time new FFT data is available
    // print it all to the Arduino Serial Monitor
    Serial.print("FFT: ");
    for (i=0; i<40; i++) {
      spec[i] = fft1024.read(i);
      if (spec[i] >= 0.01) {
        Serial.print(spec[i]);
        Serial.print(" ");
      } else {
        Serial.print("  -  "); // don't print "0.00"
      }
    }
    Serial.println();
  }*/

}
