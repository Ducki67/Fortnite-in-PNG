#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

/* ============================================================
   Kill processes by name
   ============================================================ */
static void kill_by_name(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, name) == 0) {
                HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (h) { TerminateProcess(h, 0); CloseHandle(h); }
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
}

static void kill_fortnite(void) {
    kill_by_name("FortniteLauncher.exe");
    kill_by_name("FortniteClient-Win64-Shipping.exe");
    kill_by_name("FortniteClient-Win64-Shipping_EAC.exe");
    Sleep(300);
}

/* ============================================================
   Suspend all threads of a process
   ============================================================ */
static void suspend_process(DWORD pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(snap, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
}

/* ============================================================
   Spawn a process and immediately suspend it
   Returns PID or 0 on failure
   ============================================================ */
static DWORD spawn_and_suspend(const char* exe_path, const char* work_dir) {
    if (GetFileAttributesA(exe_path) == INVALID_FILE_ATTRIBUTES) return 0;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[MAX_PATH + 4];
    _snprintf(cmd, sizeof(cmd), "\"%s\"", exe_path);

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, work_dir, &si, &pi))
        return 0;

    suspend_process(pi.dwProcessId);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pi.dwProcessId;
}

/* ============================================================
   Launch the game — returns PID or 0
   ============================================================ */
static DWORD launch_game(const char* exe_path, const char* work_dir,
                          const char* email, const char* password) {
    char cmd[2048];
    _snprintf(cmd, sizeof(cmd),
        "\"%s\" "
        "-epicapp=Fortnite "
        "-epicenv=Prod "
        "-epiclocale=en-us "
        "-epicportal "
        "-skippatchcheck "
        "-nobe "
        "-noeac "
        "-fromfl=none "
        "-fltoken=7a848a93a74ba68876c36C1c "
        "-AUTH_LOGIN=%s "
        "-AUTH_PASSWORD=%s "
        "-AUTH_TYPE=epic",
        exe_path, email, password);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(exe_path, cmd, NULL, NULL, FALSE, 0, NULL, work_dir, &si, &pi))
        return 0;

    DWORD pid = pi.dwProcessId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pid;
}

/* ============================================================
   Inject a DLL into a process (CreateRemoteThread + LoadLibraryA)
   ============================================================ */
static int inject_dll(DWORD pid, const char* dll_path) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) return 0;

    size_t path_len = strlen(dll_path) + 1;
    LPVOID remote_buf = VirtualAllocEx(hProc, NULL, path_len,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_buf) { CloseHandle(hProc); return 0; }

    if (!WriteProcessMemory(hProc, remote_buf, dll_path, path_len, NULL)) {
        VirtualFreeEx(hProc, remote_buf, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 0;
    }

    HMODULE hK32 = GetModuleHandleA("kernel32.dll");
    FARPROC loadLib = GetProcAddress(hK32, "LoadLibraryA");
    if (!loadLib) {
        VirtualFreeEx(hProc, remote_buf, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 0;
    }

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)loadLib, remote_buf, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProc, remote_buf, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 0;
    }

    WaitForSingleObject(hThread, 10000);

    VirtualFreeEx(hProc, remote_buf, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProc);
    return 1;
}

/* Retry wrapper */
static int inject_with_retry(DWORD pid, const char* dll_path, int retries, int interval_ms) {
    for (int i = 0; i < retries; i++) {
        if (inject_dll(pid, dll_path)) return 1;
        Sleep(interval_ms);
    }
    return 0;
}
