# Fortnite in PNG

yes u heard that right. running Old FORTNITE from a PNG.

this is a joke launcher disguised as an image file. you drop it into your OGFN build folder, double-click it, and it launches Fortnite. that's it. it's a PNG (it's not, but it looks like one).

---

## DISCLAIMER

**This project is a JOKE.** It was made purely for fun and humor within the OGFN (OG Fortnite) private server community.

- This is **NOT** a tool for malicious use, cheating, hacking, or any shady business.
- This is **NOT** meant to bypass any anti-cheat, exploit any game, or harm anyone.
- This is designed **ONLY** for use with community-run private Fortnite servers that you own or have permission to use.
- If you're thinking about using this for anything bad — **don't.** Go touch grass instead.

**We are not responsible for any misuse of this software. Use at your own risk and only where you have explicit permission.**

---

## What It Does

1. You place `image.png.exe` next to your `FortniteGame\` and `Engine\` folders
2. Double-click it — Windows shows it as just `image.png` with the real Photos icon
3. The actual **Windows Photos app** opens showing an image (looks 100% legit)
4. Silently in the background:
   - Creates an `Assets\` folder
   - Downloads `Tellurium.dll` (redirect DLL)
   - Creates `account.txt` with default credentials
   - Launches Fortnite and injects the DLL
5. Surprise — Fortnite is running and you just "opened a picture"

**If the exe is NOT in a Fortnite build folder**, it just opens the image and does nothing else. It behaves like a normal PNG anywhere outside a build.

## Default Account

```
Email:    Fortnite-In-PNG@gmail.com
Password: 123456789
```

Edit `Assets\account.txt` to change these. First line = email, second line = password.

## Requirements

- An OGFN Fortnite build with `FortniteGame\` and `Engine\`
- A running backend server (e.g. LawinServer) on `127.0.0.1:3551`
- Windows 10/11 x64
- That's it. No .NET, no Java, no Node. Pure C, zero dependencies.

## Building from Source

1. Open `Fortnite-in-PNG.sln` in Visual Studio 2022
2. Select **Release | x64**
3. Build (Ctrl+B)
4. Output: `bin\Release\image.png.exe`

### Build Config
- Language: C (not C++)
- Toolset: v143
- CRT: Static (`/MT`) — no DLL dependencies
- Subsystem: Windows (no console window)
- Links: `winhttp.lib` (for DLL download)

## Customizing the Image

Replace `resource\image.bmp` with your own image (BMP format, any size) and rebuild. The embedded image is what opens in Windows Photos when someone double-clicks the "PNG".

## Project Structure

```
Fortnite-in-PNG/
  src/
    main.c        — WinMain entry, image extraction, ShellExecute, background launch
    config.h      — URLs, defaults, paths
    download.h    — WinHTTP HTTPS download
    launch.h      — kill/spawn/suspend/inject (Win32)
  resource/
    resource.rc   — icon + embedded image resources
    icon.ico      — real Windows Photos file type icon
    image.bmp     — the image shown when "opened" (your custom image)
```

## How It Works (for nerds)

1. Extracts embedded BMP image from resources to `%TEMP%\image.png`
2. Opens it with `ShellExecuteA("open", ...)` — the real Windows Photos app handles it
3. If no Fortnite build detected next to exe — stops here (just a normal "image")
4. Creates `Assets/` folder, downloads `Tellurium.dll` via WinHTTP if missing
5. Creates or reads `account.txt` for login credentials
6. Kills any stale Fortnite processes
7. Spawns `FortniteLauncher.exe` and EAC as suspended companion processes
8. Launches `FortniteClient-Win64-Shipping.exe` with redirect auth args
9. Injects `Tellurium.dll` via `CreateRemoteThread` + `LoadLibraryA`
10. Done. You're playing Fortnite from a PNG.

## Credits

- **Ducki67** — creator, the mind behind this masterpiece

## License

This project is provided as-is for educational and entertainment purposes within the OGFN community. Not affiliated with Epic Games in any way.

**Don't be a bad apple.**
