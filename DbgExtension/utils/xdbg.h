#pragma once
#include "dbgextension.h"

// =============== XDbg ===============
namespace wcom {
// TODO: use Singleton DP 
class XDbg {
public:
  IDebugClient *client = nullptr;
  IDebugDataSpaces *dataspaces = nullptr;
  CComQIPtr<IDebugControl4> control;

  XDbg() {};
  void Setup(IDebugClient* c) {
      this->client = c; this->control = c; this->InitApis();
  }
  void ISetup(IDebugClient *c) {
      //wcom::XDbg _c(c);
    this->Setup(c);
  }

  XDbg(IDebugClient *x_client) : client(x_client), control(x_client) {
    InitApis();
  }

  void Verbose() {
    Log("XDbg Invoked !!\n");
  }
  void InitApis() {
    auto hr = this->control->GetWindbgExtensionApis64(&::ExtensionApis);
    if (hr != S_OK) {
      Log("ExtensionApis Failed \n");
      return;
    }

    if (this->client) {
      if ((this->client->QueryInterface(__uuidof(IDebugDataSpaces4),
                                  (void **)&this->dataspaces)) != S_OK) {
        this->dataspaces = nullptr;
        Log("IDebugDataSpaces4 Failed \n");
        return;
      }
    }
  }
};
} // namespace wcom

wcom::XDbg xDbg;
