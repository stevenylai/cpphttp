#pragma once
#include <libwebsockets.h>
#include "server/WebServer.h"
#include "server/WebSocketHandler.h"

namespace http {

class WebServerImpl
{
public:
  WebServerImpl(std::vector<URLHandler> &handler)
    : m_RequestHandlers(handler)
  {
  }
  ~WebServerImpl()
  {
    lws_context_destroy(m_pContext);
    m_pContext = nullptr;
  }
  void Setup(const WebServerSettings &settings);
  bool Start();
  void Process();
  void Stop();

  WebServerSettings m_Settings;
  std::vector<URLHandler> &m_RequestHandlers;
  struct lws_http_mount m_Dynamic;
  struct lws_http_mount m_Static;
  struct lws_context_creation_info m_ContextInfo;
  struct lws_context *m_pContext = nullptr;
  std::unordered_map<std::string, std::unique_ptr<WebSocketHandler>> m_WSHandlers;
};

}
