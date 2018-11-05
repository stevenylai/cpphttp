#pragma once

#include <unordered_map>
#include "server/WebSocketRequest.h"

namespace http {

class WebSocketHandler
{
public:
  WebSocketHandler() = default;
  virtual ~WebSocketHandler() = default;
  virtual std::shared_ptr<WebSocketRequest> OnNewRequest(WebSocketHandler *);

  std::string m_Protocol;
  std::unordered_map<void *, std::shared_ptr<WebSocketRequest>> m_Requests;

};

}
