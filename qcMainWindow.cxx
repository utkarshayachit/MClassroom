#include "qcMainWindow.h"
#include "ui_qcMainWindow.h"


#include <QApplication>
#include <QAudioDeviceInfo>
#include <QList>
#include <QtDebug>
#include <QAudioOutput>
#include <QAudioInput>
#include <QPointer>
class qcMainWindow::qcInternals
{
public:
  Ui::QCMainWindow Ui;

  qcInternals(qcMainWindow* self)
    {
    this->Ui.setupUi(self);
    }

  QPointer<QAudioOutput> Output;
  QPointer<QAudioInput> Input1;
  QPointer<QAudioInput> Input2;

};

//-----------------------------------------------------------------------------
qcMainWindow::qcMainWindow() : Internals(new qcInternals(this))
{
  // setup information about available devices.
  this->queryAvailableDevices();

  Ui::QCMainWindow &ui = this->Internals->Ui;
  QObject::connect(ui.startButton, SIGNAL(clicked()), this, SLOT(start()));
  QObject::connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stop()));
}

//-----------------------------------------------------------------------------
qcMainWindow::~qcMainWindow()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
void qcMainWindow::queryAvailableDevices()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;

  QList<QAudioDeviceInfo> audioDevices = QAudioDeviceInfo::availableDevices(
    QAudio::AudioInput);
  foreach (const QAudioDeviceInfo& info, audioDevices)
    {
    ui.audioInput1->addItem(info.deviceName());
    ui.audioInput2->addItem(info.deviceName());
    }

  audioDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
  foreach (const QAudioDeviceInfo& info, audioDevices)
    {
    ui.audioOutput->addItem(info.deviceName());
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::start()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.stopButton->setEnabled(true);
  ui.startButton->setEnabled(false);

  QList<QAudioDeviceInfo> inputAudioDevices = QAudioDeviceInfo::availableDevices(
    QAudio::AudioInput);

  QList<QAudioDeviceInfo> outputAudioDevices = QAudioDeviceInfo::availableDevices(
    QAudio::AudioOutput);

  QAudioFormat format = outputAudioDevices[ui.audioOutput->currentIndex()].preferredFormat();
  qDebug() << format.channels() << ", "
           << format.sampleSize() << ", "
           << format.sampleRate() << ", "
           << format.codec() << ", "
           << format.sampleType() ;

  const QAudioDeviceInfo& input1Device = inputAudioDevices[ui.audioInput1->currentIndex()];
  qDebug() << "Codecs: " << input1Device.supportedCodecs() <<"\n"
           << "Sample rates: " << input1Device.supportedSampleRates() << "\n"
           << "Sample sizes: " << input1Device.supportedSampleSizes() << "\n";

  QAudioFormat format2 = input1Device.preferredFormat();
  qDebug() << format2.channels() << ", "
           << format2.sampleSize() << ", "
           << format2.sampleRate() << ", "
           << format2.codec() << ", "
           << format2.sampleType() ;
  format2.setChannels(1);
  format2.setSampleSize(8);


  this->Internals->Output = new QAudioOutput(format2, this);
  this->Internals->Input1 = new QAudioInput(format2, this);

  qDebug() << "Buffer size: " << this->Internals->Input1->bufferSize() ;
  qDebug() << "Notify interval: " << this->Internals->Input1->notifyInterval() ;
  this->Internals->Input1->setBufferSize(256);
  this->Internals->Input1->setNotifyInterval(10);
  this->Internals->Output->setBufferSize(256);
  this->Internals->Output->setNotifyInterval(10);
  this->Internals->Input1->start(
    this->Internals->Output->start());
}

//-----------------------------------------------------------------------------
void qcMainWindow::stop()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.stopButton->setEnabled(false);
  ui.startButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  qcMainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
