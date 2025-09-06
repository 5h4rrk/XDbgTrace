#pragma once
#include "utils/utils.h"
#include <DbgEng.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <atlcomcli.h>
#include "modules/help.h"
#include "external/magic_enum.h"

#if !defined(_HAS_CXX23)
  import std;
#else
  #include <vector>
  #include <string>
  #include <algorithm>
  #include <format>
#endif

#define Log dprintf
#include "utils/pdboffset.h"

#if defined(KDEXT_64BIT)
#define UNICODE_STRING_LENGTH_OFFSET u64(0x8)
#else
#define UNICODE_STRING_LENGTH_OFFSET u64(0x4)
#endif


 