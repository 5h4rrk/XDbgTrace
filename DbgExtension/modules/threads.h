#pragma once
#include "utils/dbgextension.h"
#include "utils/resolver.h"

extern std::vector<ProcessInformation> processList;
extern std::string ConvertLargeIntegerToUTC(LARGE_INTEGER largeInt);
extern VOID SwitchProcessContext(IDebugClient *client, IDebugControl4 *control, u64 address);

#define INVALID_START_ADDRESS 0x0

BOOL isValidStartAddress(u64 start_address) {
  if (start_address != INVALID_START_ADDRESS) {
    return TRUE;
  }
  return FALSE;
}

BOOL isValidAddress(u64 address) {
  if (!((address) & 0x0f)) {
    return TRUE;
  }
  return FALSE;
}

STDAPI _threads(IDebugClient *client, PCSTR args) {
  if (strcmp(args, "-h") == 0) {
    _threads_help();
    return S_OK;
  }

  u64 threadListHead = 0x0;
  u64 flink = 0x0;

  CComQIPtr<IDebugControl4> control(client);

  auto hr = control->GetWindbgExtensionApis64(&ExtensionApis);
  if (FAILED(hr)) {
    return hr;
  }

  IDebugDataSpaces4 *dataspaces = nullptr;
  hr = client->QueryInterface(__uuidof(IDebugDataSpaces4), (void **)&dataspaces);
  if (hr != S_OK) {
    return E_FAIL;
  }

  LoadRequirements((std::basic_string_view<char>)PSLIST);
  ULONG bytesRead = 0x0;
  ULONG64 Flink = 0x0;
  ULONG64 NextEthread = 0x0;
  ULONG64 ActiveThreads = 0x0;
  ULONG64 Ethread = 0x0;
  _LARGE_INTEGER CreateTime;
  _LARGE_INTEGER ExitTime;
  ULONG64 ThreadId = 0x0;
  ULONG64 StartAddress = 0x0;

  Log("%-18s %8s %8s %-18s  %-24s %-24s %-24s\n",
    "ETHREAD",
    "PID",
    "TID",
    "StartAddress",
    "ProcessName",
    "CreateTime(UTC)",
    "ExitTime(UTC)");
    
  for (const auto &process : processList) {
    // Iterating threads
    threadListHead = EProcessFieldOffset("ThreadListHead");
    // LOGINFO("ThreadListHead -> [ %p ]\n", threadListHead);
    #if !defined(KDEXT_64BIT)
        dataspaces->ReadVirtual(process.Eprocess + threadListHead, &Flink, sizeof(0x4), &bytesRead);
    #else
        dataspaces->ReadVirtual(process.Eprocess + threadListHead, &Flink, sizeof(&Flink), &bytesRead);
        // LOGINFO("Process  %s  Flink : %p\n", process.process_name , Flink);
        dataspaces->ReadVirtual(process.Eprocess + EProcessFieldOffset("ActiveThreads"), &ActiveThreads, sizeof(&ActiveThreads), &bytesRead);
        // LOGINFO(" Eprocess %p ActiveThreads %d\n",process.Eprocess, ActiveThreads);
    #endif

    SwitchProcessContext(client, control, process.Eprocess);
    // LOGINFO("Active Threads for %s : %d", process.process_name, ActiveThreads);


    for (int i=0; i< ActiveThreads; i++) {
      NextEthread = dataspaces->ReadVirtual(Flink, &NextEthread, sizeof(&NextEthread), &bytesRead);
      Ethread = Flink - EThreadFieldOffset("ThreadListEntry");

      // LOGINFO("Flink %p  Ethread @ %p",Flink ,Ethread);
      if ((isValidAddress(Ethread)) == FALSE) {
        break;
      }
      
      dataspaces->ReadVirtual(Ethread + EThreadFieldOffset("CreateTime"), &CreateTime, sizeof(&CreateTime), &bytesRead);
      dataspaces->ReadVirtual(Ethread + EThreadFieldOffset("ExitTime"), &ExitTime, sizeof(&ExitTime), &bytesRead);
      dataspaces->ReadVirtual(Ethread + EThreadFieldOffset("StartAddress"), &StartAddress, sizeof(&StartAddress), &bytesRead);
      dataspaces->ReadVirtual(Ethread + EThreadFieldOffset("Cid") + 0x8, &ThreadId, sizeof(&ThreadId), &bytesRead);
      dataspaces->ReadVirtual(Ethread + EThreadFieldOffset("ThreadListEntry") , &NextEthread, sizeof(&NextEthread), &bytesRead);
      Flink = NextEthread;

      if (isValidAddress(Ethread) && isValidStartAddress(StartAddress)) {
        Log("%-18p %8d %8d %-18p  %-24s %-24s %-24s\n",
            Ethread,
            process.PID,
            ThreadId,
            StartAddress,
            process.process_name,
            ConvertLargeIntegerToUTC(CreateTime).c_str(),
            ConvertLargeIntegerToUTC(ExitTime).c_str());
      }
    }
  }
  return S_OK;
}
