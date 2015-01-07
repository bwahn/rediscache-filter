/*
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <ts/ts.h>
#include <ts/remap.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <redis3m/redis3m.hpp>

#include "default.h"
#include "redis_control.h"

typedef struct {
  char * query;
} my_data;

bool do_redis_remap(TSCont contp,TSHttpTxn txnp)
{
  TSMBuffer reqp;
  TSMLoc hdr_loc, url_loc, field_loc;
  bool ret_val = false;

  const char *request_host;
  int request_host_length = 0;
  const char *request_scheme;
  int request_scheme_length = 0;
  int request_port = 80;
  char ikey[1024];
  char *m_result = NULL;
  size_t oval_length;
  uint32_t flags;

  if (TSHttpTxnClientReqGet((TSHttpTxn) txnp, &reqp, &hdr_loc) !=
      TS_SUCCESS) {
      TSDebug(PLUGIN_NAME, "could not get request data");
      return false;
  }

  if (TSHttpHdrUrlGet(reqp, hdr_loc, &url_loc) != TS_SUCCESS) {
      TSDebug(PLUGIN_NAME, "couldn't retrieve request url");
      goto release_hdr;
  }


  field_loc =
      TSMimeHdrFieldFind(reqp, hdr_loc, TS_MIME_FIELD_HOST,
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

  TSDebug(PLUGIN_NAME, "      +++++REDIS REMAP+++++      ");

  TSDebug(PLUGIN_NAME,
          "\nINCOMING REQUEST ->\n ::: from_scheme_desc: %.*s\n ::: from_hostname: %.*s\n ::: from_port: %d",
          request_scheme_length, request_scheme, request_host_length,
          request_host, request_port);

  snprintf(ikey, 1024, "%.*s://%.*s:%d/", request_scheme_length,
           request_scheme, request_host_length, request_host,
           request_port);

  TSDebug(PLUGIN_NAME, "querying for the key %s\n", ikey);





    goto free_stuff;            // free the result set after processed

  not_found:
    //lets build up a nice 404 message for someone
    if (!ret_val) {
        TSHttpHdrStatusSet(reqp, hdr_loc, TS_HTTP_STATUS_NOT_FOUND);
        TSHttpTxnSetHttpRetStatus(txnp, TS_HTTP_STATUS_NOT_FOUND);
    }
  free_stuff:
#if (TS_VERSION_NUMBER < 2001005)
    if (request_host)
        TSHandleStringRelease(reqp, hdr_loc, request_host);
    if (request_scheme)
        TSHandleStringRelease(reqp, hdr_loc, request_scheme);
#endif

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

static int
redis_remap (TSCont contp, TSEvent event, void *edata) {
  TSHttpTxn txnp = (TSHttpTxn) edata;
  TSEvent reenable = TS_EVENT_HTTP_CONTINUE;

  switch(event) {
    case TS_EVENT_HTTP_READ_REQUEST_HDR:
      TSDebug(PLUGIN_NAME,"Reading Request");
      TSSkipRemappingSet(txnp,1);
      if (!do_redis_remap(contp,txnp)) {
        reenable = TS_EVENT_HTTP_ERROR;
      }
      break;
    default:
      break;
  }

  TSHttpTxnReenable(txnp, reenable);
  return 1;
}

void
TSPluginInit(int argc, const char *argv[])
{
  my_data * data = (my_data*) malloc(1*sizeof(my_data));

  TSPluginRegistrationInfo info;
  info.plugin_name   = const_cast<char*>(PLUGIN_NAME);
  info.vendor_name   = const_cast<char*>("Apache Software Foundation");
  info.support_email = const_cast<char*>("dev@trafficserver.apache.org");

  if (TSPluginRegister(TS_SDK_VERSION_2_0 , &info) != TS_SUCCESS) {
    TSError("redis_remap: plugin registration failed.\n");
  }

  redis_control::conn = redis3m::connection::create("127.0.0.1", 6379);
  data->query = (char*)TSmalloc(QSIZE * sizeof(char));

  TSCont cont = TSContCreate(redis_remap, TSMutexCreate());

  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);

  TSContDataSet (cont, (void *)data);

  TSDebug(PLUGIN_NAME, "plugin is successfully initialized [plugin mode]");

  return;
}
