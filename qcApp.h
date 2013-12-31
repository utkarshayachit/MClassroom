#ifndef __qcApp_h
#define __qcApp_h

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "qcDispatcher.h"
#include "qcBuffer.h"
#include "qcQUdpNetworkProtocol.h"

// Storage for application globals.
class qcApp
{
public:
  qcApp();
  virtual ~qcApp();

  static qcDispatcher<float, 2, qcQUdpNetworkProtocolSend> Dispatcher;
  static qcBuffer<float, 3, 2> AudioStream;
};
#endif
