#pragma once

#include <deque>
#include <sstream>
#include <memory>

namespace http {

class WebSocketRequest
{
public:
  WebSocketRequest(void *handler)
    : m_Handler(handler), m_wsi(nullptr)
  {}
  virtual ~WebSocketRequest() = default;
  virtual int OnReceiveMessage(const std::string &msg);
  virtual int Process();
  virtual void SendMessage(const std::string &msg);
  virtual void Close();
  void *m_Handler;
  void *m_wsi;
  std::deque<char> _m_PendingSend;
  std::stringstream _m_Received;
};

}
