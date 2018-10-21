#pragma once
#include <memory>
#include <vector>
#include "server/HTTPHandler.h"

namespace http {
  struct WebServerSettings
  {
    int Port = 0;
    std::string StaticFilePath;
    std::string DefaultFile = "index.htm";
    std::string DynamicURL = "/backend";
  };

  struct URLHandler
  {
    std::regex Pattern;
    std::unique_ptr<HTTPHandler> HandlerMatcher;
  };

  struct WebServerImpl;

  class WebServer
  {
  public:
    WebServer();
    ~WebServer() = default;
    void Setup(const WebServerSettings &settings);
    void AddHandler(const std::string &pattern, std::unique_ptr<HTTPHandler> &&handler);
    bool Start();
    void Process();
  protected:
    std::vector<URLHandler> m_RequestHandlers;
    std::unique_ptr<WebServerImpl> m_Impl;
  };
}
