# XdbgTrace

It is a basic WinDbg extension that allows you to inspect process details. It offers a collection of commands for inspecting processes and their related modules:

- `pslist`
- `dlllist`
- `psdetails`
- `threads`

> **Note:** This has only been tested on **Windows 10 (10.0.19041.1)**.It may not work as expected on other versions.

Download the [DbgExtension.dll](x64/Release/DbgExtension.dll)

#### *Analyzing Memory dump files* 

- ***!pslist*** : This command will list down the all running process by parsing `_EPROCESS` structure. 

***Output***

```bash
0: kd> !pslist
Process            Process Name           PID    PPID   Threads		VS(MB)		PVS(MB)              CreateTime(UTC+0)
=====================================================================================================================
0xffffc509b8884040  System                4      2        164      3       		14      		2024-10-12 12:19:30.253             
0xffffc509b88cb080  Registry              108    6        4        90      		125     		2024-10-12 12:19:18.884             
0xffffc509baaf4040  smss.exe              344    6        2        2101287 		2101295 		2024-10-12 12:19:30.257             
0xffffc509ba45c140  csrss.exe             456    446      10       2101335 		2101337 		2024-10-12 12:19:31. 1              
0xffffc509bc17d080  wininit.exe           532    446      2        2101314 		2101326 		2024-10-12 12:19:31.135             
0xffffc509bc16f080  csrss.exe             540    526      13       2101363 		2101371 		2024-10-12 12:19:31.140               
......
......
```

- ***!dlllist***: This command will list all loaded DLL files for each process. It enumerates the `_PEB` structure of each process, accesses the `_PEB_LDR_DATA`, and traverses the `InLoadOrderModuleList` to retrieve loaded modules.

***Output***

```bash
0: kd> !dlllist
PID       	ProcessName         	DllName                                                               	ImageSize       	DLLPath                                                                                                                         	ImageBase             	RefCnt  	LoadReason          
624       	winlogon.exe        	ntdll.dll                                                             	0x1f8000        	C:\Windows\SYSTEM32\ntdll.dll                                                                                                   	0x7ffe5edb0000        	2       	StaticDependency    
624       	winlogon.exe        	KERNEL32.DLL                                                          	0xbf000         	C:\Windows\System32\KERNEL32.DLL                                                                                                	0x7ffe5e530000        	2       	DynamicLoad         
624       	winlogon.exe        	KERNELBASE.dll                                                        	0x2f6000        	C:\Windows\System32\KERNELBASE.dll                                                                                              	0x7ffe5ca70000        	1       	StaticDependency    
624       	winlogon.exe        	msvcrt.dll                                                            	0x9e000         	C:\Windows\System32\msvcrt.dll                                                                                                  	0x7ffe5cef0000        	2       	StaticDependency    
624       	winlogon.exe        	sechost.dll                                                           	0x9c000         	C:\Windows\System32\sechost.dll                                                                                                 	0x7ffe5e430000        	2       	StaticDependency    
624       	winlogon.exe        	RPCRT4.dll                                                            	0x126000        	C:\Windows\System32\RPCRT4.dll                                                                                                  	0x7ffe5cf90000        	2       	StaticDependency    
624       	winlogon.exe        	combase.dll                                                           	0x354000        	C:\Windows\System32\combase.dll                                                                                                 	0x7ffe5d550000        	2       	StaticDependency    
624       	winlogon.exe        	ucrtbase.dll                                                          	0x100000        	C:\Windows\System32\ucrtbase.dll                                                                                                	0x7ffe5c5f0000        	2       	StaticDependency    
624       	winlogon.exe        	advapi32.dll                                                          	0xaf000         	C:\Windows\System32\advapi32.dll                                                                                                	0x7ffe5ddf0000        	2       	StaticDependency    
.....
.....
```

***!psdetails***: This command will show slight detailed information about the processes like `Eprocess`, `Cookie`, `Token`, `CommandLine`, `WindowTitle`, `DesktopInfo` etc.

***Output***

```bash
0: kd> !psdetails
Loaded Requirements

......
.......
	Process              : mspaint.exe
	PID                  : 2144
	EProceess            : ffffc509bfeee080
	Cookie               : 0x104a1bfd
	Token                : ffff9901126a35f2
	SectionObject        : ffff99010fc06770
	SectionBaseAddress   : 00007ff7f56e0000
	ProcessQuotaUsage    : 000001d681bd3818
	ProcessQuotaPeak     : 000001d681bd3828
	CurrDirHandle        : 0000000000000040
	CommandLine          : "C:\Windows\system32\mspaint.exe" 
	WindowTitle          : C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Accessories\Paint.lnk
	DesktopInfo          : Winsta0\Default
	StartingX            : 0
	StartingY            : 0
```

***!threads***: This command will display active threads of each process with their StartAddress, StartTime, ExitTime, TID and so on.

***Output***

```bash   
0: kd> !threads
ETHREAD                 PID      TID StartAddress        ProcessName              CreateTime(UTC)          ExitTime(UTC)           
ffffc509b887c040          4       12 fffff8071e3e35e0    System                   1601-1 -1  0 :0 :0 .306      -                   
ffffc509b8879080          4       16 fffff8071e3b44d0    System                   1601-1 -1  0 :0 :0 .306      -                   
ffffc509b889c400          4       20 fffff8071e3b44d0    System                   1601-1 -1  0 :0 :0 .306      -                   
ffffc509b8885040          4       24 fffff8071e7e8ca0    System                   1601-1 -1  0 :0 :0 .306      -                   
ffffc509b8897340          4       28 fffff8071e7bea90    System                   2024-10-12 12:19:17.754      -                   
ffffc509b89d6140          4       36 fffff8071e3cdf60    System                   2024-10-12 12:19:17.820      -                   
ffffc509b89da140          4       40 fffff8071e3cc110    System                   2024-10-12 12:19:17.821      -        
[[.....]]
[[.....]]
ffffc509c1f2b080       1372     7440 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.528      -                   
ffffc509bd3d5080       1372     8088 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.529      -                   
ffffc509bde32080       1372     8000 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.529      -                   
ffffc509bc1e3080       1372     8104 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.542      -                   
ffffc509b891c080       1372     2348 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.542      -                   
ffffc509c1dd6080       1372     2924 7ffe5ee02680        conhost.exe              2024-10-12 12:57:57.578      -                   
ffffc509c1bb3040       1372     5772 7ffe5ee02680        conhost.exe              2024-10-12 12:57:58.120      -                   

```

***!sessions***: This command will print all processes with the session id to which it belongs to. It pulls details like `SessionObject`, `SessionHandle`, `IoState`, `SessionId` & `CreateTime` from `_MM_SESSION_SPACE` structure.

```bash
0: kd> !sessions
Loaded Requirements
 SessionId          ProcessName      SessionObject      SessionHandle              IoState         CreateTime
         0               System                  0                  0                 Null                    0
         0             Registry                  0                  0                 Null                    0
         0             smss.exe                  0                  0                 Null                    0
         0            csrss.exe   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         0          wininit.exe   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         1            csrss.exe   ffffa607d3f39d00   ffffffff800005f4            Connected              5b1e7a3
         0         services.exe   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         1         winlogon.exe   ffffa607d3f39d00   ffffffff800005f4            Connected              5b1e7a3
         0            lsass.exe   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
....
....
         0          svchost.exe   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         0       SearchProtocol   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         0       SearchFilterHo   ffffa607d3d516d0   ffffffff80000530         Disconnected              58a5ebb
         2           DumpIt.exe   ffffa607d6b34c80   ffffffff80001af0             LoggedOn              dbd2e9f
         2          conhost.exe   ffffa607d6b34c80   ffffffff80001af0             LoggedOn              dbd2e9f

```

***!envars***: This command retrieves the environment variables of each process by parsing `_PEB->ProcessParameters`, which is defined by the _RTL_USER_PROCESS_PARAMETERS structure containing fields like `Environment`, `EnvironmentSize`, and more.

```bash
...
...

  Process=OneDrive.exe
  PPID=0x174(372)
  PID=0x3b4(948)
  Peb=0x000000384083a000
  Params=0x000001d8b38b22f0
  Environment=0x000001d8b38b0fe0
  EnvironmentSize=0x1306
	=::=::\
	ALLUSERSPROFILE=C:\ProgramData
	APPDATA=C:\Users\testvm\AppData\Roaming
	CLIENTNAME=TESTVM
	CommonProgramFiles=C:\Program Files\Common Files
	CommonProgramFiles(x86)=C:\Program Files (x86)\Common Files
	CommonProgramW6432=C:\Program Files\Common Files
	COMPUTERNAME=DESKTOP-FPGOBQG
	ComSpec=C:\Windows\system32\cmd.exe
	DriverData=C:\Windows\System32\Drivers\DriverData
	FPS_BROWSER_APP_PROFILE_STRING=Internet Explorer
	FPS_BROWSER_USER_PROFILE_STRING=Default
	HOMEDRIVE=C:
	HOMEPATH=\Users\testvm
	LOCALAPPDATA=C:\Users\testvm\AppData\Local
	LOGONSERVER=\\DESKTOP-FPGOBQG
	NUMBER_OF_PROCESSORS=2
	OneDrive=C:\Users\testvm\OneDrive
	OS=Windows_NT
	Path=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\;C:\SysinternalsSuite;C:\Program Files (x86)\Microsoft SQL Server\160\Tools\Binn\;C:\Program Files\Microsoft SQL Server\160\Tools\Binn\;C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\;C:\Program Files\Microsoft SQL Server\160\DTS\Binn\;C:\Program Files\010 Editor;C:\Users\testvm\AppData\Local\Programs\Python\Python313\Scripts\;C:\Users\testvm\AppData\Local\Programs\Python\Python313\;C:\Users\testvm\AppData\Local\Programs\Python\Launcher\;C:\Users\testvm\AppData\Local\Microsoft\WindowsApps;C:\Users\testvm\AppData\Local\Programs\Microsoft VS Code\bin;C:\SysinternalsSuite;
	PATHEXT=.COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC
	PROCESSOR_ARCHITECTURE=AMD64
	PROCESSOR_IDENTIFIER=Intel64 Family 6 Model 140 Stepping 1, GenuineIntel
	PROCESSOR_LEVEL=6
	PROCESSOR_REVISION=8c01
	ProgramData=C:\ProgramData
	ProgramFiles=C:\Program Files
	ProgramFiles(x86)=C:\Program Files (x86)
	ProgramW6432=C:\Program Files
	PSModulePath=C:\Program Files\WindowsPowerShell\Modules;C:\Windows\system32\WindowsPowerShell\v1.0\Modules;C:\Program Files (x86)\Microsoft SQL Server\160\Tools\PowerShell\Modules\
	PUBLIC=C:\Users\Public
	QSG_RENDER_LOOP=basic
	QT_ACCESSIBLE_SORT_TREE_CHILDREN=1
	QT_ACCESSIBLE_TREEVIEW=1
	QT_QUICK_BACKEND=softwarecontext
	QT_SCALE_FACTOR=1
	SESSIONNAME=RDP-Tcp#1
	SystemDrive=C:
	SystemRoot=C:\Windows
	TEMP=C:\Users\testvm\AppData\Local\Temp
	TMP=C:\Users\testvm\AppData\Local\Temp
	USERDOMAIN=DESKTOP-FPGOBQG
	USERDOMAIN_ROAMINGPROFILE=DESKTOP-FPGOBQG
	USERNAME=testvm
	USERPROFILE=C:\Users\testvm
	windir=C:\Windows
	....

```

***!help***: For help

```bash
0: kd> !help
DbgExtension Help
	supported commands
	process [flags] - Process Information
	dlllist - Dll List
	psdetails - displays detail info about process
	threads - dispalys active threads of each process
	sessions - dispalys all the process with the sessions
	envars - dumps the environment variables for all processes
```

```bash
0: kd> !pslist -h
Process [flags]
	-p: Sort by process id
	-a: Sort by Activethreads
	-v: Sort by VirtualSize
	-pvs: Sort by PeakVirtualSize
	-ct: Sort by CreationTime
	-n: Sort by ProcessName
	-nr: Sort by ProcessName reverse
```


