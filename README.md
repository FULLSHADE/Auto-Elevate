# Auto-Elevate

This tool demonstrates the power of UAC bypasses and built-in features of Windows. This utility auto-locates `winlogon.exe`, steals and impersonates it's process TOKEN, and spawns a new SYSTEM-level process with the stolen token. Combined with UAC bypass method #41 (ICMLuaUtil UAC bypass) from hfiref0x's UACME utility, this utility can auto-elevate a low privileged Administrative account to NT AUTHORITY\SYSTEM.

----

The following image demonstrates using UACME combined with Auto-Elevate to go from a low-privileged Administrator account to NT AUTHORITY\SYSTEM on Windows 10 21H1.

![image](https://user-images.githubusercontent.com/54753063/146399560-1572c819-da69-4a17-89b2-03b830586b00.png)

The following steps are performed by Auto-Elevate to escalation from a high-privileged Administrator account to SYSTEM without a UAC bypass

![image](https://user-images.githubusercontent.com/54753063/146398983-2a5ba807-dc72-4692-bbe2-44795d37df63.png)

## Technical Explanation

The following steps are performed by Auto-Elevate to escalate from a low-privileged Administrator to SYSTEM:

![image](https://user-images.githubusercontent.com/54753063/146405439-40ecc0ec-d3a0-48bb-b3a4-16de725b174f.png)
  
### UACME ICMLuaUtil Bypass

1. test

### Auto-Elevate

1.  The winlogon.exe process is located by enumerating the systems running processes with CreateToolhelp32Snapshot, Process32First, and Process32Next
2.  SeDebugPrivilege is enabled for the current process via a call to AdjustTokenPrivileges, as it's required to open a HANDLE to winlogon.exe
3.  A handle to the winlogon.exe process is opened by calling OpenProcess, for this call PROCESS\_ALL\_ACCESS is used (however, it's overkill)
4.  A handle to winlogon's process token is retrieved by calling OpenProcessToken combined with the previously obtained process handle 
5.  The user (SYSTEM) of winlogon is impersonated by calling ImpersonateLoggedOnUser
6.  The impersonated token handle is duplicated by calling DuplicateTokenEx with SecurityImpersonation, this creates a duplicated token we can use
7.  Using the duplicated, and impersonated token a new CMD instance is spawned by calling CreateProcessWithTokenW

## To-Do

- [ ] Implement a standalone version of method 41 from UACME (or similar) to automate the process further

## MITRE ATT&CK Mapping

- Token Manipulation: [T1134](https://attack.mitre.org/techniques/T1134/)
- Access Token Manipulation: Token Impersonation/Theft: [T1134.001](https://attack.mitre.org/techniques/T1134/001/)
- Access Token Manipulation: Create Process with Token: [T1134.002](https://attack.mitre.org/techniques/T1134/002/)
- Access Token Manipulation: Make and Impersonate Token: [T1134.003](https://attack.mitre.org/techniques/T1134/003/)
