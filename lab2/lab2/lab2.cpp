#include <Windows.h>
#include <tchar.h>
#include "defs.h"
#include <string.h>

#define PATH_LENGTH 260


void replace(char* text, char oldChar, char newChar) {
	while (*text != '\0') {
		if (*text == oldChar) {
			*text = newChar;
		}
		text++;
	}
}


DWORD writeHiveToFile(HKEY hKey, LPCSTR fileName) {
	DWORD subKeys;
	DWORD maxSubKeyLen;
	FILETIME fileTime;

	LSTATUS status = RegQueryInfoKey(hKey, NULL, NULL, NULL, &subKeys, &maxSubKeyLen, NULL, NULL, NULL, NULL, NULL, &fileTime);
	CHECK(status == ERROR_SUCCESS, 1, "Error at RegQueryInfoKey\n");

	SYSTEMTIME systemTime;
	CHECK(FileTimeToSystemTime(&fileTime, &systemTime) != 0, 1, "Error at FileTimeToSystemTime\n");

	
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, 1, "An error has occurred at CreateFile in writeToFile\n");

	char text[1000];
	sprintf_s(text, "SubKeys: %d\nMaxSubKeyLen: %d\nLastWriteTime: %d/%d/%d %d:%d:%d", 
		subKeys, maxSubKeyLen, systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wSecond, systemTime.wMilliseconds);
	DWORD bytesWritten;
	CHECK(WriteFile(hFile, text, strlen(text), &bytesWritten, NULL) == TRUE, 1, "An error has occurred at WriteFile\n", CloseHandle(hFile));


	CloseHandle(hFile);
	return 0;
}


DWORD recursiveFind(LPCSTR dirName, LPCSTR writeTo) {
	WIN32_FIND_DATA findFileData;
	char filesPath[PATH_LENGTH];
	strcpy_s(filesPath, dirName);
	strcat_s(filesPath, "\\*");

	HANDLE hFind = FindFirstFile(filesPath, &findFileData);
	CHECK(hFind != INVALID_HANDLE_VALUE, 1, "An error has occurred at FindFirstFile\n");

	HANDLE hFile = CreateFile(writeTo, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CHECK(hFile != INVALID_HANDLE_VALUE, 1, "An error has occurred at CreateFile in writeToFile\n", FindClose(hFind));
	char text[1000];

	do {
		if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;
		}

		char fileFullName[PATH_LENGTH];
		sprintf_s(fileFullName, "%s\\%s", dirName, findFileData.cFileName);

		if (_tcscmp(fileFullName, writeTo) == 0) {
			continue;
		}

		ULONGLONG fileSize = (ULONGLONG)(findFileData.nFileSizeHigh * MAXDWORD) + findFileData.nFileSizeLow;
		sprintf_s(text, "File path: %s; FileSize: %I64u bytes\n", fileFullName, fileSize);

		DWORD bytesWritten;
		BOOL writeRes = WriteFile(hFile, text, strlen(text), &bytesWritten, NULL);
		CHECK(writeRes == TRUE, 1, "An error has occurred at WriteFile\n", CloseHandle(hFile), FindClose(hFind));

	} while (FindNextFile(hFind, &findFileData));


	CloseHandle(hFile);
	FindClose(hFind);
	return 0;
}


DWORD makeFilesForHives(HKEY hKey, LPCSTR subKeyName, LPCSTR dirPath) {
	HKEY openedHKey;
	CHECK(RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &openedHKey) == ERROR_SUCCESS, -1, "Error at RegOpenKeyEx\n");

	DWORD subKeys;
	DWORD maxSubKeyLen;

	LSTATUS status = RegQueryInfoKey(openedHKey, NULL, NULL, NULL, &subKeys, &maxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);
	CHECK(status == ERROR_SUCCESS, -1, "Error at RegQueryInfoKey\n", RegCloseKey(openedHKey));

	char* fileName = new char[maxSubKeyLen + 1];
	DWORD nameLen = maxSubKeyLen + 1;
	char fileFullName[PATH_LENGTH];

	for (DWORD i = 0; i < subKeys; ++i) {
		status = RegEnumKeyEx(openedHKey, i, fileName, &nameLen, 0, 0, 0, 0);
		CHECK(status == ERROR_SUCCESS, -1, "Error at RegEnumKeyEx\n", RegCloseKey(openedHKey), delete[] fileName);

		fileName[nameLen] = '\0';

		sprintf_s(fileFullName, "%s\\%s", dirPath, fileName);
		replace(fileFullName, '/', '_');

		HANDLE hFile = CreateFile(fileFullName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		CHECK(hFile != INVALID_HANDLE_VALUE, -1, "An error has occurred at CreateFile in writeToFile\n", RegCloseKey(openedHKey), delete[] fileName);
		CloseHandle(hFile);
		nameLen = maxSubKeyLen + 1;
	}
	delete[] fileName;

	RegCloseKey(openedHKey);
	return subKeys;
}


DWORD makeSubKey(LPCSTR newSubKeyName, HKEY hKey, LPCSTR pathDir, DWORD filesCreated)
{
	HKEY openedHKey;
	LONG result = RegCreateKeyEx(hKey, newSubKeyName, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &openedHKey, NULL);
	CHECK(result == ERROR_SUCCESS, 1, "Error at RegCreateKeyEx");

	DWORD valueDataSize = (DWORD)(strlen(pathDir) + 1);
	result = RegSetValueEx(openedHKey, "PathDir", 0, REG_SZ, (BYTE*)pathDir, valueDataSize);
	CHECK(result == ERROR_SUCCESS, 1, "Error at RegSetValueEx", RegCloseKey(openedHKey));


	valueDataSize = (DWORD)(sizeof(filesCreated));
	result = RegSetValueEx(openedHKey, "FilesCreatedNr", 0, REG_DWORD, (BYTE*)&filesCreated, valueDataSize);
	CHECK(result == ERROR_SUCCESS, 1, "Error at RegSetValueEx", RegCloseKey(openedHKey));

	RegCloseKey(openedHKey);
	return 0;
}


int main()
{
	LPCSTR parts[] = { "InstalledSoftware", "Rezultate" };
	char dirPath[PATH_LENGTH] = "C:\\Facultate\\CSSO\\Laboratoare\\Week2";

	for (auto part : parts) {
		char path[PATH_LENGTH];
		sprintf_s(path, "%s\\%s", dirPath, part);
		BOOL dirReturn = CreateDirectory(path, NULL);
		CHECK(dirReturn != 0 || GetLastError() == ERROR_ALREADY_EXISTS, 1, "An error has occurred at creating a directory!\n");
	}

	HKEY hives[] = {HKEY_LOCAL_MACHINE , HKEY_CURRENT_CONFIG, HKEY_CURRENT_USER};
	LPCSTR files[] = { "HKLM.txt", "HKCC.txt", "HKCU.txt" };

	strcat_s(dirPath, "\\Rezultate");
	char path[PATH_LENGTH];
	for (int i = 0; i < sizeof(hives) / sizeof(hives[0]); ++i) {
		sprintf_s(path, "%s\\%s", dirPath, files[i]);
		writeHiveToFile(hives[i], path);
	}

	sprintf_s(path, "%s\\%s", dirPath, "sumar.txt");
	recursiveFind(dirPath, path);

	LPCSTR pathDir = "C:\\Facultate\\CSSO\\Laboratoare\\Week2\\InstalledSoftware";
	DWORD filesCreated = makeFilesForHives(HKEY_CURRENT_USER, "Software", pathDir);

	makeSubKey("Software\\CSSO\\Week2", HKEY_CURRENT_USER, pathDir, filesCreated);

	return 0;
}