#pragma once
#include <memory>
#include <vector>
#include "server/HTTPHandler.h"
#include "server/WebSocketHandler.h"

namespace http {

struct WebServerSettings
{
  int Port = 0;
  std::string StaticFilePath;
  std::string DefaultFile = "index.html";
  std::string DynamicURL = "/backend";
};

struct URLHandler
{
  std::regex Pattern;
  std::unique_ptr<HTTPHandler> Handler;
  URLHandler(const std::string &pattern, std::unique_ptr<HTTPHandler> &&handler)
  {
    this->Pattern.assign(pattern);
    this->Handler = std::move(handler);
  }
};

struct WebServerImpl;

class WebServer
{
public:
  WebServer();
  ~WebServer();
  void Setup(const WebServerSettings &settings);
  void AddHandler(const std::string &pattern, std::unique_ptr<HTTPHandler> &&handler);
  bool Start();
  int GetPort() const;
  void Process();

  std::vector<URLHandler> m_RequestHandlers;
  std::unordered_map<std::string, std::unique_ptr<WebSocketHandler>> m_WSHandlers;
  std::unique_ptr<WebServerImpl> m_Impl;
};

}
