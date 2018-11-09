#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <iostream>
#include "server/WebServer.h"
using namespace http;

class RestHandler : public HTTPHandler
{
public:
  RestHandler(WebServer *server)
    : m_Server(server)
  {}
  virtual ~RestHandler() = default;
  void RespondAndForward(const std::string &notes, HTTPRequest &request)
  {
    std::string msg = notes + ": " + request.m_Body.str();
    request.m_ResponseBody = "Sending " + msg + "\n";
    auto &reqMap = m_Server->m_WSHandlers["lws-minimal"]->m_Requests;
    for (auto iter = reqMap.begin(); iter != reqMap.end(); ++iter)
    {
      iter->second->SendMessage(msg);
    }
  }
  void Get(std::smatch &match, HTTPRequest &request) override
  { this->RespondAndForward("GET", request); }
  void Put(std::smatch &match, HTTPRequest &request) override
  { this->RespondAndForward("PUT", request); }
  void Post(std::smatch &match, HTTPRequest &request) override
  { this->RespondAndForward("POST", request); }
  void Delete(std::smatch &match, HTTPRequest &request) override
  { this->RespondAndForward("DELETE", request); }
  WebServer *m_Server = nullptr;
};

int main()
{
  WebServerSettings settings;
  settings.StaticFilePath = ROOT;
  settings.Port = 3367;
  WebServer server;
  server.Setup(settings);
  server.AddHandler("/test", std::make_unique<RestHandler>(&server));
  server.m_WSHandlers["lws-minimal"] = std::make_unique<WebSocketHandler>();
  server.Start();
  std::cout << "Starting server at port: " << server.GetPort() << std::endl;
  while (true)
  {
    server.Process();
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }
  return 0;
}
