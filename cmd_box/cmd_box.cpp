
#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include "framework.h"
#include "cmd_box.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <combaseapi.h>
#include <pathcch.h>
#pragma comment(lib, "Pathcch.lib")
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

constexpr auto MAXPATH = 260;
wchar_t  dirs[][MAXPATH] = { L"C:\\Windows\\system32", L"C:\\Windows" };

// Checks if the given path is an absolute path
static int is_absolute_path(const wchar_t* p) {
    if (!p || !p[0]) return 0;

    // UNC
    if (p[0] == L'\\' && p[1] == L'\\')
        return 1;

    // Drive letter
    if (iswalpha((unsigned char)p[0]) && p[1] == L':' && p[2] == L'\\')
        return 1;

    return 0;
}

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

// Checks if the exe exists inside the paths
static PWSTR is_executable_exists(const wchar_t* exe, wchar_t** path_tokens, size_t path_count) {
    PWSTR exe_path = NULL;
    wchar_t exe_name[MAXPATH];

    // Concats exe to the given executable name
    swprintf_s(exe_name, L"%s.exe", exe);

    // If exe is already an absolute path, check it directly
    if (is_absolute_path(exe_name)) {

        wprintf(L"%s is an absolute path\n", exe);

        if (PathFileExistsW(exe_name)) {
            size_t len = wcslen(exe_name) + 1;
            exe_path = (PWSTR)CoTaskMemAlloc(len * sizeof(wchar_t));
            if (exe_path) {
                wcscpy_s(exe_path, len, exe_name);
                return exe_path;
            }
        }
        return NULL;
    }

    wprintf(L"%s is a relative path\n", exe);

    // If the exe isnt an absolute path


    for (int i = 0; i < path_count; i++) {
        wchar_t* dir = path_tokens[i];

        if (!dir || dir[0] == L'\0') {
            continue; // skip empty tokens
        }

        HRESULT hr = PathAllocCombine(dir, exe_name, PATHCCH_NONE, &exe_path);
        if (FAILED(hr)) {
            // PathAllocCombine didn't produce a path; try next token
            exe_path = NULL;
            continue;
        }

        if (PathFileExistsW(exe_path)) {
            return exe_path;  // Caller must free this
        }

        CoTaskMemFree(exe_path);
        exe_path = NULL;
    }

    return NULL;  // Not found or all attempts failed
}

int path_to_paths(const wchar_t* path_buffer,
    wchar_t*** outTokens,
    size_t* outCount,
    wchar_t** outBuffer)
{
    if (!path_buffer || !outTokens || !outCount) return 1;

    // Make a mutable copy because wcstok overwrites delimiters
    wchar_t* buffer = _wcsdup(path_buffer);
    if (!buffer) return 1;

    // Dynamic array for the pointers
    size_t cap = 4;
    size_t count = 0;
    wchar_t** tokens = new wchar_t* [cap];
    if (!tokens) { free(buffer); return 1; }

    wchar_t* ctx = nullptr;
    wchar_t* tok = wcstok_s(buffer, L";", &ctx);

    while (tok) {

        // Grow the array if needed
        if (count >= cap) {
            cap *= 2;
            wchar_t** tmp = new wchar_t* [cap];
            if (!tmp) { delete[] tokens; free(buffer); return 1; }

            for (size_t i = 0; i < count; ++i)
                tmp[i] = tokens[i];

            delete[] tokens;
            tokens = tmp;
        }

        tokens[count++] = tok;

        // move to next token
        tok = wcstok_s(NULL, L";", &ctx);
    }

    // Transfer ownership to the caller
    *outTokens = tokens;
    *outCount = count;
    *outBuffer = buffer;

    return 0;
}


// This is an example of an exported function.
int fncmdbox(const wchar_t* args)
{
    bool absolute_path;
    wchar_t first_word[256];
    wchar_t** path_tokens = nullptr;
    size_t path_count = 0;
    wchar_t* path_buffer_after;
    
    get_first_word(args, first_word, sizeof(first_word) / sizeof(wchar_t));

    // This is to make the path buffer the corrext size
    DWORD size = GetEnvironmentVariableW(L"PATH", NULL, 0);
    wchar_t* path_buffer_before = new wchar_t[size];

    // Sets the paths variables
    GetEnvironmentVariableW(L"PATH", path_buffer_before, size);

    // Splits the path variable to an array of paths
    if (path_to_paths(path_buffer_before, &path_tokens, &path_count, &path_buffer_after) == 0) {
        for (size_t i = 0; i < path_count; ++i) {
            wprintf(L"token[%zu] = %ls\n", i, path_tokens[i]);
        }
    }

    // Verifies the absolute path given by the user.
    // If it is a relative path, also returns the full path
    PWSTR exe_path = is_executable_exists(first_word, path_tokens, path_count);

    if (exe_path == NULL) {
        printf("Coudn't find the executable :(\n");
        CoTaskMemFree(exe_path);
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

    free(path_buffer_after); //Delete path_buffer_after only at the end, otherwise it's interupting the tokens
    CoTaskMemFree(exe_path);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}