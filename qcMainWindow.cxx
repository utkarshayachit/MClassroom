#include "qcMainWindow.h"
#include "ui_qcMainWindow.h"

#include "RtAudio.h"
#include <iostream>
#include <map>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;

#include <QApplication>
#include <QList>
#include <QtDebug>
#include <QPointer>

class qcMainWindow::qcInternals
{
public:
  Ui::QCMainWindow Ui;
  RtAudio Input1;
  RtAudio Input2;
  RtAudio Output;

  qcInternals(qcMainWindow* self)
    {
    this->Ui.setupUi(self);
    }

  void updateUi(RtAudio::DeviceInfo& info, QComboBox* format, QComboBox* rate)
    {
    format->clear();
    rate->clear();
    if (info.nativeFormats & RTAUDIO_SINT8)
      {
      format->addItem("8-bit int", static_cast<int>(RTAUDIO_SINT8));
      }
    if (info.nativeFormats & RTAUDIO_SINT16)
      {
      format->addItem("16-bit int", (int)RTAUDIO_SINT16);
      }
    if (info.nativeFormats & RTAUDIO_SINT24)
      {
      format->addItem("24-bit int", (int)RTAUDIO_SINT24);
      }
    if (info.nativeFormats & RTAUDIO_SINT32)
      {
      format->addItem("32-bit int", (int)RTAUDIO_SINT32);
      }
    if (info.nativeFormats & RTAUDIO_FLOAT32)
      {
      format->addItem("32-bit float",(int) RTAUDIO_FLOAT32);
      }
    if (info.nativeFormats & RTAUDIO_FLOAT64)
      {
      format->addItem("64-bit float", (int)RTAUDIO_FLOAT64);
      }

    for (size_t cc=0; cc < info.sampleRates.size(); cc++)
      {
      rate->addItem(QString::number(info.sampleRates[cc]), info.sampleRates[cc]);
      }
    }
};

//-----------------------------------------------------------------------------
qcMainWindow::qcMainWindow() : Internals(new qcInternals(this))
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  QObject::connect(ui.audioInput1, SIGNAL(currentIndexChanged(int)),
    this, SLOT(audioInput1Changed(int)));
  QObject::connect(ui.audioInput2, SIGNAL(currentIndexChanged(int)),
    this, SLOT(audioInput2Changed(int)));
  QObject::connect(ui.audioOutput, SIGNAL(currentIndexChanged(int)),
    this, SLOT(audioOutputChanged(int)));

  // setup information about available devices.
  this->queryAvailableDevices();

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

  // Create an api map.
  std::map<int, std::string> apiMap;
  apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
  apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
  apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
  apiMap[RtAudio::UNIX_JACK] = "Jack Client";
  apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
  apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
  apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
  apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

  // Query RtAudio for devices information using the current API.
  cout << "-----------------------------------------------------" << endl;
  cout << " RtAudio Version: " << RtAudio::getVersion() << endl;
  cout << " Available APIs: " << endl;
  std::vector<RtAudio::Api> apis;
  RtAudio::getCompiledApi(apis);
  for (size_t cc=0; cc < apis.size(); cc++)
    {
    cout << (cc==0? " * " : "   ") << apiMap[apis[cc]] << endl;
    }

  ui.audioInput1->addItem("None", -1);
  ui.audioInput2->addItem("None", -1);
  ui.audioOutput->addItem("None", -1);

  RtAudio audio ;
  // Let RtAudio print messages to stderr.
  audio.showWarnings(true);

  unsigned int num_devices = audio.getDeviceCount();
  for (unsigned int cc=0; cc < num_devices; cc++)
    {
    RtAudio::DeviceInfo info = audio.getDeviceInfo(cc);
    if (!info.probed)
      {
      cout << "Failed to probe device: " << info.name << endl;
      continue;
      }
    if (info.inputChannels > 0)
      {
      ui.audioInput1->addItem(info.name.c_str(), cc);
      ui.audioInput2->addItem(info.name.c_str(), cc);
      }
    if (info.outputChannels > 0)
      {
      ui.audioOutput->addItem(info.name.c_str(), cc);
      }
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::audioInput1Changed(int index)
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.formatInput1->setEnabled(index > 0);
  ui.rateInput1->setEnabled(index > 0);
  if (index > 0)
    {
    RtAudio audio;
    RtAudio::DeviceInfo info = audio.getDeviceInfo(
      ui.audioInput1->itemData(index).value<unsigned int>());
    this->Internals->updateUi(info, ui.formatInput1,  ui.rateInput1);
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::audioInput2Changed(int index)
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.formatInput2->setEnabled(index > 0);
  ui.rateInput2->setEnabled(index > 0);
  if (index > 0)
    {
    RtAudio audio;
    RtAudio::DeviceInfo info = audio.getDeviceInfo(
      ui.audioInput2->itemData(index).value<unsigned int>());
    this->Internals->updateUi(info, ui.formatInput2,  ui.rateInput2);
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::audioOutputChanged(int index)
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.formatOutput->setEnabled(index > 0);
  ui.rateOutput->setEnabled(index > 0);

  if (index > 0)
    {
    RtAudio audio;
    RtAudio::DeviceInfo info = audio.getDeviceInfo(
      ui.audioOutput->itemData(index).value<unsigned int>());
    this->Internals->updateUi(info, ui.formatOutput,  ui.rateOutput);
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::start()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.stopButton->setEnabled(true);
  ui.startButton->setEnabled(false);
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
