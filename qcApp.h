#ifndef __qcApp_h
#define __qcApp_h
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <boost/shared_ptr.hpp>
#include "qcDispatcher.h"

class qcNiceProtocol;
class qcReceiverBase;

// Storage for application globals.
class qcApp
{
public:
  qcApp();
  virtual ~qcApp();

  static qcBuffer<float, 3, 2> AudioStream;

  static boost::shared_ptr<qcNiceProtocol> CommunicationChannel;
  static qcDispatcher<float, 2, qcNiceProtocol> Dispatcher;
  static boost::shared_ptr<qcReceiverBase> Receiver;

  // Opus only supports these sample rates: 8000, 12000, 16000, 24000, or 48000
  static const int SampleRate = 48000;
};
#endif
