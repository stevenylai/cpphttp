#include "server/WebServerImpl.h"
#include "server/HTTPRequest.h"

namespace http {

void WebServerImpl::Setup(const WebServerSettings &settings)
{
  m_Settings = settings;
  m_Dynamic = {
    /* .mount_next */		NULL,		/* linked-list "next" */
    /* .mountpoint */		m_Settings.StaticFilePath.c_str(), /* mountpoint URL */
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
    /* .mountpoint_len */		static_cast<unsigned char>(m_Settings.StaticFilePath.length()),		/* char count */
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
      for (auto &item : server->m_RequestHandlers)
      {
        std::smatch match;
        if (std::regex_search(pss->request->m_Path, match, item.Pattern))
        {
          pss->request->m_Validated = item.Handler->Prepare(match, *(pss->request));
        }
      }
      break;
    }
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
      if (!pss->request->m_Validated || !pss->request->m_Method.length())
      {
        return -1;
      }
      break;
    case LWS_CALLBACK_HTTP_BODY:
    {
      pss->request->m_Handler = wsi;
      pss->request->m_Body << reinterpret_cast<const char *>(in);
      break;
    }
    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
    {
      pss->request->m_Handler = wsi;
      pss->request->m_Body << reinterpret_cast<const char *>(in);
      if (pss->request->m_Method.length())
      {
        for (auto &item : server->m_RequestHandlers)
        {
          std::smatch match;
          if (std::regex_search(pss->request->m_Path, match, item.Pattern))
          {
            if (pss->request->m_Method == "get")
            {
              item.Handler->Get(match, *(pss->request));
            }
            else if (pss->request->m_Method == "put")
            {
              item.Handler->Put(match, *(pss->request));
            }
            else if (pss->request->m_Method == "post")
            {
              item.Handler->Post(match, *(pss->request));
            }
            else if (pss->request->m_Method == "delete")
            {
              item.Handler->Delete(match, *(pss->request));
            }
          }
        }
        // Write response back
      }
      break;
    }
    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
      delete pss->request;
      pss->request = nullptr;
      break;
    default:
      break;
  }
  return lws_callback_http_dummy(wsi, reason, user, in, len);
}
static struct lws_protocols protocols[] = {
  { "http", callback_http, sizeof(void *), 0 },
  { NULL, NULL, 0, 0 } /* terminator */
};

bool WebServerImpl::Start()
{
  int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
    /* for LLL_ verbosity above NOTICE to be built into lws,
     * lws must have been configured and built with
     * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
    /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
    /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
    /* | LLL_DEBUG */;
  lws_set_log_level(logs, NULL);

  std::memset(&m_ContextInfo, 0, sizeof(m_ContextInfo));
  m_ContextInfo.port = m_Settings.Port;
  m_ContextInfo.protocols = protocols;
  m_ContextInfo.mounts = &m_Static;
  m_ContextInfo.user = this;

  std::free(m_pContext);
  m_pContext = lws_create_context(&m_ContextInfo);
  if (!m_pContext)
  {
    return false;
  }
  return true;

}
void WebServerImpl::Process()
{
  lws_service(m_pContext, 1000);
}

}
