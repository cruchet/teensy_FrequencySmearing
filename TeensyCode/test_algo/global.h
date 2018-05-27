#ifndef global_h_
#define global_h_

#define COMPRESSED                  // define to use row-indexed sparse storage
#define FFT_LEN     256             // frame length
#define QUEUE_LEN   AUDIO_BLOCK_SAMPLES    // Need to be the same as in AudioStream.h
#define FS          16000           // sampling frequency. See setI2SFreq() for possible values
#define B           6               // smearing coefficient. Look in smear_mat.h for possible values
#define BUFF_LEN    10*QUEUE_LEN    // circular buffers' length

#define BUTTON1_PIN  2
#define LED_PIN      1

#define SDCARD_CS_PIN    BUILTIN_SDCARD  // teensy 3.6
//#define SDCARD_CS_PIN  10              // teensy 3.2
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#endif
