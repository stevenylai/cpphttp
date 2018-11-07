#include "server/WebServerImpl.h"

namespace http {


/* one of these is created for each client connecting to us */

struct per_session_data
{
  WebSocketRequest *request;
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data
{
  struct lws_context *context;
  struct lws_vhost *vhost;
  const struct lws_protocols *protocol;
  WebSocketHandler *handler;
};

static int
callback_ws(struct lws *wsi, enum lws_callback_reasons reason,
            void *user, void *in, size_t len)
{
  auto *server = reinterpret_cast<WebServerImpl *>(lws_context_user(lws_get_context(wsi)));
  struct per_session_data *pss = (struct per_session_data *)user;
  struct per_vhost_data *vhd =
    (struct per_vhost_data *) lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                                       lws_get_protocol(wsi));

	switch (reason) {
      case LWS_CALLBACK_PROTOCOL_INIT:
      {
		vhd = (struct per_vhost_data *)lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                                                   lws_get_protocol(wsi),
                                                                   sizeof(struct per_vhost_data));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
        vhd->handler = server->m_WSHandlers[vhd->protocol->name].get();
		break;
      }
      case LWS_CALLBACK_PROTOCOL_DESTROY:
      {
        vhd->handler->m_Requests.clear();
        break;
      }
      case LWS_CALLBACK_ESTABLISHED:
      {
        auto req = vhd->handler->OnNewRequest(vhd->handler);
        req->m_wsi = wsi;
        pss->request = req.get();
        vhd->handler->m_Requests[req.get()] = std::move(req);
		break;
      }
      case LWS_CALLBACK_CLOSED:
      {
        vhd->handler->m_Requests.erase(pss->request);
		break;
      }
      case LWS_CALLBACK_SERVER_WRITEABLE:
      {
        if (pss->request->_m_PendingSend.size())
        {
          std::vector<char> buf;
          for (unsigned i = 0; i < LWS_PRE; ++i)
          {
            buf.emplace_back(' ');
          }
          unsigned i = 0;
          for (; i < pss->request->_m_PendingSend.size() && i < 2048; ++i)
          {
            buf.emplace_back(pss->request->_m_PendingSend[i]);
          }
          int written = lws_write(wsi, reinterpret_cast<uint8_t *>(&buf[LWS_PRE]), i, LWS_WRITE_TEXT);
          if (written < 0)
          {
            return 1;
          }
          else
          {
            for (int i = 0; i < written; ++i)
            {
              pss->request->_m_PendingSend.pop_front();
            }
          }
        }
        if (pss->request->_m_PendingSend.size())
        {
          lws_callback_on_writable(wsi);
        }
		break;
      }
      case LWS_CALLBACK_RECEIVE:
      {
        std::string data{static_cast<char *>(in), len};
        pss->request->_m_Received << data;
        if (lws_is_final_fragment(wsi))
        {
          std::string msg = pss->request->_m_Received.str();
          pss->request->_m_Received.str("");
          pss->request->OnReceiveMessage(msg);
        }
        break;
      }
      default:
		break;
	}

	return 0;
}

void WebServerImpl::AddWebSocketProtocols()
{
  for (auto iter = m_WSHandlers.begin(); iter != m_WSHandlers.end(); ++iter)
  {
    m_Protocols.emplace_back();
    m_Protocols.back().name = iter->first.c_str();
    m_Protocols.back().callback = callback_ws;
    m_Protocols.back().per_session_data_size = sizeof(struct per_session_data);
    m_Protocols.back().rx_buffer_size = 128;
    m_Protocols.back().id = 0;
    m_Protocols.back().user = nullptr;
    m_Protocols.back().tx_packet_size = 0;
  }
}

}
