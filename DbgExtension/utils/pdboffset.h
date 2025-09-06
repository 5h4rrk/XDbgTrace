#pragma once
#include "dbgextension.h"

#define X(x) LPCSTR(x)
#define LDR_DATA_TABLE_ENTRY X("nt!_LDR_DATA_TABLE_ENTRY")
#define PSLIST X("pslist")
#define DLLLIST X("dlllist")

u64 EProcessFieldOffset(LPCSTR field)
{
  ULONG offset = 0x0;
  GetFieldOffset("nt!_EPROCESS", field, &offset);
  return offset;
}

u64 PebFieldOffset(LPCSTR field)
{
  ULONG offset = 0x0;
  GetFieldOffset("nt!_PEB", field, &offset);
  return offset;
}
u64 ProcessParamsFieldOffset(LPCSTR field)
{
  ULONG offset = 0x0;
  GetFieldOffset("nt!_RTL_USER_PROCESS_PARAMETERS", field, &offset);
  return offset;
}

u64 FieldOffset(LPCSTR type, LPCSTR field)
{
  ULONG offset = 0x0;
  GetFieldOffset(type, field, &offset);
  return offset;
}

u64 EThreadFieldOffset(LPCSTR field)
{
	ULONG offset = 0x0;
    GetFieldOffset("nt!_ETHREAD", field, &offset);
    return offset;
}
