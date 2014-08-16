#ifndef __qcReceiver_h
#define __qcReceiver_h

#include "qcApp.h"
#include <opus/opus.h>
#include <cassert>

class qcReceiverBase
{
public:
  virtual void processPacket(const unsigned char* packet, unsigned int size) = 0;
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
  // Opus only supports these sample rates: 8000, 12000, 16000, 24000, or 48000
  assert(qcApp::SampleRate == 8000 ||
    qcApp::SampleRate == 12000 ||
    qcApp::SampleRate == 16000 ||
    qcApp::SampleRate == 24000 ||
    qcApp::SampleRate == 48000);
  this->Decoder = opus_decoder_create(
    /*sample rate=*/ qcApp::SampleRate,
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

  virtual void processPacket(const unsigned char* packet, unsigned int size)
    {
    int availableFrames = opus_decode_float(
      this->Decoder, packet, size, this->DecodingBuffer, 5760, 0);
    //cout << "Processed: " << availableFrames << " frames" << endl;
    if (availableFrames < 0)
      {
      cout << "Invalid packet, ignoring" << endl;
      return;
      }
    qcApp::AudioStream.push(2, this->DecodingBuffer, availableFrames);
    }
};
#endif
