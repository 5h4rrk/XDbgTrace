# TESTING

It explains how to use the `kd` command-line utility to load a Windows dump file and run exported commands from a custom debugger extension (`DbgExtension.dll`).

## Steps

### 1. Load the Dump File

Run the following command to open the `.dmp` file:

```bash
kdx64.exe -z C:\Users\testuser\OneDrive\Desktop\DESKTOP-FPGOBQG-20250802-065651.dmp
```

**NOTE**
- Make sure the dump file architecture matches the corresponding debugger binary.
- Since this dump is from an Intel (x64) system, we use `kdx64.exe`. For an x86 dump, the equivalent `kdx86.exe` binary should be used.

- Inside the debugger, load your extension:

```bash
    .load E:\DbgExtension\x64\Release\DbgExtension.dll
```

- To automate loading the module and executing the !sessions command in one step:

```bash
kdx64.exe -z C:\Users\testuser\OneDrive\Desktop\DESKTOP-FPGOBQG-20250802-065651.dmp ^
-c ".load E:\DbgExtension\x64\Release\DbgExtension.dll; !sessions"

```
