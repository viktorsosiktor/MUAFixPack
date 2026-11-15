// MUAFixPack.cpp - MUAFixPack Beta v1
// A plugin for Marvel: Ultimate Alliance (2006 PC), loaded by Ultimate ASI Loader
// (ThirteenAG). Project lead, design and testing: viktorsosiktor. Code written with Claude
// (Anthropic). Thanks to the Marvel Mods community for their tools and resources.
// What each feature does for players: see the README. Every feature can be turned off in
// MUAFixPack.ini (see Settings below).
//
// How it works: MUAFixPack changes the game's own code and data in memory while the game
// runs: MUA.exe and the engine's libIGDisplay.dll, libIGGfx.dll and libCriMovie.dll. It
// checks the expected bytes before every change and leaves a different version alone (the
// log says so). No game file is changed. The log records game data only.
//
// 1. Resolution (MUA.exe)
//    - Every supported resolution
//      Limitation: the game asks Direct3D for every display mode the graphics card and
//      monitor support, but its check (0x74CA00) only keeps a mode whose width x height is
//      on a fixed list of 14 sizes, so Options only offers those.
//      Fix: the check keeps every mode, so Options shows every size the card and monitor
//      can show (for example 2560x1440 or 3840x2160).
//    - Safe start
//      Limitation: at startup the game looks up its saved resolution in its mode list
//      without checking where the list ends, so a size the monitor can't show (for example
//      after moving the game to another monitor with a different resolution) crashes the
//      game.
//      Fix: the saved resolution is checked against the list first (0x720A94, 0x74DE67).
//      A size that is not in the list is replaced by the desktop resolution, as the game's
//      own mode list keeps it, or by the largest size in the list. The game saves it the
//      next time display settings are applied in Options.
//    - Text and button icon size
//      Limitation: the game sizes text and button icons by screen pixels, so above 1080p
//      they get smaller and smaller.
//      Fix: above 1080p, the game's text scale (0x721512, 0x721540, 0x71F257, 0x71F26E)
//      and the Controls Options icon size (0x6DAA18) are scaled by height / 1080, so they
//      keep their 1080p size. At 1080p and below nothing changes.
//
// 2. Movies (libCriMovie.dll)
//    - Freeze fix
//      Limitation: the movie player pins its threads and the game thread to the first CPU
//      core. When a movie stops, the game thread waits for a movie thread that never gets
//      to run on that shared core, and the game freezes.
//      Fix: the player's own pinning step is skipped, so its threads can use any core and
//      the movie thread always finishes.
//    - Larger movies
//      Limitation: the player refuses movies larger than 720x576.
//      Fix: its header check is raised to 4095x4095 (the MPEG-1 maximum).
//
// 3. Graphics (libIGGfx.dll, MUA.exe)
//    - Anisotropic filtering
//      Limitation: the engine has an anisotropic filter and a maxAnisotropy setting, but
//      the game never uses them: mipmapped textures get plain linear filtering and blur
//      in the distance.
//      Fix: the engine's filter table uses its own anisotropic entry for mipmapped
//      textures, and the engine gets its maxAnisotropy setting right before it reads it,
//      capped at the graphics card's maximum as the engine itself keeps it.
//    - Antialiasing
//      Limitation: the game itself asks Direct3D which antialiasing levels the card
//      supports, but its question stops after the first three of the six levels in its
//      list (0x74D104), and the way from the Options menu to Direct3D only knows off, 2x
//      and 4x, so Options stops at 4x.
//      Fix: the game's question goes through all six levels, and the way to Direct3D
//      knows all six, so Options offers every level the card supports (for example 8x).
//
// 4. Windowed mode (libIGDisplay.dll, libIGGfx.dll, MUA.exe)
//    - The engine's own windowed mode
//      Limitation: the engine can run in a window, but MUA.exe turns exclusive fullscreen
//      on at startup, in three places.
//      Fix: those three places write "off", so the game starts in the engine's own
//      windowed mode. The engine keeps the picture at the game's resolution whatever the
//      window's size, and the window is placed with the engine's own setPosition. Border,
//      place and the hotkeys are set in MUAFixPack.ini.
//    - Fill the screen: the window gets the largest size with the game's shape that fits
//      the monitor, centered, and Direct3D stretches the picture to it. When the game's
//      shape is not the monitor's shape, a plain black window behind the game covers the
//      rest of the monitor (the bars at the sides or at the top and bottom).
//    - Mouse in a window
//      Limitation: the game sets its mouse area (where its picture is on the screen, used to
//      keep the mouse inside and to work out where it points) only when display settings
//      change, because in fullscreen the window never moves. It also reads the mouse in
//      window pixels, so on a stretched window clicks would land in the wrong place.
//      Fix: the game's mouse area is updated the game's own way (0x403C60) whenever the
//      window moves or changes, and while the screen is filled the game's mouse readings
//      (its main read at 0x41645F and its four menu reads) are scaled back to the game's
//      resolution.
//    - Hotkeys come from the game window's key presses in the engine's own message slot
//      (setWindowProc), passed on to the game's own function (or another mod's).
//    - During a resolution change the window holds its place and size until the engine's
//      first frame at the new size (the engine's own end of frame), so it doesn't jump.
//
// 5. Controllers (MUA.exe)
//    - Xbox
//      Limitation: the game has a list of known controllers, each with its own automatic
//      layout, but only the wired Xbox 360 controller and several Logitech controllers are
//      on it. Other Xbox controllers (for example wireless Xbox 360, Xbox One/Elite, and
//      Xbox Series X/S) get the generic layout, which doesn't match the Xbox layout, so
//      actions land on the wrong buttons and the right stick isn't read correctly.
//      Fix: when the game's search of the list (0x406AA0) finds nothing for an Xbox
//      controller, it is answered with the game's own Xbox 360 entry.
//    - PlayStation
//      Limitation: PlayStation controllers get the generic layout, which fits them apart
//      from Back. When buttons are set by hand, the resting L2/R2 triggers count as
//      pressed, so they are caught at once.
//      Fix: the list's Logitech RumblePad 2 entry, which has the generic layout, becomes
//      the PlayStation entry with Back on the touchpad, and PlayStation controllers are
//      answered with it (a RumblePad 2 still gets the generic layout). When buttons are
//      set by hand, the resting L2/R2 triggers no longer count as pressed (0x409BD0).
//    - Every controller, Xbox or PlayStation, keeps its own product ID. The game files
//      each controller's settings by product ID, so different controllers keep their own
//      settings.
//    - Button icons: the game picks its texs files and icons itself; its questions are
//      answered so it shows Xbox icons in the order of the Official Characters Pack (OCP)
//      by Marvel Mods (XboxIcons=1), and the OCP PlayStation additional controller support
//      files (PlayStationIcons=1).
//    - Other mods that read the controller themselves (such as Third Person Modes from
//      Marvel Mods) see player 1's PlayStation controller as an Xbox controller. The game
//      is not affected.
//
// Settings: MUAFixPack.ini next to the .asi. If it is missing, one is created with the
// defaults; a missing line counts as the default. Everything is on by default, and every
// feature can be turned off on its own (the other Windowed settings and hotkeys only
// matter with [Windowed] Enabled=1, XboxIcons only with Xbox=1, PlayStationIcons only
// with PlayStation=1). The defaults:
//     [Resolution]  Enabled=1                                   ; section 1
//     [Movies]      Enabled=1, LargeMovies=1                    ; section 2
//     [Graphics]    Anisotropy=16, Antialiasing=1               ; section 3
//     [Windowed]    Enabled=1, Border=0, Position=5 (center), FillScreen=1,
//                   AlwaysOnTop=0, and six hotkeys              ; section 4
//     [Controllers] Xbox=1, XboxIcons=1, PlayStation=1, PlayStationIcons=1   ; section 5
// The hotkeys and every setting are explained in MUAFixPack.ini and the README.

#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include <intrin.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#pragma comment(lib, "user32.lib")
// the black brush only (fill screen background, game window background)
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dinput8.lib")   // standard joystick data format only (c_dfDIJoystick2)
#pragma comment(lib, "dxguid.lib")    // standard DirectInput GUIDs only

// ---------------------------------------------------------------------------
// Log
// ---------------------------------------------------------------------------

static char    g_log[131072];
static int     g_len;
static HMODULE g_self;
static CRITICAL_SECTION g_logCs;

static void Log(const char* fmt, ...)
{
    EnterCriticalSection(&g_logCs);
    if (g_len >= (int)sizeof(g_log) - 1) { LeaveCriticalSection(&g_logCs); return; }
    va_list ap;
    va_start(ap, fmt);
    int n = _vsnprintf(g_log + g_len, sizeof(g_log) - 1 - g_len, fmt, ap);
    va_end(ap);
    if (n > 0) g_len += n;
    else g_len = sizeof(g_log) - 1;
    g_log[g_len] = 0;
    LeaveCriticalSection(&g_logCs);
}

// Full path of a file next to the .asi, with the .asi's name and a new ending.
static bool SidePath(char* out, const char* ending)
{
    DWORD n = GetModuleFileNameA(g_self, out, MAX_PATH);
    if (n == 0 || n >= MAX_PATH - 16) return false;
    char* dot = strrchr(out, '.');
    char* slash = strrchr(out, '\\');
    if (dot && (!slash || dot > slash)) *dot = 0;
    strcat(out, ending);
    return true;
}

static void WriteLog()
{
    EnterCriticalSection(&g_logCs);
    char path[MAX_PATH];
    if (!SidePath(path, ".log")) { LeaveCriticalSection(&g_logCs); return; }
    HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        WriteFile(h, g_log, (DWORD)g_len, &written, NULL);
        CloseHandle(h);
    }
    LeaveCriticalSection(&g_logCs);
}

static bool Write(BYTE* at, const BYTE* data, size_t len)
{
    DWORD old;
    if (!VirtualProtect(at, len, PAGE_EXECUTE_READWRITE, &old)) return false;
    memcpy(at, data, len);
    VirtualProtect(at, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), at, len);
    return true;
}

// Finds an import by name. Where the module keeps no name list for that DLL
// (mua.exe does this for ole32.dll), it is found by its address instead.
static bool HookImport(HMODULE mod, const char* dll, const char* func, void* hook, void** realOut)
{
    BYTE* base = (BYTE*)mod;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress) return false;
    HMODULE target = GetModuleHandleA(dll);
    ULONG_PTR address = target ? (ULONG_PTR)GetProcAddress(target, func) : 0;
    for (IMAGE_IMPORT_DESCRIPTOR* imp = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir.VirtualAddress); imp->Name; ++imp)
    {
        if (_stricmp((char*)(base + imp->Name), dll) != 0) continue;
        IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)(base + imp->FirstThunk);
        IMAGE_THUNK_DATA* names = imp->OriginalFirstThunk
                                ? (IMAGE_THUNK_DATA*)(base + imp->OriginalFirstThunk) : NULL;
        for (; thunk->u1.Function; ++thunk)
        {
            IMAGE_THUNK_DATA* name = names ? names++ : NULL;
            if (name)
            {
                if (IMAGE_SNAP_BY_ORDINAL(name->u1.Ordinal)) continue;
                IMAGE_IMPORT_BY_NAME* ibn = (IMAGE_IMPORT_BY_NAME*)(base + name->u1.AddressOfData);
                if (strcmp((char*)ibn->Name, func) != 0) continue;
            }
            else if (!address || thunk->u1.Function != address) continue;
            DWORD old;
            if (!VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old)) return false;
            *realOut = (void*)thunk->u1.Function;
            thunk->u1.Function = (ULONG_PTR)hook;
            VirtualProtect(&thunk->u1.Function, sizeof(void*), old, &old);
            return true;
        }
    }
    return false;
}

// Settings: every feature can be turned off in MUAFixPack.ini next to the .asi
// (see the Settings part of the header). A missing line counts as on.
static bool g_resOn   = true;   // section 1
static bool g_movieOn = true;   // section 2
static bool g_largeMovies = true;   // section 2
static bool g_winOn   = true;   // section 4
static bool g_xboxOn  = true;   // section 5
static bool g_psOn    = true;   // section 5
static bool g_xboxIconsOcp = true;   // XboxIcons=1: the Xbox texs files are the OCP ones
// PlayStationIcons=1: the OCP PlayStation texs files are in place
static bool g_psIcons      = true;
static int  g_anisoAsked   = 16;     // Anisotropy= as written (section 3)
static int  g_aniso        = 16;     // Anisotropy level used: 16, 8, 4, 2, or 1 = off (section 3)
static bool g_aaOn         = true;   // Antialiasing=1 (section 3)

// Written next to the .asi (with the .asi's name) when there is no .ini yet.
static const char kDefaultIni[] =
    "; MUAFixPack settings. 1 = on, 0 = off. Everything is on by default.\r\n"
    "; Every setting is explained in the README.\r\n"
    "\r\n"
    "[Resolution]\r\n"
    "; Every supported resolution, safe start, 1080p text and icon size.\r\n"
    "Enabled=1\r\n"
    "\r\n"
    "[Movies]\r\n"
    "; Movie freeze fix.\r\n"
    "Enabled=1\r\n"
    "; 1 = Movies up to 4095x4095 (MPEG-1 limit). 0 = Original limit (720x576).\r\n"
    "LargeMovies=1\r\n"
    "\r\n"
    "[Graphics]\r\n"
    "; Anisotropic filtering: 16, 8, 4 or 2. 1 = Original.\r\n"
    "Anisotropy=16\r\n"
    "; 1 = Every antialiasing level the card supports. 0 = Original (up to 4x).\r\n"
    "Antialiasing=1\r\n"
    "\r\n"
    "[Windowed]\r\n"
    "; 1 = Windowed (bordered and borderless). 0 = Original fullscreen.\r\n"
    "Enabled=1\r\n"
    "; 1 = Fill the screen (keeps shape). 0 = Size based on resolution.\r\n"
    "FillScreen=1\r\n"
    "; 1 = After Alt+Tab, stays in front of other windows. 0 = Normal window.\r\n"
    "AlwaysOnTop=0\r\n"
    "; 1 = Border and title bar. 0 = Borderless.\r\n"
    "Border=0\r\n"
    "; Window position on screen, like a number pad:\r\n"
    "; 7 8 9 top, 4 5 6 middle (5 centered), 1 2 3 bottom.\r\n"
    "Position=5\r\n"
    "; Toggle hotkeys (see the README for hotkey instructions)\r\n"
    "HotkeyBorder=Ctrl+B\r\n"
    "HotkeyFillScreen=Ctrl+N\r\n"
    "; Move hotkeys (when fill screen is off)\r\n"
    "HotkeyUp=Ctrl+I\r\n"
    "HotkeyLeft=Ctrl+J\r\n"
    "HotkeyDown=Ctrl+K\r\n"
    "HotkeyRight=Ctrl+L\r\n"
    "\r\n"
    "[Controllers]\r\n"
    "; Xbox controllers: the game's Xbox 360 layout.\r\n"
    "Xbox=1\r\n"
    "; 1 = OCP Xbox texs files. 0 = The game's original texs files.\r\n"
    "XboxIcons=1\r\n"
    "; Touchpad as Back, L2/R2 fix, Third Person Modes support.\r\n"
    "PlayStation=1\r\n"
    "; 1 = OCP PlayStation texs files (without them, menu text disappears).\r\n"
    "PlayStationIcons=1\r\n";

static void ReadSettings()
{
    char ini[MAX_PATH];
    if (!SidePath(ini, ".ini")) return;
    if (GetFileAttributesA(ini) == INVALID_FILE_ATTRIBUTES)
    {
        HANDLE h = CreateFileA(ini, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            DWORD written;
            WriteFile(h, kDefaultIni, (DWORD)(sizeof(kDefaultIni) - 1), &written, NULL);
            CloseHandle(h);
            Log("Settings: no .ini found, created one with the default settings\r\n");
        }
    }
    g_resOn  = GetPrivateProfileIntA("Resolution",  "Enabled",     1, ini) != 0;
    g_movieOn = GetPrivateProfileIntA("Movies",     "Enabled",     1, ini) != 0;
    g_largeMovies = GetPrivateProfileIntA("Movies", "LargeMovies", 1, ini) != 0;
    g_winOn  = GetPrivateProfileIntA("Windowed",    "Enabled",     1, ini) != 0;
    g_xboxOn = GetPrivateProfileIntA("Controllers", "Xbox",        1, ini) != 0;
    g_psOn   = GetPrivateProfileIntA("Controllers", "PlayStation", 1, ini) != 0;
    g_xboxIconsOcp = GetPrivateProfileIntA("Controllers", "XboxIcons",        1, ini) != 0;
    g_psIcons      = GetPrivateProfileIntA("Controllers", "PlayStationIcons", 1, ini) != 0;
    g_aaOn         = GetPrivateProfileIntA("Graphics",    "Antialiasing",     1, ini) != 0;
    // Anisotropy: 16, 8, 4, 2, or 1 = off. 0 or a missing line = 16; another value is lowered
    // to the next level.
    g_anisoAsked = GetPrivateProfileIntA("Graphics", "Anisotropy", 16, ini);
    int a = g_anisoAsked;
    if (a <= 0) a = 16;
    g_aniso = a >= 16 ? 16 : a >= 8 ? 8 : a >= 4 ? 4 : a >= 2 ? 2 : 1;
}

// ---------------------------------------------------------------------------
// 1. Resolution: every supported resolution
// ---------------------------------------------------------------------------

// The game's check (0x74CA00) is only a yes/no filter: for each mode the graphics card
// reports, it answers whether the width x height pair is on its fixed 14-entry list.
// Its first bytes are replaced with "answer yes" (mov al,1 / ret 10), so every mode is kept.
static const DWORD kModeCheck = 0x74CA00;
static const BYTE  kExpModeCheck[] = { 0x8B, 0x4C, 0x24, 0x08, 0x8B, 0x54, 0x24, 0x04, 0x33, 0xC0 };

static bool PatchResolutionList()
{
    BYTE* p = (BYTE*)kModeCheck;
    if (memcmp(p, kExpModeCheck, sizeof(kExpModeCheck)) != 0)
    {
        Log("Resolution list: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n"
            "This plugin was made for the original 2006 PC mua.exe.\r\n");
        return false;
    }
    static const BYTE yes[] = { 0xB0, 0x01, 0xC2, 0x10, 0x00 };   // mov al,1 / ret 10
    bool ok = Write(p, yes, sizeof(yes));
    Log("Resolution list: %s\r\n", ok ? "applied (every mode the graphics card and monitor support)" : "FAILED to write patch");
    return ok;
}

// ---------------------------------------------------------------------------
// 1. Resolution: safe start
// ---------------------------------------------------------------------------

// At startup the game finds its saved resolution in its mode list with the function at
// 0x74CB80 (called from one place, 0x74DE67). For each mode it writes "%dx%d" and compares
// it with the saved text at 0xD402A4; it never checks where the list ends. The mode
// list object holds the modes at +0x57C (16 bytes each: width, height, ...) and their
// count at +0x580. The saved text has room for 10 bytes (the next game value starts
// at 0xD402AE), so a replacement is at most 9 characters plus the ending zero.
static const DWORD kResLookupCall = 0x74DE67;   // call 0x74CB80
static const DWORD kResLookup     = 0x74CB80;
static char* const kSavedRes      = (char*)0xD402A4;
static const int   kSavedResSize  = 10;

struct GameMode { int w, h; DWORD a, b; };

// The game calls this lookup often (its screen layout code runs it repeatedly, not only
// at startup), so a resolution already found in the same list is not checked again,
// and the log only gets a line when the result changes.
static char        g_resOkText[kSavedResSize + 1];   // last saved text found in the list
static const void* g_resOkModes;
static int         g_resOkCount;

static void __cdecl CheckSavedResolution(BYTE* list)
{
    if (!list) return;
    const GameMode* modes = *(const GameMode**)(list + 0x57C);
    int count = *(int*)(list + 0x580);

    char saved[kSavedResSize + 1];
    memcpy(saved, kSavedRes, kSavedResSize);
    saved[kSavedResSize] = 0;

    if (modes == g_resOkModes && count == g_resOkCount && strcmp(saved, g_resOkText) == 0)
        return;   // same resolution, same list: already checked

    // The desktop resolution as the game's own mode list keeps it (0x74D1B0 asks Direct3D for
    // the main monitor's current mode at 0x74D2C6 and stores it at +0x584 width, +0x588 height).
    int deskW = *(int*)(list + 0x584), deskH = *(int*)(list + 0x588);
    bool haveDesk = deskW > 0 && deskH > 0;

    if (!modes || count <= 0)
    {
        Log("Resolution check: the game's mode list is empty, left unchanged\r\n");
        WriteLog();
        return;
    }

    char text[32];
    for (int i = 0; i < count; ++i)
    {
        sprintf(text, "%dx%d", modes[i].w, modes[i].h);
        if (strcmp(text, saved) == 0)
        {
            Log("Resolution check: saved resolution %s is in the list, kept\r\n", saved);
            WriteLog();
            strcpy(g_resOkText, saved);
            g_resOkModes = modes;
            g_resOkCount = count;
            return;
        }
    }

    // Not in the list: use the desktop resolution if it is there, otherwise the
    // largest mode in the list. Only modes whose text fits the game's 10 bytes.
    int pick = -1;
    bool desk = false;
    long long best = -1;
    for (int i = 0; i < count; ++i)
    {
        int n = sprintf(text, "%dx%d", modes[i].w, modes[i].h);
        if (n <= 0 || n >= kSavedResSize) continue;
        if (haveDesk && modes[i].w == deskW && modes[i].h == deskH)
        {
            pick = i;
            desk = true;
            break;
        }
        long long area = (long long)modes[i].w * modes[i].h;
        if (area > best) { best = area; pick = i; }
    }
    if (pick < 0)
    {
        Log("Resolution check: saved resolution %s is not in the list and no replacement fits, left unchanged\r\n",
            saved[0] ? saved : "(empty)");
        WriteLog();
        return;
    }
    int n = sprintf(text, "%dx%d", modes[pick].w, modes[pick].h);
    memcpy(kSavedRes, text, n + 1);
    Log("Resolution check: saved resolution %s is not in the list, using %s (%s)\r\n",
        saved[0] ? saved : "(empty)", text, desk ? "desktop resolution" : "largest in the list");
    WriteLog();
}

// Runs the check, then continues into the game's own function with everything as it was
// (ecx = the mode list object, the game's argument still on the stack).
__declspec(naked) static void Hook_ResLookup()
{
    __asm {
        push ecx
        push ecx
        call CheckSavedResolution
        add  esp, 4
        pop  ecx
        push 0x74CB80
        ret
    }
}

// The same check, earlier: at startup the game turns the saved text into the width and
// height it creates its window and display with (0x7209DA: sscanf into the startup
// settings at [edi+8] / [edi+0xC]), builds the mode list (0x720A85), then looks up the
// refresh rate for that size (0x720A94: call 0x74CAC0(width, height)). The check runs
// there, before anything is created, and the width and height are read again from the
// (possibly replaced) text. edi holds the startup settings at that point.
static const DWORD kStartSizeCall = 0x720A94;   // call 0x74CAC0
static const DWORD kModeRefresh   = 0x74CAC0;

static void __cdecl FixStartupSize(BYTE* list, BYTE* settings)
{
    CheckSavedResolution(list);
    char text[kSavedResSize + 1];
    memcpy(text, kSavedRes, kSavedResSize);
    text[kSavedResSize] = 0;
    int w = 0, h = 0;
    if (settings && sscanf(text, "%dx%d", &w, &h) == 2 && w > 0 && h > 0)
    {
        *(int*)(settings + 8)   = w;
        *(int*)(settings + 0xC) = h;
    }
}

// [esp+4] = width, [esp+8] = height (the game's arguments), ecx = the mode list.
__declspec(naked) static void Hook_StartSize()
{
    __asm {
        push ecx
        push edi
        push ecx
        call FixStartupSize
        add  esp, 8
        pop  ecx
        mov  eax, [edi+8]
        mov  [esp+4], eax
        mov  eax, [edi+0xC]
        mov  [esp+8], eax
        push 0x74CAC0
        ret
    }
}

static bool RedirectCall(DWORD at, DWORD expectedTarget, void* hook)
{
    BYTE* p = (BYTE*)at;
    if (p[0] != 0xE8 || *(DWORD*)(p + 1) != expectedTarget - (at + 5)) return false;
    BYTE code[5];
    code[0] = 0xE8;
    *(DWORD*)(code + 1) = (DWORD)hook - (at + 5);
    return Write(p, code, 5);
}

// True if there is a call to 'target' at 'at'.
static bool CallIs(DWORD at, DWORD target)
{
    const BYTE* p = (const BYTE*)at;
    return p[0] == 0xE8 && *(const DWORD*)(p + 1) == target - (at + 5);
}

static void PatchResolutionCheck()
{
    BYTE* p = (BYTE*)kResLookupCall;
    BYTE* q = (BYTE*)kStartSizeCall;
    if (p[0] != 0xE8 || *(DWORD*)(p + 1) != kResLookup - (kResLookupCall + 5) ||
        q[0] != 0xE8 || *(DWORD*)(q + 1) != kModeRefresh - (kStartSizeCall + 5))
    {
        Log("Resolution check: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return;
    }
    bool ok = RedirectCall(kStartSizeCall, kModeRefresh, (void*)Hook_StartSize) &&
              RedirectCall(kResLookupCall, kResLookup, (void*)Hook_ResLookup);
    Log("Resolution check: %s\r\n", ok ? "applied (a saved resolution not in the list falls back to a supported one)"
                                         : "FAILED to write patch");
}

// ---------------------------------------------------------------------------
// 1. Resolution: text and button icon size above 1080p
// ---------------------------------------------------------------------------

// The game sizes text and button icons from two values (stored at 0xCD295C / 0xCD2960):
// virtual UI width / screen width and virtual UI height (384) / screen height. Above 1080p that
// makes text smaller and smaller. The values are computed at startup (0x721512 / 0x721540) and
// again when display settings such as antialiasing are changed in Options (0x71F257 /
// 0x71F26E). All four store instructions call a small stub that stores the value as before
// and then, only when the screen is taller than 1080, scales it by height / 1080.
static const DWORD kTextStoreX  = 0x721512;   // startup:          fstp dword ptr [ebp+204]
static const DWORD kTextStoreY  = 0x721540;   // startup:          fstp dword ptr [ebp+208]
static const DWORD kTextStoreX2 = 0x71F257;   // settings changed: fstp dword ptr [ebp+204]
static const DWORD kTextStoreY2 = 0x71F26E;   // settings changed: fstp dword ptr [ebp+208]
static const BYTE  kExpStoreX[] = { 0xD9, 0x9D, 0x04, 0x02, 0x00, 0x00 };
static const BYTE  kExpStoreY[] = { 0xD9, 0x9D, 0x08, 0x02, 0x00, 0x00 };
static float       g_k1080 = 1080.0f;

// [ebp+1E8] holds the screen height. ebp is the game's display object.
__declspec(naked) static void TextStoreX()
{
    __asm
    {
        fstp    dword ptr [ebp+0x204]
        cmp     dword ptr [ebp+0x1E8], 1080
        jle     done
        fld     dword ptr [ebp+0x204]
        fild    dword ptr [ebp+0x1E8]
        fmulp   st(1), st
        fdiv    dword ptr [g_k1080]
        fstp    dword ptr [ebp+0x204]
    done:
        ret
    }
}

__declspec(naked) static void TextStoreY()
{
    __asm
    {
        fstp    dword ptr [ebp+0x208]
        cmp     dword ptr [ebp+0x1E8], 1080
        jle     done
        fld     dword ptr [ebp+0x208]
        fild    dword ptr [ebp+0x1E8]
        fmulp   st(1), st
        fdiv    dword ptr [g_k1080]
        fstp    dword ptr [ebp+0x208]
    done:
        ret
    }
}

// Replace one 6-byte store with "call stub" + nop.
static bool RedirectStore(DWORD at, void* stub)
{
    BYTE c[6] = { 0xE8, 0, 0, 0, 0, 0x90 };
    DWORD rel = (DWORD)(UINT_PTR)stub - (at + 5);
    memcpy(c + 1, &rel, 4);
    return Write((BYTE*)(UINT_PTR)at, c, sizeof(c));
}

static bool PatchTextScale()
{
    BYTE* px  = (BYTE*)(UINT_PTR)kTextStoreX;
    BYTE* py  = (BYTE*)(UINT_PTR)kTextStoreY;
    BYTE* px2 = (BYTE*)(UINT_PTR)kTextStoreX2;
    BYTE* py2 = (BYTE*)(UINT_PTR)kTextStoreY2;
    // Check all four places before changing anything.
    if (memcmp(px,  kExpStoreX, sizeof(kExpStoreX)) != 0 ||
        memcmp(py,  kExpStoreY, sizeof(kExpStoreY)) != 0 ||
        memcmp(px2, kExpStoreX, sizeof(kExpStoreX)) != 0 ||
        memcmp(py2, kExpStoreY, sizeof(kExpStoreY)) != 0)
    {
        Log("Text size: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return false;
    }
    bool ok = RedirectStore(kTextStoreX,  (void*)TextStoreX) &&
              RedirectStore(kTextStoreY,  (void*)TextStoreY) &&
              RedirectStore(kTextStoreX2, (void*)TextStoreX) &&
              RedirectStore(kTextStoreY2, (void*)TextStoreY);
    Log("Text size: %s (keeps 1080p text size on taller screens, also after changing display settings)\r\n",
        ok ? "applied" : "FAILED to write patch");
    return ok;
}

// ---------------------------------------------------------------------------
// 2. Movies: freeze fix and larger movies
// ---------------------------------------------------------------------------

// Why it freezes: the movie player (libCriMovie.dll) pins its worker threads and the game's
// main thread to the first CPU core. When the game stops a movie or cutscene, the main thread
// waits in a busy loop for a movie worker; the main thread has the higher priority, so on the
// shared core the worker never runs again (seen at 4K after changing antialiasing, and when
// movies end).
// The movie player pins its threads in one place: at the end of its thread setup
// (libCriMovie.dll RVA 0x2BDA) it calls SetThreadAffinityMask(thread, 1) four times in a row
// (its three threads, and the game thread that called it: GetCurrentThread) and then returns
// success (xor eax,eax at RVA 0x2C2C). The player has no setting of its own for this. The
// first instruction of the four calls (mov eax,[thread], 5 bytes) starts with a 2-byte jump
// to the player's own "success" line, so it skips the pinning and returns success as before.
// The addresses in the checked bytes follow the DLL's load address (it may be moved).
// mov eax,[0x1014C63C] / mov esi,[SetThreadAffinityMask]
static const DWORD kCriPinRva     = 0x2BDA;
static const DWORD kCriPinDoneRva = 0x2C2C;    // xor eax,eax / pop esi / ret
static const DWORD kCriThreadRva  = 0x14C63C;  // the player's first worker thread handle
static const DWORD kCriAffIatRva  = 0x43074;   // the player's SetThreadAffinityMask import

static void PatchMovieThreads()
{
    HMODULE cri = GetModuleHandleA("libCriMovie.dll");
    if (!cri) { Log("Movie fix: NOT applied (libCriMovie.dll not loaded)\r\n"); return; }
    BYTE* base = (BYTE*)cri;
    BYTE* p = base + kCriPinRva;
    BYTE* done = base + kCriPinDoneRva;
    if (p[0] != 0xA1 || *(DWORD*)(p + 1) != (DWORD)(base + kCriThreadRva) ||
        p[5] != 0x8B || p[6] != 0x35 || *(DWORD*)(p + 7) != (DWORD)(base + kCriAffIatRva) ||
        done[0] != 0x33 || done[1] != 0xC0 || done[2] != 0x5E || done[3] != 0xC3)
    {
        Log("Movie fix: NOT applied (libCriMovie.dll is a different version)\r\n");
        return;
    }
    BYTE jmp[2] = { 0xEB, (BYTE)(done - (p + 2)) };   // jmp short to the player's "success" line
    bool ok = Write(p, jmp, sizeof(jmp));
    Log("Movie fix: %s\r\n", ok ? "applied (movie threads may use any CPU core)" : "FAILED to write patch");
}

// Larger movies: the movie player's size check (section 2). When a movie opens, the player
// reads its header and checks width x height against 720x576 (cmp ecx,0x65400), the largest
// TV picture; larger movies, such as the Xbox 360 release's 1024x768 and 1280x720 movies, fail
// with "Can not get header information". The player sizes its buffers from each movie's own
// width and height after this check, so raising the limit is safe.
static const DWORD kCriSizeCheck = 0x14817;   // libCriMovie.dll RVA: cmp ecx,0x65400
static void PatchMovieSize()
{
    HMODULE cri = GetModuleHandleA("libCriMovie.dll");
    if (!cri) { Log("Larger movies: NOT applied (libCriMovie.dll not loaded)\r\n"); return; }
    BYTE* p = (BYTE*)cri + kCriSizeCheck;
    static const BYTE expect[6] = { 0x81, 0xF9, 0x00, 0x54, 0x06, 0x00 };
    if (memcmp(p, expect, sizeof(expect)) != 0)
    {
        Log("Larger movies: NOT applied (libCriMovie.dll is a different version)\r\n");
        return;
    }
    DWORD limit = 4095 * 4095, old;   // the largest MPEG-1 picture: no limit
    VirtualProtect(p + 2, 4, PAGE_EXECUTE_READWRITE, &old);
    memcpy(p + 2, &limit, 4);
    VirtualProtect(p + 2, 4, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 6);
    Log("Larger movies: applied (movies of any size can play)\r\n");
}


// ---------------------------------------------------------------------------
// 3. Graphics: anisotropic filtering and antialiasing
// ---------------------------------------------------------------------------

// --- Anisotropic filtering --------------------------------------------------
// The engine sends its own maxAnisotropy setting with every texture, but the game never sets
// it (it stays 1), and mipmapped textures get plain linear filtering. (The game gives the
// engine its other texture settings at 0x720B33, after the engine has already read them, so
// those never take effect either.)
// libIGGfx.dll: the engine's filter table (one entry per engine filter: nearest, linear,
// nearest/mip nearest, nearest/mip linear, linear/mip nearest, linear/mip linear,
// anisotropic) and its mip filter table. Entries 4 and 5 (mipmapped linear) become
// Direct3D anisotropic (3), the value of the engine's own anisotropic entry.
static const DWORD kFilterTableRva = 0x1EC1A0;
static const DWORD kMipTableRva    = 0x1EC1BC;
static const int   kExpFilterTable[7] = { 1, 2, 1, 1, 2, 2, 3 };
static const int   kExpMipTable[7]    = { 0, 0, 1, 2, 1, 2, 3 };
// The engine's six magnification reads of the filter table ("mov reg,[index*4+table]");
// they get a copy of the table with the engine's original values.
static const DWORD kMagReadRva[6] = { 0x46A10, 0x46A56, 0x46C6D, 0x50C21, 0x50E69, 0x50E91 };
static int         g_magTable[7];
// The engine reads its maxAnisotropy setting once, while it sets up its display context:
// libIGGfx.dll, "mov ecx,esi / call 0x10046EF0" at RVA 0x68A16 (inside the display context's
// setup, no arguments, plain ret). The value is given to the engine right before that read,
// with the engine's own igRegistry::setValue. The cap comes from the engine's own answer from
// the graphics card: its device object (igDxDevice, at [display context + 0x318C]) asks the
// card when it is set up (GetDeviceCaps, 0x1003B41B) and keeps the answer (D3DCAPS9) at
// [device + 0x10]; MaxAnisotropy is at +0x6C. The answer is there at the read, and the
// read happens once per game start.
static const DWORD kEngAnisoCallRva = 0x68A18;   // call 0x10046EF0
static const DWORD kEngAnisoReadRva = 0x46EF0;
static const DWORD kArkCoreSlot    = 0x798AC0;   // MUA.exe import: ArkCore_function
static const DWORD kGetRegSlot     = 0x798ABC;   // MUA.exe import: igArkCore::getRegistry
// MUA.exe import: igRegistry::setValue(int, const char*, int)
static const DWORD kRegSetIntSlot  = 0x79880C;
typedef bool  (__thiscall* RegSetInt_t)(void*, int, const char*, int);
typedef void* (__cdecl*    ArkCore_t)();
typedef void* (__thiscall* GetReg_t)(void*);
typedef void  (__fastcall* EngAnisoRead_t)(void* self, void* edx);
static EngAnisoRead_t g_realEngAnisoRead;

// The card's maximum anisotropy as the engine keeps it (0 if it is not there).
static int EngineMaxAnisotropy(void* vc)
{
    int max = 0;
    __try
    {
        BYTE* dev  = *(BYTE**)((BYTE*)vc + 0x318C);
        BYTE* caps = dev ? *(BYTE**)(dev + 0x10) : NULL;
        // DeviceType 0 = not filled
        if (caps && *(DWORD*)caps != 0) max = (int)*(DWORD*)(caps + 0x6C);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { max = 0; }
    return max;
}

// Right before the engine reads maxAnisotropy: the setting, capped at the card's maximum.
static void __fastcall Hook_EngAnisoRead(void* self, void* edx)
{
    int level = g_aniso;
    int max = EngineMaxAnisotropy(self);
    if (max >= 1 && level > max)
    {
        while (level > max) level /= 2;
        Log("Anisotropy: the graphics card supports up to %dx, using %dx\r\n", max, level);
    }
    else if (max < 1)
        Log("Anisotropy: the graphics card's maximum is not known, using %dx\r\n", level);
    void* core = ((ArkCore_t)*(void**)kArkCoreSlot)();
    void* reg  = core ? ((GetReg_t)*(void**)kGetRegSlot)(core) : NULL;
    bool ok = reg && ((RegSetInt_t)*(void**)kRegSetIntSlot)(reg, 4, "maxAnisotropy", level);
    if (!ok) Log("Anisotropy: the engine did not accept maxAnisotropy\r\n");
    WriteLog();
    g_realEngAnisoRead(self, edx);
}

static void PatchAnisotropy()
{
    if (g_aniso <= 1)
    {
        Log("Anisotropy: off (Anisotropy=1, the game's original filtering)\r\n");
        return;
    }
    HMODULE gfx = GetModuleHandleA("libIGGfx.dll");
    int* filt = gfx ? (int*)((BYTE*)gfx + kFilterTableRva) : NULL;
    int* mip  = gfx ? (int*)((BYTE*)gfx + kMipTableRva) : NULL;
    DWORD readCall = gfx ? (DWORD)gfx + kEngAnisoCallRva : 0;
    DWORD readFunc = gfx ? (DWORD)gfx + kEngAnisoReadRva : 0;
    bool match = filt && memcmp(filt, kExpFilterTable, sizeof(kExpFilterTable)) == 0 &&
                 memcmp(mip, kExpMipTable, sizeof(kExpMipTable)) == 0 &&
                 CallIs(readCall, readFunc);
    for (int i = 0; i < 6 && match; ++i)
    {
        const BYTE* q = (const BYTE*)gfx + kMagReadRva[i];
        if (q[0] != 0x8B || *(DWORD*)(q + 3) != (DWORD)filt) match = false;
    }
    if (!match)
    {
        Log("Anisotropy: NOT applied. The code in libIGGfx.dll does not match the expected bytes.\r\n");
        return;
    }
    memcpy(g_magTable, filt, sizeof(g_magTable));
    DWORD magAt = (DWORD)g_magTable;
    bool ok = true;
    for (int i = 0; i < 6 && ok; ++i)
        ok = Write((BYTE*)gfx + kMagReadRva[i] + 3, (const BYTE*)&magAt, 4);
    static const int aniso = 3;   // Direct3D anisotropic
    ok = ok && Write((BYTE*)&filt[4], (const BYTE*)&aniso, 4) && Write((BYTE*)&filt[5], (const BYTE*)&aniso, 4);
    g_realEngAnisoRead = (EngAnisoRead_t)readFunc;
    ok = ok && RedirectCall(readCall, readFunc, (void*)Hook_EngAnisoRead);
    if (g_anisoAsked != g_aniso && g_anisoAsked > 0)
        Log("Anisotropy: Anisotropy=%d is not a level, using %d\r\n", g_anisoAsked, g_aniso);
    if (ok) Log("Anisotropy: applied (%dx)\r\n", g_aniso);
    else    Log("Anisotropy: FAILED to write patch\r\n");
}

// --- Antialiasing ------------------------------------------------------------
// The game keeps a list of six antialiasing levels (off, 2x, 4x, 6x, 8x, 16x; 0x74D081), and
// its Options menu only offers the levels marked as supported (0x74D110). When it marks them,
// it asks Direct3D about the first three only (the loop at 0x74D104); it now asks about all
// six. The way from the menu to Direct3D only knew off, 2x and 4x; four small translations are
// extended to all six levels, numbered by their place in the game's list (as the engine
// already numbers off, 2x and 4x): the saved choice at startup (0x720AC5), the choice in
// Options (0x71F0DA), and the engine's level to Direct3D sample count (libIGGfx.dll
// 0x1005AD64 and 0x10018D29).
static const DWORD kAaLoopAt    = 0x74D104;   // cmp ebx,3 / jl 0x74D0B5
static const BYTE  kExpAaLoop[] = { 0x83, 0xFB, 0x03, 0x7C, 0xAC };
// The engine's level (= the place in the game's list) -> Direct3D sample count.
static const DWORD kAaSamples[6] = { 0, 2, 4, 6, 8, 16 };
// Direct3D sample count (0-16) -> the engine's level, -1 = not a level.
static const signed char kAaLevelOf[17] = { 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, -1, -1, -1, -1, -1, -1, 5 };
static DWORD g_engAa1Back;   // where the engine continues after Hook_AaEngine1

// MUA.exe 0x720AC5 (startup): [esp+0x78] = the engine's level, from the saved choice (0xD40298).
__declspec(naked) static void Hook_AaStart()
{
    __asm {
        mov  eax, dword ptr ds:[0xD40298]
        cmp  eax, 5
        ja   start_keep
        mov  [esp+0x78], eax
    start_keep:
        push 0x720AEC
        ret
    }
}

// MUA.exe 0x71F0DA (Options): eax = the chosen sample count. The game's own
// "mov [esp+0x2c],edx" is repeated; esi and [esp+0x34] = the engine's level.
__declspec(naked) static void Hook_AaApply()
{
    __asm {
        mov   [esp+0x2C], edx
        cmp   eax, 16
        ja    apply_keep
        movsx edx, byte ptr [kAaLevelOf+eax]
        test  edx, edx
        js    apply_keep
        mov   esi, edx
        mov   [esp+0x34], esi
    apply_keep:
        push  0x71F100
        ret
    }
}

// libIGGfx.dll (+0x5AD64): ecx = the engine's level -> ecx = sample count.
__declspec(naked) static void Hook_AaEngine1()
{
    __asm {
        cmp  ecx, 5
        ja   eng1_none
        mov  ecx, dword ptr [kAaSamples+ecx*4]
        jmp  dword ptr [g_engAa1Back]
    eng1_none:
        xor  ecx, ecx
        jmp  dword ptr [g_engAa1Back]
    }
}

// libIGGfx.dll (+0x18D29, igDxDevice::getMultiSampleType): eax = the engine's level ->
// sample count.
__declspec(naked) static void Hook_AaEngine2()
{
    __asm {
        cmp  eax, 5
        ja   eng2_none
        mov  eax, dword ptr [kAaSamples+eax*4]
        ret
    eng2_none:
        xor  eax, eax
        ret
    }
}

static bool JmpTo(BYTE* at, size_t len, void* to)
{
    BYTE code[64];
    if (len < 5 || len > sizeof(code)) return false;
    memset(code, 0x90, len);
    code[0] = 0xE9;
    *(DWORD*)(code + 1) = (DWORD)to - ((DWORD)at + 5);
    return Write(at, code, len);
}

static const BYTE kExpAaStart[] = { 0xA1, 0x98, 0x02, 0xD4, 0x00, 0x2B, 0xC6, 0x74, 0x1A, 0x48, 0x74, 0x0D, 0x48, 0x75, 0x18,
                                    0xC7, 0x44, 0x24, 0x78, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x0E, 0xC7, 0x44, 0x24, 0x78, 0x01,
                                    0x00, 0x00, 0x00, 0xEB, 0x04, 0x89, 0x74, 0x24, 0x78 };
static const BYTE kExpAaApply[] = { 0x2B, 0xC7, 0x89, 0x54, 0x24, 0x2C, 0x74, 0x18, 0x83, 0xE8, 0x02, 0x74, 0x0C, 0x83, 0xE8,
                                    0x02, 0x75, 0x14, 0xBE, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x09, 0xBE, 0x01, 0x00, 0x00, 0x00,
                                    0xEB, 0x02, 0x33, 0xF6, 0x89, 0x74, 0x24, 0x34 };
static const BYTE kExpAaEng1[]  = { 0x49, 0x74, 0x0E, 0x49, 0x74, 0x04, 0x33, 0xC9, 0xEB, 0x0C, 0xB9, 0x04, 0x00, 0x00, 0x00,
                                    0xEB, 0x05, 0xB9, 0x02, 0x00, 0x00, 0x00 };
static const BYTE kExpAaEng2[]  = { 0x48, 0x74, 0x0C, 0x48, 0x74, 0x03, 0x33, 0xC0, 0xC3, 0xB8, 0x04, 0x00, 0x00, 0x00, 0xC3,
                                    0xB8, 0x02, 0x00, 0x00, 0x00, 0xC3 };

static void PatchAntialiasing()
{
    if (!g_aaOn)
    {
        Log("Antialiasing: off (Antialiasing=0, the game's original choices)\r\n");
        return;
    }
    HMODULE gfx = GetModuleHandleA("libIGGfx.dll");
    BYTE* e1 = gfx ? (BYTE*)gfx + 0x5AD64 : NULL;
    BYTE* e2 = gfx ? (BYTE*)gfx + 0x18D29 : NULL;
    if (!e1 || memcmp(e1, kExpAaEng1, sizeof(kExpAaEng1)) != 0 || memcmp(e2, kExpAaEng2, sizeof(kExpAaEng2)) != 0 ||
        memcmp((void*)0x720AC5, kExpAaStart, sizeof(kExpAaStart)) != 0 ||
        memcmp((void*)0x71F0DA, kExpAaApply, sizeof(kExpAaApply)) != 0 ||
        memcmp((void*)kAaLoopAt, kExpAaLoop, sizeof(kExpAaLoop)) != 0)
    {
        Log("Antialiasing: NOT applied. The code in this mua.exe or libIGGfx.dll does not match the expected bytes.\r\n");
        return;
    }
    g_engAa1Back = (DWORD)gfx + 0x5AD7A;
    static const BYTE six = 0x06;
    bool ok = JmpTo(e1, sizeof(kExpAaEng1), (void*)Hook_AaEngine1) &&
              JmpTo(e2, sizeof(kExpAaEng2), (void*)Hook_AaEngine2) &&
              JmpTo((BYTE*)0x720AC5, sizeof(kExpAaStart), (void*)Hook_AaStart) &&
              JmpTo((BYTE*)0x71F0DA, sizeof(kExpAaApply), (void*)Hook_AaApply) &&
              Write((BYTE*)kAaLoopAt + 2, &six, 1);
    Log("Antialiasing: %s\r\n", ok ? "applied (Options offers every level the graphics card supports)"
                                   : "FAILED to write patch");
}

// ---------------------------------------------------------------------------
// 4. Windowed mode
// ---------------------------------------------------------------------------
// The game's engine has its own windowed mode: the window class igWin32Window in
// libIGDisplay.dll switches between fullscreen and windowed in setVideoMode(fullscreen,
// width, height, bits) (vtable entry +0x80), and opens the game window with
// open(title, width, height) (+0x5C). MUA.exe always turns fullscreen on at startup
// (0x720A5A, 0x720B1C, 0x720B21); MUAFixPack makes those three places write "off", so
// the engine starts the game windowed. With [Windowed] Enabled=0 nothing is changed and
// the game starts in its original fullscreen.
// Fill the screen (hotkey, default Ctrl+N): the window becomes the largest size with the
// game's shape that fits the monitor, centered and borderless; Direct3D stretches the
// picture to it (the picture itself stays at the game's resolution, see PatchEngineSize).
// A plain black window from MUAFixPack sits right behind it and covers the monitor, so the
// rest of the screen is black. It can't be clicked into or focused and isn't in the
// taskbar or alt-tab. The game reads the mouse at 0x416444-0x416464 (cursor minus the
// picture's top-left corner, used as a position on the picture); while the screen is
// filled, that position is scaled by the game's resolution / the window's size at
// 0x41645F. The same hotkey goes back to the normal window.
// Normally the window is the size of the game's resolution (no stretching). Border=1
// gives it a border and title bar; the border hotkey turns the border on and off.
// Position (1-9, laid out like a number pad: 7 8 9 top, 4 5 6 middle, 1 2 3 bottom)
// places it on the screen; the four move hotkeys move it one step on that 3x3 grid.
// The game's own calls to setVideoMode (for example when the resolution is changed in
// Options) are kept windowed. After the engine has set up the window, MUAFixPack sets
// its border and place. Hotkeys are read from the key presses the engine passes to the
// game's message function, and run right there, on the game's own thread. Switching to the
// engine's fullscreen while the game runs is not offered:
// Direct3D refuses the rebuild that the engine's fullscreen switch asks for.



static volatile bool   g_border;            // the window has a border and title bar
// place on the 3x3 grid (0-2 each; row 0 = top), default center
static volatile int    g_col = 1, g_row = 1;
// AlwaysOnTop: the window stays in front of other windows
static bool            g_onTop;
static HWND            g_gameWnd;

// Hotkeys: border on/off, and moving the window one step up, left, down, right.
enum { kKeyBorder, kKeyUp, kKeyLeft, kKeyDown, kKeyRight, kKeyFill, kKeyCount };
static const int kRefill = 100;   // not a hotkey: fill the screen again after a resolution change
struct Hotkey { UINT vk; bool ctrl, alt, shift; };
static Hotkey g_hotkeys[kKeyCount] =
{
    { 'B', true, false, false },   // Ctrl+B
    { 'I', true, false, false },   // Ctrl+I
    { 'J', true, false, false },   // Ctrl+J
    { 'K', true, false, false },   // Ctrl+K
    { 'L', true, false, false },   // Ctrl+L
    { 'N', true, false, false },   // Ctrl+N
};
static const char* const kHotkeyKeys[kKeyCount] = { "HotkeyBorder", "HotkeyUp", "HotkeyLeft", "HotkeyDown", "HotkeyRight", "HotkeyFillScreen" };

// --- Start windowed: MUA.exe's three "fullscreen on" places write "off" -------
// mov byte ptr [edi+4],1    (engine fullScreen setting)
static const DWORD kFullScreenSetting = 0x720B21;
// mov byte ptr [esp+0x60],1 (display format: full screen)
static const DWORD kFormatFull1       = 0x720A5A;
// mov byte ptr [esp+0x60],1 (set again before use)
static const DWORD kFormatFull2       = 0x720B1C;
static const BYTE  kExpSetting[] = { 0xC6, 0x47, 0x04, 0x01 };
static const BYTE  kExpFormat[]  = { 0xC6, 0x44, 0x24, 0x60, 0x01 };

static bool PatchStartWindowed()
{
    BYTE* a = (BYTE*)kFullScreenSetting;
    BYTE* b = (BYTE*)kFormatFull1;
    BYTE* c = (BYTE*)kFormatFull2;
    if (memcmp(a, kExpSetting, sizeof(kExpSetting)) != 0 ||
        memcmp(b, kExpFormat, sizeof(kExpFormat)) != 0 ||
        memcmp(c, kExpFormat, sizeof(kExpFormat)) != 0)
    {
        Log("Windowed: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return false;
    }
    static const BYTE zero = 0x00;   // the last byte of each instruction is the value written
    bool ok = Write(a + 3, &zero, 1) && Write(b + 4, &zero, 1) && Write(c + 4, &zero, 1);
    if (!ok) Log("Windowed: FAILED to write patch\r\n");
    return ok;
}

// --- The engine's window (libIGDisplay.dll) ----------------------------------
static const DWORD kWinVtableRva    = 0xBBC0;   // igWin32Window vtable
static const DWORD kWinOpenRva      = 0x77A0;   // open(const char*, int, int)        at +0x5C
static const DWORD kWinVideoModeRva = 0x60B0;   // setVideoMode(bool, int, int, int) at +0x80
static const BYTE  kExpVideoMode[] = { 0x83, 0xEC, 0x1C, 0x53, 0x8A, 0x5C, 0x24, 0x24 };

typedef bool (__thiscall* WinOpen_t)(void*, const char*, int, int);
typedef bool (__thiscall* WinVideoMode_t)(void*, bool, int, int, int);
typedef HWND (__thiscall* WinGetHwnd_t)(const void*);
typedef void (__thiscall* WinSetPos_t)(void*, int, int, bool);
// igWin32Window::setPosition (vtable +0x6C): moves the window and updates the engine's copy
// of its place
static WinSetPos_t    g_winSetPos;
static WinOpen_t      g_realWinOpen;
static WinVideoMode_t g_realWinVideoMode;
static WinGetHwnd_t   g_winGetHwnd;
static void**         g_winCurrent;        // igWin32Window::_CurrentWindow
static UINT           g_hotkeyMsg;         // message the hotkeys send to the game window
static WNDPROC        g_oldWndProc;
// the game's picture size (its resolution), as last given to the engine
static volatile int   g_picW, g_picH;
static volatile bool  g_dragging;          // the player is dragging the window by its title bar
static volatile bool  g_fill;              // fill the screen is on
static volatile int   g_fillW, g_fillH;    // the window's size while the screen is filled
static HWND           g_backdrop;          // the black window behind the game window

static LRESULT CALLBACK Win_WndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp);

// --- The game's own mouse area (MUA.exe) ---------------------------------------
// Besides the engine, MUA.exe has its own mouse handling (object at [0xD40570], read by
// 0x758D80). It keeps the screen area of the game picture, uses it to keep the mouse
// inside and to work out where the mouse is on the picture, and sets it with 0x403C60
// (on, ?, area). The game only sets that area when the display settings change
// (0x71F2E9-0x71F332: GetWindowRect, then 0x403C60(1, 0, area)), because in fullscreen
// the window never moves. MUAFixPack sets it the same way whenever the window moves or
// changes. The area is given as { top, bottom, left, right } (the order 0x403C60 reads).
static const DWORD kGameMouseVar  = 0xD40570;
static const DWORD kGameMouseGet  = 0x758D80;   // mov eax,[0xD40570] / ret
static const DWORD kGameMouseArea = 0x403C60;
static const BYTE  kExpMouseGet[]  = { 0xA1, 0x70, 0x05, 0xD4, 0x00, 0xC3 };
static const BYTE  kExpMouseArea[] = { 0x8B, 0x41, 0x04, 0x8A, 0x4C, 0x24, 0x04, 0x84, 0xC9 };
typedef void (__thiscall* GameMouseArea_t)(void*, bool, bool, int*);
static bool g_gameMouseOk;      // the code above matched

static void UpdateGameMouseArea()
{
    if (!g_gameMouseOk || !g_gameWnd || g_picW <= 0 || g_picH <= 0) return;
    void* mouse = *(void**)kGameMouseVar;
    if (!mouse) return;
    BYTE* state = *(BYTE**)((BYTE*)mouse + 4);
    if (!state) return;
    POINT o = { 0, 0 };
    ClientToScreen(g_gameWnd, &o);                       // top-left of the game picture
    int pw = g_fill ? g_fillW : g_picW, ph = g_fill ? g_fillH : g_picH;
    int area[4] = { o.y, o.y + ph, o.x, o.x + pw };
    bool on = state[1] != 0;
    if (on) ((GameMouseArea_t)kGameMouseArea)(mouse, true, state[2] != 0, area);
    else
    {
        // Keeping the mouse inside is off right now: only the area is updated.
        int* r = (int*)(state + 0x40);                   // left, top, right, bottom
        r[0] = o.x; r[1] = o.y; r[2] = o.x + pw; r[3] = o.y + ph;
    }
}

// --- Fill the screen: the black window behind the game window ---------------------
static LRESULT CALLBACK Backdrop_WndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_MOUSEACTIVATE) return MA_NOACTIVATE;   // clicking it never takes the focus
    return DefWindowProcA(w, msg, wp, lp);
}

static void ShowBackdrop(HWND game, const RECT& m)
{
    if (!g_backdrop)
    {
        WNDCLASSA wc;
        memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc   = Backdrop_WndProc;
        wc.hInstance     = g_self;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor       = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
        wc.lpszClassName = "MUAFixPack_Backdrop";
        RegisterClassA(&wc);
        g_backdrop = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, "MUAFixPack_Backdrop", "",
                                     WS_POPUP, m.left, m.top, m.right - m.left, m.bottom - m.top,
                                     NULL, NULL, g_self, NULL);
        if (!g_backdrop) { Log("Windowed: could not create the black background\r\n"); return; }
    }
    SetWindowPos(g_backdrop, game, m.left, m.top, m.right - m.left, m.bottom - m.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

static void KeepBackdropBehind()
{
    if (g_fill && g_backdrop && g_gameWnd && IsWindowVisible(g_backdrop))
        SetWindowPos(g_backdrop, g_gameWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

static void HideBackdrop()
{
    if (g_backdrop) ShowWindow(g_backdrop, SW_HIDE);
}

// The game reads the mouse (0x416444-0x416464): eax/ecx = cursor minus the picture's
// top-left corner. While the screen is filled, both are scaled to the game's resolution
// before the game stores them; the game's own instructions at 0x41645F (pop edi / pop esi /
// mov [ebp+4],eax) are repeated here. The picture's area from GetWindowInfo is at
// [esp+0x50] left, +0x54 top, +0x58 right, +0x5C bottom (4 more here after push ecx).
static const DWORD kMouseReadAt = 0x41645F;
static const BYTE  kExpMouseRead[] = { 0x5F, 0x5E, 0x89, 0x45, 0x04 };
__declspec(naked) static void Hook_MouseScale()
{
    __asm {
        cmp  byte ptr [g_fill], 0
        je   ms_done
        push ecx
        mov  ecx, [esp+0x5C]
        sub  ecx, [esp+0x54]
        jle  ms_x_keep
        imul eax, dword ptr [g_picW]
        cdq
        idiv ecx
    ms_x_keep:
        xchg eax, [esp]
        mov  ecx, [esp+0x60]
        sub  ecx, [esp+0x58]
        jle  ms_y_keep
        imul eax, dword ptr [g_picH]
        cdq
        idiv ecx
    ms_y_keep:
        mov  ecx, eax
        pop  eax
    ms_done:
        pop  edi
        pop  esi
        mov  [ebp+4], eax
        push 0x416464
        ret
    }
}

// The game's menus read the mouse with ScreenToClient and use the answer as a position on
// the picture. MUA.exe calls ScreenToClient in exactly four places (0x752872, 0x7528C1,
// 0x752D19, 0x75B98A, each "call [0x798308]"). Those four calls are changed to call the
// function below, which asks Windows the same question and, only while the screen is filled
// and only for the game window, scales the answer to the game's resolution.
typedef BOOL (WINAPI* ScreenToClient_t)(HWND, LPPOINT);
static ScreenToClient_t g_realScreenToClient;
static BOOL WINAPI Hook_ScreenToClient(HWND h, LPPOINT p)
{
    BOOL ok = g_realScreenToClient(h, p);
    if (!ok || !p || !g_fill || h != g_gameWnd) return ok;
    RECT c;
    if (GetClientRect(h, &c) && c.right > 0 && c.bottom > 0 && g_picW > 0 && g_picH > 0)
    {
        p->x = (LONG)((__int64)p->x * g_picW / c.right);
        p->y = (LONG)((__int64)p->y * g_picH / c.bottom);
    }
    return ok;
}

// Fill the screen: where the game window goes. The largest size with the game's shape
// (cw x ch) that fits the monitor the window is on, centered. False if unknown.
static bool FillGeometry(HWND h, int cw, int ch, RECT* mon, int* fx, int* fy, int* fw, int* fh)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    HMONITOR m = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    if (!m || !GetMonitorInfoA(m, &mi) || cw <= 0 || ch <= 0) return false;
    const RECT& r = mi.rcMonitor;
    int mw = r.right - r.left, mh = r.bottom - r.top;
    int w = mw, hh = (int)((__int64)mw * ch / cw);
    if (hh > mh) { hh = mh; w = (int)((__int64)mh * cw / ch); }
    *mon = r;
    *fw = w;
    *fh = hh;
    *fx = r.left + (mw - w) / 2;
    *fy = r.top + (mh - hh) / 2;
    return true;
}

// Border and size of the game window, after the engine set it up. 'place' puts it on its
// grid position; otherwise a window with a border stays where the player dragged it.
static void StyleWindowNow(void* win, bool place);

// The engine window's own _resizeContext setting (libIGDisplay, igWin32Window +0x45,
// on by default): "when the window changes size, does the picture change with it?". The
// engine reads it only in its WM_SIZE handler (0x5FC2), where "yes" rebuilds the display.
// For the plugin's own window changes (fill, border, place) the picture stays at the game's
// resolution, so the setting is off while they happen and back as it was right after.
// Resolution changes (the engine's setVideoMode) still rebuild the display as before.
static const int kResizeContextAt = 0x45;

// The window is held at its old size and place during a resolution change (fill
// off), and takes its new size and place after the engine has shown its first frame at the
// new resolution (the engine's own endDraw, see PatchEndDraw).
static volatile bool g_holding;      // the window keeps g_holdRect (WM_WINDOWPOSCHANGING)
static volatile bool g_holdWaiting;  // waiting for the engine's next finished frame
static RECT g_holdRect;
static bool g_endDrawOn;             // the engine's endDraw is hooked
static const int kPlaceAfterChange = 101;   // not a hotkey: place the window after the hold

static void StyleWindow(void* win, bool place)
{
    g_holding = false;       // any placement ends a hold
    g_holdWaiting = false;
    BYTE* rc = (BYTE*)win + kResizeContextAt;
    BYTE was = *rc;
    *rc = 0;
    StyleWindowNow(win, place);
    *rc = was;
}

static void StyleWindowNow(void* win, bool place)
{
    HWND h = g_winGetHwnd ? g_winGetHwnd(win) : NULL;
    if (!h) return;
    g_gameWnd = h;
    if (!g_oldWndProc)
    {
        g_oldWndProc = (WNDPROC)SetWindowLongPtrA(h, GWLP_WNDPROC, (LONG_PTR)Win_WndProc);
        if (!g_oldWndProc) Log("Windowed: could not watch the game window (resolution change and fill holds and mouse area not available)\r\n");
    }

    RECT client, wr;
    GetClientRect(h, &client);
    GetWindowRect(h, &wr);
    int cw = client.right - client.left, ch = client.bottom - client.top;
    if (g_picW > 0 && g_picH > 0) { cw = g_picW; ch = g_picH; }   // the game's resolution

    LONG style = GetWindowLongA(h, GWL_STYLE);
    if (g_fill)
    {
        // Fill the screen: the largest size with the game's shape, centered, borderless.
        RECT m;
        int fx, fy, fw, fh;
        if (FillGeometry(h, cw, ch, &m, &fx, &fy, &fw, &fh))
        {
            g_fillW = fw;
            g_fillH = fh;
            style = (style & ~0x00CF0000) | WS_POPUP;
            SetWindowLongA(h, GWL_STYLE, style);
            // Place and size in one step (no stop at the old place on the way). The
            // engine's setPosition can't give the fill size (it sizes the window from the
            // game's resolution); the engine records the new place itself (its WM_MOVE
            // handler, libIGDisplay 0x3520).
            SetWindowPos(h, g_onTop ? HWND_TOPMOST : HWND_NOTOPMOST, fx, fy, fw, fh,
                         SWP_FRAMECHANGED | SWP_NOACTIVATE);
            ShowBackdrop(h, m);
            Log("Windowed: fill screen, %dx%d shown at %dx%d\r\n", cw, ch, fw, fh);
            UpdateGameMouseArea();
            WriteLog();
            return;
        }
        g_fill = false;   // no monitor information: stay a normal window
    }
    HideBackdrop();
    if (g_border) style = (style | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX) & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    // the same bits the engine removes for fullscreen
    else          style = (style & ~0x00CF0000) | WS_POPUP;
    SetWindowLongA(h, GWL_STYLE, style);

    RECT r = { 0, 0, cw, ch };
    if (g_border) AdjustWindowRectEx(&r, style, FALSE, GetWindowLongA(h, GWL_EXSTYLE));
    int w = r.right - r.left, hh = r.bottom - r.top;
    int x = wr.left, y = wr.top;
    if (place || !g_border)
    {
        // The grid: the usable screen area (above the taskbar), or the whole monitor
        // in a direction the window does not fit (a window as large as the screen).
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        HMONITOR mon = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
        if (mon && GetMonitorInfoA(mon, &mi))
        {
            const RECT& wa = mi.rcWork;
            const RECT& m  = mi.rcMonitor;
            int waW = wa.right - wa.left, waH = wa.bottom - wa.top;
            x = w >= waW ? m.left : wa.left + g_col * (waW - w) / 2;
            y = hh >= waH ? m.top : wa.top + g_row * (waH - hh) / 2;
        }
    }
    // The border first, without moving or sizing anything. Then the engine's own
    // setPosition places and sizes the window in one step: it sizes it from the engine's
    // _windowWidth / _windowHeight (the game's resolution, kept so by the engine size fix)
    // plus the border. setPosition does nothing when the window is already at that place,
    // so then only the size is set (the window doesn't move).
    SetWindowPos(h, g_onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    if (g_winSetPos) g_winSetPos(win, x, y, true);
    else SetWindowPos(h, NULL, x, y, w, hh, SWP_NOZORDER | SWP_NOACTIVATE);
    RECT now;
    if (GetWindowRect(h, &now) && (now.right - now.left != w || now.bottom - now.top != hh))
        SetWindowPos(h, NULL, 0, 0, w, hh, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    UpdateGameMouseArea();
    WriteLog();
}

// The engine opens the game window (at startup).
static bool __fastcall Hook_WinOpen(void* self, void*, const char* title, int w, int h)
{
    if (w > 0 && h > 0) { g_picW = w; g_picH = h; }
    bool ok = g_realWinOpen(self, title, w, h);
    Log("Windowed: game window opened, %dx%d\r\n", w, h);
    if (ok) StyleWindow(self, true);
    else WriteLog();
    return ok;
}

// The game sets the video mode (for example a new resolution in Options): kept windowed.
static bool __fastcall Hook_WinVideoMode(void* self, void*, bool fullscreen, int w, int h, int bits)
{
    if (w > 0 && h > 0) { g_picW = w; g_picH = h; }
    // With fill off, the window is held where it is while the engine changes resolution.
    HWND hw = g_winGetHwnd ? g_winGetHwnd(self) : NULL;
    // It needs the game window and MUAFixPack's own window message, to place the window
    // after the first new frame; without them it is placed right away, as before.
    bool hold = !g_fill && g_endDrawOn && hw && GetWindowRect(hw, &g_holdRect);
    if (hold && (!g_gameWnd || !g_hotkeyMsg))
    {
        Log("Windowed: resolution change hold not available, the window is placed right away\r\n");
        hold = false;
    }
    if (hold)
    {
        g_holdWaiting = false;
        g_holding = true;
    }
    bool ok = g_realWinVideoMode(self, false, w, h, bits);
    Log("Windowed: video mode set to windowed %dx%d (asked: %s), result %d\r\n",
        w, h, fullscreen ? "fullscreen" : "windowed", ok ? 1 : 0);
    if (hold && g_holding)
    {
        // The engine put its border style back (0x61E9): take it off again without moving
        // or sizing anything (the engine's _resizeContext off, so no extra rebuild), then
        // wait for the engine's next finished frame.
        BYTE* rc = (BYTE*)self + kResizeContextAt;
        BYTE was = *rc;
        *rc = 0;
        LONG style = GetWindowLongA(hw, GWL_STYLE);
        if (g_border) style = (style | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX) & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        else          style = (style & ~0x00CF0000) | WS_POPUP;
        SetWindowLongA(hw, GWL_STYLE, style);
        SetWindowPos(hw, NULL, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        *rc = was;
        g_holdWaiting = true;
        WriteLog();
        return ok;
    }
    StyleWindow(self, false);
    // The engine finishes its own window changes after this; with the screen filled, fill
    // it again once those are done (a message to the game window, handled in order).
    if (g_fill && g_gameWnd && g_hotkeyMsg) PostMessageA(g_gameWnd, g_hotkeyMsg, (WPARAM)kRefill, 0);
    return ok;
}

// A hotkey, or one of MUAFixPack's own window messages (kRefill, kPlaceAfterChange).
static void RunWindowAction(void* win, int key)
{
    if (key == kRefill)
    {
        if (g_fill) StyleWindow(win, true);
    }
    else if (key == kPlaceAfterChange)
    {
        if (g_holding) StyleWindow(win, false);
    }
    else if (key == kKeyFill)
    {
        g_fill = !g_fill;
        Log("Windowed: hotkey pressed, fill screen %s\r\n", g_fill ? "on" : "off");
        StyleWindow(win, true);
    }
    else if (key == kKeyBorder)
    {
        g_border = !g_border;
        Log("Windowed: hotkey pressed, border %s\r\n", g_border ? "on" : "off");
        StyleWindow(win, true);
    }
    else
    {
        int col = g_col, row = g_row;
        if (key == kKeyUp)    row = row > 0 ? row - 1 : 0;
        if (key == kKeyDown)  row = row < 2 ? row + 1 : 2;
        if (key == kKeyLeft)  col = col > 0 ? col - 1 : 0;
        if (key == kKeyRight) col = col < 2 ? col + 1 : 2;
        if (col != g_col || row != g_row)
        {
            g_col = col;
            g_row = row;
            StyleWindow(win, true);
        }
    }
}

// The game's own message function for the game window (windowed), given to the engine with
// setWindowProc at 0x720CBD. The engine's window procedure (libIGDisplay 0x7B10) calls it
// first for every message while the engine handles the game window's messages; the last
// argument says whether the engine handles the message too.
typedef LRESULT (__cdecl* GameMsgFunc_t)(HWND, UINT, WPARAM, LPARAM, bool*);
// push 0x71EB70 / mov ecx,ebx / call [0x798B40] (setWindowProc)
static const DWORD kGameMsgFuncPush = 0x720CBD;
// after the push
static const BYTE  kExpGameMsgCall[] = { 0x8B, 0xCB, 0xFF, 0x15, 0x40, 0x8B, 0x79, 0x00 };
// the function that was given there before: the game's, or another mod's
static GameMsgFunc_t g_nextMsgFunc;

// Every message goes on, unchanged, to the function that was given there before (the game's
// own, or another mod's that passes it on to the game's). A key press (not a repeat)
// that matches a hotkey runs it first. Ctrl, Alt and Shift are read as they were for this
// key press (GetKeyState).
static LRESULT __cdecl Hook_GameMsgFunc(HWND h, UINT msg, WPARAM wp, LPARAM lp, bool* passOn)
{
    if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(lp & 0x40000000) && h == g_gameWnd)
    {
        bool ctrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        bool alt   = (GetKeyState(VK_MENU)    & 0x8000) != 0;
        bool shift = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
        for (int k = 0; k < kKeyCount; ++k)
        {
            const Hotkey& hk = g_hotkeys[k];
            if (!hk.vk || hk.vk != (UINT)wp || hk.ctrl != ctrl || hk.alt != alt || hk.shift != shift) continue;
            void* win = g_winCurrent ? *g_winCurrent : NULL;
            if (win) RunWindowAction(win, k);
            break;
        }
    }
    return g_nextMsgFunc(h, msg, wp, lp, passOn);
}

static void PatchGameMsgFunc()
{
    BYTE* p = (BYTE*)kGameMsgFuncPush;
    // The push and the setWindowProc call after it must be as expected. The function pushed may
    // already be another mod's instead of the game's; MUAFixPack passes everything on to it.
    DWORD next = *(DWORD*)(p + 1);
    if (p[0] != 0x68 || memcmp(p + 5, kExpGameMsgCall, sizeof(kExpGameMsgCall)) != 0 || !next)
    {
        Log("Windowed: hotkeys NOT available (the code in this mua.exe does not match the expected bytes)\r\n");
        return;
    }
    g_nextMsgFunc = (GameMsgFunc_t)next;
    DWORD f = (DWORD)Hook_GameMsgFunc;
    if (!Write(p + 1, (const BYTE*)&f, 4)) Log("Windowed: FAILED to write the hotkey patch\r\n");
}

static LRESULT CALLBACK Win_WndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == g_hotkeyMsg && g_hotkeyMsg)
    {
        void* win = g_winCurrent ? *g_winCurrent : NULL;
        if (win) RunWindowAction(win, (int)wp);
        return 0;
    }
    switch (msg)
    {
    // The window was moved (dragged by its title bar, or moved by anything else): the
    // game's mouse area follows. While dragging, only once at the end.
    case WM_ENTERSIZEMOVE:
        g_dragging = true;
        break;
    case WM_EXITSIZEMOVE:
        g_dragging = false;
        UpdateGameMouseArea();
        break;
    case WM_WINDOWPOSCHANGING:
        // During a hold, any move or resize keeps the window's old size and place.
        if (g_holding && !g_fill && lp)
        {
            WINDOWPOS* wpos = (WINDOWPOS*)lp;
            if ((wpos->flags & (SWP_NOSIZE | SWP_NOMOVE)) != (SWP_NOSIZE | SWP_NOMOVE))
            {
                wpos->x  = g_holdRect.left;
                wpos->y  = g_holdRect.top;
                wpos->cx = g_holdRect.right - g_holdRect.left;
                wpos->cy = g_holdRect.bottom - g_holdRect.top;
                wpos->flags &= ~(SWP_NOSIZE | SWP_NOMOVE);
            }
            break;
        }
        // While the screen is filled, any move or resize of the game window (for example by
        // the engine after a resolution change) gets the fill place and size directly, so the
        // window never shows at its normal size in between.
        if (g_fill && lp && g_picW > 0 && g_picH > 0)
        {
            WINDOWPOS* wpos = (WINDOWPOS*)lp;
            if ((wpos->flags & (SWP_NOSIZE | SWP_NOMOVE)) != (SWP_NOSIZE | SWP_NOMOVE))
            {
                RECT m;
                int fx, fy, fw, fh;
                if (FillGeometry(w, g_picW, g_picH, &m, &fx, &fy, &fw, &fh))
                {
                    wpos->x = fx;
                    wpos->y = fy;
                    wpos->cx = fw;
                    wpos->cy = fh;
                    wpos->flags &= ~(SWP_NOSIZE | SWP_NOMOVE);
                    g_fillW = fw;
                    g_fillH = fh;
                }
            }
        }
        break;
    case WM_WINDOWPOSCHANGED:
        {
            LRESULT r = CallWindowProcA(g_oldWndProc, w, msg, wp, lp);   // the engine first
            KeepBackdropBehind();
            return r;
        }
    case WM_MOVE:
        if (!g_dragging)
        {
            LRESULT r = CallWindowProcA(g_oldWndProc, w, msg, wp, lp);   // the engine first
            UpdateGameMouseArea();
            return r;
        }
        break;
    }
    return CallWindowProcA(g_oldWndProc, w, msg, wp, lp);
}

static bool PatchSlot(void** slot, void* expected, void* hook, void** realOut)
{
    if (*slot != expected) return false;
    DWORD old;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &old)) return false;
    *realOut = *slot;
    *slot = hook;
    VirtualProtect(slot, sizeof(void*), old, &old);
    return true;
}

// The engine's own end of frame. igDxVisualContext::endDraw (libIGGfx RVA 0x3D860,
// slot +0x2D4 of the visual context's function table) ends each frame and shows it (EndScene,
// then Present, 0x3D899 / 0x3D8B3). After a resolution change with fill off, the first frame
// it finishes is the first one at the new resolution: then the window is placed (a message to
// the game window, handled in order).
typedef void (__fastcall* EndDraw_t)(void* self, void* edx);
static EndDraw_t g_realEndDraw;
static const DWORD kEndDrawRva     = 0x3D860;
static const DWORD kDx9VcVtableRva = 0x106D48;   // igDx9VisualContext
static const DWORD kDxVcVtableRva  = 0x1067A8;   // igDxVisualContext
static const BYTE  kExpEndDraw[]   = { 0x51, 0x56, 0x8B, 0xF1, 0x8A, 0x86, 0x94, 0x31, 0x00, 0x00 };

static void __fastcall Hook_EndDraw(void* self, void* edx)
{
    g_realEndDraw(self, edx);
    if (g_holdWaiting && g_holding && g_gameWnd && g_hotkeyMsg)
    {
        g_holdWaiting = false;
        PostMessageA(g_gameWnd, g_hotkeyMsg, (WPARAM)kPlaceAfterChange, 0);
    }
}

static void PatchEndDraw()
{
    HMODULE gfx = GetModuleHandleA("libIGGfx.dll");
    BYTE* base = (BYTE*)gfx;
    if (!gfx || memcmp(base + kEndDrawRva, kExpEndDraw, sizeof(kExpEndDraw)) != 0)
    {
        Log("Windowed: the window is not held during resolution changes (the engine's end of frame does not match the expected bytes)\r\n");
        return;
    }
    void** vt9 = (void**)(base + kDx9VcVtableRva);
    void** vt  = (void**)(base + kDxVcVtableRva);
    void* real = NULL;
    bool ok = PatchSlot(&vt9[0x2D4 / 4], base + kEndDrawRva, (void*)Hook_EndDraw, (void**)&g_realEndDraw);
    if (ok) PatchSlot(&vt[0x2D4 / 4], base + kEndDrawRva, (void*)Hook_EndDraw, &real);
    g_endDrawOn = ok;
    if (!ok) Log("Windowed: the window is not held during resolution changes (the engine's function table does not match)\r\n");
}

// A black background for the engine's game window. The engine registers its window
// class "igWin32WindowClass" (libIGDisplay 0x84B0, RegisterClassA at 0x8536) with no
// background color (hbrBackground 0, written at 0x8526), so until the game draws its first
// frame Windows shows the window as white. Changed: the class is registered with the black
// stock brush. The menu field next to it is already 0 (set at 0x84E5), so the second store
// that is replaced (mov [esp+0x2C],esi) is not needed.
//   89 74 24 28 89 74 24 2C  ->  C7 44 24 28 <black brush>
static const DWORD kClassBrushRva  = 0x8526;
static const BYTE  kExpClassBrush[] = { 0x89, 0x74, 0x24, 0x28, 0x89, 0x74, 0x24, 0x2C };

static void PatchBlackWindow()
{
    HMODULE disp = GetModuleHandleA("libIGDisplay.dll");
    BYTE* at = disp ? (BYTE*)disp + kClassBrushRva : NULL;
    if (!at || memcmp(at, kExpClassBrush, sizeof(kExpClassBrush)) != 0)
    {
        Log("Windowed: the window is not black at launch (the code in libIGDisplay.dll does not match the expected bytes)\r\n");
        return;
    }
    WNDCLASSA wc;
    if (GetClassInfoA(GetModuleHandleA(NULL), "igWin32WindowClass", &wc))
    {
        Log("Windowed: the window is not black at launch (the engine set up its window before MUAFixPack loaded)\r\n");
        return;
    }
    BYTE code[8] = { 0xC7, 0x44, 0x24, 0x28, 0, 0, 0, 0 };
    *(DWORD*)(code + 4) = (DWORD)(DWORD_PTR)GetStockObject(BLACK_BRUSH);
    if (!Write(at, code, sizeof(code))) Log("Windowed: FAILED to write the black window patch\r\n");
}

static bool SetupEngineWindow()
{
    HMODULE disp = GetModuleHandleA("libIGDisplay.dll");
    if (!disp) { Log("Windowed: NOT applied (libIGDisplay.dll not loaded)\r\n"); return false; }
    BYTE* base = (BYTE*)disp;
    void** vt = (void**)(base + kWinVtableRva);
    g_winGetHwnd  = (WinGetHwnd_t) GetProcAddress(disp, "?getNativeWindowHandle@igWin32Window@Display@Gap@@QBEPAUHWND__@@XZ");
    g_winCurrent  = (void**)       GetProcAddress(disp, "?_CurrentWindow@igWin32Window@Display@Gap@@0PAV123@A");
    if (vt[0x6C / 4] == base + 0x5D80) g_winSetPos = (WinSetPos_t)vt[0x6C / 4];   // setPosition
    else Log("Windowed: the engine's setPosition was not found, the window is moved directly\r\n");
    if (!g_winGetHwnd || !g_winCurrent ||
        vt[0x5C / 4] != base + kWinOpenRva || vt[0x80 / 4] != base + kWinVideoModeRva ||
        memcmp(base + kWinVideoModeRva, kExpVideoMode, sizeof(kExpVideoMode)) != 0)
    {
        Log("Windowed: NOT applied. The code in this libIGDisplay.dll does not match the expected bytes.\r\n");
        return false;
    }
    bool ok = PatchSlot(&vt[0x5C / 4], base + kWinOpenRva,      (void*)Hook_WinOpen,      (void**)&g_realWinOpen) &&
              PatchSlot(&vt[0x80 / 4], base + kWinVideoModeRva, (void*)Hook_WinVideoMode, (void**)&g_realWinVideoMode);
    if (!ok) { Log("Windowed: FAILED to write patch\r\n"); return false; }
    g_hotkeyMsg = RegisterWindowMessageA("MUAFixPack_WindowHotkey");
    g_gameMouseOk = memcmp((void*)kGameMouseGet, kExpMouseGet, sizeof(kExpMouseGet)) == 0 &&
                    memcmp((void*)kGameMouseArea, kExpMouseArea, sizeof(kExpMouseArea)) == 0;
    if (!g_gameMouseOk) Log("Windowed: the game's mouse area code does not match the expected bytes (not updated)\r\n");
    if (memcmp((void*)kMouseReadAt, kExpMouseRead, sizeof(kExpMouseRead)) == 0)
    {
        BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
        *(DWORD*)(jmp + 1) = (DWORD)Hook_MouseScale - (kMouseReadAt + 5);
        if (!Write((BYTE*)kMouseReadAt, jmp, sizeof(jmp)))
            Log("Windowed: FAILED to write the fill screen mouse patch\r\n");
    }
    else Log("Windowed: the game's mouse code does not match the expected bytes (fill screen not available)\r\n");
    {
        static const DWORD kStcCalls[4] = { 0x752872, 0x7528C1, 0x752D19, 0x75B98A };
        static const BYTE  kExpStc[6] = { 0xFF, 0x15, 0x08, 0x83, 0x79, 0x00 };   // call [0x798308]
        bool match = true;
        for (int i = 0; i < 4; ++i)
            if (memcmp((void*)kStcCalls[i], kExpStc, sizeof(kExpStc)) != 0) match = false;
        if (match)
        {
            g_realScreenToClient = *(ScreenToClient_t*)0x798308;   // the game's own ScreenToClient
            bool ok = true;
            for (int i = 0; i < 4; ++i)
            {
                BYTE call[6] = { 0xE8, 0, 0, 0, 0, 0x90 };
                *(DWORD*)(call + 1) = (DWORD)Hook_ScreenToClient - (kStcCalls[i] + 5);
                ok = Write((BYTE*)kStcCalls[i], call, sizeof(call)) && ok;
            }
            if (!ok) Log("Windowed: FAILED to write the fill screen menu mouse patch\r\n");
        }
        else Log("Windowed: the game's menu mouse code does not match the expected bytes (fill screen: menu mouse not scaled)\r\n");
    }
    return true;
}

// --- The engine's own picture size (windowed mode) --------------------------------
// libIGDisplay.dll, the window's WM_SIZE handler (RVA 0x5F90): it measures the window
// (GetClientRect) and, because the window's _resizeContext setting is on (the engine's
// default), gives that size to the display (setRenderDestinationSize) and keeps it as its
// _windowWidth / _windowHeight. In windowed mode the engine's setVideoMode resizes the display
// only through this handler. Changed: the handler takes the size from _windowWidth (+0x20)
// and _windowHeight (+0x24), which the engine sets to the game's resolution before it
// resizes the window (open, setSize).
//   8B 4C 24 10 8B 5C 24 18 8B 7C 24 14 8B 54 24 0C 2B D9 8B 4E 08 2B FA
//   -> mov edi,[esi+0x20] / mov ebx,[esi+0x24] / mov ecx,[esi+8] / nop x14
// libIGGfx.dll, igDxVisualContext::setRenderDestinationSize (RVA 0x600E0): in windowed mode it
// writes 0x0 to the display settings ("the window's size"); in fullscreen it writes the size
// it was given. Changed: the jump to the windowed branch (RVA 0x60172, 75 11) is removed, so
// windowed keeps the given size as fullscreen does.
static const DWORD kDispSizeRva   = 0x5FA7;
static const BYTE  kExpDispSize[] = { 0x8B, 0x4C, 0x24, 0x10, 0x8B, 0x5C, 0x24, 0x18, 0x8B, 0x7C, 0x24, 0x14,
                                      0x8B, 0x54, 0x24, 0x0C, 0x2B, 0xD9, 0x8B, 0x4E, 0x08, 0x2B, 0xFA };
// cmp [eax+0x38],ebx / mov edx,eax / jne +0x11 / mov [edx+0x18],ecx
static const DWORD kGfxSizeRva    = 0x6016D;
static const BYTE  kExpGfxSize[]  = { 0x39, 0x58, 0x38, 0x8B, 0xD0, 0x75, 0x11, 0x89, 0x4A, 0x18 };

// Windowed mode depends on these two: without them the picture would take the window's
// size (with fill the screen, the whole monitor). So they are checked before anything
// else of windowed mode is changed, and written only once windowed mode is set up (in
// fullscreen they are not wanted).
static BYTE* EngineSizeAt(const char* dll, DWORD rva)
{
    HMODULE m = GetModuleHandleA(dll);
    return m ? (BYTE*)m + rva : NULL;
}

static bool EngineSizeMatches()
{
    BYTE* d = EngineSizeAt("libIGDisplay.dll", kDispSizeRva);
    BYTE* g = EngineSizeAt("libIGGfx.dll", kGfxSizeRva);
    return d && g && memcmp(d, kExpDispSize, sizeof(kExpDispSize)) == 0 &&
           memcmp(g, kExpGfxSize, sizeof(kExpGfxSize)) == 0;
}

static bool PatchEngineSize()
{
    BYTE* d = EngineSizeAt("libIGDisplay.dll", kDispSizeRva);
    BYTE* g = EngineSizeAt("libIGGfx.dll", kGfxSizeRva);
    BYTE code[sizeof(kExpDispSize)];
    memset(code, 0x90, sizeof(code));
    static const BYTE loads[9] = { 0x8B, 0x7E, 0x20, 0x8B, 0x5E, 0x24, 0x8B, 0x4E, 0x08 };
    memcpy(code, loads, sizeof(loads));
    static const BYTE nops[2] = { 0x90, 0x90 };
    bool ok = Write(d, code, sizeof(code)) && Write(g + 5, nops, sizeof(nops));
    if (!ok) Log("Windowed: FAILED to write the picture size patch\r\n");
    return ok;
}


// "Ctrl+B", "Ctrl+Alt+W", "Shift+F7" ... Letters, digits, F1-F24, Up, Down, Left, Right,
// Numpad0-Numpad9, Enter, Tab, Space, Home, End, Insert, Delete, PageUp, PageDown.
static bool ParseHotkey(const char* text, Hotkey* out)
{
    char buf[64];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    bool ctrl = false, alt = false, shift = false;
    UINT vk = 0;
    for (char* tok = strtok(buf, "+ "); tok; tok = strtok(NULL, "+ "))
    {
        if      (!_stricmp(tok, "Ctrl") || !_stricmp(tok, "Control")) ctrl = true;
        else if (!_stricmp(tok, "Alt"))      alt = true;
        else if (!_stricmp(tok, "Shift"))    shift = true;
        else if (!_stricmp(tok, "Enter"))    vk = VK_RETURN;
        else if (!_stricmp(tok, "Tab"))      vk = VK_TAB;
        else if (!_stricmp(tok, "Space"))    vk = VK_SPACE;
        else if (!_stricmp(tok, "Home"))     vk = VK_HOME;
        else if (!_stricmp(tok, "End"))      vk = VK_END;
        else if (!_stricmp(tok, "Insert"))   vk = VK_INSERT;
        else if (!_stricmp(tok, "Delete"))   vk = VK_DELETE;
        else if (!_stricmp(tok, "PageUp"))   vk = VK_PRIOR;
        else if (!_stricmp(tok, "PageDown")) vk = VK_NEXT;
        else if (!_stricmp(tok, "Up"))       vk = VK_UP;
        else if (!_stricmp(tok, "Down"))     vk = VK_DOWN;
        else if (!_stricmp(tok, "Left"))     vk = VK_LEFT;
        else if (!_stricmp(tok, "Right"))    vk = VK_RIGHT;
        else if (!_strnicmp(tok, "Numpad", 6) && tok[6] >= '0' && tok[6] <= '9' && !tok[7])
            vk = VK_NUMPAD0 + (tok[6] - '0');
        else if ((tok[0] == 'F' || tok[0] == 'f') && tok[1] >= '1' && tok[1] <= '9')
        {
            int n = atoi(tok + 1);
            if (n < 1 || n > 24) return false;
            vk = VK_F1 + n - 1;
        }
        else if (strlen(tok) == 1 && isalnum((unsigned char)tok[0])) vk = (UINT)toupper((unsigned char)tok[0]);
        else return false;
    }
    if (!vk) return false;
    out->vk = vk; out->ctrl = ctrl; out->alt = alt; out->shift = shift;
    return true;
}

static void HotkeyName(const Hotkey& k, char* out)
{
    char key[16];
    if (k.vk >= VK_F1 && k.vk <= VK_F24) sprintf(key, "F%u", k.vk - VK_F1 + 1);
    else if (k.vk >= VK_NUMPAD0 && k.vk <= VK_NUMPAD9) sprintf(key, "Numpad%u", k.vk - VK_NUMPAD0);
    else if (k.vk == VK_UP) strcpy(key, "Up");
    else if (k.vk == VK_DOWN) strcpy(key, "Down");
    else if (k.vk == VK_LEFT) strcpy(key, "Left");
    else if (k.vk == VK_RIGHT) strcpy(key, "Right");
    else if (k.vk >= '0' && k.vk <= 'Z') sprintf(key, "%c", (char)k.vk);
    else sprintf(key, "key %u", k.vk);
    sprintf(out, "%s%s%s%s", k.ctrl ? "Ctrl+" : "", k.alt ? "Alt+" : "", k.shift ? "Shift+" : "", key);
}

// Windowed mode (section 4) and its hotkeys.
static void SetupDisplayHooks()
{
    if (g_winOn)
    {
        char ini[MAX_PATH];
        int pos = 5;
        g_fill = true;
        if (SidePath(ini, ".ini"))
        {
            g_border = GetPrivateProfileIntA("Windowed", "Border",      0, ini) != 0;
            g_onTop  = GetPrivateProfileIntA("Windowed", "AlwaysOnTop", 0, ini) != 0;
            pos      = GetPrivateProfileIntA("Windowed", "Position",    5, ini);
            g_fill   = GetPrivateProfileIntA("Windowed", "FillScreen",  1, ini) != 0;
            for (int k = 0; k < kKeyCount; ++k)
            {
                char key[64] = "";
                GetPrivateProfileStringA("Windowed", kHotkeyKeys[k], "", key, sizeof(key), ini);
                Hotkey h;
                if (key[0] && ParseHotkey(key, &h)) g_hotkeys[k] = h;
                else if (key[0]) Log("Windowed: could not read %s=%s, using the default\r\n", kHotkeyKeys[k], key);
            }
        }
        if (pos < 1 || pos > 9) { Log("Windowed: Position=%d is not 1 to 9, using 5 (center)\r\n", pos); pos = 5; }
        g_col = (pos - 1) % 3;
        g_row = 2 - (pos - 1) / 3;
        // A key given to two hotkeys only does the first one.
        for (int k = 1; k < kKeyCount; ++k)
            for (int j = 0; j < k; ++j)
                if (g_hotkeys[k].vk && memcmp(&g_hotkeys[k], &g_hotkeys[j], sizeof(Hotkey)) == 0)
                {
                    Log("Windowed: %s is the same key as %s, ignored\r\n", kHotkeyKeys[k], kHotkeyKeys[j]);
                    g_hotkeys[k].vk = 0;
                }

        if (!EngineSizeMatches())
        {
            Log("Windowed: NOT applied. The code in libIGDisplay.dll or libIGGfx.dll does not match the expected bytes.\r\n");
            g_winOn = false;
        }
        else if (!SetupEngineWindow() || !PatchStartWindowed()) g_winOn = false;
        else
        {
            PatchEngineSize();
            PatchEndDraw();
            PatchBlackWindow();
            PatchGameMsgFunc();
            char names[kKeyCount][48];
            for (int k = 0; k < kKeyCount; ++k)
            {
                if (g_hotkeys[k].vk) HotkeyName(g_hotkeys[k], names[k]);
                else strcpy(names[k], "none");
            }
            Log("Windowed: applied; %s, position %d%s, fill screen %s; hotkeys: border %s, fill screen %s, move %s %s %s %s (up left down right)\r\n",
                g_border ? "with border" : "borderless", pos, g_onTop ? ", always on top" : "", g_fill ? "on" : "off",
                names[kKeyBorder], names[kKeyFill], names[kKeyUp], names[kKeyLeft], names[kKeyDown], names[kKeyRight]);
        }
    }
    else
        Log("Windowed: off, the game starts in its original fullscreen (set in MUAFixPack.ini)\r\n");
}

// ---------------------------------------------------------------------------
// 5. Controllers: Xbox
// ---------------------------------------------------------------------------

// The Xbox 360 layout for every Xbox controller: see "The game's controller list" in the
// PlayStation part below, where the game's search of its list is answered.

// --- Xbox icons in the OCP order --------------------------------------------
// The game's own Xbox texs files keep each icon in the same place as its numbered twin
// (A where "2"/Cross is, B where "3"/Circle is ...). The Official Characters Pack (OCP)
// files use another order, made for the game's generic layout. XboxIcons=1 in the .ini
// says the OCP files are in the texs folder. Wherever the game picks an icon number, it is
// given the number of that icon's place in the OCP order (font: 0x618C60 below; full key
// set: the button icons part below). The game then draws the right icon itself.
// Where each icon the game asks for is found in the OCP order:
//   font button row (glyphs 162-176): game's place -> place in the OCP order
static const BYTE kFontMod[15] = { 2, 0, 1, 3, 11, 10, 6, 7, 9, 8, 5, 4, 12, 14, 13 };
//   full key set Xbox row (places 0-26, y 600-624) and big icons (0x40-0x43, y 800-960)
static const BYTE kKeyRowMod[27] = { 2, 0, 1, 3, 18, 24, 6, 7, 5, 4, 10, 11, 12, 13, 14, 15,
                                     16, 17, 25, 19, 20, 21, 22, 23, 26, 8, 9 };
static const BYTE kBigMod[4] = { 2, 0, 1, 3 };

// the game's last font choice was the Xbox file (button icons part)
static volatile bool g_xboxFont;
static volatile bool g_fontModOrder;   // XboxIcons=1: font icons in the OCP order
static volatile bool g_keyModOrder;    // XboxIcons=1: full key set in the OCP order

// The game asks each player's controller object (method at 0x618C60, vtable entry
// 0x7BFB0C) which font glyph shows a given button; the answer is a place on the button
// row. While the Xbox font file is in use and it is in the OCP order, the answer is moved
// to where that icon really is.
static const DWORD kGlyphSlot = 0x7BFB0C;
static const DWORD kGlyphFunc = 0x618C60;
static BYTE g_fontModTable[256];

__declspec(naked) static void Hook_ButtonGlyph()
{
    __asm {
        mov eax, [esp+4]
        movzx eax, byte ptr [eax + ecx + 0x260]
        cmp byte ptr [g_fontModOrder], 0
        je  short done
        cmp byte ptr [g_xboxFont], 0
        je  short done
        movzx eax, byte ptr [g_fontModTable + eax]
    done:
        ret 4
    }
}

static void PatchButtonGlyph()
{
    g_fontModOrder = g_keyModOrder = g_xboxIconsOcp;
    Log("Xbox icons: %s\r\n", g_xboxIconsOcp ? "OCP files (XboxIcons=1), icon places adjusted"
                                             : "the game's original files (XboxIcons=0), unchanged");
    for (int i = 0; i < 256; ++i) g_fontModTable[i] = (BYTE)(i < 15 ? kFontMod[i] : i);
    if (*(DWORD*)kGlyphSlot != kGlyphFunc)
    {
        Log("Xbox icons: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return;
    }
    DWORD hook = (DWORD)Hook_ButtonGlyph;
    if (!Write((BYTE*)kGlyphSlot, (const BYTE*)&hook, 4))
        Log("Xbox icons: FAILED to write patch\r\n");
}

// ---------------------------------------------------------------------------
// 5. Controllers: PlayStation
// ---------------------------------------------------------------------------

typedef DWORD   (WINAPI* XGetState_t)(DWORD, XINPUT_STATE*);
typedef DWORD   (WINAPI* XGetCaps_t)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD   (WINAPI* XSetState_t)(DWORD, XINPUT_VIBRATION*);
typedef HRESULT (WINAPI* DI8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HMODULE (WINAPI* LoadLibraryA_t)(LPCSTR);
typedef HMODULE (WINAPI* LoadLibraryW_t)(LPCWSTR);

static CRITICAL_SECTION g_padCs;

// Sony product IDs. All of these send the same DirectInput layout.
static bool IsSonyPid(WORD pid)
{
    return pid == 0x05C4    // DualShock 4
        || pid == 0x09CC    // DualShock 4 (second version)
        || pid == 0x0BA0    // DualShock 4 USB wireless adapter
        || pid == 0x0CE6    // DualSense
        || pid == 0x0DF2;   // DualSense Edge
}

// --- The game's own controller slots and player choice ---------------------------
static const DWORD kGameInputVar = 0xD40570;   // the game's input object
static const DWORD kGameCtrlCfg  = 0xD4056C;   // the game's input settings (+0x24)
static const int   kCtrlSlots    = 10;
static const DWORD kCtrlSlotSize = 0x698;

static BYTE* GameCtrlSlot(int i)
{
    BYTE* in = *(BYTE**)kGameInputVar;
    return in ? in + 8 + i * kCtrlSlotSize : NULL;
}

// The product ID text of a slot: true for a PlayStation controller (Sony, a known product).
static bool SlotIsSony(const BYTE* c, WORD* pidOut)
{
    if (!c) return false;
    unsigned len = *(const unsigned*)(c + 0xA0);
    const wchar_t* t = *(const wchar_t* const*)(c + 0xA8);
    if (len < 8 || !t) return false;
    unsigned v = 0;
    for (int i = 0; i < 8; ++i)
    {
        wchar_t ch = t[i];
        int d = ch >= L'0' && ch <= L'9' ? ch - L'0' : ch >= L'A' && ch <= L'F' ? ch - L'A' + 10
              : ch >= L'a' && ch <= L'f' ? ch - L'a' + 10 : -1;
        if (d < 0) return false;
        v = (v << 4) | (unsigned)d;
    }
    if (LOWORD(v) != 0x054C || !IsSonyPid(HIWORD(v))) return false;
    if (pidOut) *pidOut = HIWORD(v);
    return true;
}

// Is a PlayStation controller in one of the game's slots? (game thread)
static bool GameHasPsController()
{
    for (int i = 0; i < kCtrlSlots; ++i)
        if (SlotIsSony(GameCtrlSlot(i), NULL)) return true;
    return false;
}

// The slot of the controller a player uses, or -1 (none chosen, keyboard, mouse).
static int GamePlayerSlot(int player)
{
    BYTE* cfg = *(BYTE**)kGameCtrlCfg;
    if (!cfg) return -1;
    cfg += 0x24;
    BYTE* recs  = *(BYTE**)cfg;
    DWORD nrec  = *(DWORD*)(cfg + 4);
    BYTE* pl    = *(BYTE**)(cfg + 0xC);
    if (!recs || !pl) return -1;
    DWORD rec = *(DWORD*)(pl + player * 12);
    DWORD sub = *(DWORD*)(pl + player * 12 + 8);
    if (rec >= nrec) return -1;
    BYTE* r = recs + rec * 0xB8;
    DWORD* devs = *(DWORD**)(r + 0xAC);
    DWORD  n    = *(DWORD*)(r + 0xB0);
    if (!devs || sub >= n) return -1;
    DWORD dev = devs[sub];
    if (dev < 3 || dev >= 3 + kCtrlSlots) return -1;
    return (int)dev - 3;
}

// The instance ID text of a slot (the game wrote it as %08X-%04X-%04X-%02X%02X-%02X...).
static bool SlotInstance(const BYTE* c, GUID* out)
{
    unsigned len = *(const unsigned*)(c + 0x50);
    const wchar_t* t = *(const wchar_t* const*)(c + 0x58);
    if (len < 36 || !t) return false;
    wchar_t buf[37];
    memcpy(buf, t, 36 * sizeof(wchar_t));
    buf[36] = 0;
    unsigned d1, d2, d3, b[8];
    if (swscanf(buf, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                &d1, &d2, &d3, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]) != 11) return false;
    out->Data1 = d1; out->Data2 = (WORD)d2; out->Data3 = (WORD)d3;
    for (int k = 0; k < 8; ++k) out->Data4[k] = (BYTE)b[k];
    return true;
}

// The PlayStation controller the bridge reads: player 1's, only when player 1 has chosen a
// PlayStation controller in Controls Options. With keyboard, mouse or an Xbox controller for
// player 1 there is none, so other mods get Windows' own answer. (A Bluetooth controller that
// is paired but off stays on Windows' list of connected devices; it is never read this way.)
static bool GameSonyInstanceRead(GUID* out)
{
    int ps = GamePlayerSlot(0);
    return ps >= 0 && SlotIsSony(GameCtrlSlot(ps), NULL) && SlotInstance(GameCtrlSlot(ps), out);
}

// Read from MUAFixPack's own thread while the game may be changing its data, so it is guarded.
static bool GameSonyInstance(GUID* out)
{
    bool ok = false;
    __try { ok = GameSonyInstanceRead(out); }
    __except (EXCEPTION_EXECUTE_HANDLER) { ok = false; }
    return ok;
}

// --- Reading the PlayStation controller -------------------------------------

static LPDIRECTINPUT8W       g_di;
static LPDIRECTINPUTDEVICE8W g_pad;          // guarded by g_padCs
static GUID                  g_padGuid;      // instance ID of g_pad
static volatile LONG         g_padSlot = -1; // Xbox slot answered with the PlayStation controller
static XGetState_t           g_slotCheck;    // a real XInputGetState, for picking the slot
static XINPUT_GAMEPAD        g_lastPad;
static DWORD                 g_packet = 1;


static void OpenPad()
{
    // Read player 1's PlayStation controller. If the open one is a different controller, or
    // player 1 no longer uses a PlayStation controller, let it go.
    GUID gameGuid;
    bool haveGame = GameSonyInstance(&gameGuid);
    EnterCriticalSection(&g_padCs);
    if (g_pad && (!haveGame || !IsEqualGUID(g_padGuid, gameGuid)))
    {
        g_pad->Unacquire();
        g_pad->Release();
        g_pad = NULL;
    }
    bool open = g_pad != NULL;
    LeaveCriticalSection(&g_padCs);
    if (open || !haveGame) return;

    if (!g_di)
    {
        HMODULE dm = LoadLibraryW(L"dinput8.dll");
        DI8Create_t create = dm ? (DI8Create_t)GetProcAddress(dm, "DirectInput8Create") : NULL;
        if (!create || FAILED(create(g_self, DIRECTINPUT_VERSION, IID_IDirectInput8W, (LPVOID*)&g_di, NULL)))
        {
            g_di = NULL;
            return;
        }
    }

    LPDIRECTINPUTDEVICE8W dev = NULL;
    HRESULT made = g_di->CreateDevice(gameGuid, &dev, NULL);
    if (FAILED(made) || !dev) return;
    if (FAILED(dev->SetDataFormat(&c_dfDIJoystick2))) { dev->Release(); return; }

    DIPROPRANGE range;
    range.diph.dwSize       = sizeof(range);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow        = DIPH_DEVICE;
    range.diph.dwObj        = 0;
    range.lMin              = -32768;
    range.lMax              = 32767;
    dev->SetProperty(DIPROP_RANGE, &range.diph);
    dev->Acquire();

    EnterCriticalSection(&g_padCs);
    g_pad = dev;
    g_padGuid = gameGuid;
    LeaveCriticalSection(&g_padCs);
}

// First Xbox slot without a real Xbox controller.
static void PickSlot()
{
    if (!g_slotCheck) return;
    LONG slot = -1;
    XINPUT_STATE s;
    for (DWORD i = 0; i < XUSER_MAX_COUNT && slot < 0; ++i)
        if (g_slotCheck(i, &s) != ERROR_SUCCESS) slot = (LONG)i;
    InterlockedExchange(&g_padSlot, slot);
}

static SHORT Stick(LONG v)   { return (SHORT)(v < -32768 ? -32768 : (v > 32767 ? 32767 : v)); }
static BYTE  Trigger(LONG v) { return (BYTE)((v + 32768) * 255 / 65535); }   // rests at -32768 = 0

// Call with g_padCs held. False if no PlayStation controller can be read.
static bool ReadPad(XINPUT_GAMEPAD* gp)
{
    if (!g_pad) return false;

    DIJOYSTATE2 js;
    g_pad->Poll();
    HRESULT hr = g_pad->GetDeviceState(sizeof(js), &js);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
    {
        g_pad->Acquire();
        g_pad->Poll();
        hr = g_pad->GetDeviceState(sizeof(js), &js);
    }
    if (FAILED(hr))   // unplugged: let go, the worker looks for it again
    {
        g_pad->Unacquire();
        g_pad->Release();
        g_pad = NULL;
        return false;
    }

    // DirectInput layout: buttons 0 Square, 1 Cross, 2 Circle, 3 Triangle, 4 L1, 5 R1,
    // 9 Options, 10 L3, 11 R3, 13 touchpad click. Left stick X/Y, right stick Z/Rz, L2 Rx,
    // R2 Ry, d-pad POV 0.
    static const struct { int button; WORD xbox; } kMap[] =
    {
        { 1, XINPUT_GAMEPAD_A },             { 2, XINPUT_GAMEPAD_B },
        { 0, XINPUT_GAMEPAD_X },             { 3, XINPUT_GAMEPAD_Y },
        { 4, XINPUT_GAMEPAD_LEFT_SHOULDER }, { 5, XINPUT_GAMEPAD_RIGHT_SHOULDER },
        { 13, XINPUT_GAMEPAD_BACK },         { 9, XINPUT_GAMEPAD_START },
        { 10, XINPUT_GAMEPAD_LEFT_THUMB },   { 11, XINPUT_GAMEPAD_RIGHT_THUMB },
    };
    WORD w = 0;
    for (int i = 0; i < (int)(sizeof(kMap) / sizeof(kMap[0])); ++i)
        if (js.rgbButtons[kMap[i].button] & 0x80) w |= kMap[i].xbox;

    DWORD pov = js.rgdwPOV[0];   // hundredths of a degree, 0 = up, clockwise
    if (LOWORD(pov) != 0xFFFF)
    {
        if (pov >= 31500 || pov <= 4500)  w |= XINPUT_GAMEPAD_DPAD_UP;
        if (pov >= 4500  && pov <= 13500) w |= XINPUT_GAMEPAD_DPAD_RIGHT;
        if (pov >= 13500 && pov <= 22500) w |= XINPUT_GAMEPAD_DPAD_DOWN;
        if (pov >= 22500 && pov <= 31500) w |= XINPUT_GAMEPAD_DPAD_LEFT;
    }

    gp->wButtons      = w;
    gp->bLeftTrigger  = Trigger(js.lRx);
    gp->bRightTrigger = Trigger(js.lRy);
    gp->sThumbLX      = Stick(js.lX);
    gp->sThumbLY      = Stick(-js.lY);    // DirectInput: down is positive; Xbox: up is positive
    gp->sThumbRX      = Stick(js.lZ);
    gp->sThumbRY      = Stick(-js.lRz);
    return true;
}

// --- Other plugins: the PlayStation controller as an Xbox controller ---------
// Plugins that read controllers through XInput (for example camera mods) get player 1's
// PlayStation controller in the first Xbox slot without a real Xbox controller.
// xinput1_1.dll passes its reads on to xinput1_4.dll on current Windows; the depth
// counter makes sure such a read is answered once, by the outer call.

static __declspec(thread) int t_depth;
static BYTE* g_gameBase;
static DWORD g_gameSize;

// Reads made by the game itself are left alone: the game reads the PlayStation
// controller through its own DirectInput path.
static bool FromGame(void* caller)
{
    return (BYTE*)caller >= g_gameBase && (BYTE*)caller < g_gameBase + g_gameSize;
}

static DWORD AnswerGetState(XGetState_t real, void* caller, DWORD i, XINPUT_STATE* s)
{
    if (t_depth || FromGame(caller)) return real(i, s);
    ++t_depth;
    DWORD r = real(i, s);
    --t_depth;
    if (r == ERROR_SUCCESS || !s || (LONG)i != g_padSlot) return r;

    EnterCriticalSection(&g_padCs);
    XINPUT_GAMEPAD gp;
    if (ReadPad(&gp))
    {
        if (memcmp(&gp, &g_lastPad, sizeof(gp)) != 0) { g_lastPad = gp; ++g_packet; }
        s->dwPacketNumber = g_packet;
        s->Gamepad = gp;
        r = ERROR_SUCCESS;
    }
    LeaveCriticalSection(&g_padCs);
    return r;
}

static DWORD AnswerGetCaps(XGetCaps_t real, void* caller, DWORD i, DWORD flags, XINPUT_CAPABILITIES* c)
{
    if (t_depth || FromGame(caller)) return real(i, flags, c);
    ++t_depth;
    DWORD r = real(i, flags, c);
    --t_depth;
    if (r == ERROR_SUCCESS || !c || (LONG)i != g_padSlot) return r;

    EnterCriticalSection(&g_padCs);
    bool have = g_pad != NULL;
    LeaveCriticalSection(&g_padCs);
    if (!have) return r;

    ZeroMemory(c, sizeof(*c));
    c->Type    = XINPUT_DEVTYPE_GAMEPAD;
    c->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
    c->Gamepad.wButtons      = 0xF3FF;            // every standard button
    c->Gamepad.bLeftTrigger  = 255;
    c->Gamepad.bRightTrigger = 255;
    c->Gamepad.sThumbLX = c->Gamepad.sThumbLY = c->Gamepad.sThumbRX = c->Gamepad.sThumbRY = (SHORT)0xFFC0;
    return ERROR_SUCCESS;
}

static DWORD AnswerSetState(XSetState_t real, void* caller, DWORD i, XINPUT_VIBRATION* v)
{
    if (t_depth || FromGame(caller)) return real(i, v);
    ++t_depth;
    DWORD r = real(i, v);
    --t_depth;
    if (r == ERROR_SUCCESS || (LONG)i != g_padSlot) return r;

    EnterCriticalSection(&g_padCs);
    bool have = g_pad != NULL;
    LeaveCriticalSection(&g_padCs);
    return have ? ERROR_SUCCESS : r;   // accepted, not passed on
}

// One entry per XInput file.
struct XInputDll
{
    const char* name;
    bool        done;
    XGetState_t getState;   // where the real function continues
    XGetCaps_t  getCaps;
    XSetState_t setState;
};
static XInputDll g_xinput[] =
{
    { "xinput1_1.dll" }, { "xinput1_2.dll" }, { "xinput1_3.dll" }, { "xinput1_4.dll" }, { "xinput9_1_0.dll" },
};
static const int kXInputCount = sizeof(g_xinput) / sizeof(g_xinput[0]);

template <int N> static DWORD WINAPI Hook_XGetState(DWORD i, XINPUT_STATE* s)
{ return AnswerGetState(g_xinput[N].getState, _ReturnAddress(), i, s); }
template <int N> static DWORD WINAPI Hook_XGetCaps(DWORD i, DWORD f, XINPUT_CAPABILITIES* c)
{ return AnswerGetCaps(g_xinput[N].getCaps, _ReturnAddress(), i, f, c); }
template <int N> static DWORD WINAPI Hook_XSetState(DWORD i, XINPUT_VIBRATION* v)
{ return AnswerSetState(g_xinput[N].setState, _ReturnAddress(), i, v); }

static void* const kGetStateHooks[] = { (void*)Hook_XGetState<0>, (void*)Hook_XGetState<1>, (void*)Hook_XGetState<2>, (void*)Hook_XGetState<3>, (void*)Hook_XGetState<4> };
static void* const kGetCapsHooks[]  = { (void*)Hook_XGetCaps<0>,  (void*)Hook_XGetCaps<1>,  (void*)Hook_XGetCaps<2>,  (void*)Hook_XGetCaps<3>,  (void*)Hook_XGetCaps<4>  };
static void* const kSetStateHooks[] = { (void*)Hook_XSetState<0>, (void*)Hook_XSetState<1>, (void*)Hook_XSetState<2>, (void*)Hook_XSetState<3>, (void*)Hook_XSetState<4> };

static BYTE* g_tramp;
static int   g_trampUsed;

// Send a system function to 'hook'. Returns where the real function continues, or NULL.
static void* HotPatch(void* func, void* hook)
{
    BYTE* f = (BYTE*)func;
    if (!f) return NULL;

    // Hot-patch layout: "mov edi,edi" at the start and 5 spare bytes in front.
    if (f[0] == 0x8B && f[1] == 0xFF)
    {
        BYTE* pad = f - 5;
        bool spare = true;
        for (int i = 0; i < 5; ++i) if (pad[i] != 0x90 && pad[i] != 0xCC) spare = false;
        if (spare)
        {
            DWORD old;
            if (!VirtualProtect(pad, 7, PAGE_EXECUTE_READWRITE, &old)) return NULL;
            pad[0] = 0xE9;                                              // jmp hook
            *(DWORD*)(pad + 1) = (DWORD)((BYTE*)hook - (pad + 5));
            InterlockedExchange16((SHORT*)f, (SHORT)0xF9EB);            // jmp short back 5
            VirtualProtect(pad, 7, old, &old);
            FlushInstructionCache(GetCurrentProcess(), pad, 7);
            return f + 2;
        }
    }

    // Otherwise the first 5 bytes must be whole instructions that can run elsewhere:
    //   8B FF 55 8B EC  (mov edi,edi / push ebp / mov ebp,esp)
    //   55 8B EC 6A xx  (push ebp / mov ebp,esp / push xx)
    bool movable = (f[0] == 0x8B && f[1] == 0xFF && f[2] == 0x55 && f[3] == 0x8B && f[4] == 0xEC)
                || (f[0] == 0x55 && f[1] == 0x8B && f[2] == 0xEC && f[3] == 0x6A);
    if (!movable) return NULL;

    if (!g_tramp) g_tramp = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tramp || g_trampUsed + 16 > 4096) return NULL;
    BYTE* t = g_tramp + g_trampUsed;
    g_trampUsed += 16;
    memcpy(t, f, 5);                                                    // the 5 bytes, then
    // jmp back into the function
    t[5] = 0xE9;
    *(DWORD*)(t + 6) = (DWORD)((f + 5) - (t + 10));
    FlushInstructionCache(GetCurrentProcess(), t, 10);

    BYTE jmp[5] = { 0xE9 };
    *(DWORD*)(jmp + 1) = (DWORD)((BYTE*)hook - (f + 5));
    if (!Write(f, jmp, sizeof(jmp))) return NULL;
    return t;
}

// Hook every XInput file that is loaded and not hooked yet.
static void HookLoadedXInput()
{
    bool changed = false;
    EnterCriticalSection(&g_padCs);
    for (int n = 0; n < kXInputCount; ++n)
    {
        XInputDll& x = g_xinput[n];
        if (x.done) continue;
        HMODULE m = GetModuleHandleA(x.name);
        if (!m) continue;
        x.done = true;
        changed = true;

        XGetState_t gs = (XGetState_t)GetProcAddress(m, "XInputGetState");
        XGetCaps_t  gc = (XGetCaps_t) GetProcAddress(m, "XInputGetCapabilities");
        XSetState_t ss = (XSetState_t)GetProcAddress(m, "XInputSetState");
        x.getState = gs ? (XGetState_t)HotPatch((void*)gs, kGetStateHooks[n]) : NULL;
        x.getCaps  = gc ? (XGetCaps_t) HotPatch((void*)gc, kGetCapsHooks[n])  : NULL;
        x.setState = ss ? (XSetState_t)HotPatch((void*)ss, kSetStateHooks[n]) : NULL;

        // A function that could not be hooked keeps working as before.
        if (gs && !x.getState) x.getState = gs;
        if (gc && !x.getCaps)  x.getCaps  = gc;
        if (ss && !x.setState) x.setState = ss;

        bool ok = (!gs || x.getState != gs) && (!gc || x.getCaps != gc) && (!ss || x.setState != ss);
        if (!ok) Log("PlayStation controllers: one controller file could not be prepared for other plugins (left unchanged)\r\n");
        if (gs && !g_slotCheck) g_slotCheck = x.getState;
    }
    LeaveCriticalSection(&g_padCs);
    if (changed) WriteLog();
}

// The game loads xinput1_1.dll while starting; hook it as soon as it is loaded.
static LoadLibraryA_t g_realLoadLibraryA;
static LoadLibraryW_t g_realLoadLibraryW;

static HMODULE WINAPI Hook_LoadLibraryA(LPCSTR name)
{
    HMODULE h = g_realLoadLibraryA(name);
    if (h) HookLoadedXInput();
    return h;
}

static HMODULE WINAPI Hook_LoadLibraryW(LPCWSTR name)
{
    HMODULE h = g_realLoadLibraryW(name);
    if (h) HookLoadedXInput();
    return h;
}


// --- Setting buttons by hand: L2/R2 ---------------------------------------------
// The game's capture of a button set by hand (0x409BD0, Controls Options) reads every input
// number of a DirectInput controller in order (axis halves 0-15 first, then the d-pad, then
// the buttons) and takes the first above 0.75. A PlayStation controller sends L2/R2 twice: as
// axes (Rx/Ry) and as buttons 6 and 7. At rest the axes are at one end (-1000), so "Rx-" (7)
// and "Ry-" (9) always read 1.0 and were caught at once. Pressed fully, "Rx+" (6) and "Ry+"
// (8) can be caught before the buttons; the game's list entry for the controller (0x4069F0
// looks the input up in it) only has the buttons (0x4A, 0x4B), so such a choice had no
// button icon. The capture's read (call 0x4151A0 at 0x409CAF; eax = the controller object,
// ecx = the input number, answer in xmm0) gives 0 for 6 to 9 when the controller is a
// PlayStation controller, so L2/R2 are caught as their buttons, as the automatic layout has
// them.
// The game keeps each controller's product ID as text ("05C4054C-...": product, vendor) at
// +0xA0 (length) / +0xA8 (text) of its controller object; it is the text the game's own
// controller list is searched with (0x406F00). Gameplay reads are not changed.
static const DWORD kCaptureReadCall = 0x409CAF;   // call 0x4151A0 (inside 0x409BD0)
static const DWORD kInputRead       = 0x4151A0;

static bool __cdecl IsSonyController(const BYTE* c, int)
{
    if (!c) return false;
    unsigned len = *(const unsigned*)(c + 0xA0);
    const wchar_t* t = *(const wchar_t* const*)(c + 0xA8);
    if (len < 8 || !t) return false;
    unsigned v = 0;
    for (int i = 0; i < 8; ++i)
    {
        wchar_t ch = t[i];
        int d = ch >= L'0' && ch <= L'9' ? ch - L'0' : ch >= L'A' && ch <= L'F' ? ch - L'A' + 10
              : ch >= L'a' && ch <= L'f' ? ch - L'a' + 10 : -1;
        if (d < 0) return false;
        v = (v << 4) | (unsigned)d;
    }
    return LOWORD(v) == 0x054C && IsSonyPid(HIWORD(v));
}

// eax = controller object, ecx = input number. The capture loop keeps its threshold in xmm1,
// so xmm1 is kept here around the check.
__declspec(naked) static void Hook_CaptureRead()
{
    __asm {
        cmp   ecx, 6
        jl    cap_game
        cmp   ecx, 9
        jg    cap_game
        push  eax
        push  ecx
        push  edx
        sub   esp, 16
        movups [esp], xmm1
        push  ecx
        push  eax
        call  IsSonyController
        add   esp, 8
        movups xmm1, [esp]
        add   esp, 16
        test  al, al
        pop   edx
        pop   ecx
        pop   eax
        jne   cap_zero
    cap_game:
        push  0x4151A0
        ret
    cap_zero:
        xorps xmm0, xmm0
        ret
    }
}

static void PatchCaptureRead()
{
    bool ok = RedirectCall(kCaptureReadCall, kInputRead, (void*)Hook_CaptureRead);
    if (!ok) Log("PlayStation controllers: setting L2/R2 by hand NOT applied (the code in this mua.exe does not match the expected bytes)\r\n");
}

// --- The game's controller list: the PlayStation place, and the answer to its search -----
// The list is built at startup (0x792B60) from MUA.exe's own text and numbers. The Logitech
// RumblePad 2 entry (place 2) has the generic layout; its Back line (line 20, written at
// 0x792F1D: 76 = button 8) becomes 81 = button 13, the touchpad, and its name (0x7D3D78, read
// only by the builder) becomes text no controller has (same length; letters that are not hex
// digits). If the game has already built the list, the built entry (0xD42760: name at +8,
// lines from +0x50) is changed the same way. The game's search of the list (0x406AA0; ecx = the
// product ID text: length at +0, text at +8; answer = place, 0 = generic) is then answered with
// place 2 for a PlayStation product ID it does not find.
static const DWORD kPsEntryName   = 0x7D3D78;   // L"C218046D-0000-0000-0000-504944564944"
static const DWORD kPsEntryBack   = 0x792F1D;   // mov dword ptr [0xD42800],0x4C
static const BYTE  kExpPsEntryBack[] = { 0xC7, 0x05, 0x00, 0x28, 0xD4, 0x00, 0x4C, 0x00, 0x00, 0x00 };
static const DWORD kPsEntryBuilt  = 0xD42760;   // the built entry (place 2)
static const int   kPsEntryPlace  = 2;
// 36 characters, like the old name
static const wchar_t kPsEntryNewName[] = L"MUAFIXPK-PLAY-STAT-ION0-000000000000";
static const DWORD kListLookup    = 0x406AA0;
static const DWORD kListLookupCalls[3] = { 0x406966, 0x4069F4, 0x406A30 };
static bool g_psEntryOk;
static bool g_xboxLookupOk;
static bool g_lookupOk;
static const int kXboxEntryPlace = 5;   // the wired Xbox 360 controller (028E045E)

// Every controller keeps its own product ID: the game also files each controller's settings
// by product ID (0x408710, run when Controls Options opens; controllers with the same product
// ID share one settings record). An earlier version gave every Xbox controller one stand-in
// product ID, so different controllers shared a record (doubled icons, crashes when switching
// configuration). Here only the answer to the list search changes.
// Does this product ID text belong to a controller the game set up as an Xbox (XInput)
// controller? The game's 10 controller slots (input object 0xD40570, +8, 0x698 bytes each):
// product ID text at +0xA0 (length) / +0xA8 (text), Xbox flag at +0xF4.
static bool TextIsXboxSlot(const BYTE* str)
{
    BYTE* in = *(BYTE**)0xD40570;
    unsigned len = *(const unsigned*)str;
    const wchar_t* t = *(const wchar_t* const*)(str + 8);
    if (!in || !len || !t) return false;
    for (int i = 0; i < 10; ++i)
    {
        const BYTE* c = in + 8 + i * 0x698;
        if (!c[0xF4]) continue;
        unsigned l2 = *(const unsigned*)(c + 0xA0);
        const wchar_t* t2 = *(const wchar_t* const*)(c + 0xA8);
        if (l2 == len && t2 && memcmp(t, t2, len * sizeof(wchar_t)) == 0) return true;
    }
    return false;
}

static int __cdecl ListLookupFallback(const BYTE* str)
{
    if (!str) return 0;
    if (g_xboxLookupOk && TextIsXboxSlot(str)) return kXboxEntryPlace;
    if (!g_psEntryOk) return 0;
    unsigned len = *(const unsigned*)str;
    const wchar_t* t = *(const wchar_t* const*)(str + 8);
    if (len < 8 || !t) return 0;
    unsigned v = 0;
    for (int i = 0; i < 8; ++i)
    {
        wchar_t ch = t[i];
        int d = ch >= L'0' && ch <= L'9' ? ch - L'0' : ch >= L'A' && ch <= L'F' ? ch - L'A' + 10
              : ch >= L'a' && ch <= L'f' ? ch - L'a' + 10 : -1;
        if (d < 0) return 0;
        v = (v << 4) | (unsigned)d;
    }
    WORD vid = LOWORD(v), pid = HIWORD(v);
    if (vid != 0x054C || !IsSonyPid(pid)) return 0;
    return kPsEntryPlace;
}

// Called in place of the game's search: the game's own search first; only when it finds
// nothing, the fallback above. ecx = the product ID text (as the game passes it).
__declspec(naked) static void Hook_ListLookup()
{
    __asm {
        push  ecx
        mov   eax, 0x406AA0
        call  eax
        pop   ecx
        test  eax, eax
        jne   lk_done
        push  ecx
        call  ListLookupFallback
        add   esp, 4
    lk_done:
        ret
    }
}

// The game's search of its list is answered from here (PlayStation entry, Xbox 360 entry).
static bool PatchListLookup()
{
    if (g_lookupOk) return true;
    for (int i = 0; i < 3; ++i)
    {
        const BYTE* c = (const BYTE*)kListLookupCalls[i];
        if (c[0] != 0xE8 || *(const DWORD*)(c + 1) != kListLookup - (kListLookupCalls[i] + 5)) return false;
    }
    bool ok = true;
    for (int i = 0; i < 3 && ok; ++i)
    {
        BYTE call[5] = { 0xE8 };
        *(DWORD*)(call + 1) = (DWORD)Hook_ListLookup - (kListLookupCalls[i] + 5);
        ok = Write((BYTE*)kListLookupCalls[i], call, 5);
    }
    g_lookupOk = ok;
    return ok;
}

static void PatchXboxLookup()
{
    g_xboxLookupOk = PatchListLookup();
    Log("Xbox controllers: %s\r\n", g_xboxLookupOk
        ? "applied (the game's Xbox 360 layout and icons)"
        : "NOT applied. The code in this mua.exe does not match the expected bytes.");
}

static void PatchPsListEntry()
{
    const wchar_t kOld[] = L"C218046D-0000-0000-0000-504944564944";
    wchar_t* name = (wchar_t*)kPsEntryName;
    bool callsOk = true;
    for (int i = 0; i < 3; ++i)
    {
        const BYTE* c = (const BYTE*)kListLookupCalls[i];
        if (c[0] != 0xE8 || *(const DWORD*)(c + 1) != kListLookup - (kListLookupCalls[i] + 5)) callsOk = false;
    }
    if (memcmp(name, kOld, sizeof(kOld)) != 0 ||
        memcmp((void*)kPsEntryBack, kExpPsEntryBack, sizeof(kExpPsEntryBack)) != 0 || !callsOk)
    {
        Log("PlayStation controllers: touchpad as Back NOT applied (the code in this mua.exe does not match the expected bytes)\r\n");
        return;
    }
    static const DWORD back = 81;   // button 13, the touchpad
    bool ok = Write((BYTE*)name, (const BYTE*)kPsEntryNewName, 36 * sizeof(wchar_t)) &&
              Write((BYTE*)kPsEntryBack + 6, (const BYTE*)&back, 4);
    // Already built? (name length at +0, text at +8; Back at +0x50 + 20*4)
    BYTE* e = (BYTE*)kPsEntryBuilt;
    wchar_t* built = *(wchar_t**)(e + 8);
    bool wasBuilt = built && *(DWORD*)e >= 36 && memcmp(built, kOld, 36 * sizeof(wchar_t)) == 0;
    if (wasBuilt)
    {
        memcpy(built, kPsEntryNewName, 36 * sizeof(wchar_t));
        *(DWORD*)(e + 0x50 + 20 * 4) = back;
    }
    ok = ok && PatchListLookup();
    g_psEntryOk = ok;
    if (!ok) Log("PlayStation controllers: FAILED to write the touchpad as Back patch\r\n");
}

// ---------------------------------------------------------------------------
// 5. Controllers: button icons (Xbox and PlayStation)
// ---------------------------------------------------------------------------
// The game chooses its texs files itself; MUAFixPack only answers its questions.
//  - Font icons: 0x68BB50 asks 0x75FA40 "is any controller an Xbox controller?" (call at
//    0x68BBAD). Yes -> texs\FontTexture0_beenox360_hd.png, no -> the name at 0x68BBE2
//    (texs\FontTexture0_beenoxLogitech_hd.png). That name is read from MUAFixPack's own
//    copy, which holds the PS4 name while PlayStation icons are in use.
//  - Full key set: the file name is the text the pointer at 0x826160 points to
//    ("texs\Fullkeyset_") + language + ".png". The game reads it at 0x68A22F (0x68A210,
//    vtable entry 0x7C55F8), 0x68BC14 (0x68BB50) and 0x6981FE (0x6981D0, called at
//    0x698C8B). Before each, the pointer is set to "texs\Fullkeyset_PS4_" while PlayStation
//    icons are in use, otherwise back to the game's own text.
//  - Full key set row: 0x6981D0 asks the same question at 0x6987F7 (big icons: numbered
//    = Xbox + 4) and 0x698994 (row icons: numbered = Xbox + 0x20). While PlayStation icons
//    are in use the answer is "Xbox": the OCP PlayStation symbols are in the Xbox row.
//  - Controls Options icons: 0x6DA5A0 fills the page's icon table every frame and picks
//    the row per controller at 0x6DA8D7: the controller's own XInput flag
//    ([controller+0xF4]), then "Xbox" in the device name, else numbered (+0x20). The flag
//    read is answered "Xbox" while the PlayStation full key set is in use.
//  - Xbox icons in the OCP order (XboxIcons=1): the full key set icon number is changed to
//    the OCP place where the game picks it: Controls Options (place 0-26, before the font
//    lookup call at 0x6DA9E0), HUD big icons (0x40-0x43 in ebp, after 0x6987F7) and HUD row
//    icons (place 0-26 in ebp, at 0x69898C when the game uses the Xbox row without asking,
//    after 0x698994 when it asks). Not while the PlayStation full key set is in use.
// PlayStation icons are in use when PlayStation=1, PlayStationIcons=1, a PlayStation
// controller is in the game's controller slots and no Xbox controller is connected.
// PlayStationIcons=1 means
// the OCP PlayStation files are in the texs folder; they are required (a missing font file
// blanks all text in that font).

typedef bool (__cdecl* AnyXbox_t)();
static const DWORD kAnyXbox        = 0x75FA40;
static const DWORD kFontAskCall    = 0x68BBAD;   // call 0x75FA40 (font file)
static const DWORD kFontNameHd     = 0x68BBE2;   // mov eax,0x7C5530 (Logitech HD name)
static const BYTE  kExpFontNameHd[] = { 0xB8, 0x30, 0x55, 0x7C, 0x00 };
static const DWORD kRowAskBig      = 0x6987F7;   // call 0x75FA40 (big icons)
static const DWORD kRowAskRow      = 0x698994;   // call 0x75FA40 (row icons)
static const DWORD kHudSetupCall   = 0x698C8B;   // call 0x6981D0
static const DWORD kHudSetup       = 0x6981D0;
static const DWORD kKeysetLoadSlot = 0x7C55F8;   // vtable entry -> 0x68A210
static const DWORD kKeysetLoad     = 0x68A210;
static const char** const kKeysetPrefix = (const char**)0x826160;
static const DWORD kKeysetPrefixGame = 0x7C540C; // "texs\Fullkeyset_"
static const DWORD kInputManager   = 0xD40570;   // read by 0x75FA40 (through 0x758D80)

static const char kLogiFontName[] = "texs\\FontTexture0_beenoxLogitech_hd.png";
static const char kPsFontName[]   = "texs\\FontTexture0_beenoxPS4_hd.png";
static const char kPsKeysetPrefix[] = "texs\\Fullkeyset_PS4_";
static char           g_fontNameHd[64];     // the name the game reads at 0x68BBE2
static volatile bool  g_psKeyNow;           // PlayStation icons are in use right now
static bool           g_iconChoiceOk;       // the choice hooks are applied

// --- The choice ---------------------------------------------------------------

// The game's own question. Only asked when the game's input object exists.
static bool AskAnyXbox()
{
    if (!*(void**)kInputManager) return false;
    return ((AnyXbox_t)kAnyXbox)();
}

// Sets the names the game reads next.
static void ChooseIcons(bool anyXbox)
{
    bool ps = g_psOn && g_psIcons && !anyXbox && GameHasPsController();
    g_psKeyNow = ps;
    strcpy(g_fontNameHd, ps ? kPsFontName : kLogiFontName);
    *kKeysetPrefix = ps ? kPsKeysetPrefix : (const char*)kKeysetPrefixGame;
}

// 0x68BBAD: the font file question.
static bool __cdecl Hook_FontAsk()
{
    bool xbox = AskAnyXbox();
    ChooseIcons(xbox);
    g_xboxFont = xbox;
    return xbox;
}

// 0x6987F7 / 0x698994: the full key set row question.
static bool RowAsk()
{
    bool xbox = AskAnyXbox();
    ChooseIcons(xbox);
    return xbox || g_psKeyNow;
}
static bool __cdecl Hook_RowAskBig() { return RowAsk(); }
static bool __cdecl Hook_RowAskRow() { return RowAsk(); }

// Before the game builds the full key set name (0x6981D0 and 0x68A210).
static void __cdecl BeforeKeyset()
{
    ChooseIcons(AskAnyXbox());
}

__declspec(naked) static void Hook_HudSetup()
{
    __asm {
        push ecx
        call BeforeKeyset
        pop  ecx
        push 0x6981D0
        ret
    }
}

__declspec(naked) static void Hook_KeysetLoad()
{
    __asm {
        push ecx
        call BeforeKeyset
        pop  ecx
        push 0x68A210
        ret
    }
}

// 0x6DA8D7 (Controls Options): the controller's XInput flag, answered "Xbox" while the
// PlayStation full key set is in use.
static const DWORD kPageRowAt = 0x6DA8D7;      // mov al,[ebp+0xF4]
static const BYTE  kExpPageRow[] = { 0x8A, 0x85, 0xF4, 0x00, 0x00, 0x00 };
static int __cdecl PageRowAsk(int xinput)
{
    return (xinput != 0 || g_psKeyNow) ? 1 : 0;
}
__declspec(naked) static void Hook_PageRow()
{
    __asm {
        movzx eax, byte ptr [ebp+0xF4]
        push  eax
        call  PageRowAsk
        add   esp, 4
        ret
    }
}

// --- Xbox icons in the OCP order: the game's own full key set choices ----------
// call 0x68D7A0 (Controls Options, before the lookup)
static const DWORD kPageGlyphCall = 0x6DA9E0;
static const DWORD kFontManager   = 0x68D7A0;
// mov al,[esp+0x23] / test al,al (all row icon cases jump here)
static const DWORD kHudRowAt      = 0x69898C;
static const BYTE  kExpHudRow[]   = { 0x8A, 0x44, 0x24, 0x23, 0x84, 0xC0 };

// Controls Options: [esp+0x14] = place on the row (0-26, +0x20 numbered), [esp+0x1f] = the
// numbered row flag, [esp+0x2c] = controller icon (0 = keyboard/mouse). One more on the
// stack here (the return address).
__declspec(naked) static void Hook_PageGlyph()
{
    __asm {
        cmp  byte ptr [g_keyModOrder], 0
        je   page_keep
        cmp  byte ptr [g_psKeyNow], 0
        jne  page_keep
        cmp  byte ptr [esp+0x30], 0
        je   page_keep
        cmp  byte ptr [esp+0x23], 0
        jne  page_keep
        mov  eax, [esp+0x18]
        cmp  eax, 27
        jae  page_keep
        movzx eax, byte ptr [kKeyRowMod+eax]
        mov  [esp+0x18], eax
    page_keep:
        push 0x68D7A0
        ret
    }
}

// HUD big icons: the row question (0x6987F7), then ebp = 0x40-0x43 moved to the OCP place
// when the Xbox icons are used.
__declspec(naked) static void Hook_RowAskBigOcp()
{
    __asm {
        call Hook_RowAskBig
        test al, al
        je   big_keep
        cmp  byte ptr [g_keyModOrder], 0
        je   big_keep
        cmp  byte ptr [g_psKeyNow], 0
        jne  big_keep
        lea  ecx, [ebp-0x40]
        cmp  ecx, 3
        ja   big_keep
        movzx ebp, byte ptr [kBigMod+ecx]
        add  ebp, 0x40
    big_keep:
        ret
    }
}

// HUD row icons: ebp = place on the row (0-26). [esp+0x23] = 0: the game uses the Xbox row
// without asking, so the place is moved here; otherwise the game asks at 0x698994 and the
// place is moved there if the answer is the Xbox row. The game's two instructions are
// repeated at the end (the next jump reads the flags they leave).
__declspec(naked) static void Hook_HudRowIcon()
{
    __asm {
        cmp  byte ptr [esp+0x23], 0
        jne  row_keep
        cmp  byte ptr [g_keyModOrder], 0
        je   row_keep
        cmp  byte ptr [g_psKeyNow], 0
        jne  row_keep
        cmp  ebp, 27
        jae  row_keep
        movzx ebp, byte ptr [kKeyRowMod+ebp]
    row_keep:
        mov  al, byte ptr [esp+0x23]
        test al, al
        push 0x698992
        ret
    }
}

// HUD row icons when the game asks (0x698994): the question, then the place moved to the
// OCP place if the answer is the Xbox row.
__declspec(naked) static void Hook_RowAskRowOcp()
{
    __asm {
        call Hook_RowAskRow
        test al, al
        je   ask_keep
        cmp  byte ptr [g_keyModOrder], 0
        je   ask_keep
        cmp  byte ptr [g_psKeyNow], 0
        jne  ask_keep
        cmp  ebp, 27
        jae  ask_keep
        movzx ebp, byte ptr [kKeyRowMod+ebp]
    ask_keep:
        ret
    }
}

static void SetupIcons()
{
    if (g_psOn && g_psIcons)
        Log("PlayStation icons: on (PlayStationIcons=1, the OCP PlayStation files are used)\r\n");
    else if (g_psOn)
        Log("PlayStation icons: off (PlayStationIcons=0), the game's numbered icons are kept\r\n");

    if (!CallIs(kFontAskCall, kAnyXbox) || !CallIs(kRowAskBig, kAnyXbox) || !CallIs(kRowAskRow, kAnyXbox) ||
        !CallIs(kHudSetupCall, kHudSetup) || *(DWORD*)kKeysetLoadSlot != kKeysetLoad ||
        memcmp((void*)kFontNameHd, kExpFontNameHd, sizeof(kExpFontNameHd)) != 0 ||
        (DWORD)*kKeysetPrefix != kKeysetPrefixGame ||
        memcmp((void*)kPageRowAt, kExpPageRow, sizeof(kExpPageRow)) != 0 ||
        !CallIs(kPageGlyphCall, kFontManager) ||
        memcmp((void*)kHudRowAt, kExpHudRow, sizeof(kExpHudRow)) != 0)
    {
        Log("Button icons: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return;
    }
    strcpy(g_fontNameHd, kLogiFontName);
    DWORD nameAt = (DWORD)g_fontNameHd;
    DWORD keysetHook = (DWORD)Hook_KeysetLoad;
    g_iconChoiceOk = Write((BYTE*)(kFontNameHd + 1), (const BYTE*)&nameAt, 4) &&
                     RedirectCall(kFontAskCall, kAnyXbox, (void*)Hook_FontAsk) &&
                     RedirectCall(kRowAskBig,   kAnyXbox, (void*)Hook_RowAskBigOcp) &&
                     RedirectCall(kRowAskRow,   kAnyXbox, (void*)Hook_RowAskRowOcp) &&
                     RedirectCall(kHudSetupCall, kHudSetup, (void*)Hook_HudSetup) &&
                     Write((BYTE*)kKeysetLoadSlot, (const BYTE*)&keysetHook, 4);
    if (g_iconChoiceOk)
    {
        BYTE call[6] = { 0xE8, 0, 0, 0, 0, 0x90 };
        *(DWORD*)(call + 1) = (DWORD)Hook_PageRow - (kPageRowAt + 5);
        g_iconChoiceOk = Write((BYTE*)kPageRowAt, call, sizeof(call)) &&
                         RedirectCall(kPageGlyphCall, kFontManager, (void*)Hook_PageGlyph);
    }
    if (g_iconChoiceOk)
    {
        BYTE jmp[6] = { 0xE9, 0, 0, 0, 0, 0x90 };
        *(DWORD*)(jmp + 1) = (DWORD)Hook_HudRowIcon - (kHudRowAt + 5);
        g_iconChoiceOk = Write((BYTE*)kHudRowAt, jmp, sizeof(jmp));
    }
    Log("Button icons: %s\r\n", g_iconChoiceOk ? "applied (the game's own icon file and row choices)"
                                               : "FAILED to write patch");
}

// --- Controls Options icon size --------------------------------------------------
// 0x6DA5A0 sizes each icon from the full key set glyph (24x24 pixels). The game shrinks it
// on screens under 800 pixels wide (0x6DAA18: cmp eax,0x320 / jae 0x6DAA53) and otherwise
// keeps the pixel size, so icons get smaller and smaller above 1080p. Like the text (section
// 1), the size is scaled by height / 1080 on screens taller than 1080; at 1080p and below
// nothing changes. Part of [Resolution].
static const DWORD kIconSizeAt = 0x6DAA18;
static const BYTE  kExpIconSize[] = { 0x3D, 0x20, 0x03, 0x00, 0x00, 0x73, 0x34 };
typedef void* (__cdecl* GetDisplay_t)();
typedef int (__thiscall* DisplayInt_t)(void*);
static int __cdecl ScaleIconPx(int px)
{
    void* d = ((GetDisplay_t)0x71FCA0)();                       // the game's display object
    if (!d) return px;
    int h = ((DisplayInt_t)(*(void***)d)[0x98 / 4])(d);          // screen height (as at 0x6DA67D)
    if (h <= 1080) return px;
    return (px * h + 540) / 1080;
}
__declspec(naked) static void Hook_IconSize()
{
    __asm {
        cmp  eax, 0x320                // eax = screen width (as the original)
        jb   narrow_screen
        push ebp                       // ebp = icon width
        call ScaleIconPx
        add  esp, 4
        mov  ebp, eax
        push dword ptr [esp+0x38]      // [esp+0x38] = icon height
        call ScaleIconPx
        add  esp, 4
        mov  [esp+0x38], eax
        push 0x6DAA53
        ret
    narrow_screen:
        push 0x6DAA1F
        ret
    }
}
static void PatchKeysetIconSize()
{
    if (memcmp((void*)kIconSizeAt, kExpIconSize, sizeof(kExpIconSize)) != 0)
    {
        Log("Controls icon size: NOT applied. The code in this mua.exe does not match the expected bytes.\r\n");
        return;
    }
    BYTE jmp[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
    *(DWORD*)(jmp + 1) = (DWORD)Hook_IconSize - (kIconSizeAt + 5);
    bool ok = Write((BYTE*)kIconSizeAt, jmp, sizeof(jmp));
    Log("Controls icon size: %s (Controls Options icons keep their 1080p size on taller screens)\r\n",
        ok ? "applied" : "FAILED to write patch");
}

// Every 2 seconds: hooks XInput files loaded since, opens player 1's PlayStation controller
// (or lets it go when player 1 no longer uses one), and picks its Xbox slot.
static DWORD WINAPI ControllerThread(LPVOID)
{
    if (!g_psOn) return 0;
    for (;;)
    {
        HookLoadedXInput();
        OpenPad();
        PickSlot();
        Sleep(2000);
    }
}

static void SetupControllers()
{
    InitializeCriticalSection(&g_padCs);
    HMODULE game = GetModuleHandleA(NULL);
    g_gameBase = (BYTE*)game;
    g_gameSize = ((IMAGE_NT_HEADERS*)(g_gameBase + ((IMAGE_DOS_HEADER*)g_gameBase)->e_lfanew))->OptionalHeader.SizeOfImage;

    if (!g_xboxOn && !g_psOn) { Log("PlayStation controllers: off (set in MUAFixPack.ini)\r\n"); return; }

    if (g_psOn)
    {
        HookImport(game, "kernel32.dll", "LoadLibraryA", (void*)Hook_LoadLibraryA, (void**)&g_realLoadLibraryA);
        HookImport(game, "kernel32.dll", "LoadLibraryW", (void*)Hook_LoadLibraryW, (void**)&g_realLoadLibraryW);
        Log("PlayStation controllers: applied\r\n");
        PatchCaptureRead();
        PatchPsListEntry();
        HookLoadedXInput();   // XInput files other plugins loaded before this one
    }
    else
        Log("PlayStation controllers: off (set in MUAFixPack.ini)\r\n");
    if (g_xboxOn) PatchXboxLookup();
    SetupIcons();
    HANDLE t = CreateThread(NULL, 0, ControllerThread, NULL, 0, NULL);
    if (t) CloseHandle(t);
}

// ---------------------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_self = hModule;
        InitializeCriticalSection(&g_logCs);
        DisableThreadLibraryCalls(hModule);
        Log("MUAFixPack Beta v1\r\n");
        ReadSettings();
        if (g_resOn)
        {
            PatchResolutionList();
            PatchResolutionCheck();
            PatchTextScale();
            PatchKeysetIconSize();
        }
        else
            Log("Resolution: off (set in MUAFixPack.ini)\r\n");
        if (g_movieOn) PatchMovieThreads();
        else Log("Movie fix: off (set in MUAFixPack.ini)\r\n");
        if (g_largeMovies) PatchMovieSize();
        else Log("Larger movies: off (set in MUAFixPack.ini)\r\n");
        PatchAnisotropy();
        PatchAntialiasing();
        SetupDisplayHooks();
        if (g_xboxOn)
        {
            PatchButtonGlyph();
        }
        else
            Log("Xbox controllers: off (set in MUAFixPack.ini)\r\n");
        SetupControllers();
        WriteLog();
    }
    return TRUE;
}
