#pragma once

/* ---- Tellurium DLL download ---- */
#define DLL_HOST       L"raw.githubusercontent.com"
#define DLL_PATH       L"/Ducki67/RebotV2/main/Tellurium.dll"
#define DLL_FILENAME   "Tellurium.dll"

/* ---- Default account ---- */
#define DEFAULT_EMAIL    "Fortnite-In-PNG@gmail.com"
#define DEFAULT_PASSWORD "123456789"

/* ---- Folder / file names ---- */
#define ASSETS_FOLDER  "Assets"
#define ACCOUNT_FILE   "account.txt"

/* ---- Game launch args ---- */
#define GAME_ARGS \
    "-epicapp=Fortnite " \
    "-epicenv=Prod " \
    "-epiclocale=en-us " \
    "-epicportal " \
    "-skippatchcheck " \
    "-nobe " \
    "-noeac " \
    "-fromfl=none " \
    "-fltoken=7a848a93a74ba68876c36C1c "

/* ---- Process names ---- */
#define PROC_LAUNCHER  "FortniteLauncher.exe"
#define PROC_GAME      "FortniteClient-Win64-Shipping.exe"
#define PROC_EAC       "FortniteClient-Win64-Shipping_EAC.exe"

/* ---- Relative paths from exe root ---- */
#define REL_LAUNCHER   "FortniteLauncher.exe"
#define REL_GAME       "FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping.exe"
#define REL_EAC        "FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping_EAC.exe"
