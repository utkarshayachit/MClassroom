#ifndef __qcDispatcher_h
#define __qcDispatcher_h

#include "qcBuffer.h"

#include <map>
#include <vector>
#include <opus/opus.h>

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>

template <class T, unsigned int numChannels>
class qcDispatcher
{
  qcBuffer<T, 1, numChannels> Buffer;
  OpusEncoder *Encoder;
  T *EncodingBuffer;

  static const int PACKET_SIZE = 1024 * 1024;
  boost::circular_buffer<unsigned char> EncodedBuffer;
  boost::mutex EncodedBufferMutex;
  boost::condition EncodedBufferNotEmpty;
public:
  qcDispatcher() :
    EncodingBuffer(new T[2880 * numChannels])
    {
    this->EncodedBuffer.set_capacity(PACKET_SIZE);

    int error;
    this->Encoder = opus_encoder_create(
      /*sample rate=*/48000,
      /*channels=*/numChannels,
      /*application=*/OPUS_APPLICATION_RESTRICTED_LOWDELAY,
      &error);
    }
  virtual ~qcDispatcher()
    {
    opus_encoder_destroy(this->Encoder);
    this->Encoder = NULL;
    delete [] this->EncodingBuffer;
    }

  // @threadsafe
  // Push raw data into the queue for dispatching. The data will be encoded
  // and pushed into an internal buffer (EncodedBuffer). If a packet gets
  // encoded, then we notify any awaiting threads so they can dispatch that
  // packet.
  void pushRawData(T* data, const unsigned int numFrames)
    {
    this->Buffer.push(0, data, numFrames);
    // opus needs exactly 2.5, 5, 10, 20, 40, or 60 ms of audio per encode call.
    // so for 48000Hz sample rate, we have
    static const double validFrameCounts[6] =
      {120, 240, 480, 960, 1920, 2880};

    unsigned int availableFrames = this->Buffer.availableFrames();
    unsigned int framesToEncode = 0;
    for (int cc=5; cc >=0; --cc)
      {
      if (validFrameCounts[cc] <= availableFrames)
        {
        framesToEncode = validFrameCounts[cc];
        break;
        }
      }
    if (framesToEncode > 0)
      {
      // FIXME: avoid this extra memcpy
      this->Buffer.pop(this->EncodingBuffer, framesToEncode);

      unsigned char packet[128];
      opus_int32 bytes = opus_encode_float(
        this->Encoder, this->EncodingBuffer, framesToEncode,
        packet, 128);
      boost::mutex::scoped_lock lock(this->EncodedBufferMutex);
      this->EncodedBuffer.insert(this->EncodedBuffer.end(),
        packet, packet+bytes);
      lock.unlock();
      this->EncodedBufferNotEmpty.notify_one();
      cout << "Encoded  " << framesToEncode << " frames as "
           << bytes << " bytes" << endl;
      }
    }

  // @threadsafe
  // pop encoded packet from the queue upto numBytes. This call will block until
  // some encoded data becomes available. Returns the number of bytes popped.
  unsigned int popEncodedData(unsigned char* data, const unsigned int numBytes)
    {
    boost::mutex::scoped_lock lock(this->EncodedBufferMutex);
    this->EncodedBufferNotEmpty.wait(lock,
      boost::bind(&qcDispatcher<T, numChannels>::encodedBufferNotEmpty, this));

    unsigned int count = std::min(numBytes,
      static_cast<unsigned int>(this->EncodedBuffer.size()));
    memcpy(&this->EncodedBuffer[0], data, count);
    this->EncodedBuffer.erase_begin(count);
    lock.unlock();

    return count;
    }
private:
  bool encodedBufferNotEmpty() { return this->EncodedBuffer.size(); }
};

#endif
