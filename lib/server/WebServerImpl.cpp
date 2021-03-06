#include <iostream>
#include "server/WebServerImpl.h"
#include "server/HTTPRequest.h"

namespace http {

void WebServerImpl::Setup(const WebServerSettings &settings)
{
  m_Settings = settings;
  m_Dynamic = {
    /* .mount_next */		NULL,		/* linked-list "next" */
    /* .mountpoint */		m_Settings.DynamicURL.c_str(), /* mountpoint URL */
    /* .origin */			NULL,	/* protocol */
    /* .def */			NULL,
    /* .protocol */			"http",
    /* .cgienv */			NULL,
    /* .extra_mimetypes */		NULL,
    /* .interpret */		NULL,
    /* .cgi_timeout */		0,
    /* .cache_max_age */		0,
    /* .auth_mask */		0,
    /* .cache_reusable */		0,
    /* .cache_revalidate */		0,
    /* .cache_intermediaries */	0,
    /* .origin_protocol */		LWSMPRO_CALLBACK, /* dynamic */
    /* .mountpoint_len */		static_cast<unsigned char>(m_Settings.DynamicURL.length()),		/* char count */
    /* .basic_auth_login_file */	NULL,
  };
  m_Static = {
    /* .mount_next */	&m_Dynamic,		/* linked-list "next" */
    /* .mountpoint */		"/",		/* mountpoint URL */
    /* .origin */		m_Settings.StaticFilePath.c_str(),	/* serve from dir */
    /* .def */			m_Settings.DefaultFile.c_str(),	/* default filename */
    /* .protocol */			NULL,
    /* .cgienv */			NULL,
    /* .extra_mimetypes */		NULL,
    /* .interpret */		NULL,
    /* .cgi_timeout */		0,
    /* .cache_max_age */		0,
    /* .auth_mask */		0,
    /* .cache_reusable */		0,
    /* .cache_revalidate */		0,
    /* .cache_intermediaries */	0,
    /* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
    /* .mountpoint_len */		1,		/* char count */
    /* .basic_auth_login_file */	NULL,
  };
}

struct pss
{
  HTTPRequest *request;
};

static int
callback_http(struct lws *wsi, enum lws_callback_reasons reason,
              void *user, void *in, size_t len)
{
  auto *server = reinterpret_cast<WebServerImpl *>(lws_context_user(lws_get_context(wsi)));
  struct pss *pss = reinterpret_cast<struct pss *>(user);

  switch (reason)
  {
    case LWS_CALLBACK_HTTP:
    {
      /* in contains the url part after our mountpoint /dyn, if any */
      if (pss->request)
      {
        free(pss->request);
        pss->request = nullptr;
      }
      pss->request = new HTTPRequest();
      pss->request->m_Handler = wsi;
      pss->request->m_Path = reinterpret_cast<const char *>(in);
      if (lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI))
      {
        pss->request->m_Method = "get";
      }
      else if (lws_hdr_total_length(wsi, WSI_TOKEN_PUT_URI))
      {
        pss->request->m_Method = "put";
      }
      else if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
      {
        pss->request->m_Method = "post";
      }
      else if (lws_hdr_total_length(wsi, WSI_TOKEN_DELETE_URI))
      {
        pss->request->m_Method = "delete";
      }
      int bodyLen = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH);
      if (bodyLen)
      {
        std::vector<char> buff(bodyLen + 1);
        lws_hdr_copy(wsi, &(buff[0]), bodyLen + 1, WSI_TOKEN_HTTP_CONTENT_LENGTH);
        try
        {
          pss->request->m_BodyLen = std::stoi(std::string(&(buff[0])));
        }
        catch (const std::invalid_argument &e)
        {
        }

      }
      for (auto &item : server->m_RequestHandlers)
      {
        std::smatch match;
        if (std::regex_search(pss->request->m_Path, match, item.Pattern))
        {
          if ((pss->request->m_Validated = item.Handler->Prepare(match, *(pss->request))))
          {
            if (pss->request->m_BodyLen == 0)
            {
              server->InvokeREST(*(item.Handler), match, *(pss->request));
            }
            break;
          }
        }
      }
      if (!pss->request->m_Validated)
      {
        delete pss->request;
        pss->request = nullptr;
      }
      else if (pss->request->m_BodyLen == 0)
      {
        return server->PrepareResponseHeader(wsi, *(pss->request));
      }
      break;
    }
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    {
      break;
    }
    case LWS_CALLBACK_HTTP_BODY:
    {
      if (pss && pss->request)
      {
        pss->request->m_Handler = wsi;
        pss->request->m_Body << std::string(reinterpret_cast<const char *>(in), len);
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
    {
      if (pss && pss->request)
      {
        pss->request->m_Handler = wsi;
        pss->request->m_Body << std::string(reinterpret_cast<const char *>(in), len);
        if (pss->request->m_Method.length())
        {
          for (auto &item : server->m_RequestHandlers)
          {
            std::smatch match;
            if (std::regex_search(pss->request->m_Path, match, item.Pattern))
            {
              server->InvokeREST(*(item.Handler), match, *(pss->request));
              break;
            }
          }
          return server->PrepareResponseHeader(wsi, *(pss->request));
        }
      }
      break;
    }
    case LWS_CALLBACK_HTTP_WRITEABLE:
    {
      if (pss && pss->request)
      {
        pss->request->_m_ResponseBodyRaw.clear();
        for (unsigned i = 0; i < LWS_PRE; ++i)
        {
          pss->request->_m_ResponseBodyRaw.emplace_back(' ');
        }
        unsigned i = 0;
        for (; pss->request->_m_ResponseProgress + i < pss->request->m_ResponseBody.size() && i < 2048; ++i)
        {
          pss->request->_m_ResponseBodyRaw.emplace_back(pss->request->m_ResponseBody[pss->request->_m_ResponseProgress + i]);
        }
        enum lws_write_protocol n = LWS_WRITE_HTTP;
        if (pss->request->_m_ResponseProgress + i >= pss->request->m_ResponseBody.size())
        {
          n = LWS_WRITE_HTTP_FINAL;
        }
        int written = lws_write(wsi, reinterpret_cast<uint8_t *>(&pss->request->_m_ResponseBodyRaw[LWS_PRE]), i, n);
        pss->request->_m_ResponseProgress += written;
        /*
         * HTTP/1.0 no keepalive: close network connection
         * HTTP/1.1 or HTTP1.0 + KA: wait / process next transaction
         * HTTP/2: stream ended, parent connection remains up
         */
        if (n == LWS_WRITE_HTTP_FINAL && pss->request->_m_ResponseProgress >= pss->request->m_ResponseBody.size())
        {
          if (lws_http_transaction_completed(wsi))
          {
            return -1;
          }
        }
        else
        {
          lws_callback_on_writable(wsi);
        }
      }
      break;
    }
    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
    {

      if (pss && pss->request)
      {
        delete pss->request;
        pss->request = nullptr;
      }
      break;
    }
    default:
      break;
  }
  if (pss && pss->request)
  {
    return 0;
  }
  else
  {
    return lws_callback_http_dummy(wsi, reason, user, in, len);
  }
}

void WebServerImpl::InvokeREST(HTTPHandler &handler, std::smatch &match, HTTPRequest &req)
{
  if (req.m_Method == "get")
  {
    handler.Get(match, req);
  }
  else if (req.m_Method == "put")
  {
    handler.Put(match, req);
  }
  else if (req.m_Method == "post")
  {
    handler.Post(match, req);
  }
  else if (req.m_Method == "delete")
  {
    handler.Delete(match, req);
  }
}

int WebServerImpl::PrepareResponseHeader(struct lws *wsi, HTTPRequest &req)
{
  /*
   * prepare and write http headers... with regards to content-
   * length, there are three approaches:
   *
   *  - http/1.0 or connection:close: no need, but no pipelining
   *  - http/1.1 or connected:keep-alive
   *     (keep-alive is default for 1.1): content-length required
   *  - http/2: no need, LWS_WRITE_HTTP_FINAL closes the stream
   *
   * giving the api below LWS_ILLEGAL_HTTP_CONTENT_LEN instead of
   * a content length forces the connection response headers to
   * send back "connection: close", disabling keep-alive.
   *
   * If you know the final content-length, it's always OK to give
   * it and keep-alive can work then if otherwise possible.  But
   * often you don't know it and avoiding having to compute it
   * at header-time makes life easier at the server.
   */
  uint8_t buf[LWS_PRE + 2048], *start = &buf[LWS_PRE], *p = start,
    *end = &buf[sizeof(buf) - LWS_PRE - 1];
  if (lws_add_http_common_headers(wsi, req.m_ResponseStatus,
                                  req.m_ResponseContentType.c_str(),
                                  req.m_ResponseBody.length(),
                                  &p, end))
  {
    return 1;
  }
  if (lws_finalize_write_http_header(wsi, start, &p, end))
  {
    return 1;
  }
  /* write the body separately */
  lws_callback_on_writable(wsi);
  return 0;
}

bool WebServerImpl::Start()
{
  int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
    /* for LLL_ verbosity above NOTICE to be built into lws,
     * lws must have been configured and built with
     * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
    /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
    /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
    /* | LLL_DEBUG */;
  lws_set_log_level(logs, NULL);

  m_Protocols.emplace_back();
  m_Protocols.back().name = "http";
  m_Protocols.back().callback = callback_http;
  m_Protocols.back().per_session_data_size = sizeof(struct pss);
  m_Protocols.back().rx_buffer_size = 0;

  this->AddWebSocketProtocols();

  m_Protocols.emplace_back();
  m_Protocols.back().name = nullptr;
  m_Protocols.back().callback = nullptr;
  m_Protocols.back().per_session_data_size = 0;
  m_Protocols.back().rx_buffer_size = 0;

  std::memset(&m_ContextInfo, 0, sizeof(m_ContextInfo));
  m_ContextInfo.port = m_Settings.Port;
  m_ContextInfo.protocols = reinterpret_cast<struct lws_protocols *>(&m_Protocols[0]);
  m_ContextInfo.mounts = &m_Static;
  m_ContextInfo.user = this;
  m_ContextInfo.ws_ping_pong_interval = 10;

  m_pContext = lws_create_context(&m_ContextInfo);
  if (!m_pContext)
  {
    return false;
  }
  if (m_Settings.Port == 0)
  {
    m_Settings.Port = lws_get_vhost_port(lws_create_vhost(m_pContext, &m_ContextInfo));
  }
  return true;

}
void WebServerImpl::Process()
{
  lws_service(m_pContext, 1000);
}

void WebServerImpl::Stop()
{
  if (m_pContext)
  {
    lws_context_destroy(m_pContext);
    m_pContext = nullptr;
  }
}

}
