#ifndef global_h_
#define global_h_

#define FFT_LEN     256
#define QUEUE_LEN   128
#define FS          44100
#define B           6
#define BUFF_LEN    50*FFT_LEN

#define SDCARD_CS_PIN    BUILTIN_SDCARD  // teensy 3.6
//#define SDCARD_CS_PIN    10                // teensy 3.2
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#endif
