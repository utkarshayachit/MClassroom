#ifndef __qcReceiver_h
#define __qcReceiver_h

#include "qcApp.h"
#include <opus/opus.h>

class qcReceiverBase
{
public:
  virtual void processDatagram(const unsigned char* packet, unsigned int size) = 0;
};


template <class T, unsigned int numChannels>
class qcReceiver : public qcReceiverBase
{
private:
  OpusDecoder *Decoder;
  T* DecodingBuffer;
public:
  qcReceiver() :
    DecodingBuffer(new T[5760 * numChannels]) // maximum frame size per decode call.
  {
  int error;
  this->Decoder = opus_decoder_create(
    /*sample rate=*/ 48000,
    /*channels=*/numChannels,
    &error);
  }
  ~qcReceiver()
    {
    opus_decoder_destroy(this->Decoder);
    this->Decoder = NULL;
    delete [] this->DecodingBuffer;
    this->DecodingBuffer = NULL;
    }

  virtual void processDatagram(const unsigned char* packet, unsigned int size)
    {
    int availableFrames = opus_decode_float(
      this->Decoder, packet, size, this->DecodingBuffer, 5760, 0);
    cout << "Processed: " << availableFrames << " frames" << endl;
    qcApp::AudioStream.push(2, this->DecodingBuffer, availableFrames);
    }
};
#endif
