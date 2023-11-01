#include <Windows.h>
#include <tchar.h>
#include "defs.h"


BOOL setPrivilege(LPCSTR privilegeName) {
	LUID luid;
	CHECK(LookupPrivilegeValue(NULL, privilegeName, &luid), FALSE, "Error at LookupPrivilegeValue\n");

	HANDLE hToken;
	CHECK(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken), FALSE, "Error at OpenProcessToken\n");

	TOKEN_PRIVILEGES tokenPrivileges;
	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luid;
	tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	
	BOOL res = AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	CHECK(res, FALSE, "Error at AdjustTokenPrivileges\n", CloseHandle(hToken));
	
	CloseHandle(hToken);
	return TRUE;
}

DWORD WINAPI searchInFile(LPVOID lpThreadParameter)
{
	char* fileName = *(char**)lpThreadParameter;
	char* searchString = ((char**)lpThreadParameter)[1];
	
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, -1, "An error has occurred at CreateFile in searchInFile\n");

	DWORD fileSize = GetFileSize(hFile, NULL);
	CHECK(fileSize != INVALID_FILE_SIZE, -1, "An error has occurred at GetFileSize in searchInFile\n", CloseHandle(hFile));
	
	char* fileContent = new char[fileSize + 1];
	memset(fileContent, 0, fileSize + 1);
	CHECK(ReadFile(hFile, fileContent, fileSize, NULL, NULL), -1, "An error has occurred at ReadFile in searchInFile\n", CloseHandle(hFile));

	DWORD count = 0;
	char* p = fileContent;
	
	while (*p != '\0') {
		while (*p != '\n' && *p != '\0') {
			p++;
		}
		if (*p == '\n') {
			*p = '\0';
			
			for (int i = 0; i < strlen(fileContent) - strlen(searchString) + 1; i++) {
				if (searchString[0] == fileContent[i]) {
					for (int j = 1; j < strlen(searchString); j++) {
						if (searchString[j] != fileContent[i + j]) {
							break;
						}
						if (j == strlen(searchString) - 1) {
							count++;
						}
					}
				}
			}
			p++;
			fileContent = p;
		}
	}

	printf("%s: %d\n", fileName, count);
	return count;
}


int main() {
	setPrivilege(SE_SYSTEM_PROFILE_NAME);

	LPCSTR fileNames[3] = {
		"C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\fire.txt",
		"C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\module_process.txt",
		"C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\procese.txt"
	};

	HANDLE hThreads[3];
	LPCSTR* params[3];

	for (int i = 0; i < 3; i++) {
		params[i] = new LPCSTR[2]{ fileNames[i], "svchost.exe" };
		hThreads[i] = CreateThread(NULL, 0, searchInFile, params[i], 0, NULL);
		CHECK(hThreads[i] != NULL, -1, "Error at CreateThread\n");
	}

	WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);


	for (int i = 0; i < 3; i++) {
		CloseHandle(hThreads[i]);
		delete[] params[i];
	}
	
	printf("Sleep\n");
	Sleep(1000 * 1800);
	
	return 0;
}