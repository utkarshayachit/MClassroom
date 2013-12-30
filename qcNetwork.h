#ifndef __qcNetwork_h
#define __qcNetwork_h

#include <QThread>
#include <QHostAddress>

class qcNetwork : public QThread
{
  Q_OBJECT;
  typedef QThread Superclass;
public:
  qcNetwork(QObject* parentObject=NULL);
  virtual ~qcNetwork();

public slots:
  bool startServer(
    const QHostAddress& address, quint16 port);

  void connectToHost(
    const QHostAddress& address, quint16 port);

private slots:
  void readPendingDatagrams();

  void handleSocketError();

protected:
  virtual void run();
private:
  Q_DISABLE_COPY(qcNetwork);
  class qcInternals;
  friend class qcInternals;
  qcInternals* Internals;
};

#endif
