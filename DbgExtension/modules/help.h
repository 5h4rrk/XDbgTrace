#pragma once
#include "DbgEng.h"
#include <wdbgexts.h>

VOID _help()
{
  dprintf("DbgExtension Help\n");
  dprintf("\tsupported commands\n");
  dprintf("\tprocess [flags] - Process Information\n");
  dprintf("\tdlllist - Dll List\n");
  dprintf("\tpsdetails - displays detail info about process\n");
  dprintf("\tthreads - dispalys active threads of each process\n");
  dprintf("\tsessions - dispalys all the process with the sessions\n");
  dprintf("\tenvars - dumps the environment variables for all processes\n");
}

VOID _process_help()
{
  dprintf("Process [flags]\n");
  dprintf("\t-p: Sort by process id\n");
  dprintf("\t-a: Sort by Activethreads\n");
  dprintf("\t-v: Sort by VirtualSize\n");
  dprintf("\t-pvs: Sort by PeakVirtualSize\n");
  dprintf("\t-ct: Sort by CreationTime\n");
  dprintf("\t-n: Sort by ProcessName\n");
  dprintf("\t-nr: Sort by ProcessName reverse\n");
}

VOID psdetails_help()
{
  dprintf("!psdetails\n");
  dprintf("\t\tdisplays detailed information about the process\n");
}


VOID _threads_help(){
  dprintf("!threads\n");
  dprintf("\t\tdisplays the threads");
}