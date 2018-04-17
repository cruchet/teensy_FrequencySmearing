#include "circ_buff_util.h"

#include <CircularBuffer.h>
#include <Audio.h>
#include "Arduino.h"

CircularBuffer<float, BUFF_LEN> buffIn;
CircularBuffer<float, BUFF_LEN> buffOut;


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

// copy input signal in arrayIn in blocks. Retruns 0 if queue is not big enough
int read_array_form_queue(float arrayIn[], int array_length, AudioRecordQueue* queueIn) {
  int i=0;

  if(array_length%QUEUE_LEN==0) {
    if(queueIn->available()<array_length/QUEUE_LEN) { 
      return 0;     // not enough samples in queue
    }     
    for(i=0; i<array_length/QUEUE_LEN; i++) {
      arm_q15_to_float(queueIn->readBuffer(), arrayIn, array_length);
      queueIn->freeBuffer();
    }
  }
  else {
    Serial.println(F("error in read_array_form_queue: array_length must be a multiple of QUEUE_LEN"));
    return 0;
  }
  return 1;
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
