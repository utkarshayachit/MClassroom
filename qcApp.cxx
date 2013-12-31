#include "qcApp.h"

qcDispatcher<float, 2, qcQUdpNetworkProtocolSend> qcApp::Dispatcher;
qcBuffer<float, 3, 2> qcApp::AudioStream;

//-----------------------------------------------------------------------------
qcApp::qcApp()
{
}

//-----------------------------------------------------------------------------
qcApp::~qcApp()
{
}

