#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <iostream>
#include "server/WebServer.h"
using namespace http;

class SimpleHTTPHandler : public HTTPHandler
{
public:
  void Get(std::smatch &match, HTTPRequest &request) override
  { request.m_ResponseBody = request.m_Body.str(); }
  void Put(std::smatch &match, HTTPRequest &request) override
  { request.m_ResponseBody = request.m_Body.str(); }
  void Post(std::smatch &match, HTTPRequest &request) override
  { request.m_ResponseBody = request.m_Body.str(); }
  void Delete(std::smatch &match, HTTPRequest &request) override
  { request.m_ResponseBody = request.m_Body.str(); }
};

int main()
{
  WebServerSettings settings;
  settings.StaticFilePath = ROOT;
  WebServer server;
  server.Setup(settings);
  server.AddHandler("/backend/test", std::make_unique<SimpleHTTPHandler>());
  server.Start();
  std::cout << "Starting server at port: " << server.GetPort() << std::endl;
  while (true)
  {
    server.Process();
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }
  return 0;
}
