#ifndef __qcReceiver_h
#define __qcReceiver_h

#include "qcApp.h"

#include <QtDebug>
#include <QUdpSocket>
#include <QObject>

class qcReceiverBase
{
public:
  virtual void process(const QByteArray& datagram) = 0;
};

class qcReceiverHandler : public QObject
{
  Q_OBJECT;
  typedef QObject Superclass;
  QUdpSocket Socket;
  qcReceiverBase& Self;
public:
  qcReceiverHandler(quint16 port, qcReceiverBase& self) : Self(self)
    {
    this->connect(&this->Socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(handleSocketError()));
    this->connect(&this->Socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
    if (!this->Socket.bind(QHostAddress::LocalHost, port))
      {
      qCritical("Failed to start server socket!!!");
      }
    }
  virtual ~qcReceiverHandler() { }

private slots:
  void readPendingDatagrams()
    {
    while (this->Socket.hasPendingDatagrams())
      {
      QByteArray datagram;
      datagram.resize(this->Socket.pendingDatagramSize());
      QHostAddress sender;
      quint16 senderPort;
      this->Socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
      this->Self.process(datagram);
      }
    }

  void handleSocketError()
    {
    qCritical() << "SOCKET ERROR: " << this->Socket.errorString();
    }

private:
  Q_DISABLE_COPY(qcReceiverHandler);
};

#include <opus/opus.h>

template <class T, unsigned int numChannels>
class qcReceiver : public qcReceiverBase
{
private:
  qcReceiverHandler Handler;
  OpusDecoder *Decoder;
  T* DecodingBuffer;
public:
  qcReceiver(quint16 receiverPort) : Handler(receiverPort, *this),
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

  virtual void process(const QByteArray& datagram)
    {
    int availableFrames = opus_decode_float(
      this->Decoder,
      reinterpret_cast<const unsigned char*>(datagram.data()), datagram.size(),
      this->DecodingBuffer, 5760, 0);
    cout << "Processed: " << availableFrames << " frames" << endl;
    qcApp::AudioStream.push(2, this->DecodingBuffer, availableFrames);
    }
};
#endif
