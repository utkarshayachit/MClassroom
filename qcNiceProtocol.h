#ifndef __qcNiceProtocol_h
#define __qcNiceProtocol_h


#include <QHostAddress>
#include <QObject>

class qcNiceProtocol : public QObject
{
  Q_OBJECT;
  typedef QObject Superclass;

public:
  qcNiceProtocol(
    const QHostAddress& stunServer, quint16 stunServerPort=3478);
  ~qcNiceProtocol();

  // return my connection ticket. Empty if setup failed.
  const QString& ticket() const
    { return this->Ticket; }
  bool connect(const QString& remoteTicket);

  // send a packet.
  void sendPacket(const unsigned char* packet, unsigned int packet_size);

private slots:
  void processEvents();

private:
  // We know information about ourselves. Let the UI display this information so
  // users can communicate this to one another.
  void candidateGatheringDone(unsigned int streamid);

private:
  Q_DISABLE_COPY(qcNiceProtocol);
  QString Ticket;

  class qcInternals;
  friend class qcInternals;
  qcInternals* Internals;
};

#endif
