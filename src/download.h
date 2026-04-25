#pragma once
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

#pragma comment(lib, "winhttp.lib")

/*
 * Download a file over HTTPS. Follows one redirect (GitHub raw → CDN).
 * Returns 1 on success, 0 on failure.
 */
static int download_file(const WCHAR* host, const WCHAR* path, const char* out_path) {
    int result = 0;

    HINTERNET hSession = WinHttpOpen(L"Fortnite-in-PNG/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) return 0;

    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return 0; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    /* Allow redirects */
    DWORD opt = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &opt, sizeof(opt));

    if (!WinHttpSendRequest(hRequest, NULL, 0, NULL, 0, 0, 0)) goto cleanup;
    if (!WinHttpReceiveResponse(hRequest, NULL)) goto cleanup;

    /* Check status */
    DWORD statusCode = 0, statusSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        NULL, &statusCode, &statusSize, NULL);

    if (statusCode != 200) goto cleanup;

    /* Write to file */
    {
        HANDLE hFile = CreateFileA(out_path, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) goto cleanup;

        DWORD totalBytes = 0;
        BYTE buf[8192];
        DWORD bytesRead = 0;

        while (WinHttpReadData(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0) {
            DWORD written = 0;
            WriteFile(hFile, buf, bytesRead, &written, NULL);
            totalBytes += written;
            bytesRead = 0;
        }

        CloseHandle(hFile);

        /* Sanity check — DLL should be at least 1 KB */
        if (totalBytes < 1024) {
            DeleteFileA(out_path);
            goto cleanup;
        }

        result = 1;
    }

cleanup:
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}
