#include "qcApp.h"

#include "qcQUdpNetworkProtocol.h"
qcDispatcher<float, 2, qcQUdpNetworkProtocolSend> qcApp::Dispatcher;
qcBuffer<float, 3, 2> qcApp::AudioStream;
boost::shared_ptr<qcQUdpNetworkProtocolReceive> qcApp::Receiver;

//-----------------------------------------------------------------------------
qcApp::qcApp()
{
}

//-----------------------------------------------------------------------------
qcApp::~qcApp()
{
}

