#ifndef __qcQUdpNetworkProtocol_h
#define __qcQUdpNetworkProtocol_h

#include <QHostAddress>
#include <QPointer>
#include <QUdpSocket>
#include <QtDebug>
#include "qcReceiver.h"

class qcQUdpNetworkProtocolSend
{
  QHostAddress Address;
  quint16 Port;
  QPointer<QUdpSocket> Socket;
public:
  qcQUdpNetworkProtocolSend(const QHostAddress& addr, quint16 port)
    : Address(addr), Port(port)
    {
    Q_ASSERT(!addr.isNull() && port > 0);
    }

  ~qcQUdpNetworkProtocolSend()
    {
    this->Socket->deleteLater();
    }

  unsigned int sendPacket(const unsigned char* packet, unsigned int size)
    {
    if (!this->Socket) { this->Socket = new QUdpSocket(); }
    return static_cast<unsigned int>(
      this->Socket->writeDatagram(
        reinterpret_cast<const char*>(packet), size, this->Address, this->Port));
    }
};

class qcQUdpNetworkProtocolReceive : public QObject
{
  quint16 Port;
  QUdpSocket Socket;
  boost::shared_ptr<qcReceiverBase> Receiver;
  Q_OBJECT;
  typedef QObject Superclass;

public:
  qcQUdpNetworkProtocolReceive(quint16 port, boost::shared_ptr<qcReceiverBase> receiver) :
    Port(port),
    Receiver(receiver)
    {
    Q_ASSERT(port > 0);
    Q_ASSERT(receiver != NULL);
    this->connect(&this->Socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(handleSocketError()));
    this->connect(&this->Socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
    if (!this->Socket.bind(QHostAddress::LocalHost, port))
      {
      qCritical("Failed to start server socket!!!");
      }
    }
  ~qcQUdpNetworkProtocolReceive()
    {
    }
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
      this->Receiver->processDatagram(
        reinterpret_cast<const unsigned char*>(datagram.data()),
        static_cast<unsigned int>(datagram.size()));
      }
    }

  void handleSocketError()
    {
    qCritical() << "SOCKET ERROR: " << this->Socket.errorString();
    }

private:
  Q_DISABLE_COPY(qcQUdpNetworkProtocolReceive);

};
#endif
