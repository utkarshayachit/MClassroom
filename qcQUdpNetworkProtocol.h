#ifndef __qcQUdpNetworkProtocol_h
#define __qcQUdpNetworkProtocol_h

#include <QUdpSocket>
#include <QHostAddress>
#include <QPointer>

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
#endif
