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
};
#endif
