# Auto-Elevate

This tool demonstrates the power of UAC bypasses and built-in features of Windows. This utility auto-locates `winlogon.exe`, steals and impersonates it's process TOKEN, and spawns a new SYSTEM-level process with the stolen token. Combined with UAC bypass method #41 (ICMLuaUtil UAC bypass) from hfiref0x's UACME utility, this utility can auto-elevate a medium integrity user to NT AUTHORITY\SYSTEM using built-in Windows features.

----

The following image demonstrates using UACME combined with Auto-Elevate to go from a medium-integrity user to NT AUTHORITY\SYSTEM on Windows 10 21H1.

![image](https://user-images.githubusercontent.com/54753063/146302977-d5c7fe2d-2c25-43e7-a09e-d5ef98670913.png)

Here is an example of executing Auto-Elevate from an Administrator level to escalate to SYSTEM

![image](https://user-images.githubusercontent.com/54753063/146302621-2b4eaa92-f84c-4b15-b044-d85ebfdb3edb.png)


## Technical Explanation

The following steps are performed by Auto-Elevate to escalation from Administrator to SYSTEM, combining Auto-Elevate with UACME can automate the user to Admin escalation process:

![image](https://user-images.githubusercontent.com/54753063/146303452-5743a5dc-1239-4df9-8353-91f3fd9bb4a0.png)
  
1.  The winlogon.exe process is located by enumerating the systems running processes with CreateToolhelp32Snapshot, Process32First, and Process32Next
2.  SeDebugPrivilege is enabled for the current process via a call to AdjustTokenPrivileges, as it's required to open a HANDLE to winlogon.exe
3.  A handle to the winlogon.exe process is opened by calling OpenProcess, for this call PROCESS\_ALL\_ACCESS is used (however, it's overkill)
4.  A handle to winlogon's process token is retrieved by calling OpenProcessToken combined with the previously obtained process handle 
5.  The user (SYSTEM) of winlogon is impersonated by calling ImpersonateLoggedOnUser
6.  The impersonated token handle is duplicated by calling DuplicateTokenEx with SecurityImpersonation, this creates a duplicated token we can use
7.  Using the duplicated, and impersonated token a new CMD instance is spawned by calling CreateProcessWithTokenW

## To-Do

- [ ] Implement a standalone version of method 41 from UACME to automate the process further
