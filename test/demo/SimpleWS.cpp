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
  return 0;
}
