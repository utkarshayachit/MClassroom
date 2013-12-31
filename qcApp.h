#ifndef __qcApp_h
#define __qcApp_h
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <boost/shared_ptr.hpp>
#include "qcDispatcher.h"
#include "qcBuffer.h"

class qcQUdpNetworkProtocolSend;
class qcQUdpNetworkProtocolReceive;

// Storage for application globals.
class qcApp
{
public:
  qcApp();
  virtual ~qcApp();

  static qcDispatcher<float, 2, qcQUdpNetworkProtocolSend> Dispatcher;
  static boost::shared_ptr<qcQUdpNetworkProtocolReceive> Receiver;
  static qcBuffer<float, 3, 2> AudioStream;
};
#endif
