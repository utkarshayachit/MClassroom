#include "qcApp.h"

#include "qcNiceProtocol.h"
#include "qcReceiver.h"

qcDispatcher<float, 2, qcNiceProtocol> qcApp::Dispatcher;
qcBuffer<float, 3, 2> qcApp::AudioStream;
boost::shared_ptr<qcNiceProtocol> qcApp::CommunicationChannel;
boost::shared_ptr<qcReceiverBase> qcApp::Receiver;

//-----------------------------------------------------------------------------
qcApp::qcApp()
{
}

//-----------------------------------------------------------------------------
qcApp::~qcApp()
{
}

