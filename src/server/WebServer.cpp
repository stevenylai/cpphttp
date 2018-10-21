#include "server/WebServer.h"

namespace http {
  WebServer::WebServer()
  {
    m_Impl = std::make_unique<WebServerImpl>(m_RequestHandlers)
  }
  void WebServer::Setup(const WebServerSettings &settings)
  {
    m_Impl->Setup(settings);
  }
  bool WebServer::Start()
  {
    return m_Impl->Start();
  }
  void WebServer::Process()
  {
    m_Impl->Process();
  }
  void AddHandler(const std::string &pattern, std::unique_ptr<HTTPHandler> &&handler)
  {
    m_RequestHandlers.emplace_back({std::regex(pattern), std::move(handler)});
  }
}
