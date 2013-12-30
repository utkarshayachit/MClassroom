#include "qcNetwork.h"

#include "qcApp.h"

#include <QtDebug>
#include <QUdpSocket>

class qcNetwork::qcInternals
{
public:
  QUdpSocket Socket;
  QHostAddress DestinationAddr;
  quint16 DestinationPort;
};

//-----------------------------------------------------------------------------
qcNetwork::qcNetwork(QObject* parentObject) :
  Superclass(parentObject),
  Internals(new qcNetwork::qcInternals())
{
  this->connect(&this->Internals->Socket,
    SIGNAL(error(QAbstractSocket::SocketError)), SLOT(handleSocketError()));
  this->connect(&this->Internals->Socket,
    SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
}

//-----------------------------------------------------------------------------
qcNetwork::~qcNetwork()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
bool qcNetwork::startServer(
  const QHostAddress& addr, quint16 port)
{
  if (!this->Internals->Socket.bind(addr, port))
    {
    qCritical() << "Failed to bind to socket" << endl;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void qcNetwork::connectToHost(
  const QHostAddress& address, quint16 port)
{
  this->Internals->DestinationAddr = address;
  this->Internals->DestinationPort = port;
  this->start();
}

//-----------------------------------------------------------------------------
void qcNetwork::readPendingDatagrams()
{
  QUdpSocket *udpSocket = reinterpret_cast<QUdpSocket*>(this->sender());
  while (udpSocket->hasPendingDatagrams())
    {
    QByteArray datagram;
    datagram.resize(udpSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    udpSocket->readDatagram(datagram.data(), datagram.size(),
      &sender, &senderPort);

    //processTheDatagram(datagram);
    cout << "Read: " << datagram.size() << endl;
    }
}

//-----------------------------------------------------------------------------
void qcNetwork::run()
{
  unsigned char *data = new unsigned char[1024];
  QUdpSocket sendSocket;
  // FIXME: add logic to terminate gracefully.
  while (qcApp::Dispatcher.popEncodedData(data, 1024) > 0)
    {
    quint16 bytesSent = sendSocket.writeDatagram(
      reinterpret_cast<char*>(data), 1024,
      this->Internals->DestinationAddr, this->Internals->DestinationPort);
    }
  delete [] data;
}

//-----------------------------------------------------------------------------
void qcNetwork::handleSocketError()
{
  qCritical() << "SOCKET ERROR: " << this->Internals->Socket.errorString();
}
