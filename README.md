# WinDbg-Extension

It is a basic WinDbg extension that allows you to inspect process details. It offers a collection of commands for inspecting processes and their related modules:

- `pslist`
- `dlllist`
- `psdetails`

> **Note:** This has only been tested on **Windows 10 (10.0.19041.1)**.It may not work as expected on other versions.

Download the [DbgExtension.dll](x64/Release/DbgExtension.dll)

#### *Analyzing Memory dump files* 

- ***!pslist*** : This command will list down the all running process by parsing `_EPROCESS` structure. 

***Output***

```bash
: kd> !pslist
Process            Process Name           PID    PPID   Threads		VS(MB)		PVS(MB)              CreateTime
=====================================================================================================================
0x00000000b8884040  System                4      2        164      3       		14      		2024-10-12 12:19:30.253             
0x00000000b88cb080  Registry              108    6        4        90      		125     		2024-10-12 12:19:18.884             
0x00000000baaf4040  smss.exe              344    6        2        2101287 		2101295 		2024-10-12 12:19:30.257             
.........
.........
0x00000000c42ec0c0  conhost.exe           1372   3262     7        2101400 		2101400 		2024-10-12 12:57:57.508             
```

- ***dlllist***: This command will list all loaded DLL files for each process. It enumerates the `_PEB` structure of each process, accesses the `_PEB_LDR_DATA`, and traverses the `InLoadOrderModuleList` to retrieve loaded modules.

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

***psdetails***: This command will show slight detailed information about the processes like `Eprocess`, `Cookie`, `Token`, `CommandLine`, `WindowTitle`, `DesktopInfo` etc.

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

***help***: For help

```bash
0: kd> !help
DbgExtension Help
	supported commands
	process [flags] - Process Information
	dlllist - Dll List
	psdetails - displays detail info about process
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


