#pragma once

#include <string>
#include <map>

namespace http {

class HTTPRequest
{
public:
  void *m_Handler = nullptr;

  bool m_Validated = true;
  std::string m_Path;
  std::string m_Method;
  std::stringstream m_Body;

  int m_ResponseStatus = 200;
  std::string m_ResponseContentType = "text/html";
  std::map<std::string, std::string> m_ResponseHeaders;
  std::string m_ResponseBody;
};

}
