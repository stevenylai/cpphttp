#pragma once

#include <string>
#include <map>
#include <vector>

namespace http {

class HTTPRequest
{
public:
  void *m_Handler = nullptr;

  bool m_Validated = false;
  std::string m_Path;
  std::string m_Method;
  std::stringstream m_Body;

  int m_ResponseStatus = 200;
  std::string m_ResponseContentType = "text/html";
  std::map<std::string, std::string> m_ResponseHeaders;
  std::string m_ResponseBody;
  std::vector<char> _m_ResponseBodyRaw;
  size_t _m_ResponseProgress = 0;
};

}
