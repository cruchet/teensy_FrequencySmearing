#ifndef circ_buff_util_h_
#define circ_buff_util_h_

#include <CircularBuffer.h>
#include <Audio.h>
#include "global.h"

extern CircularBuffer<float, 10*FFT_LEN> buffIn;
extern CircularBuffer<float, 10*FFT_LEN> buffOut;

// process overlapp and add in the output buffer
void overlap_add(float frame[], int array_length);

// copy input signal in arrayIn in block of FFT_LEN/2
void read_array_form_queue(float arrayIn[], int array_length, AudioRecordQueue* queueIn);

// read elements in buffIn with 50% overlap to create frame
void read_frame_from_buffer(float frame[], int frame_l);


#endif
