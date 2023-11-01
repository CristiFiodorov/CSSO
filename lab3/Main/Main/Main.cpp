#include "defs.h"
#include <Windows.h>

DWORD runProcess(LPCSTR exePath, DWORD waitingTime) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    CHECK(CreateProcess(exePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi), -1, "Error at CreateProcess\n", printf("%d\n", GetLastError()));


    WaitForSingleObject(pi.hProcess, waitingTime);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}


int main()
{
    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, "cssoh3basicsync");
    CHECK(hMapping != NULL, -1, "Error at CreateFileMapping\n");

    CHECK(runProcess("external\\Collect.exe", INFINITE) == 0, -1, "Error at runProcess Collect.exe\n", CloseHandle(hMapping));

    LPSTR fileMapP = (char*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    CHECK(fileMapP != NULL, -1, "Error at MapViewOfFile", CloseHandle(hMapping));
    printf("%s\n", fileMapP);
    CloseHandle(hMapping);

    CHECK(runProcess("external\\Filter.exe", 0) == 0, -1, "Error at runProcess Filter.exe\n");
    CHECK(runProcess("external\\Killer.exe", INFINITE) == 0, -1, "Error at runProcess Killer.exe\n");
    return 0;
}