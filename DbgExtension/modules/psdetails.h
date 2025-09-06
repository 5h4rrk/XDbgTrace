#pragma once
#include "utils/dbgextension.h"
#include "help.h"
#include "utils/resolver.h"

extern std::vector<ProcessInformation> processList;
extern VOID SwitchProcessContext(IDebugClient *client, IDebugControl4 *control, u64 address);

#define DbgLog(control, fmt, ...) control->ControlledOutput(DEBUG_OUTCTL_THIS_CLIENT, DEBUG_OUTPUT_NORMAL, fmt, __VA_ARGS__)
#define DbgLogWide(control, fmt, ...) control->ControlledOutputWide(DEBUG_OUTCTL_THIS_CLIENT, DEBUG_OUTPUT_NORMAL, fmt, __VA_ARGS__)

extern "C" VOID
InitString(_STRING *dest)
{
  RtlInitAnsiString(dest, nullptr);
}

extern "C" VOID
FreeString(_STRING *dest)
{
  RtlFreeAnsiString(dest);
}

_STRING dest;
// Free it
extern "C" PCHAR DecodeUnicode(UNICODE_STRING *source)
{
  if (!source || !source->Buffer)
    Log("Null Found\n");
  return nullptr;

  int sizeNeeded =
      WideCharToMultiByte(CP_ACP, 0, source->Buffer, -1, NULL, 0, NULL, NULL);
  if (sizeNeeded <= 0)
    Log("Size is null\n");
  return nullptr;

  char *ansiStr = (char *)VirtualAlloc(
      NULL, sizeNeeded, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!ansiStr)
    Log("Failed to convert\n");
  return nullptr;

  int result = WideCharToMultiByte(CP_ACP, 0, source->Buffer, -1, ansiStr, sizeNeeded, NULL, NULL);
  if (result == 0)
  {
    VirtualFree(ansiStr, 0, MEM_RELEASE);
    return nullptr;
  }
  return ansiStr;
}

VOID InitUnicodeStringBuffer(IDebugDataSpaces4 *dataspaces, ProcessDetails *pDetails, u64 pParams, PCSTR fieldName)
{
  u64 size = 0x0;
  ULONG bytesRead = 0x0;
  dataspaces->ReadVirtual(pParams + ProcessParamsFieldOffset(fieldName), &size, sizeof(UINT16), &bytesRead);
  pDetails->CommandLine = (CHAR *)VirtualAlloc(nullptr, (size / 2) + 0x1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

VOID DumpPsInfoEProcess(IDebugControl4 *control, ProcessInformation *psInfo)
{
  DbgLog(control, "\tProcess              : %s\n", psInfo->process_name);
  DbgLog(control, "\tPID                  : %d\n", psInfo->PID);
  DbgLog(control, "\tEProceess            : %p\n", psInfo->Eprocess);
  DbgLog(control, "\tCookie               : 0x%x\n", psInfo->Cookie);
  DbgLog(control, "\tToken                : %p\n", psInfo->Token);
  DbgLog(control, "\tSectionObject        : %p\n", psInfo->SectionObject);
  DbgLog(control, "\tSectionBaseAddress   : %p\n", psInfo->SectionBaseAddress);
  DbgLog(control, "\tProcessQuotaUsage    : %p\n", psInfo->ProcessQuotaUsage);
  DbgLog(control, "\tProcessQuotaPeak     : %p\n", psInfo->ProcessQuotaPeak);
}

VOID ReadPsDetails(IDebugDataSpaces4 *dataspaces, IDebugControl4 *control, ProcessInformation *psInfo)
{

  ULONG bytesRead = 0x0;
  u64 pParams = psInfo->Peb + PebFieldOffset("ProcessParameters");

  dataspaces->ReadVirtual(pParams, &pParams, sizeof(&pParams), &bytesRead);
  if (pParams == 0x0)
  {
    return;
  }

  u64 bufferAddress = 0x0;
  bufferAddress = pParams + ProcessParamsFieldOffset("CommandLine") + UNICODE_STRING_LENGTH_OFFSET;
  dataspaces->ReadVirtual(bufferAddress, &psInfo->pDetails->CommandLine, sizeof(&psInfo->pDetails->CommandLine), &bytesRead);

  bufferAddress = pParams + ProcessParamsFieldOffset("WindowTitle") + UNICODE_STRING_LENGTH_OFFSET;
  dataspaces->ReadVirtual(bufferAddress, &psInfo->pDetails->WindowTitle, sizeof(&psInfo->pDetails->WindowTitle), &bytesRead);

  bufferAddress = pParams + ProcessParamsFieldOffset("DesktopInfo") + UNICODE_STRING_LENGTH_OFFSET;
  dataspaces->ReadVirtual(bufferAddress, &psInfo->pDetails->DesktopInfo, sizeof(&psInfo->pDetails->DesktopInfo), &bytesRead);

  bufferAddress = pParams + ProcessParamsFieldOffset("CurrentDirectory");
  dataspaces->ReadVirtual(bufferAddress + FieldOffset("nt!_CURDIR", "Handle"), &psInfo->pDetails->CurrentDirectory.Handle, sizeof(&psInfo->pDetails->CurrentDirectory.Handle), &bytesRead);

  // bufferAddress = bufferAddress + FieldOffset("nt!_CURDIR", "DosPath") + UNICODE_STRING_LENGTH_OFFSET;
  // Log("BufferAddress : %p\n", bufferAddress);
  // dataspaces->ReadVirtual(bufferAddress + FieldOffset("nt!_CURDIR", "DosPath") + FieldOffset("nt!_UNICODE_STRING", "Buffer"), &psInfo->pDetails->CurrentDirectory.DosPath, sizeof(&psInfo->pDetails->CurrentDirectory.DosPath), &bytesRead);

  bufferAddress = pParams + ProcessParamsFieldOffset("StartingX");
  dataspaces->ReadVirtual(bufferAddress, &psInfo->pDetails->StartingX, sizeof(&psInfo->pDetails->StartingX), &bytesRead);

  bufferAddress = pParams + ProcessParamsFieldOffset("StartingY");
  dataspaces->ReadVirtual(bufferAddress, &psInfo->pDetails->StartingY, sizeof(&psInfo->pDetails->StartingY), &bytesRead);

  // DbgLog(control, "\tCurrentWorkingDir    : %p\n", psInfo->pDetails->CurrentDirectory.DosPath);
  DbgLog(control, "\tCurrDirHandle        : %p\n", psInfo->pDetails->CurrentDirectory.Handle);
  DbgLog(control, "\tCommandLine          : %mu\n", psInfo->pDetails->CommandLine);
  DbgLog(control, "\tWindowTitle          : %mu\n", psInfo->pDetails->WindowTitle);
  DbgLog(control, "\tDesktopInfo          : %mu\n", psInfo->pDetails->DesktopInfo);
  DbgLog(control, "\tStartingX            : %d\n", psInfo->pDetails->StartingX);
  DbgLog(control, "\tStartingY            : %d\n", psInfo->pDetails->StartingY);
}

STDAPI
_psdetails(IDebugClient *client, PCSTR args)
{
  if (strncmp(args, "-h", 2) == 0)
  {
    psdetails_help();
    return S_OK;
  }
  CComQIPtr<IDebugControl4> control(client);

  auto hr = control->GetWindbgExtensionApis64(&ExtensionApis);
  if (FAILED(hr))
  {
    return hr;
  }

  IDebugDataSpaces4 *dataspaces = nullptr;
  hr = client->QueryInterface(__uuidof(IDebugDataSpaces4),
                              (void **)&dataspaces);
  if (hr != S_OK)
  {
    return E_FAIL;
  }
  LoadRequirements((std::basic_string_view<char>)PSLIST);

  ULONG bytesRead = 0x0;

  for (auto &process : processList)
  {
    u64 current = process.Eprocess;
    dataspaces->ReadVirtual(current + EProcessFieldOffset("ProcessQuotaUsage"), &process.ProcessQuotaUsage, sizeof(&process.ProcessQuotaUsage), &bytesRead);
    dataspaces->ReadVirtual(current + EProcessFieldOffset("ProcessQuotaPeak"), &process.ProcessQuotaPeak, sizeof(&process.ProcessQuotaPeak), &bytesRead);
    dataspaces->ReadVirtual(current + EProcessFieldOffset("SectionObject"), &process.SectionObject, sizeof(&process.SectionObject), &bytesRead);
    dataspaces->ReadVirtual(current + EProcessFieldOffset("SectionBaseAddress"), &process.SectionBaseAddress, sizeof(&process.SectionBaseAddress), &bytesRead);
    dataspaces->ReadVirtual(current + EProcessFieldOffset("Cookie"), &process.Cookie, sizeof(&process.Cookie), &bytesRead);
    dataspaces->ReadVirtual(current + EProcessFieldOffset("Token"), &process.Token, sizeof(&process.Token), &bytesRead);
    Log("\n");
    DumpPsInfoEProcess(control, &process);
    SwitchProcessContext(client, control, process.Eprocess);
    ReadPsDetails(dataspaces, control, &process);
    ;
  }
  dataspaces->Release();
  return S_OK;
}
