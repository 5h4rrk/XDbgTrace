#pragma once
#include "utils/dbgextension.h"

#define MEGA_BYTES (1024 * 1024)
#define GIGA_BYTES (1024 * 1024 * 1024)
extern std::vector<ProcessInformation> processList;
BOOL parsepslist(BOOL flag, PCSTR args);

VOID displayBanner()
{
  Log("%-18s %-20s %-8s %s   %s\t\t%s\t\t%-20s %s\n", "Process", "Process Name",
      "  PID", "PPID", "Threads", "VS(MB)", "PVS(MB)", "CreateTime(UTC+0)");
  Log("=====================================================================================================================\n");
}

VOID SwitchProcessContext(IDebugClient *client, IDebugControl4 *control, u64 address)
{
  char cmd_prep[0x40];
  sprintf_s(cmd_prep, ".process /p %llx", address);
  control->Execute(DEBUG_OUTCTL_IGNORE, cmd_prep, DEBUG_EXECUTE_NOT_LOGGED);
}

std::string
ConvertLargeIntegerToUTC(LARGE_INTEGER largeInt)
{
  FILETIME ft;
  ft.dwLowDateTime = largeInt.LowPart;
  ft.dwHighDateTime = largeInt.HighPart;

  SYSTEMTIME stUtc;
  if (FileTimeToSystemTime(&ft, &stUtc))
  {
    return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}", stUtc.wYear,
                       stUtc.wMonth, stUtc.wDay, stUtc.wHour, stUtc.wMinute,
                       stUtc.wSecond, stUtc.wMilliseconds);

  }
  else
  {
    return std::format("    -    ");
  }
}

VOID displayEntries(BOOL flag)
{
  if (flag)
  {
    for (const auto &process : processList)
    {
      Log("0x%p  %-20s  %-6u %-6u   %-8d %-8d\t\t%-8d\t\t%-36s\n",
          process.Eprocess, process.process_name, process.PID, process.PPID,
          process.ActiveThreads, process.VirtualSize / (MEGA_BYTES),
          process.PeakVirtualSize / (MEGA_BYTES), ConvertLargeIntegerToUTC(process.CreateTime).c_str());
    }
  }
}

VOID check_args(PCSTR args)
{

  if (processList.empty())
  {
    parsepslist(FALSE, args);
  }

  if (strcmp(args, "-p") == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return a.PID < b.PID;
              });
  }
  else if (strncmp(args, "-a", 2) == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return a.ActiveThreads < b.ActiveThreads;
              });
  }
  else if (strncmp(args, "-v", 2) == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return a.VirtualSize < b.VirtualSize;
              });
  }
  else if (strncmp(args, "-pvs", 4) == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return a.PeakVirtualSize < b.PeakVirtualSize;
              });
  }
  else if (strncmp(args, "-ct", 3) == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return a.CreateTime.QuadPart < b.CreateTime.QuadPart;
              });
  }
  else if (strcmp(args, "-n") == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return strcmp(a.process_name, b.process_name) < 0;
              });
  }
  else if (strncmp(args, "-nr", 3) == 0)
  {
    std::sort(processList.begin(), processList.end(),
              [](const ProcessInformation &a, const ProcessInformation &b)
              {
                return strcmp(a.process_name, b.process_name) > 0;
              });
  }
}

BOOL parsepslist(BOOL flag, PCSTR args)
{
  if (!processList.empty())
  {
    processList.clear();
  }
  auto process_head = GetExpression("nt!PsActiveProcessHead");

  if (process_head == 0x0)
  {
    Log("Failed to locate nt!PsActiveProcessHead\n");
    return FALSE;
  }

  ULONG64 flink = 0;
  ULONG64 blink = 0;
  ULONG bytesRead = 0;
  ReadMemory(process_head, &flink, sizeof(flink), &bytesRead);

#if defined(KDEXT_64BIT)
  ReadMemory(process_head + 0x8, &blink, sizeof(blink), &bytesRead);
#else
  ReadMemory(process_head + 0x4, &blink, sizeof(blink), &bytesRead);
#endif

  // Aligining
  flink -= EProcessFieldOffset("ActiveProcessLinks");
  blink -= EProcessFieldOffset("ActiveProcessLinks");

  for (ULONG64 current = flink; current != process_head;)
  {
    ProcessInformation processInfo;
    InitProcessInformation(&processInfo);

    processInfo.Eprocess = current;
    ReadMemory(current + EProcessFieldOffset("UniqueProcessId"), &processInfo.PID, sizeof(&processInfo.PID), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("Peb"), &processInfo.Peb, sizeof(&processInfo.Peb), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("ActiveProcessLinks"), &processInfo.NextProcess, sizeof(&processInfo.NextProcess), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("ImageFileName"), processInfo.process_name, sizeof(processInfo.process_name), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("OwnerProcessId"), &processInfo.PPID, sizeof(&processInfo.PPID), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("ActiveThreads"), &processInfo.ActiveThreads, sizeof(&processInfo.ActiveThreads), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("VirtualSize"), &processInfo.VirtualSize, sizeof(&processInfo.VirtualSize), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("PeakVirtualSize"), &processInfo.PeakVirtualSize, sizeof(&processInfo.PeakVirtualSize), &bytesRead);
    ReadMemory(current + EProcessFieldOffset("CreateTime"), &processInfo.CreateTime, sizeof(&processInfo.CreateTime), &bytesRead);
    current = processInfo.NextProcess - 0x448;

    if (current == process_head || processInfo.NextProcess == 0x0 ||
        processInfo.PID == 0x0)
    {
      break;
    }
    processList.push_back(processInfo);
  }

  if (flag)
  {
    displayBanner();
    displayEntries(TRUE);
  }

  return TRUE;
}
