/*
 * Fortnite-in-PNG  —  the dumbest launcher ever made
 *
 * Drop this .exe (named image.png.exe) next to FortniteGame\ and Engine\.
 * Double-click it. Windows Photos opens showing a normal image.
 * Meanwhile Fortnite silently launches in the background. Surprise!
 */

#include <windows.h>
#include <stdio.h>
#include <shellapi.h>

#include "config.h"
#include "download.h"
#include "launch.h"

/* ---- resource IDs ---- */
#define IDR_IMAGE  101

/* ---- helpers ---- */
static void get_exe_dir(char* buf, int size) {
    GetModuleFileNameA(NULL, buf, size);
    char* last = strrchr(buf, '\\');
    if (last) *last = '\0';
}

static int file_exists(const char* path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

/* ============================================================
   Extract embedded BMP resource to a file
   ============================================================ */
static int extract_resource_image(HINSTANCE hInst, const char* out_path) {
    HRSRC hRes = FindResourceA(hInst, MAKEINTRESOURCEA(IDR_IMAGE), RT_BITMAP);
    if (!hRes) return 0;

    HGLOBAL hData = LoadResource(hInst, hRes);
    if (!hData) return 0;

    DWORD dataSize = SizeofResource(hInst, hRes);
    void* pData = LockResource(hData);
    if (!pData || dataSize == 0) return 0;

    /*
     * RT_BITMAP resources contain raw DIB data (BITMAPINFOHEADER + pixels)
     * but no BMP file header. We need to prepend a BITMAPFILEHEADER.
     */
    BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)pData;
    DWORD headerSize = sizeof(BITMAPFILEHEADER);
    DWORD pixelOffset = headerSize + bih->biSize;

    /* Account for color table if present (for <=8bpp images) */
    if (bih->biBitCount <= 8) {
        DWORD colors = bih->biClrUsed ? bih->biClrUsed : (1u << bih->biBitCount);
        pixelOffset += colors * sizeof(RGBQUAD);
    }

    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 0x4D42; /* "BM" */
    bfh.bfSize = headerSize + dataSize;
    bfh.bfOffBits = pixelOffset;

    HANDLE hFile = CreateFileA(out_path, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    DWORD written = 0;
    WriteFile(hFile, &bfh, sizeof(bfh), &written, NULL);
    WriteFile(hFile, pData, dataSize, &written, NULL);
    CloseHandle(hFile);
    return 1;
}

/* ============================================================
   WinMain — entry point (no console window at all)
   ============================================================ */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    (void)hPrev; (void)lpCmd; (void)nShow;

    char root[MAX_PATH];
    get_exe_dir(root, MAX_PATH);

    /* ---- paths ---- */
    char game_exe[MAX_PATH], launcher_exe[MAX_PATH], eac_exe[MAX_PATH];
    _snprintf(game_exe,     MAX_PATH, "%s\\" REL_GAME,     root);
    _snprintf(launcher_exe, MAX_PATH, "%s\\" REL_LAUNCHER, root);
    _snprintf(eac_exe,      MAX_PATH, "%s\\" REL_EAC,      root);

    char game_dir[MAX_PATH];
    _snprintf(game_dir, MAX_PATH, "%s\\FortniteGame\\Binaries\\Win64", root);

    /* ---- extract image to temp and open with real Photos ---- */
    char temp_dir[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_dir);

    char image_path[MAX_PATH];
    _snprintf(image_path, MAX_PATH, "%s\\image.png", temp_dir);

    if (extract_resource_image(hInst, image_path)) {
        /* Open with default image viewer (Windows Photos) */
        ShellExecuteA(NULL, "open", image_path, NULL, NULL, SW_SHOWNORMAL);
    }

    /* ---- if game exe not found, just show the image and bail ---- */
    if (!file_exists(game_exe)) return 0;

    /* ---- create Assets folder ---- */
    char assets_dir[MAX_PATH];
    _snprintf(assets_dir, MAX_PATH, "%s\\Assets", root);
    CreateDirectoryA(assets_dir, NULL);

    /* ---- download Tellurium.dll if missing ---- */
    char dll_path[MAX_PATH];
    _snprintf(dll_path, MAX_PATH, "%s\\%s", assets_dir, DLL_FILENAME);

    if (!file_exists(dll_path)) {
        if (!download_file(DLL_HOST, DLL_PATH, dll_path))
            return 0;
    }

    /* ---- account setup ---- */
    char account_path[MAX_PATH];
    _snprintf(account_path, MAX_PATH, "%s\\%s", assets_dir, ACCOUNT_FILE);

    char email[256]    = DEFAULT_EMAIL;
    char password[256] = DEFAULT_PASSWORD;

    if (!file_exists(account_path)) {
        FILE* f = fopen(account_path, "w");
        if (f) {
            fprintf(f, "%s\n%s\n", DEFAULT_EMAIL, DEFAULT_PASSWORD);
            fclose(f);
        }
    } else {
        FILE* f = fopen(account_path, "r");
        if (f) {
            if (fgets(email, sizeof(email), f)) {
                char* nl = strchr(email, '\n');    if (nl) *nl = '\0';
                nl = strchr(email, '\r');           if (nl) *nl = '\0';
            }
            if (fgets(password, sizeof(password), f)) {
                char* nl = strchr(password, '\n');  if (nl) *nl = '\0';
                nl = strchr(password, '\r');         if (nl) *nl = '\0';
            }
            fclose(f);
        }
    }

    /* ---- kill stale ---- */
    kill_fortnite();

    /* ---- env ---- */
    SetEnvironmentVariableA("OPENSSL_ia32cap", "~0x20000000");

    /* ---- spawn & suspend companions ---- */
    spawn_and_suspend(launcher_exe, root);
    spawn_and_suspend(eac_exe, game_dir);

    /* ---- launch game ---- */
    DWORD game_pid = launch_game(game_exe, game_dir, email, password);
    if (!game_pid) return 1;

    /* ---- inject ---- */
    inject_with_retry(game_pid, dll_path, 30, 500);

    return 0;
}
