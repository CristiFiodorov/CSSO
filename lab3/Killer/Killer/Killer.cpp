#include <Windows.h>
#include "defs.h"
#include <tlhelp32.h>

DWORD findPID(LPCSTR processName) {
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	CHECK(hProcessSnap != INVALID_HANDLE_VALUE, -1, "Error at CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)\n");

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	CHECK(Process32First(hProcessSnap, &pe32), -1, "Error at Process32First\n", CloseHandle(hProcessSnap));

	do
	{
		if (_stricmp(pe32.szExeFile, processName) == 0)
		{
			CloseHandle(hProcessSnap);
			return pe32.th32ProcessID;
		}
	} while (Process32Next(hProcessSnap, &pe32));


	CloseHandle(hProcessSnap);
	return -1;
}



int main()
{
	DWORD PID = findPID("Filter.exe");
	CHECK(PID != -1, 0, "Cannot find PID of Filter.exe\n");

	printf("Filter.exe has PID: %d\n", PID);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, PID);
    CHECK(hProcess != 0, -1, "Error at OpenProcess\n");

    if (TerminateProcess(hProcess, 1)) {
        printf("Process with PID %d was killed\n", PID);
    }
    else {
        printf("Process with PID %d cannot be killed\n", PID);
    }

    CloseHandle(hProcess);
}