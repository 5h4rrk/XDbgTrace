#pragma once
#include "utils/dbgextension.h"
#include "modules/dlllist.h"
#include "modules/envars.h"
#include "modules/objects.h"
#include "utils/processutils.h"
#include "modules/psdetails.h"
#include "modules/pslist.h"
#include "modules/sessions.h"
#include "modules/threads.h"
#include "threads.h"
#include "utils/xdbg.h"

#pragma comment(lib, "dbgeng")

WINDBG_EXTENSION_APIS64 ExtensionApis{sizeof(ExtensionApis)};

STDAPI DebugExtensionInitialize(PULONG Version, PULONG Flags) {

  CComPtr<IDebugClient> client;
  auto hr = DebugCreate(__uuidof(IDebugClient), (void **)&client);
  if (FAILED(hr)) {
    return hr;
  }

  CComQIPtr<IDebugControl> control(client);

  hr = control->GetWindbgExtensionApis64(&ExtensionApis);
  if (FAILED(hr)) {
    return hr;
  }

  *Version = DEBUG_EXTENSION_VERSION(1, 0);
  *Flags = 0;
  return S_OK;
}

std::vector<ProcessInformation> processList;
std::vector<PebInformation> PebInfo;


STDAPI dumpinfo(IDebugClient *client, PCSTR args) {
  DUMP_HEADER64 dumpHeader = {0};
  ULONG bytesRead = 0;
  return S_OK;
}

STDAPI help(IDebugClient *client, PCSTR args) {
  _help();
  return S_OK;
}

STDAPI pslist(IDebugClient *client, PCSTR args) {

  if (strncmp(args, "-h", 2) == 0) {
    _process_help();
    return S_OK;
  }

  if (args != nullptr) {
    check_args(args);
  }

  if (!processList.empty()) {
    displayBanner();
    displayEntries(TRUE);
    parsepslist(FALSE, (LPSTR) "");
    return S_OK;
  }

  parsepslist(TRUE, args);
  return S_OK;
}

STDAPI dlllist(IDebugClient *client, PCSTR args) {
  if (_dlllist(client, args) == S_OK) {
    return S_OK;
  } else {
    dprintf("Parsing Failed \n");
    return E_FAIL;
  }
}

STDAPI psdetails(IDebugClient *client, PCSTR args) {
  return _psdetails(client, args);
}

STDAPI threads(IDebugClient *client, PCSTR args) {
  return _threads(client, args);
}

STDAPI objects(IDebugClient *client, PCSTR args) {
  return _objects(client, args);
}

STDAPI sessions(IDebugClient *client, PCSTR args) {
  return _sessions(client, args);
}

STDAPI envars(IDebugClient *client, PCSTR args) {
  xDbg.ISetup(client);
  return _envars();
}