#include "circ_buff_util.h"

#include <CircularBuffer.h>
#include <Audio.h>
#include "Arduino.h"

CircularBuffer<float, 10*FFT_LEN> buffIn;
CircularBuffer<float, 10*FFT_LEN> buffOut;


// process overlapp and add in the output buffer
void overlap_add(float frame[], int array_length) {
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
void read_array_form_queue(float arrayIn[], int array_length, AudioRecordQueue* queueIn) {
  int i=0;
  int k=0;
  int16_t* temp;

  if(array_length%QUEUE_LEN==0) {
    while(queueIn->available()<=array_length/QUEUE_LEN) { 
    }     // wait to have enough samples
    
    for(i=0; i<array_length/QUEUE_LEN; i++) {
      temp = queueIn->readBuffer();
      queueIn->freeBuffer();
      for(k=0; k<QUEUE_LEN; k++) {
        arrayIn[k+i*QUEUE_LEN] = (float)temp[k];
      }
    }
  }
  else {
    Serial.println(F("error in read_array_form_queue: array_length must be a multiple of QUEUE_LEN"));
  }  
}

// read elements in buffIn with 50% overlap to create frame
void read_frame_from_buffer(float frame[], int frame_l) {
  int i=0;
  
  //read first half and remove it from buffIn
  for(i=0;i<frame_l/2;i++) {
    frame[i] = buffIn.shift();
  }
  // read 2nd half but leave it for next frame (overlap)
  for(i=frame_l/2; i<frame_l; i++) {
    frame[i] = buffIn[i-frame_l/2];  
  }
}
