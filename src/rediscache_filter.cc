#include <ts/ts.h>
#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#include "credis.h"

using namespace std;




void
TSPluginInit(int argc, const char *argv[])
{
  my_data * data = (my_data*) malloc(1*sizeof(my_data));

  TSPluginRegistrationInfo info;
  info.plugin_name   = const_cast<char*>(PLUGIN_NAME);
  info.vendor_name   = const_cast<char*>("Apache Software Foundation");
  info.support_email = const_cast<char*>("dev@trafficserver.apache.org");

#if 0
  if (TSPluginRegister(TS_SDK_VERSION_2_0 , &info) != TS_SUCCESS) {
    TSError("rediscache_filter: plugin registration failed.\n");
  }

  data->query = (char*)TSmalloc(QSIZE * sizeof(char));

  TSCont cont = TSContCreate(rediscache_filter, TSMutexCreate());

  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);

  TSContDataSet (cont, (void *)data);

  TSDebug(PLUGIN_NAME, "plugin is successfully initialized [plugin mode]");
#endif 
  return;
}
