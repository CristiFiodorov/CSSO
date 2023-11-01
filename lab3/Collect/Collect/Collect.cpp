#include <Windows.h>
#include <tchar.h>
#include "defs.h"
#include <string.h>
#include <tlhelp32.h>


DWORD writeProcessesToFile(LPCSTR fileName) {
	HANDLE hProcessSnap;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	CHECK(hProcessSnap != INVALID_HANDLE_VALUE, 0, "Error at CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)");


	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	CHECK(Process32First(hProcessSnap, &pe32), 0, "Error at Process32First", CloseHandle(hProcessSnap));
	
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, 0, "An error has occurred at CreateFile in writeProcessesToFile\n", CloseHandle(hProcessSnap));

	DWORD count = 0;
	do
	{
		char buffer[1000];
		sprintf_s(buffer, "ParentProcessId: %d; ProcessId: %d; SzExeFile: %s\n", pe32.th32ParentProcessID ,pe32.th32ProcessID, pe32.szExeFile);
		CHECK(WriteFile(hFile, buffer, strlen(buffer), NULL, NULL), 0, "Error at WriteFile", CloseHandle(hFile), CloseHandle(hProcessSnap));
		++count;
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hFile);
	CloseHandle(hProcessSnap);
	return count;
}


DWORD writeThreadsToFile(LPCSTR fileName) {
	HANDLE hThreadSnap;

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	CHECK(hThreadSnap != INVALID_HANDLE_VALUE, 0, "Error at CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)");


	THREADENTRY32 te32;
	te32.dwSize = sizeof(te32);
	CHECK(Thread32First(hThreadSnap, &te32), 0, "Error at Thread32First", CloseHandle(hThreadSnap));

	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, 0, "An error has occurred at CreateFile in writeThreadsToFile\n", CloseHandle(hThreadSnap));

	DWORD count = 0;
	do
	{
		char buffer[1000];
		sprintf_s(buffer, "ThreadId: %d; OwnerProcessId: %d\n", te32.th32ThreadID, te32.th32OwnerProcessID);
		CHECK(WriteFile(hFile, buffer, strlen(buffer), NULL, NULL), 0, "Error at WriteFile", CloseHandle(hFile), CloseHandle(hThreadSnap));
		++count;
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hFile);
	CloseHandle(hThreadSnap);
	return count;
}


BOOL writeModulesToFile(LPCSTR fileName) {
	HANDLE hModuleSnap;

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	CHECK(hModuleSnap != INVALID_HANDLE_VALUE, 0, "Error at CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0)");


	MODULEENTRY32 me32;
	me32.dwSize = sizeof(me32);
	CHECK(Module32First(hModuleSnap, &me32), 0, "Error at Module32First", CloseHandle(hModuleSnap));

	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, 0, "An error has occurred at CreateFile in writeModulesToFile\n", CloseHandle(hModuleSnap));

	DWORD count = 0;
	do
	{
		char buffer[1000];
		sprintf_s(buffer, "ModuleID: %d; ProcessId: %d; szModule: %s; szExePath: %s\n", 
			me32.th32ModuleID, me32.th32ProcessID, me32.szModule, me32.szExePath);
		CHECK(WriteFile(hFile, buffer, strlen(buffer), NULL, NULL), 0, "Error at WriteFile", CloseHandle(hFile), CloseHandle(hModuleSnap));
		++count;
	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hFile);
	CloseHandle(hModuleSnap);
	return count;
}


int main()
{
	LPCSTR parts[] = { "Week3", "ProcessInfo" };
	char dirPath[MAX_PATH] = "C:\\Facultate\\CSSO\\Laboratoare";

	for (auto part : parts) {
		sprintf_s(dirPath, "%s\\%s", dirPath, part);
		BOOL dirReturn = CreateDirectory(dirPath, NULL);
		CHECK(dirReturn != 0 || GetLastError() == ERROR_ALREADY_EXISTS, 1, "An error has occurred at creating a directory!\n");
	}

	char fileName[MAX_PATH];

	sprintf_s(fileName, "%s\\%s", dirPath, "procese.txt");
	DWORD processes = writeProcessesToFile(fileName);

	sprintf_s(fileName, "%s\\%s", dirPath, "fire.txt");
	DWORD threads = writeThreadsToFile(fileName);

	sprintf_s(fileName, "%s\\%s", dirPath, "module_process.txt");
	DWORD modules = writeModulesToFile(fileName);


	char buffer[1000];
	sprintf_s(buffer, "1. \"Module: %d\" \n2. \"Procese: %d\" \n3. \"Fire : %d\"\n", modules, processes, threads);

	HANDLE hMapping = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, "cssoh3basicsync");
	CHECK(hMapping != NULL, -1, "Error at OpenFileMappingA");

	void* fileMapP = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
	CHECK(fileMapP != NULL, -1, "Error at MapViewOfFile", CloseHandle(hMapping));
	memcpy(fileMapP, buffer, strlen(buffer));

	CloseHandle(hMapping);
	return 0;
}