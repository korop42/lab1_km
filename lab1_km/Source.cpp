#include <windows.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc == 4) {
        std::cout << "Child process started. Args count: " << argc << std::endl;
        for (int i = 0; i < argc; i++) {
            std::cout << "Arg[" << i << "]: " << argv[i] << std::endl;
            
        }

        HANDLE hMutex = (HANDLE)_strtoui64(argv[1], NULL, 0);
        HANDLE hSemaphore = (HANDLE)_strtoui64(argv[2], NULL, 0);
        int processNumber = atoi(argv[3]);

        if (hMutex == NULL || hSemaphore == NULL) {
            std::cout << "Failed to retrieve handles." << std::endl;
            return 1;
        }

        WaitForSingleObject(hSemaphore, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        std::cout << "Process " << processNumber << " started." << std::endl;
        Sleep(2000);
        std::cout << "Process " << processNumber << " finished." << std::endl;

        ReleaseMutex(hMutex);
        ReleaseSemaphore(hSemaphore, 1, NULL);

        return 0;
    }
    HANDLE hMainMutex = CreateMutexA(NULL, FALSE, "Global\\MyUniqueMainProcessMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "Another instance is already running." << std::endl;
        return 1;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hMutex = CreateMutexA(&sa, FALSE, NULL);
    HANDLE hSemaphore = CreateSemaphoreA(&sa, 3, 3, NULL);

    PROCESS_INFORMATION pi[10];
    STARTUPINFOA si[10];

    char currentPath[MAX_PATH];
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);

    for (int i = 0; i < 10; i++) {
        ZeroMemory(&si[i], sizeof(STARTUPINFOA));
        si[i].cb = sizeof(STARTUPINFOA);
        ZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

        std::string cmd = "\"" + std::string(currentPath) + "\" " +
            std::to_string((uint64_t)hMutex) + " " +
            std::to_string((uint64_t)hSemaphore) + " " +
            std::to_string(i + 1);

        std::cout << "Starting child process: " << cmd << std::endl;

        BOOL success = CreateProcessA(
            NULL,
            (LPSTR)cmd.c_str(),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si[i],
            &pi[i]
        );

        if (!success) {
            std::cout << "Failed to create process " << i + 1 << ". Error: " << GetLastError() << std::endl;
        }
    }

    std::cout << "Main process waiting 5 seconds..." << std::endl;
    Sleep(5000);

    DWORD result;
    for (int i = 0; i < 10; i++) {
        result = WaitForSingleObject(pi[i].hProcess, 0);
        if (result == WAIT_OBJECT_0) {
            std::cout << "Process " << (i + 1) << " has finished." << std::endl;
        }
        else {
            std::cout << "Process " << (i + 1) << " is still running." << std::endl;
        }
    }

    WaitForMultipleObjects(10, (HANDLE*)(&pi[0].hProcess), TRUE, INFINITE);
    std::cout << "All child processes completed." << std::endl;

    for (int i = 0; i < 10; i++) {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }

    CloseHandle(hMutex);
    CloseHandle(hSemaphore);
    CloseHandle(hMainMutex);

    return 0;
}