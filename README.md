# MUAFixPack

**Beta v1**

> This is a beta. MUAFixPack has been tested on the PCs, controllers and mods listed under [Tested with](#tested-with), and there may still be problems on other setups. If something doesn't work, turn off the matching feature in `MUAFixPack.ini`, or remove `MUAFixPack.asi` to return the game to normal. Please report problems in [Issues](../../issues), with your `MUAFixPack.log` attached.

A plugin for the original PC version of Marvel: Ultimate Alliance (2006). It fixes display and controller problems and limitations on modern PCs and adds a windowed mode. It does not replace any game files, and every feature can be turned off.

Other mods, including some from the Marvel Mods community, have also worked on resolution and controller support for MUA. MUAFixPack is an independent take: a single plugin that fixes the game's own code, built from scratch. Each major fix can be turned off on its own, so you can choose what to use and combine it with other mods.

## What it does

### Resolution
- **Every supported resolution:** the resolution menu shows every resolution your graphics card and monitor support, instead of the game's original list of 14 resolutions.
- **Safe start after a monitor change:** if the saved resolution is one your monitor doesn't support (often after switching to a monitor with a lower resolution), the game starts at your desktop resolution instead of crashing.
- **Text size above 1080p:** text and button icons, including the icons in Controls Options, keep their 1080p size on larger screens, instead of getting smaller and smaller at 1440p and 4K.

### Movies
- **Movie freeze fix:** fixes the game freezing when a movie or cutscene stops, often when skipping a movie or after changing display settings such as antialiasing.
- **Movies of any size:** the movie player's 720x576 limit is removed. Movies up to the MPEG-1 maximum of 4095x4095 can play, which is essentially no limit, so for example the Xbox 360 release's 720p movies can replace the PC movies (not included).

### Graphics
- **Anisotropic filtering:** the game's engine has anisotropic filtering, but the game never turned it on. MUAFixPack turns it on (16x by default), so floors, walls and roads stay sharp into the distance instead of turning blurry. If your graphics card supports less, the highest level it supports is used.
- **More antialiasing levels:** the Anti-Aliasing setting in Options offers every level your graphics card supports, for example 8x, instead of stopping at 4x.

### Windowed mode
- The game runs in a window at the resolution you choose in Options, borderless and centered by default. You can change the resolution in Options at any time; the window follows. For the original fullscreen, set `[Windowed] Enabled=0` in `MUAFixPack.ini`.
- **Fill the screen** (on by default): the game's picture fills your screen, keeping its shape, with black bars where it doesn't fit. At your monitor's own resolution it covers the whole screen and looks the same as fullscreen, but your monitor doesn't switch resolution and Alt+Tab is smooth. At a lower resolution, the picture is stretched up to fill the screen, the way the original fullscreen did.
- **Mouse in a window:** the mouse points where it should in a window, also after the window moves, changes size or gets a border, and while the screen is filled.
- **Hotkeys** (keyboard shortcuts), which can be changed in `MUAFixPack.ini`:
  - **Ctrl+N** turns fill the screen on and off. With it off, the window is the size of the game's resolution.
  - **Ctrl+B** turns the border and title bar on and off.
  - **Ctrl+I, Ctrl+J, Ctrl+K and Ctrl+L** move the window one step up, left, down or right, between 9 places on the screen (when it is smaller than the screen).

  While the screen is filled, the window is always borderless and centered; the border and move hotkeys take effect once fill the screen is off.
- **Smooth resolution changes:** when you change the resolution in Options with fill the screen off, the window keeps its place and size until the game shows its first picture at the new resolution, so it doesn't jump around.

### Controllers
- **Xbox controllers:** every Xbox controller gets the game's own Xbox 360 layout. With the automatic layout, every action is on the right button, the right stick works, and the matching Xbox button icons are shown.
- **Xbox button icons** work with the game's original texs files and with the texs files included with Marvel Mods' [Official Characters Pack (OCP)](https://marvelmods.com/forum/index.php/topic,11037.0.html) (see `XboxIcons` below).
- **PlayStation controllers:** DualShock 4 and DualSense controllers work with the automatic layout, and buttons can also be set by hand. The touchpad works as the Back button.
- **PlayStation button icons:** shown when the OCP's PlayStation texs files are in the texs folder (see `PlayStationIcons` below).
- **Third Person Modes** from Marvel Mods works with Xbox and PlayStation controllers.

## Requirements
- Marvel: Ultimate Alliance (2006), PC version.
- Ultimate ASI Loader (by ThirteenAG), to load the plugin (not included). Download `Ultimate-ASI-Loader-NoPDB.zip` from the [Ultimate ASI Loader releases page](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) and put its `dinput8.dll` in the game folder, next to `MUA.exe`.
- Tested with Ultimate ASI Loader v10.0.2 and v9.4.0 (the version included with the 50 Roster Hack).
- The default settings expect the [Official Characters Pack (OCP)](https://marvelmods.com/forum/index.php/topic,11037.0.html) by Marvel Mods, for its controller texs files. Without the OCP, use `MUAFixPack_BaseGame.ini` (see [Installation](#installation)).
- For PlayStation button icons: the PS4 texs files from the OCP's "1 - More controller support" folder, copied into the game's texs folder as they are (no renaming needed):
  - `FontTexture0_beenoxPS4_hd.png`
  - `Fullkeyset_PS4_eng.png`

## Installation
If you use other plugins that fix the same things (resolution, widescreen, windowed or controller fixes), remove them first, or turn off the matching MUAFixPack feature to keep them (see [Tested with](#tested-with)).

1. Install Ultimate ASI Loader, if you don't have it yet (see [Requirements](#requirements)).
2. Copy `MUAFixPack.asi` into the game's plugins folder.
3. Copy the settings file that matches your game into the plugins folder:
   - With the Official Characters Pack: `MUAFixPack.ini`.
   - Without it: `MUAFixPack_BaseGame.ini`, renamed to `MUAFixPack.ini` (remove `_BaseGame` from the name).

   If `MUAFixPack.ini` is missing, MUAFixPack creates one with the default settings (the OCP ones).
4. Start the game. It starts at your saved resolution, filling the screen.

To remove MUAFixPack, delete `MUAFixPack.asi`, `MUAFixPack.ini` and `MUAFixPack.log` from the plugins folder.

## Using a controller
- Connect the controller before starting the game.
- In Options > Controls, choose your controller and set Configuration to Automatic.
- With an Xbox and a PlayStation controller connected at the same time, the Xbox controller shows Xbox button icons and the PlayStation controller shows the game's numbered icons. This is the game's own rule: it uses one icon file for everyone, and an Xbox controller takes priority at this time.

## Windowed mode tips
- While the game is the active window, it keeps the mouse inside its picture. To move the window with a border, or to use another program, Alt+Tab out first.
- Alt+Tab switches to other windows; the game stays open behind them instead of minimizing. To get to the desktop (for example for a shortcut behind the game), press Win+D. Note that Win+D minimizes every open window.
- A window with a border at your desktop resolution doesn't fully fit on the screen, because the title bar and border need extra room. Use borderless at your desktop resolution, or a smaller resolution with the border.
- Switching between windowed and fullscreen while the game runs isn't possible at this time. For fullscreen, set `[Windowed] Enabled=0`.

## Settings (MUAFixPack.ini)
Every fix is on by default (`AlwaysOnTop` and `Border` start off). Set a line to 0 to turn that feature off, for example if another mod already does the same thing. A missing line uses that setting's default.

### [Resolution]
`Enabled=1`\
Every supported resolution, the safe start after a monitor change, and text and Controls Options icon size above 1080p. `Enabled=0` turns off all of them, the safe start included. Turn it off if another mod does the same things.

Before turning it off, set the resolution in Options to one of the game's original 14 that your monitor supports:\
640x480, 720x480, 800x600, 1024x768, 1280x720, 1280x768, 1280x960, 1280x1024, 1440x900, 1600x1200, 1680x1050, 1920x1080, 1920x1200, 2048x1536.

Any other saved resolution (one that MUAFixPack added, for example 2560x1440 or 3840x2160) can make the game crash at startup without the safe start. If the game no longer starts, set it in regedit instead (`HKEY_CURRENT_USER\Software\Activision\Marvel Ultimate Alliance\Settings\Display`, value `Resolution`), for example to 1920x1080.

### [Movies]
`Enabled=1`\
The movie freeze fix. Only turn it off if another mod already fixes it. Without it, the game can freeze when a movie or cutscene stops.

`LargeMovies=1`\
1 = movies up to 4095x4095 can play (the MPEG-1 limit), such as the Xbox 360 release's 720p movies. 0 = the movie player's original limit (720x576).

### [Graphics]
`Anisotropy=16`\
Anisotropic filtering: floors and walls stay sharp into the distance. 16 (default), 8, 4 or 2. 1 = the game's original filtering. If your graphics card supports less than you set, MUAFixPack uses the highest level it supports.

`Antialiasing=1`\
1 = the Anti-Aliasing setting in Options offers every level your graphics card supports (for example 8x). 0 = the game's original choices (up to 4x). If a level above 4x was chosen in Options, the game runs without antialiasing until you choose another level there.

### [Windowed]
`Enabled=1`\
1 = the game runs in a window. 0 = the game's original fullscreen (the other windowed settings and hotkeys then do nothing).

`FillScreen=1`\
1 = the game starts with its picture filling the screen, keeping its shape (always borderless and centered). 0 = the window is the size of the game's resolution.

`AlwaysOnTop=0`\
1 = the window stays in front of other windows, also after Alt+Tab. 0 = a normal window: other windows can cover it when you switch to them.

`Border=0`\
1 = the window starts with a border and title bar. 0 = borderless.

`Position=5`\
Where the window sits on the screen, laid out like a number pad: 7 8 9 = top, 4 5 6 = middle (5 = center), 1 2 3 = bottom. Only matters when the window is smaller than the screen.

`HotkeyBorder=Ctrl+B`\
`HotkeyFillScreen=Ctrl+N`\
Turn the border on and off, and fill the screen on and off.

`HotkeyUp=Ctrl+I`\
`HotkeyLeft=Ctrl+J`\
`HotkeyDown=Ctrl+K`\
`HotkeyRight=Ctrl+L`\
Move the window one step between the 9 positions above (when fill screen is off).

Each hotkey is one key, or one key with Ctrl, Alt and/or Shift. The key can be a letter, a number, F1 to F24, Up, Down, Left, Right, Numpad0 to Numpad9, Enter, Tab, Space, Home, End, Insert, Delete, PageUp or PageDown. Write it with + between the parts, for example `Ctrl+B`, `Shift+F7` or just `F7`. The game and other mods still get the key too, so choose keys they don't use; the defaults are keys the game doesn't use. If two hotkeys are given the same key, only the first works.

### [Controllers]
`Xbox=1`\
Every Xbox controller gets the game's own Xbox 360 layout.

`XboxIcons=1`\
Which Xbox button texs files are in the texs folder. 1 = the Official Characters Pack (OCP) texs files. Their icons are in another order; MUAFixPack has the game show each icon for the right button. 0 = the game's original texs files. If this doesn't match your files, the Xbox button icons don't match the buttons.

`PlayStation=1`\
The L2/R2 fix, the touchpad as Back, and support for mods like Third Person Modes.

`PlayStationIcons=1`\
1 = PlayStation button icons. The OCP files `FontTexture0_beenoxPS4_hd.png` and `Fullkeyset_PS4_eng.png` must be in the texs folder. Without them, the game's menu text and button icons don't show while a PlayStation controller is in use. 0 = the game's generic numbered button icons.

## What it touches on your PC
- **Registry:** MUAFixPack writes nothing. The game itself still saves its own Resolution setting when display settings are applied in Options, as it always has. After a safe start, the corrected resolution is saved the next time you apply a change there.
- **Files:** MUAFixPack writes `MUAFixPack.log`, and `MUAFixPack.ini` only if it is missing. It doesn't open, read or watch any other files. With `PlayStationIcons=1`, the game itself loads the OCP PlayStation texs files in place of its own. `MUA.exe` and the game's other files are never changed: every change is made in memory while the game runs.
- **Windows:** MUAFixPack only asks Windows for information it needs (which monitor the game is on and its size, how much room a border takes, whether Ctrl, Alt or Shift is held, Windows' standard black color). In windowed mode it sets the game window's border and place, and while the screen is filled it shows a plain black window behind the game. For mods like Third Person Modes, it presents player 1's PlayStation controller as an Xbox controller to those mods only; the game itself reads the controller as before.
- No Windows or graphics card settings are changed. Every change is made in the game's memory while it runs; nothing stays changed after the game closes.

## Log file
MUAFixPack writes `MUAFixPack.log` next to the .asi each time the game starts. It lists which fixes were applied. It only records information about the game, nothing about your PC or your files. If something doesn't work, the log helps to find out why.

## Tested with
- **Controllers:** Xbox Elite controller (Bluetooth), Xbox Series X controller, DualShock 4, DualSense and DualSense Edge.
- **Displays:** 3840x2160 (4K), 2560x1440 and 1920x1080.
- **Marvel Mods:** Official Characters Pack, 50 Roster Hack, Debug Menu, Third Person Modes and MUA Patch.

MUAFixPack has not been tested together with other resolution, widescreen, windowed or controller fix plugins. If you have one, remove it so the two don't conflict. Each fix in MUAFixPack can also be turned off if you want to keep using another solution, like dgVoodoo (graphics) or DS4Windows (controllers): turn off the matching feature in `MUAFixPack.ini` (for example `[Resolution] Enabled=0`, `[Movies] Enabled=0`, `[Windowed] Enabled=0` or `[Controllers] PlayStation=0`), and check `MUAFixPack.log` if something doesn't work.

**MUA Patch from Marvel Mods:** its remastered shadow quality setting doesn't work with MUAFixPack at this time. Shadows appear fuzzy. The game's shadows look good without it.

## Compatibility
Every fix checks the game's code first: `MUA.exe`, and also `libIGDisplay.dll` and `libIGGfx.dll` for windowed mode, `libIGGfx.dll` for the graphics features, and `libCriMovie.dll` for the movie fixes. With a different version, a fix that does not match is left out and the log says so.

## Potential future additions (pending research)
- A widescreen view for lower resolutions (the game draws a wider view instead of stretching the picture).
- Switching between fullscreen and windowed while the game runs.
- Controller enhancements, including more button icon support.
- Smoother fill screen at resolutions that don't scale evenly to the screen (for example 1440p on a 4K monitor, a 1.5x stretch).

## Building from source
In an x86 Native Tools Command Prompt for Visual Studio, in the folder with `MUAFixPack.cpp`:
```
cl /nologo /O2 /MT /LD MUAFixPack.cpp /link /OUT:MUAFixPack.asi
```

## Credits
- viktorsosiktor: project lead, design and testing.
- Code written with Claude (Anthropic).
- Thanks to the Marvel Mods community for tools and resources.
- Ultimate ASI Loader by ThirteenAG.
