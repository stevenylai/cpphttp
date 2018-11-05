#include <libwebsockets.h>
#include "server/WebSocketRequest.h"

namespace http {

int WebSocketRequest::OnReceiveMessage(const std::string &msg)
{
  return 0;
}
int WebSocketRequest::Process()
{
  return 0;
}

void WebSocketRequest::SendMessage(const std::string &msg)
{
  struct lws *wsi = static_cast<struct lws *>(this->m_wsi);
  for (char c : msg)
  {
    _m_PendingSend.push_back(c);
  }
  lws_callback_on_writable(wsi);
}

void WebSocketRequest::Close()
{
  struct lws *wsi = static_cast<struct lws *>(this->m_wsi);
  lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
}

}
