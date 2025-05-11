#pragma once
#include "dbgextension.h"
#include "resolver.h"

typedef enum CW32__LDR_DLL_LOAD_REASON
{
  StaticDependency,
  StaticForwarderDependency,
  DynamicForwarderDependency,
  DelayloadDependency,
  DynamicLoad,
  AsImageLoad,
  AsDataLoad,
  ReasonUnknown = -1,
} CW32_LDR_DLL_LOAD_REASON,
    *CW32_PLDR_DLL_LOAD_REASON;

extern std::vector<ProcessInformation> processList;

auto ParseLoadReason(u64 Flink, IDebugDataSpaces4 *dataspaces)
{
  ULONG bytesRead;
  u64 loadReason;

  dataspaces->ReadVirtual(
      Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "LoadReason"),
      &loadReason, sizeof(&loadReason), &bytesRead);
  return magic_enum::enum_name((CW32_LDR_DLL_LOAD_REASON)loadReason).data();
}

VOID ParseDllNames(IDebugControl4 *control, IDebugDataSpaces4 *dataspaces,
                   u64 Ldr, ProcessInformation process)
{
  if (Ldr == INVALID_OFFSET)
  {
    Log("Ldr is Null\n");
    return;
  }

  u64 Flink = 0x0;
  u64 prevFlink = 0x0;
  u64 SizeOfImage = 0;
  u64 ImageBaseAddress = 0;
  u64 RefCount = 0;
  _UNICODE_STRING DllName;
  _UNICODE_STRING dllPath = {0};
  if (!InitUnicodeString(&dllPath) && !InitUnicodeString(&DllName))
  {
    Log("Initialization of Buffer Failed\n");
    return;
  }
  ULONG bytesRead = 0x0;
  u64 count = 0;

  dataspaces->ReadVirtual(Ldr + FieldOffset("_PEB_LDR_DATA", "InLoadOrderModuleList"), &Flink,
                          sizeof(&Flink),
                          &bytesRead);
  do
  {
    if (Flink == INVALID_OFFSET)
    {
      break;
    }
    prevFlink = Flink;
    dataspaces->ReadVirtual(Flink, &Flink, sizeof(&Flink), &bytesRead);

    if (((u64)prevFlink == (u64)Flink))
    {
      return;
    }
    dataspaces->ReadVirtual(
        Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "SizeOfImage"),
        &SizeOfImage, sizeof(&SizeOfImage), &bytesRead);
    dataspaces->ReadVirtual(
        Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "FullDllName"),
        &dllPath, sizeof(&dllPath) + 0x8, &bytesRead);
    dataspaces->ReadVirtual(
        Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "DllBase"),
        &ImageBaseAddress, sizeof(&ImageBaseAddress), &bytesRead);
    dataspaces->ReadVirtual(
        Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "ReferenceCount"),
        &RefCount, sizeof(&RefCount), &bytesRead);
    dataspaces->ReadVirtual(
        Flink + FieldOffset(LDR_DATA_TABLE_ENTRY, "BaseDllName"),
        &DllName, sizeof(&DllName) + 0x8, &bytesRead);

    if (dllPath.Length == ZERO_SIZE && dllPath.MaximumLength == ZERO_SIZE &&
        dllPath.Buffer == nullptr)
    {
      return;
    }

    if (SizeOfImage == ZERO_SIZE)
    {
      return;
    }

    control->ControlledOutput(DEBUG_OUTCTL_THIS_CLIENT, DEBUG_OUTPUT_NORMAL,
                              "%-10d\t%-20s\t%-70mu\t0x%-14x\t%-128mu\t0x%-20p\t%-8d\t%-20s\n",
                              process.PID, process.process_name, DllName.Buffer, SizeOfImage, dllPath.Buffer,
                              ImageBaseAddress, RefCount,
                              ParseLoadReason(Flink, dataspaces));
    dllPath.Length = 0;
    dllPath.MaximumLength = 0;
    ImageBaseAddress = 0;
    SizeOfImage = 0;
  } while (TRUE);
}

STDAPI _dlllist(IDebugClient *client, PCSTR args)
{
  LoadRequirements((std::basic_string_view<char>)PSLIST);
  CComQIPtr<IDebugControl4> control(client);

  auto hr = control->GetWindbgExtensionApis64(&ExtensionApis);
  if (FAILED(hr))
  {
    return hr;
  }

  IDebugDataSpaces4 *dataspaces = nullptr;
  hr =
      client->QueryInterface(__uuidof(IDebugDataSpaces4), (void **)&dataspaces);
  if (hr != S_OK)
  {
    Log("Failed to get IDebugDataSpaces4 interface\n");
    return E_FAIL;
  }

  ULONG64 Peb = 0;
  ULONG64 Ldr = 0;
  ULONG bytesRead = 0;
  if (processList.empty())
  {
    parsepslist(FALSE, (PCSTR) "-p");
  }
  control->ControlledOutput(DEBUG_OUTCTL_THIS_CLIENT, DEBUG_OUTPUT_NORMAL,
                            "%-10s\t%-20s\t%-70s\t%-16s\t%-128s\t%-22s\t%-8s\t%-20s\n", "PID",
                            "ProcessName", "DllName", "ImageSize", "DLLPath",
                            "ImageBase", "RefCnt",
                            "LoadReason");

  for (const auto &process : processList)
  {
    dataspaces->ReadVirtual(process.Eprocess + EProcessFieldOffset("Peb"), &Peb,
                            sizeof(&Peb), &bytesRead);
    if (Peb == 0x0)
    {
      continue;
    }

    PebInformation peb_info;
    InitPebInformation(&peb_info);

    peb_info.Eprocess = process.Eprocess;
    dataspaces->ReadVirtual(Peb + PebFieldOffset("ImageBaseAddress"),
                            &peb_info.ImageBaseAddress,
                            sizeof(&peb_info.ImageBaseAddress), &bytesRead);

    dataspaces->ReadVirtual(Peb + PebFieldOffset("Ldr"), &peb_info.PebLdrData,
                            sizeof(&peb_info.PebLdrData), &bytesRead);

    SwitchProcessContext(client, control, process.Eprocess);

    if (isPPL(process.Eprocess))
    {
      if (Ldr == 0x0)
      {
        continue;
      }
      continue;
    }
    dataspaces->ReadVirtual(Peb + PebFieldOffset("Ldr"), &Ldr, sizeof(Ldr),
                            &bytesRead);
    ParseDllNames(control, dataspaces, Ldr, process);
  }
  return S_OK;
}
