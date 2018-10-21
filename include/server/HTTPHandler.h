#pragma once

#include <regex>
#include "server/HTTPRequest.h"

namespace http {
  class HTTPHandler
  {
  public:
    virtual bool Prepare(HTTPRequest &request)
    {
      return true;
    }
    virtual void Get(std::smatch &match, HTTPRequest &request)
    {}
    virtual void Post(std::smatch &match, HTTPRequest &request)
    {}
    virtual void Put(std::smatch &match, HTTPRequest &request)
    {}
    virtual void Delete(std::smatch &match, HTTPRequest &request)
    {}
  };
}
