#pragma once
#include "utils/dbgextension.h"
#include "utils/resolver.h"
#include "utils/utils.h"
#include <string>

#define END_OF_STRING '\x00'
extern std::vector<ProcessInformation> processList;

typedef enum _IO_SESSION_STATE {
  Null = 0x0,
  Created = 0x1,
  Initialized = 0x2,
  Connected = 0x3,
  Disconnected = 0x4,
  DisconnectedLoggedOn = 0x5,
  LoggedOn = 0x6,
  LoggedOff = 0x7,
  Terminated = 0x8,
  Max = 0x9,
} IO_SESSION_STATE;

class SessionInfo1 {
public:
  u32 SessionId;
  u64 SessionObject;
  u64 SessionObjectHandle;
  u64 CreateTime;
  u32 IoState;
  std::string process_name;
  // char process_name[0xff];
  std::string userName;
  std::string domainName;
  std::string sessionName;
};

std::vector<SessionInfo1> SessionInfoList{0};

void session_banner() {
  Log("%10s %20s %18s %18s %20s %18s", "SessionId", "ProcessName",
      "SessionObject", "SessionHandle", "IoState", "CreateTime");
  NEWLINE();
}

void session_print() {
  for (auto s_info : SessionInfoList) {
    Log("%10d %20s %18p %18p %20s %20p\n", s_info.SessionId,
        s_info.process_name, s_info.SessionObject, s_info.SessionObjectHandle,
        magic_enum::enum_name((IO_SESSION_STATE)s_info.IoState).data(),
        s_info.CreateTime);
  }
}

class SessionInfoBuilder {
private:
  SessionInfo1 s_info;

public:
  SessionInfoBuilder &setSessionId(u32 id) {
    s_info.SessionId = id;
    return *this;
  }
  SessionInfoBuilder &setSessionObject(u64 object) {
    s_info.SessionObject = object;
    return *this;
  }
  SessionInfoBuilder &setSessionObjectHandle(u64 obHdle) {
    s_info.SessionObjectHandle = obHdle;
    return *this;
  }
  SessionInfoBuilder &setCreateTime(u64 time) {
    s_info.CreateTime = time;
    return *this;
  }
  SessionInfoBuilder &setProcessName(char* name) {
    s_info.process_name = (std::string) name;
    return *this;
  }

  SessionInfoBuilder &setIoState(u32 ioState) {
    s_info.IoState = ioState;
    return *this;
  }

  SessionInfoBuilder &setDomainName(std::string domain_name) {
    s_info.domainName = domain_name;
    return *this;
  }
  SessionInfoBuilder &setUserName(std::string user_name) {
    s_info.userName = user_name;
    return *this;
  }
  SessionInfoBuilder &setSessionName(std::string session_name) {
    s_info.sessionName = session_name;
    return *this;
  }

  SessionInfo1 build() { return s_info; }
};

STDAPI _sessions(IDebugClient *client, PCSTR args) {
  CComQIPtr<IDebugControl4> control(client);

  auto hr = control->GetWindbgExtensionApis64(&ExtensionApis);
  if (FAILED(hr)) {
    return hr;
  }

  IDebugDataSpaces4 *dataspaces = nullptr;
  hr =
      client->QueryInterface(__uuidof(IDebugDataSpaces4), (void **)&dataspaces);
  if (hr != S_OK) {
    return E_FAIL;
  }

  LoadRequirements((std::basic_string_view<char>)PSLIST);

  ULONG bytesRead = 0x0;
  u64 Session;

  SessionInfo s_info;
  InitSessionInfo(&s_info);
  session_banner();

  for (auto process : processList) {
    // Iterate the process and get the session details !!
    dataspaces->ReadVirtual(process.Eprocess + EProcessFieldOffset("Session"),
                            &Session, sizeof(Session), &bytesRead);

    u64 SessionIdOffset = FieldOffset("_MM_SESSION_SPACE", "SessionId");
    u64 SessionObjectOffset = FieldOffset("_MM_SESSION_SPACE", "SessionObject");
    u64 SessionObHandleOffset =
        FieldOffset("_MM_SESSION_SPACE", "SessionObjectHandle");
    u64 CreateTimeOffset = FieldOffset("_MM_SESSION_SPACE", "CreateTime");
    u64 IoStateOffset = FieldOffset("_MM_SESSION_SPACE", "IoState");
    u64 processPramas = FieldOffset("_PEB", "ProcessParameters");
    u64 EnvSize = ProcessParamsFieldOffset("EnvironmentSize");
    u64 EnvironmentVariable = ProcessParamsFieldOffset("Environment");

    // Setting dataspaces
    dataspaces->ReadVirtual(Session + SessionIdOffset, &s_info.SessionId,
                            sizeof(s_info.SessionId), &bytesRead);
    dataspaces->ReadVirtual(Session + SessionObjectOffset,
                            &s_info.SessionObject, sizeof(s_info.SessionObject),
                            &bytesRead);
    dataspaces->ReadVirtual(Session + SessionObHandleOffset,
                            &s_info.SessionObjectHandle,
                            sizeof(s_info.SessionObjectHandle), &bytesRead);
    dataspaces->ReadVirtual(Session + CreateTimeOffset, &s_info.CreateTime,
                            sizeof(s_info.CreateTime), &bytesRead);
    dataspaces->ReadVirtual(Session + IoStateOffset, &s_info.IoState,
                            sizeof(s_info.IoState), &bytesRead);

    
    // Switch the process context
    // Read the PEB and get the _RTL_USER_PROCESS_PARAMETERS
    // Read the Environment variable and dereference it
    // Parse it and store it in hashmap
    // Look for USERNAME, DOMAINNAME, SESSIONNAME
    // copy it to struct and display it

    SwitchProcessContext(client, control, process.Eprocess);

    char *temp = process.process_name;
    // Log(temp);
    // Log("\n");
    SessionInfo1 s_info1 =
        SessionInfoBuilder()
            .setSessionId(s_info.SessionId)
            .setIoState(s_info.IoState)
            .setCreateTime(s_info.CreateTime)
            .setProcessName(temp)
            .setSessionObject(s_info.SessionObject)
            .setSessionObjectHandle(s_info.SessionObjectHandle)
            .build();
    //  s_info1.show();
    SessionInfoList.push_back(s_info1);
  }
  session_print();
  return S_OK;
}
