#ifndef __qcDispatcher_h
#define __qcDispatcher_h

#include "qcBuffer.h"

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>

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
private:
  qcBuffer<T, 1, numChannels> Buffer;
  OpusEncoder *Encoder;
  T *EncodingBuffer;
  boost::condition BufferNotEmpty;
  boost::mutex BufferMutex;

  std::pair<QHostAddress, quint16> Destination;
  boost::mutex DestinationMutex;

  class qcDispatcherThread : public QThread
  {
  qcDispatcher<T, numChannels>& Self;
public:
  qcDispatcherThread(qcDispatcher<T, numChannels>& self)
    : Self(self)
    {
    }
  ~qcDispatcherThread()
    {
    }
protected:
  virtual void run()
    {
    unsigned char *data = new unsigned char[1024];
    QUdpSocket sendSocket;
    while (unsigned int packet_size = this->Self.popEncodedData(data, 1024))
      {
      std::pair<QHostAddress, quint16> reply = this->Self.destination();
      if (!reply.first.isNull() && reply.second > 0)
        {
        quint16 bytesSent = sendSocket.writeDatagram(
          reinterpret_cast<char*>(data), packet_size, reply.first, reply.second);
        }
      }
    delete [] data;
    cout << "Quitting receiver thread." << endl;
    }
private:
  Q_DISABLE_COPY(qcDispatcherThread);
  };

  qcDispatcherThread SenderThread;
  bool Abort;
public:
  qcDispatcher() :
    EncodingBuffer(new T[2880 * numChannels]), // maximum frame size per encode call.
    SenderThread(*this)
    {

    int error;
    this->Encoder = opus_encoder_create(
      /*sample rate=*/48000,
      /*channels=*/numChannels,
      /*application=*/OPUS_APPLICATION_RESTRICTED_LOWDELAY,
      &error);
    }
  virtual ~qcDispatcher()
    {
      {
      // stop thread gracefully.
      boost::mutex::scoped_lock lock(this->BufferMutex);
      this->Abort = true;
      }
    this->SenderThread.wait();

    opus_encoder_destroy(this->Encoder);
    this->Encoder = NULL;
    delete [] this->EncodingBuffer;
    }

  // @threadsafe
  // Specify the destination where to ship packets.
  void setDestination(const QHostAddress& addr, quint16 port)
    {
    boost::mutex::scoped_lock lock(this->DestinationMutex);
    this->Destination = std::pair<QHostAddress, quint16>(addr, port);
    if (!this->SenderThread.isRunning())
      {
      this->SenderThread.start();
      }
    }

  // @threadsafe
  std::pair<QHostAddress, quint16> destination()
    {
    boost::mutex::scoped_lock lock(this->DestinationMutex);
    std::pair<QHostAddress, quint16> reply = this->Destination;
    return reply;
    }

  // @threadsafe
  // Push raw data into the queue for dispatching. Any threads awaiting for
  // buffer to have data will be notified.
  void pushRawData(T* data, const unsigned int numFrames)
    {
    this->Buffer.push(0, data, numFrames);
    this->BufferNotEmpty.notify_one();
    }

private:
  bool bufferNotEmpty() { return this->Buffer.availableFrames() >= 120; }

  // @threadsafe
  // pop encoded packet from the queue upto numBytes. This call will block until
  // some encoded data becomes available. Returns the number of bytes popped.
  unsigned int popEncodedData(unsigned char* data, const unsigned int numBytes)
    {
    boost::mutex::scoped_lock lock(this->BufferMutex);
    while (!this->BufferNotEmpty.timed_wait(lock,
      boost::posix_time::seconds(0.1),
      boost::bind(&qcDispatcher<T, numChannels>::bufferNotEmpty, this)))
      {
      // timed out.
      if (this->Abort)
        {
        return 0;
        }
      }
    if (this->Abort)
      {
      return 0;
      }
    lock.unlock();

    // opus needs exactly 2.5, 5, 10, 20, 40, or 60 ms of audio per encode call.
    // so for 48000Hz sample rate, we have
    static const double validFrameCounts[] =
        {2880, 1920, 960, 480, 240, 120, 0};

    unsigned int availableFrames = this->Buffer.availableFrames();
    unsigned int framesToEncode = 0;
    for (int cc=0; validFrameCounts[cc] != 0; ++cc)
      {
      if (validFrameCounts[cc] <= availableFrames)
        {
        framesToEncode = validFrameCounts[cc];
        break;
        }
      }

    if (framesToEncode == 0)
      {
      return 0;
      }

    // FIXME: avoid this extra memcpy
    this->Buffer.pop(this->EncodingBuffer, framesToEncode);

    opus_int32 bytes = opus_encode_float(
      this->Encoder, this->EncodingBuffer, framesToEncode, data, numBytes);
    //cout << "Encoded  " << framesToEncode << " frames as "
    //  << bytes << " bytes" << endl;
    return bytes;
    }
};

#endif
