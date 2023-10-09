#include <Windows.h>
#include <tchar.h>
#include "defs.h"
#include <string.h>

#define PATH_LENGTH 260

DWORD checkText(LPCSTR fileName, LPCSTR text) {
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	CHECK(hFile != INVALID_HANDLE_VALUE, "An error has occurred at CreateFile in checkText\n", printf("FileName: %s\n", fileName));

	DWORD bytesRead = 0;
	char* buffer = new char[0x4001];
	CHECK(ReadFile(hFile, buffer, 0x4000, &bytesRead, NULL) == TRUE, "An error has occurred at ReadFile\n", CloseHandle(hFile), delete[] buffer);
	CloseHandle(hFile);

	DWORD result = -1;

	if (bytesRead > 0) {
		buffer[bytesRead - 1] = '\0';
		char* found = strstr(buffer, text);
		result = found == NULL ? -1 : (found - buffer);
	}

	delete[] buffer;
	return result;
}

BOOL writeToFile(LPCSTR fileName, LPCSTR text) {
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	CHECK(hFile != INVALID_HANDLE_VALUE, "An error has occurred at CreateFile in writeToFile\n");

	SetFilePointer(hFile, 0, NULL, FILE_END);

	DWORD bytesWritten;
	CHECK(WriteFile(hFile, text, strlen(text), &bytesWritten, NULL) == TRUE, "An error has occurred at WriteFile\n", CloseHandle(hFile));
	
	CloseHandle(hFile);
}


BOOL recursiveFind(LPCSTR dirName, const char* text) {
	WIN32_FIND_DATA FindFileData;
	char filesPath[PATH_LENGTH];
	strcpy_s(filesPath, dirName);
	strcat_s(filesPath, "\\*");

	HANDLE hFind  = FindFirstFile(filesPath, &FindFileData);
	CHECK(hFind != INVALID_HANDLE_VALUE, "An error has occurred at FindFirstFile\n");

	do{
		if (_tcscmp(FindFileData.cFileName, ".") == 0 || _tcscmp(FindFileData.cFileName, "..") == 0) {
			continue;
		}
		char fileFullName[PATH_LENGTH];
		sprintf_s(fileFullName, "%s\\%s", dirName, FindFileData.cFileName);

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			recursiveFind(fileFullName, text);
		}
		else {
			DWORD result = checkText(fileFullName, text);
			if (result != -1) {
				char text[400];
				sprintf_s(text, "%s, pe pozitia: %d\n", fileFullName, result);
				writeToFile("C:\\Facultate\\CSSO\\Laboratoare\\Week1\\Rezultate\\sumar.txt", text);
			}
		}

	} while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);
}


int main() {
	LPCSTR parts[] = { "Facultate", "CSSO", "Laboratoare", "Week1", "Rezultate"};
	char createDirPath[PATH_LENGTH] = "C:";

	for (auto part : parts) {
		strcat_s(createDirPath, "\\");
		strcat_s(createDirPath, part);
		BOOL dirReturn = CreateDirectory(createDirPath, NULL);
		CHECK(dirReturn != 0 || GetLastError() == ERROR_ALREADY_EXISTS, "An error has occurred at creating a directory!");
	}

	char dirPath[PATH_LENGTH];
	printf("Directory> ");
	scanf_s("%99[^\n]%*c", dirPath, (unsigned long)sizeof(dirPath));

	char* text = new char[0x4000];
	printf("Text (max 0X4000 characters)> ");
	scanf_s("%s", text, (unsigned long)sizeof(text));

	recursiveFind(dirPath, text);

	delete[] text;
	return 0;
}