#pragma once
#include "dbgextension.h"
#include "pslist.h"
#include "dlllist.h"

BOOL LoadRequirements(const std::string_view requirements)
{
  if (requirements.empty())
  {
    return FALSE;
  }
  auto value = requirements.data();
  if (strncmp(value, "pslist", 6) == 0)
  {
    parsepslist(FALSE, "");
  }
  Log(
      "Loaded Requirements\n");
  return TRUE;
}