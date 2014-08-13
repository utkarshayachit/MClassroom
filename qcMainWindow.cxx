#include "qcMainWindow.h"
#include "ui_qcMainWindow.h"
#include "ui_qcNiceConnectionDialog.h"

#include "RtAudio.h"
#include <map>
#include <vector>


#include <QApplication>
#include <QList>
#include <QtDebug>
#include <QPointer>
#include <QInputDialog>
#include <QMessageBox>

#include "qcApp.h"
#include "qcBuffer.h"
#include "qcNiceProtocol.h"
#include "qcReceiver.h"

class qcMainWindow::qcInternals
{
public:
  Ui::QCMainWindow Ui;

  qcInternals(qcMainWindow* self)
    {
    this->Ui.setupUi(self);
    }
  ~qcInternals()
    {
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

  RtAudio::StreamParameters getStreamParameters(
    QComboBox* device, QComboBox* format, QComboBox* rate)
    {
    RtAudio::StreamParameters params;
    params.deviceId = device->itemData(device->currentIndex()).toInt();
    params.nChannels = 2;
    params.firstChannel = 0;
    return params;
    }

  enum
    {
    INPUT1 = 0,
    INPUT2 = 1,
    OUTPUT = 2
    };

  RtAudio Audio[3];

  void startAudio()
    {
    unsigned long format = RTAUDIO_FLOAT32;
    unsigned int sampleRate = qcApp::SampleRate;

    RtAudio::StreamOptions options;
   // options.flags = RTAUDIO_MINIMIZE_LATENCY;

    Ui::QCMainWindow& ui = this->Ui;

    RtAudio::StreamParameters params[3];
    params[INPUT1] = this->getStreamParameters(
      ui.audioInput1, ui.formatInput1, ui.rateInput1);
    params[INPUT2] = this->getStreamParameters(
      ui.audioInput2, ui.formatInput2, ui.rateInput2);
    params[OUTPUT] = this->getStreamParameters(
      ui.audioOutput, ui.formatOutput, ui.rateOutput);

    for (int cc=0; cc <= OUTPUT; cc++)
      {
      if (params[cc].deviceId != -1)
        {
        // indicate how many frames to buffer. Too low a rate results in jitter
        // sounds since mutex overheads take over.
        unsigned int bufferFrames = 120;
        if (cc == OUTPUT)
          {
          this->Audio[cc].openStream(&params[cc], NULL, format, sampleRate,
            &bufferFrames, qcInternals::streamCallbackOutput, this, &options);
          }
        else if (cc == INPUT1)
          {
          this->Audio[cc].openStream(NULL, &params[cc], format, sampleRate,
            &bufferFrames, qcInternals::streamCallbackInput1, this, &options);
          }
        else if (cc == INPUT2)
          {
          this->Audio[cc].openStream(NULL, &params[cc], format, sampleRate,
            &bufferFrames, qcInternals::streamCallbackInput2, this, &options);
          }
        this->Audio[cc].startStream();
        cout << "Using..." << endl;
        cout << "  Latency: " << this->Audio[cc].getStreamLatency() << endl;
        cout << "  Buffer Frames: " << bufferFrames << endl;
        }
      }
    }

  void stopAudio()
    {
    for (int cc=0; cc <= OUTPUT;cc++)
      {
      if (this->Audio[cc].isStreamOpen())
        {
        this->Audio[cc].closeStream();
        }
      }
    }

  static int streamCallbackInput1(void *outputBuffer, void *inputBuffer,
    unsigned int nFrames,
    double streamTime,
    RtAudioStreamStatus status, void *userData)
    {
    //cout << "Input: " << streamTime << ": " << nFrames << endl;
    qcApp::AudioStream.push(0, reinterpret_cast<float*>(inputBuffer), nFrames);
    return 0;
    }

  static int streamCallbackInput2(void *outputBuffer, void *inputBuffer,
    unsigned int nFrames,
    double streamTime,
    RtAudioStreamStatus status, void *userData)
    {
    //cout << "Input: " << streamTime << ": " << nFrames << endl;
    qcApp::AudioStream.push(1, reinterpret_cast<float*>(inputBuffer), nFrames);
    return 0;
    }

  static int streamCallbackOutput(void *outputBuffer, void *inputBuffer,
    unsigned int nFrames,
    double streamTime,
    RtAudioStreamStatus status, void *userData)
    {
    //cout << "Output: " << streamTime << ": " << nFrames << endl;
    qcApp::AudioStream.pop(reinterpret_cast<float*>(outputBuffer), nFrames);

    // also push the data to the dispatcher.
    qcApp::Dispatcher.pushRawData(reinterpret_cast<float*>(outputBuffer), nFrames);
    return 0;
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
  this->connect(ui.actionConnect, SIGNAL(triggered()), SLOT(connectToPeer()));
  this->connect(ui.actionDisconnect, SIGNAL(triggered()), SLOT(disconnectFromPeer()));

  ui.gainInput1->setProperty("Index", QVariant(0));
  ui.gainInput2->setProperty("Index", QVariant(1));
  ui.gainRemote->setProperty("Index", QVariant(2));

  this->connect(ui.gainInput1, SIGNAL(valueChanged(int)), SLOT(gainChanged(int)));
  this->connect(ui.gainInput2, SIGNAL(valueChanged(int)), SLOT(gainChanged(int)));
  this->connect(ui.gainRemote, SIGNAL(valueChanged(int)), SLOT(gainChanged(int)));
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
void qcMainWindow::gainChanged(int gain)
{
  QDial* sender = qobject_cast<QDial*>(this->sender());
  if (sender && sender->property("Index").isValid())
    {
    qcApp::AudioStream.setGain(
      sender->property("Index").toInt(), gain/100.0);
    }
}

//-----------------------------------------------------------------------------
void qcMainWindow::start()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.stopButton->setEnabled(true);
  ui.startButton->setEnabled(false);
  ui.audioInput1->setEnabled(false);
  ui.audioInput2->setEnabled(false);
  ui.audioOutput->setEnabled(false);
  ui.formatInput1->setEnabled(false);
  ui.formatInput2->setEnabled(false);
  ui.formatOutput->setEnabled(false);
  ui.rateInput1->setEnabled(false);
  ui.rateInput2->setEnabled(false);
  ui.rateOutput->setEnabled(false);

  this->Internals->startAudio();
}

//-----------------------------------------------------------------------------
void qcMainWindow::stop()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.stopButton->setEnabled(false);
  ui.startButton->setEnabled(true);
  ui.audioInput1->setEnabled(true);
  ui.audioInput2->setEnabled(true);
  ui.audioOutput->setEnabled(true);
  ui.formatInput1->setEnabled(true);
  ui.formatInput2->setEnabled(true);
  ui.formatOutput->setEnabled(true);
  ui.rateInput1->setEnabled(true);
  ui.rateInput2->setEnabled(true);
  ui.rateOutput->setEnabled(true);
  this->Internals->stopAudio();
}

//-----------------------------------------------------------------------------
void qcMainWindow::connectToPeer()
{
  QMessageBox information(
    QMessageBox::Information,
    "Connecting ...",
    "We are gathering information about your connection\n"
    "please be patient.",
    QMessageBox::NoButton,
    this);
  information.show();
  qcApp::CommunicationChannel.reset(new qcNiceProtocol("107.23.150.92"));
  qcApp::Receiver.reset();
  information.hide();

  QDialog dialog(this);
  Ui::NiceConnectionDialog ui;
  ui.setupUi(&dialog);
  ui.selfID->setPlainText(qcApp::CommunicationChannel->ticket());
  if (dialog.exec() == QDialog::Accepted)
    {
    information.setText("We will now try to connect using the key you entered. \n"
      "This may take a while. Please be patient.");
    information.show();
    if (qcApp::CommunicationChannel->connect(ui.peerID->toPlainText()) == true)
      {
      this->Internals->Ui.actionDisconnect->setEnabled(true);
      this->Internals->Ui.actionConnect->setEnabled(false);
      this->Internals->Ui.gainRemote->setEnabled(true);
      this->Internals->Ui.audioRemote->addItem("Peer");

      qcApp::Dispatcher.setProtocol(qcApp::CommunicationChannel);
      qcApp::Receiver.reset(new qcReceiver<float, 2>());
      return;
      }
    }
  information.hide();
  qcApp::CommunicationChannel.reset();
  QMessageBox::critical(this, "Connection Failed!!!",
    "Alas! The connection failed for unknown reasons.\n"
    "Please try again. Ensure that your 'connection identifier' is correct.");
}

//-----------------------------------------------------------------------------
void qcMainWindow::disconnectFromPeer()
{
  Ui::QCMainWindow &ui = this->Internals->Ui;
  ui.actionDisconnect->setEnabled(false);
  ui.actionConnect->setEnabled(true);
  ui.gainRemote->setEnabled(false);
  ui.audioRemote->clear();

  qcApp::CommunicationChannel.reset();
  qcApp::Receiver.reset();
  qcApp::Dispatcher.setProtocol(qcApp::CommunicationChannel);
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  qcApp qcapp;

  qcMainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
