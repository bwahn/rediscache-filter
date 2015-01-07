#include <ts/ts.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>
#include <list>

#include "credis.h"
#include "default.h"
#include "iniparser.h"
#include <ext/hash_map>
#include <ext/hash_set>


using namespace std;

class UrlData
{
  UrlData();

  typedef __gnu_cxx::hash_map<std::string, std::string> UrlDataMap;
  typedef UrlDataMap::const_iterator url_it;
};

bool do_rediscache_filter(TSCont contp,TSHttpTxn txnp)
{
  bool ret_val = false;

  TSMBuffer reqp;
  TSMLoc hdr_loc;
  TSMLoc url_loc;
  TSMLoc field_loc;

  const char* request_host;
  int request_host_length = 0;
  const char* request_scheme;
  int request_scheme_length = 0;
  int request_port = 80;
  //char ikey[1024];
  char *url;
  int url_length = 0;

  if (TSHttpTxnClientReqGet((TSHttpTxn) txnp, &reqp, &hdr_loc) !=
      TS_SUCCESS) {
    TSDebug(PLUGIN_NAME, "could not get request data");
    return false;
  }

  if (TSHttpHdrUrlGet(reqp, hdr_loc, &url_loc) != TS_SUCCESS) {
    TSDebug(PLUGIN_NAME, "couldn't retrieve request url");
    goto release_hdr;
  }

  field_loc = TSMimeHdrFieldFind(reqp, hdr_loc, TS_MIME_FIELD_HOST,
                    TS_MIME_LEN_HOST);

  if (!field_loc) {
    TSDebug(PLUGIN_NAME, "couldn't retrieve request HOST header");
    goto release_url;
  }

  request_host = TSMimeHdrFieldValueStringGet(reqp, hdr_loc, field_loc, -1, &request_host_length);
  if (request_host == NULL || strlen(request_host) < 1) {
    TSDebug(PLUGIN_NAME, "couldn't find request HOST header");
    goto release_field;
  }

  request_scheme = TSUrlSchemeGet(reqp, url_loc, &request_scheme_length);
  request_port = TSUrlPortGet(reqp, url_loc);

  url = TSHttpTxnEffectiveUrlStringGet(txnp, &url_length);
  if (!url) {
    TSError("[%s] couldn't retrieve request url\n", PLUGIN_NAME);
  }

  TSDebug(PLUGIN_NAME, "      +++++REDISCACHE_FILTER+++++      ");

  TSDebug(PLUGIN_NAME, "\nINCOMING REQUEST ->\n \
                        ::: from_scheme_desc: %.*s\n \
                        ::: from_hostname: %.*s\n \
                        ::: from_port: %d",
                        request_scheme_length,
                        request_scheme,
                        request_host_length,
                        request_host,
                        request_port);

  TSDebug(PLUGIN_NAME, "\nurl:%s\n", url);

  //snprintf(ikey, 1024, "%.*s://%.*s:%d/", request_scheme_length,
  //  request_scheme, request_host_length, request_host,
  //  request_port);


    ret_val = true;

  if (url) {
    TSfree(url);
  }

release_field:
  if (field_loc)
    TSHandleMLocRelease(reqp, hdr_loc, field_loc);

release_url:
  if (url_loc)
    TSHandleMLocRelease(reqp, hdr_loc, url_loc);

release_hdr:
  if (hdr_loc)
    TSHandleMLocRelease(reqp, TS_NULL_MLOC, hdr_loc);

  return ret_val;
}

static int rediscache_filter(TSCont contp, TSEvent event, void *edata)
{
  TSHttpTxn txnp = (TSHttpTxn) edata;
  TSEvent reenable = TS_EVENT_HTTP_CONTINUE;

  switch(event) {
    case TS_EVENT_HTTP_READ_REQUEST_HDR:
      TSDebug(PLUGIN_NAME,"Reading Request");
      TSSkipRemappingSet(txnp, 1);
      if (!do_rediscache_filter(contp, txnp)) {
        reenable = TS_EVENT_HTTP_ERROR;
      }
    break;
      default:
    break;
  }

  TSHttpTxnReenable(txnp, reenable);
  return 1;
}



void TSPluginInit(int argc, const char *argv[])
{
  dictionary * ini;
  const char * host;
  int port;
  int timeout;

  REDIS rh;


  TSPluginRegistrationInfo info;
  info.plugin_name   = const_cast<char*>(PLUGIN_NAME);
  info.vendor_name   = const_cast<char*>("Apache Software Foundation");
  info.support_email = const_cast<char*>("dev@trafficserver.apache.org");


  if (TSPluginRegister(TS_SDK_VERSION_2_0 , &info) != TS_SUCCESS) {
    TSError("rediscache_filter: plugin registration failed.\n");
  }

  if (argc != 2) {
    TSError("usage : %s /path/to/sample.ini\n", argv[0]);
    return;
  }
  TSDebug(PLUGIN_NAME, ">> start");
  TSDebug(PLUGIN_NAME, ">> start, argv[1]:%s", argv[1]);

  ini = iniparser_load(argv[1]);
  if (!ini) {
    TSError("Error with ini file (1)");
    TSDebug(PLUGIN_NAME, "Error parsing ini file(1)");
    return;
  }

  //host, port, timeout
  host    = iniparser_getstring(ini, "redis_host:", (char*)"localhost");
  port    = iniparser_getint(ini, "redis_port:", 6379);
  timeout = iniparser_getint(ini, "redis_timeout:", 2000);

  TSDebug(PLUGIN_NAME, "host:%s\n", host);
  TSDebug(PLUGIN_NAME, "port:%d\n", port);
  TSDebug(PLUGIN_NAME, "timeout:%d\n", timeout);

  rh = credis_connect(host, port, timeout);

  TSCont cont = TSContCreate(rediscache_filter, TSMutexCreate());
  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);

  TSDebug(PLUGIN_NAME, "plugin is successfully initialized [plugin mode]");

  credis_close(rh);

  return;
}
