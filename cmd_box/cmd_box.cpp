
#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include "framework.h"
#include "cmd_box.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <combaseapi.h>
#include <pathcch.h>
#pragma comment(lib, "Pathcch.lib")
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

constexpr auto MAXPATH = 260;
wchar_t  dirs[][MAXPATH] = { L"C:\\Windows\\system32", L"C:\\Windows" };

static void get_first_word(const wchar_t* input, wchar_t* dest, size_t dest_size)
{
    if (!input || !dest || dest_size == 0)
        return;

    size_t i = 0;

    // Copy characters until space or end of string
    while (input[i] != L'\0' && input[i] != L' ' && i < dest_size - 1) {
        dest[i] = input[i];
        i++;
    }

    dest[i] = L'\0';
}

static int is_executable_exists(const wchar_t* exe) {

    PWSTR exe_path = NULL;

    int dir_count = sizeof(dirs) / sizeof(dirs[0]);
    
    for (int i = 0; i < dir_count; i++) {

        HRESULT hr = PathAllocCombine(dirs[i], exe, PATHCCH_NONE, &exe_path);
        if (FAILED(hr)) {
            continue;
        }

        wcscat_s(exe_path, MAXPATH, L".exe");

        if (PathFileExistsW(exe_path)){ 
            CoTaskMemFree(exe_path);
            return TRUE;
        }

        CoTaskMemFree(exe_path);
    }
    return FALSE;
}

// This is an example of an exported function.
int fncmdbox(const wchar_t* args)
{
    wchar_t first_word[256];
    
    get_first_word(args, first_word, sizeof(first_word) / sizeof(wchar_t));

    if (!is_executable_exists(first_word)) {
        printf("Coudn't find the exe path :(\n");
        return 1;
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    wchar_t cmdline[MAXPATH];
    wcscpy_s(cmdline, MAXPATH, args);

    if (!CreateProcessW(NULL, cmdline, NULL, NULL, FALSE,
        HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
        DWORD err = GetLastError();
        printf("CreateProcess failed: %lu\n", err);
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}