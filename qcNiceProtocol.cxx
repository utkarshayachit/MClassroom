#include "qcNiceProtocol.h"

#include "qcApp.h"
#include "qcReceiver.h"

#include <QtDebug>
#include <QTimer>

extern "C"
{
#include <nice/agent.h>
}

class qcNiceProtocol::qcInternals
{
  static void cb_candidate_gathering_done(NiceAgent* agent, guint streamid, gpointer data)
    {
    qDebug() << "cb_candidate_gathering_done" << endl;
    qcNiceProtocol* self =
      reinterpret_cast<qcNiceProtocol*>(data);
    self->candidateGatheringDone(static_cast<unsigned int>(streamid));
    if (self->Internals->GLoop)
      {
      g_main_loop_quit(self->Internals->GLoop);
      }
    }

  static void cb_component_state_changed(NiceAgent *agent, guint stream_id,
    guint component_id, guint state, gpointer data)
    {
    static const gchar *state_name[] = {"disconnected", "gathering", "connecting",
      "connected", "ready", "failed"};
    printf("SIGNAL: state changed %d %d %s[%d]\n",
      stream_id, component_id, state_name[state], state);
    qcNiceProtocol* self =
      reinterpret_cast<qcNiceProtocol*>(data);
    if (state == NICE_COMPONENT_STATE_READY)
      {
      qDebug() << "Success";
      g_main_loop_quit(self->Internals->GLoop);
      }
    else if (state == NICE_COMPONENT_STATE_FAILED)
      {
      g_main_loop_quit(self->Internals->GLoop);
      qCritical() << "Failed";
      }
    }

  static void cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,
    guint len, gchar *buf, gpointer data)
    {
    if (qcApp::Receiver)
      {
      qcApp::Receiver->processDatagram(
        reinterpret_cast<const unsigned char*>(buf),
        static_cast<unsigned int>(len));
      }
    }

public:
  NiceAgent* Agent;
  guint StreamId;
  GMainLoop* GLoop;
  QTimer Timer;

  qcInternals()
    : Agent(NULL), StreamId(0), GLoop(NULL)
    {
    }

  ~qcInternals()
    {
    g_object_unref(this->Agent);
    this->Agent = NULL;

    g_main_loop_unref(this->GLoop);
    this->GLoop = NULL;
    }

  void init(const QHostAddress& host, quint16 port, qcNiceProtocol* self)
    {
    this->GLoop = g_main_loop_new(NULL, FALSE);

    this->Agent = nice_agent_new(g_main_loop_get_context(this->GLoop),
      NICE_COMPATIBILITY_RFC5245);
    Q_ASSERT(this->Agent);

    qDebug() << "STUN Server: " << host.toString() << ":" << port;

    NiceAgent* agent = this->Agent;

    // Set STUN settings (I'm skipping controlling mode too see what happens).
    g_object_set(G_OBJECT(agent),
      "stun-server", host.toString().toAscii().data(),
      "stun-server-port", static_cast<guint>(port),
      "controlling-mode", 0,
      NULL);

    // connect signals
    // candidate-gathering-done is called when nice has determined a unique URL
    // for this "candidate" or client.
    g_signal_connect (G_OBJECT (agent), "candidate-gathering-done",
      G_CALLBACK (qcNiceProtocol::qcInternals::cb_candidate_gathering_done),
      self);
    g_signal_connect (G_OBJECT (agent), "component-state-changed",
      G_CALLBACK (qcNiceProtocol::qcInternals::cb_component_state_changed),
      self);
    //g_signal_connect (G_OBJECT (agent), "new-selected-pair",
    //  G_CALLBACK (qcNiceProtocol::cb_new_selected_pair), NULL);

    // create a new stream with 1 component and start gathering candidates.
    this->StreamId = nice_agent_add_stream(this->Agent, 1);
    nice_agent_set_stream_name(this->Agent, this->StreamId, "music");

    // attach a receiver
    // without this call, candidates cannot be gathered.
    nice_agent_attach_recv(this->Agent, this->StreamId, 1,
      g_main_loop_get_context(this->GLoop), cb_nice_recv, self);

    // Start gathering local candidates
    if (!nice_agent_gather_candidates(this->Agent, this->StreamId))
      {
      qCritical("Failed to start candidate gathering");
      abort();
      }
    qDebug("waiting for candidate gathering.");
    g_main_loop_run(this->GLoop);
    qDebug("Done");
    }
};

//-----------------------------------------------------------------------------
qcNiceProtocol::qcNiceProtocol(const QHostAddress& server, quint16 port)
: Internals(new qcNiceProtocol::qcInternals())
{
  // this must be the last call.
  this->Internals->init(server, port, this);
  this->Internals->Timer.setInterval(0);
  this->Internals->Timer.setSingleShot(true);
  this->Superclass::connect(&this->Internals->Timer, SIGNAL(timeout()),
    SLOT(processEvents()));
}

//-----------------------------------------------------------------------------
qcNiceProtocol::~qcNiceProtocol()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
void qcNiceProtocol::candidateGatheringDone(unsigned int streamid)
{
  Q_ASSERT(this->Internals->Agent);

  gchar* sdp = nice_agent_generate_local_sdp(this->Internals->Agent);
  Q_ASSERT(sdp != NULL);

  qDebug() << "[" << sdp << "]";

  QString ticket(QByteArray(static_cast<char*>(sdp)).toBase64());
  g_free(sdp);

  qDebug() <<"[" << ticket << "]";

  this->Ticket = ticket;
}

//-----------------------------------------------------------------------------
bool qcNiceProtocol::connect(const QString& remote)
{
  QByteArray ticket = QByteArray::fromBase64(remote.toAscii().data());
  if (nice_agent_parse_remote_sdp(this->Internals->Agent,
      static_cast<gchar*>(ticket.data())) > 0)
    {
    g_main_loop_run(this->Internals->GLoop);
    }
  else
    {
    qCritical() << "Incorrect ticket. Try again.";
    return false;
    }
  this->Internals->Timer.start();
  return true;
}

//-----------------------------------------------------------------------------
void qcNiceProtocol::sendPacket(const unsigned char* packet, unsigned int packet_size)
{
  Q_ASSERT (this->Internals->Agent);
  guint bytesSent = nice_agent_send(this->Internals->Agent,
    this->Internals->StreamId,
    NICE_COMPONENT_TYPE_RTP,
    packet_size,
    reinterpret_cast<const gchar*>(packet));
  cout << "Send: " << bytesSent << endl;
}

//-----------------------------------------------------------------------------
void qcNiceProtocol::processEvents()
{
  if (this->Internals->GLoop)
    {
    g_main_context_iteration(g_main_loop_get_context(this->Internals->GLoop), false);
    this->Internals->Timer.start();
    }
}
