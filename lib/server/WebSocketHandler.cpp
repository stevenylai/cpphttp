#include "server/WebSocketHandler.h"

namespace http {

std::shared_ptr<WebSocketRequest> WebSocketHandler::OnNewRequest(WebSocketHandler *handler)
{
  return std::make_shared<WebSocketRequest>(handler);
}

}
