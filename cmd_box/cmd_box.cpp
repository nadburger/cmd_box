
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
            exe_path = (PWSTR)malloc(len * sizeof(wchar_t));
            if (exe_path) {
                wcscpy_s(exe_path, len, exe_name);
                return exe_path;
            }
        }
        return NULL;
    }

    wprintf(L"%s is a relative path\n", exe);

    // If the exe isn't an absolute path

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

        free(exe_path);
        exe_path = NULL;
    }

    return NULL;  // Not found or all attempts failed
}

// Splits the system PATH into tokens. On success returns 0 and sets:
//  *outTokens -> allocated with new wchar_t*[count] (caller must delete[] it)
//  *outCount  -> number of tokens
//  *outBuffer -> duplicated mutable buffer allocated with malloc/_wcsdup (caller must free it)
// On failure returns nonzero; no outputs are modified on error.
int path_to_paths(wchar_t*** outTokens, size_t* outCount)
{
    if (!outTokens || !outCount) 
        return 1;

    DWORD size = GetEnvironmentVariableW(L"PATH", NULL, 0);
    if (size == 0)
        return 1;

    wchar_t* buffer = (wchar_t*)malloc(size * sizeof(wchar_t));
    if (!buffer)
        return 1;

    DWORD ret = GetEnvironmentVariableW(L"PATH", buffer, size);
    if (ret == 0 || ret >= size) {
        free(buffer);
        return 1;
    }

    // Dynamic array for the pointers
    size_t cap = 4;
    size_t count = 0;
    wchar_t** tokens = new wchar_t* [cap];
    if (!tokens) {
        free(buffer);
        return 1;
    }

    wchar_t* ctx = nullptr;
    wchar_t* tok = wcstok_s(buffer, L";", &ctx);

    while (tok) {
        if (count >= cap) {
            cap *= 2;
            wchar_t** tmp = new wchar_t* [cap];
            if (!tmp) {
                // Clean up existing tokens
                for (size_t i = 0; i < count; ++i)
                    free(tokens[i]);

                delete[] tokens;
                free(buffer);
                return 1;
            }
            for (size_t i = 0; i < count; ++i)
                tmp[i] = tokens[i];
            delete[] tokens;
            tokens = tmp;
        }

        // Copy the token
        size_t len = wcslen(tok) + 1;
        wchar_t* copy = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (!copy) {
            // Clean up existing tokens
            for (size_t i = 0; i < count; ++i)
                free(tokens[i]);

            delete[] tokens;
            free(buffer);
            return 1;
        }

        wcscpy_s(copy, len, tok);
        tokens[count++] = copy;

        tok = wcstok_s(NULL, L";", &ctx);
    }

    // Free the original buffer since we've copied all tokens
    free(buffer);

    // Transfer ownership to caller
    *outTokens = tokens;
    *outCount = count;
    return 0;
}


// This is an example of an exported function.
int fncmdbox(const wchar_t* args)
{
    wchar_t first_word[256];
    wchar_t** path_tokens = nullptr;
    size_t path_count = 0;
    
    get_first_word(args, first_word, sizeof(first_word) / sizeof(wchar_t));

    // Splits the path variable to an array of paths
    if (!path_to_paths(&path_tokens, &path_count)) {
        for (size_t i = 0; i < path_count; ++i) {
            wprintf(L"token[%zu] = %ls\n", i, path_tokens[i]);
        }
    }

    // Verifies the absolute path given by the user.
    // If it is a relative path, also returns the full path
    PWSTR exe_path = is_executable_exists(first_word, path_tokens, path_count);

    if (exe_path == NULL) {
        printf("Coudn't find the executable :(\n");
        delete[] path_tokens;
        free(exe_path);
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
        delete[] path_tokens;
        free(exe_path);
        return 1;
    }

    delete[] path_tokens;
    free(exe_path);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}