#ifndef __qcApp_h
#define __qcApp_h

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "qcDispatcher.h"

// Storage for application globals.
class qcApp
{
public:
  qcApp();
  virtual ~qcApp();

  static qcDispatcher<float, 2> Dispatcher;
};
#endif
