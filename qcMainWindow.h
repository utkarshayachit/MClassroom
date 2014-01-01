#ifndef __qcMainWindow_h
#define __qcMainWindow_h

#include <QMainWindow>

class qcMainWindow : public QMainWindow
{
  Q_OBJECT
  typedef QMainWindow Superclass;
public:
  qcMainWindow();
  virtual ~qcMainWindow();

protected slots:
  void start();
  void stop();

  void audioInput1Changed(int);
  void audioInput2Changed(int);
  void audioOutputChanged(int);
  void gainChanged(int gain);

  void connectToPeer();
  void disconnectFromPeer();
protected:
  void queryAvailableDevices();

private:
  Q_DISABLE_COPY(qcMainWindow);

  class qcInternals;
  qcInternals* Internals;
};

#endif
