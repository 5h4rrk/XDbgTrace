#pragma once
#include <Windows.h>
#include <winternl.h>

typedef ULONG64 u64;
typedef ULONG32 u32;
typedef UINT16 u16;
typedef ULONG ul;
typedef UCHAR u8;

#define INVALID_OFFSET 0x0
#define ZERO_SIZE 0x0
#define DPRINTF(...) printf(__VA_ARGS__)
#pragma comment(lib, "ntdll.lib")

BOOL InitUnicodeString(_UNICODE_STRING *str)
{

  str->Length = 0;
  str->MaximumLength = 256;

  str->Buffer = (WCHAR *)VirtualAlloc(NULL, str->MaximumLength,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (str->Buffer == NULL)
  {
    return FALSE;
  }
  return TRUE;
}

BOOL InitAnsiString(_STRING *str)
{

  str->Length = 0;
  str->MaximumLength = 256;

  str->Buffer = (CHAR *)VirtualAlloc(NULL, str->MaximumLength,
                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (str->Buffer == NULL)
  {
    return FALSE;
  }
  return TRUE;
}

extern "C" BOOL
DecodeUnicodeString(_STRING dest, _UNICODE_STRING *source)
{
  if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&dest, source, TRUE)))
  {
    return TRUE;
  }
  return FALSE;
}

typedef union _LARGE_INT
{
  struct
  {
    DWORD LowPart;
    LONG HighPart;
  } DUMMYSTRUCTNAME;
  struct
  {
    DWORD LowPart;
    LONG HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INT;

typedef struct _PHYSICAL_MEMORY_RUN64
{
  u64 BasePage;
  u64 PageCount;
} PHYSICAL_MEMORY_RUN64, *PPHYSICAL_MEMORY_RUN64;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR64
{
  ul NumberOfRuns;
  u64 NumberOfPages;
  PHYSICAL_MEMORY_RUN64 Run[1];
} PHYSICAL_MEMORY_DESCRIPTOR64, *PPHYSICAL_MEMORY_DESCRIPTOR64;

typedef struct _DUMP_HEADER64
{
  ul Signature;
  ul ValidDump;
  ul MajorVersion;
  ul MinorVersion;
  u64 DirectoryTableBase;
  u64 PfnDataBase;
  u64 PsLoadedModuleList;
  u64 PsActiveProcessHead;
  ul MachineImageType;
  ul NumberProcessors;
  ul BugCheckCode;
  u64 BugCheckParameter1;
  u64 BugCheckParameter2;
  u64 BugCheckParameter3;
  u64 BugCheckParameter4;
  CHAR VersionUser[32];
  u64 KdDebuggerDataBlock;
  /*PKDDEBUGGER_DATA64 KdDebuggerDataBlock; */
  union
  {
    PHYSICAL_MEMORY_DESCRIPTOR64 PhysicalMemoryBlock;
    u8 PhysicalMemoryBlockBuffer[700];
  };
  u8 ContextRecord[3000];
  EXCEPTION_RECORD64 ExceptionRecord;
  ul DumpType;
  LARGE_INTEGER RequiredDumpSpace;
  FILETIME SystemTime;
  CHAR Comment[0x80]; // May not be present.
  FILETIME SystemUpTime;
  ul MiniDumpFields;
  ul SecondaryDataState;
  ul ProductType;
  ul SuiteMask;
  ul WriterStatus;
  u8 Unused1;
  u8 KdSecondaryVersion; // Present only for W2K3 SP1 and better
  u8 Unused[2];
  u8 _reserved0[4016];
} DUMP_HEADER64, *PDUMP_HEADER64;

typedef struct
{
  CHAR *DosPath;
  u64 Handle;
} _CURDIR;

typedef struct
{
  u16 Flags;
  u16 Length;
  u32 TimeStamp;
  STRING DosPath;
} _RTL_DRIVE_LETTER_CURDIR;

typedef struct
{
  u32 StartingX;
  u32 StartingY;
  _CURDIR CurrentDirectory;
  CHAR *CommandLine;
  u64 Environment;
  CHAR *WindowTitle;
  CHAR *DesktopInfo;
  _RTL_DRIVE_LETTER_CURDIR *CurrentDirectores;
} ProcessDetails;

typedef struct
{
  union
  {
    u64 Object;
    u64 RefCount;
    u64 Value;
  };
} _TOKEN;

typedef struct
{
  u64 Eprocess;
  u64 PID;
  u64 PPID;
  u64 VirtualSize;
  u64 PeakVirtualSize;
  u64 NextProcess;
  u64 ActiveThreads;
  LARGE_INTEGER CreateTime;
  u64 Peb;
  char process_name[16];
  u64 ProcessQuotaUsage[2];
  u64 ProcessQuotaPeak[2];
  u64 SectionObject;
  u64 SectionBaseAddress;
  u32 Cookie;
  u64 Token;
  ProcessDetails *pDetails;
} ProcessInformation;

void InitToken(_TOKEN *Token)
{
  Token->Object = 0x0;
  Token->RefCount = 0x0;
  Token->Value = 0x0;
}

extern "C" VOID
RtlInitDriveLetter(_RTL_DRIVE_LETTER_CURDIR *CurrentDirectories)
{
  for (int i = 0; i < 32; i++)
  {
    CurrentDirectories[i].Flags = 0x0;
    CurrentDirectories[i].Length = 0x0;
    CurrentDirectories[i].TimeStamp = 0x0;
    InitAnsiString(&CurrentDirectories[i].DosPath);
  }
}

extern "C" VOID InitProcessDetails(ProcessDetails *pDetails)
{
  pDetails->StartingX = 0;
  pDetails->StartingY = 0;
  pDetails->CommandLine = nullptr;
  // pDetails->Environment = nullptr;
  pDetails->WindowTitle = nullptr;
  pDetails->DesktopInfo = nullptr;

  pDetails->CurrentDirectores = (_RTL_DRIVE_LETTER_CURDIR *)VirtualAlloc(
      nullptr, 32 * sizeof(_RTL_DRIVE_LETTER_CURDIR), MEM_COMMIT | MEM_RESERVE,
      PAGE_READWRITE);

  if (pDetails->CurrentDirectores)
  {
    RtlInitDriveLetter(pDetails->CurrentDirectores);
  }
  pDetails->CurrentDirectory.Handle = 0x0;
  pDetails->CurrentDirectory.DosPath = 0x0;
}

extern "C" VOID InitCurrentDirectory(_CURDIR *CurrentDirectory)
{
  CurrentDirectory->Handle = 0x0;
  CurrentDirectory->DosPath = 0x0;
}

void InitProcessInformation(ProcessInformation *processInfo)
{
  processInfo->Eprocess = 0;
  processInfo->PID = 0;
  processInfo->PPID = 0;
  processInfo->VirtualSize = 0;
  processInfo->PeakVirtualSize = 0;
  processInfo->NextProcess = 0;
  processInfo->Peb = 0;
  processInfo->ActiveThreads = 0;
  processInfo->CreateTime.QuadPart = 0;
  memset(processInfo->process_name, 0, sizeof(processInfo->process_name));
  processInfo->ProcessQuotaUsage[0] = 0;
  processInfo->ProcessQuotaUsage[1] = 0;
  processInfo->ProcessQuotaPeak[0] = 0;
  processInfo->ProcessQuotaPeak[1] = 0;
  processInfo->SectionObject = 0;
  processInfo->SectionBaseAddress = 0;
  processInfo->Cookie = 0;
  processInfo->Token = 0;

  processInfo->pDetails =
      (ProcessDetails *)VirtualAlloc(nullptr, sizeof(ProcessDetails),
                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  if (processInfo->pDetails)
  {
    InitProcessDetails(processInfo->pDetails);
  }
}

void FreeProcessInformation(ProcessInformation *processInfo)
{
  if (processInfo->pDetails)
  {
    if (processInfo->pDetails->CurrentDirectores)
    {
      VirtualFree(processInfo->pDetails->CurrentDirectores, 0, MEM_RELEASE);
    }
    VirtualFree(processInfo->pDetails, 0, MEM_RELEASE);
  }
}

typedef struct
{
  u32 Length;
  u8 Initialized;
  u64 SsHandle;
  u64 InLoadOrderModuleList;
  u64 InMemoryOrderModuleList;
  u64 InInitializationOrderModuleList;
  u64 EntryInProgress;
  u8 ShutdownInProgress;
  u64 ShutdownThreadId;
} PEB_LDR_DATA_64;

typedef struct
{
  u64 Eprocess;
  u64 ImageBaseAddress;
  PEB_LDR_DATA_64 PebLdrData;
} PebInformation;

void InitPebInformation(PebInformation *PebInfo)
{
  PebInfo->Eprocess = 0x0;
  PebInfo->ImageBaseAddress = 0x0;
  memset(static_cast<void *>(&PebInfo->PebLdrData), 0, sizeof(PebInfo->PebLdrData));
}
