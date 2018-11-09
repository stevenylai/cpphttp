#include "server/WebServer.h"
#include "server/WebServerImpl.h"

namespace http {

WebServer::WebServer()
{
  m_Impl = std::make_unique<WebServerImpl>(m_RequestHandlers, m_WSHandlers);
}
WebServer::~WebServer() = default;
void WebServer::Setup(const WebServerSettings &settings)
{
  m_Impl->Setup(settings);
}
int WebServer::GetPort() const
{
  return m_Impl->m_Settings.Port;
}
bool WebServer::Start()
{
  return m_Impl->Start();
}
void WebServer::Process()
{
  m_Impl->Process();
}
void WebServer::AddHandler(const std::string &pattern, std::unique_ptr<HTTPHandler> &&handler)
{
  m_RequestHandlers.emplace_back(pattern, std::move(handler));
}

}
