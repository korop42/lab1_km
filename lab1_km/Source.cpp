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

//
//#include <windows.h>
//#include <iostream>
//#include <string>
//#include <vector>
//#include <cstring>
//
//const char* SINGLE_INSTANCE_MUTEX = "Global\\SingleInstanceMutex";
//const char* SEMAPHORE_NAME = "Global\\WorkerLimitSemaphore";
//
//CRITICAL_SECTION criticalSection;
//
//void CheckError(const char* operationName) {
//    DWORD error = GetLastError();
//    if (error != ERROR_SUCCESS) {
//        LPVOID messageBuffer;
//        FormatMessageA(
//            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//            NULL,
//            error,
//            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//            (LPSTR)&messageBuffer,
//            0,
//            NULL
//        );
//        std::cerr << operationName << " failed with error: " << error << " - " << (char*)messageBuffer << std::endl;
//        LocalFree(messageBuffer);
//    }
//}
//
//std::wstring PrepareCommandLine(const std::wstring& exePath, const std::wstring& mode, HANDLE handle = NULL) {
//    if (handle) {
//        return exePath + L" " + mode + L" " + std::to_wstring(reinterpret_cast<UINT_PTR>(handle));
//    }
//    else {
//        return exePath + L" " + mode;
//    }
//}
//                                                                   
//bool CreateChildProcess(const std::wstring& commandLine, PROCESS_INFORMATION& pi) {
//    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
//
//    if (!CreateProcessW(
//        NULL,                       
//        const_cast<LPWSTR>(commandLine.c_str()),
//        NULL,                       
//        NULL,                       
//        TRUE,                       
//        0,                          
//        NULL,                      
//        NULL,                       
//        &si,                        
//        &pi                         
//    )) {
//        CheckError("CreateProcess");
//        return false;
//    }
//    return true;
//}
//
//void PerformCriticalOperation() {
//    EnterCriticalSection(&criticalSection);
//
//    std::cout << "������-�������: ��������� � �������� ������" << std::endl;
//    Sleep(1000);
//    std::cout << "������-�������: ��������� ������ � �������� ������" << std::endl;
//    Sleep(1000);
//    std::cout << "������-�������: ����� � �������� ������" << std::endl;
//
//    LeaveCriticalSection(&criticalSection);
//}
//
//int MainProcess() {
//    
//    SetConsoleCP(1251);
//    SetConsoleOutputCP(1251);
//    wchar_t exePath[MAX_PATH];
//    GetModuleFileNameW(NULL, exePath, MAX_PATH);
//
//    std::wcout << L"���� �� ������������ �����: " << exePath << std::endl;
//
//    HANDLE singleInstanceMutex = CreateMutexA(NULL, TRUE, SINGLE_INSTANCE_MUTEX);
//    if (singleInstanceMutex == NULL) {
//        CheckError("CreateMutex");
//        return 1;
//    }
//
//    if (GetLastError() == ERROR_ALREADY_EXISTS) {
//        std::cout << "�������� ��� ��������, ��������� ���� ���� �������." << std::endl;
//        CloseHandle(singleInstanceMutex);
//        return 1;
//    }
//
//    std::cout << "�������� ������ �������� (PID: " << GetCurrentProcessId() << ")" << std::endl;
//
//    SECURITY_ATTRIBUTES sa;
//    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//    sa.lpSecurityDescriptor = NULL;
//    sa.bInheritHandle = TRUE;
//
//    HANDLE inheritedMutex = CreateMutexA(&sa, FALSE, NULL);
//    if (inheritedMutex == NULL) {
//        CheckError("CreateMutex (anonymous)");
//        CloseHandle(singleInstanceMutex);
//        return 1;
//    }
//
//    PROCESS_INFORMATION piChild;
//    std::wstring childCmd = PrepareCommandLine(exePath, L"child", inheritedMutex);
//    std::wcout << L"������ �������-������� � ��������� ������: " << childCmd << std::endl;
//
//    if (!CreateChildProcess(childCmd, piChild)) {
//        CloseHandle(inheritedMutex);
//        CloseHandle(singleInstanceMutex);
//        return 1;
//    }
//
//    HANDLE semaphore = CreateSemaphoreA(NULL, 3, 3, SEMAPHORE_NAME);
//    if (semaphore == NULL) {
//        CheckError("CreateSemaphore");
//        CloseHandle(piChild.hProcess);
//        CloseHandle(piChild.hThread);
//        CloseHandle(inheritedMutex);
//        CloseHandle(singleInstanceMutex);
//        return 1;
//    }
//
//    std::vector<PROCESS_INFORMATION> workerProcesses;
//    for (int i = 0; i < 10; i++) {
//        PROCESS_INFORMATION pi;
//        std::wstring workerCmd = PrepareCommandLine(exePath, L"worker", NULL) + L" " + std::to_wstring(i + 1);
//        std::wcout << L"������ �������� ������� �" << (i + 1) << L": " << workerCmd << std::endl;
//
//        if (CreateChildProcess(workerCmd, pi)) {
//            workerProcesses.push_back(pi);
//        }
//    }
//
//    HANDLE timer = CreateWaitableTimerA(NULL, TRUE, NULL);
//    if (timer == NULL) {
//        CheckError("CreateWaitableTimer");
//    }
//    else {
//        LARGE_INTEGER dueTime;
//        dueTime.QuadPart = -50000000LL;
//
//        if (!SetWaitableTimer(timer, &dueTime, 0, NULL, NULL, FALSE)) {
//            CheckError("SetWaitableTimer");
//        }
//        else {
//            std::cout << "������� 5 ������..." << std::endl;
//
//            WaitForSingleObject(timer, INFINITE);
//
//            std::cout << "\n�������� ����� �������-�������:" << std::endl;
//            for (size_t i = 0; i < workerProcesses.size(); i++) {
//                DWORD exitCode = 0;
//                DWORD waitResult = WaitForSingleObject(workerProcesses[i].hProcess, 0);
//
//                if (waitResult == WAIT_OBJECT_0) {
//                    GetExitCodeProcess(workerProcesses[i].hProcess, &exitCode);
//                    std::cout << "������ " << (i + 1) << " ��������� (���: " << exitCode << ")" << std::endl;
//                }
//                else {
//                    std::cout << "������ " << (i + 1) << " ��� �� ����������" << std::endl;
//                }
//            }
//        }
//        CloseHandle(timer);
//    }
//
//    std::cout << "\n������� ���������� ��� �������..." << std::endl;
//    for (auto& pi : workerProcesses) {
//        WaitForSingleObject(pi.hProcess, INFINITE);
//        CloseHandle(pi.hProcess);
//        CloseHandle(pi.hThread);
//    }
//
//    WaitForSingleObject(piChild.hProcess, INFINITE);
//    CloseHandle(piChild.hProcess);
//    CloseHandle(piChild.hThread);
//
//    CloseHandle(semaphore);
//    CloseHandle(inheritedMutex);
//    CloseHandle(singleInstanceMutex);
//
//    std::cout << "�������� ������ ���������." << std::endl;
//    return 0;
//}
//
//int ChildProcess(HANDLE inheritedMutex) {
//    std::cout << "������-������� �������� (PID: " << GetCurrentProcessId() << ")" << std::endl;
//
//    if (inheritedMutex == NULL) {
//        std::cerr << "�������: ���������� �'������ �� ��������!" << std::endl;
//        return 1;
//    }
//
//    std::cout << "������-�������: �������� ���������� �'������ " << inheritedMutex << std::endl;
//
//    DWORD waitResult = WaitForSingleObject(inheritedMutex, 0);
//    if (waitResult == WAIT_FAILED) {
//        CheckError("WaitForSingleObject (inherited mutex)");
//        return 1;
//    }
//
//    InitializeCriticalSection(&criticalSection);
//
//    HANDLE eventHandle = CreateEventA(NULL, TRUE, FALSE, NULL);
//    if (eventHandle == NULL) {
//        CheckError("CreateEvent");
//        DeleteCriticalSection(&criticalSection);
//        CloseHandle(inheritedMutex);
//        return 1;
//    }
//
//    std::cout << "������-�������: ���������� �� �'�����..." << std::endl;
//
//    waitResult = WaitForSingleObject(inheritedMutex, INFINITE);
//    if (waitResult == WAIT_OBJECT_0) {
//        std::cout << "������-�������: �'����� ��������" << std::endl;
//
//        PerformCriticalOperation();
//
//        std::cout << "������-�������: ������������ ��䳿 � ���������� ����" << std::endl;
//        SetEvent(eventHandle);
//
//        HANDLE waitHandles[2] = { inheritedMutex, eventHandle };
//        std::cout << "������-�������: ���������� �� ��'����..." << std::endl;
//
//        waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, 2000);
//        switch (waitResult) {
//        case WAIT_OBJECT_0:
//            std::cout << "������-�������: �'����� ����� � ����������� ����" << std::endl;
//            break;
//        case WAIT_OBJECT_0 + 1:
//            std::cout << "������-�������: ���� � ����������� ����" << std::endl;
//            break;
//        case WAIT_TIMEOUT:
//            std::cout << "������-�������: ����-��� ����������" << std::endl;
//            break;
//        default:
//            CheckError("WaitForMultipleObjects");
//            break;
//        }
//
//        ReleaseMutex(inheritedMutex);
//        std::cout << "������-�������: �'����� ��������" << std::endl;
//    }
//    else {
//        CheckError("WaitForSingleObject (inherited mutex wait)");
//    }
//
//    CloseHandle(eventHandle);
//    DeleteCriticalSection(&criticalSection);
//
//    std::cout << "������-������� ���������." << std::endl;
//    return 0;
//}
//
//int WorkerProcess(int processNumber) {
//    std::cout << "������-������ " << processNumber << " �������� (PID: " << GetCurrentProcessId() << ")" << std::endl;
//
//    HANDLE semaphore = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, SEMAPHORE_NAME);
//    if (semaphore == NULL) {
//        CheckError("OpenSemaphore");
//        return 1;
//    }
//
//    std::cout << "������-������ " << processNumber << ": ���������� �� �������..." << std::endl;
//
//    DWORD waitResult = WaitForSingleObject(semaphore, INFINITE);
//    if (waitResult == WAIT_OBJECT_0) {
//        std::cout << "������-������ " << processNumber << ": ������� ����� �� ���������" << std::endl;
//
//        for (int i = 1; i <= 5; i++) {
//            std::cout << "������-������ " << processNumber << ": ��������� ����� " << i << "/5" << std::endl;
//            Sleep(500);
//        }
//
//        if (!ReleaseSemaphore(semaphore, 1, NULL)) {
//            CheckError("ReleaseSemaphore");
//        }
//
//        std::cout << "������-������ " << processNumber << ": ������� ��������" << std::endl;
//    }
//    else {
//        CheckError("WaitForSingleObject (semaphore)");
//    }
//
//    CloseHandle(semaphore);
//
//    std::cout << "������-������ �" << processNumber << " ���������." << std::endl;
//    return 0;
//}
//
//int main(int argc, char* argv[]) {
//    if (argc <= 1) {
//        return MainProcess();
//    }
//
//    if (strcmp(argv[1], "child") == 0) {
//        if (argc < 3) {
//            std::cerr << "�������: �� ������� ���������� �'������ ��� �������!" << std::endl;
//            return 1;
//        }
//        HANDLE inheritedMutex = reinterpret_cast<HANDLE>(static_cast<UINT_PTR>(std::stoll(argv[2])));
//        return ChildProcess(inheritedMutex);
//    }
//    else if (strcmp(argv[1], "worker") == 0) {
//        if (argc < 3) {
//            std::cerr << "�������: �� ������� ����� ������� ��� �������!" << std::endl;
//            return 1;
//        }
//        int processNumber = std::stoi(argv[2]);
//        return WorkerProcess(processNumber);
//    }
//    else {
//        std::cerr << "�������� ����� ������: " << argv[1] << std::endl;
//        std::cerr << "������� ������: child, worker" << std::endl;
//        return 1;
//    }
//}