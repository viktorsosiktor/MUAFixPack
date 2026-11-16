# MUAFixPack Beta v1

First release. Every feature can be turned off in `MUAFixPack.ini`.

## Resolution
- The resolution menu shows every resolution your graphics card and monitor support.
- Safe start after a monitor change: if the saved resolution can't be shown, the game starts at your desktop resolution instead of crashing.
- Text and button icons keep their 1080p size above 1080p, including the icons in Controls Options.

## Movies
- Fixed the freeze when a movie or cutscene stops. It has its own switch (`[Movies]`) in case another mod already fixes it.
- Movies of practically any size can play (the movie player stopped at 720x576).

## Graphics
- Anisotropic filtering, 16x by default (8x, 4x and 2x can be chosen in `MUAFixPack.ini`).
- The Anti-Aliasing setting in Options offers every level the graphics card supports, for example 8x, instead of stopping at 4x.

## Windowed mode
- The game runs in a window, borderless and centered by default, using the game engine's own windowed mode. The original fullscreen is one setting away.
- Fill the screen, on by default: the picture fills the screen and keeps its shape, at any resolution. Ctrl+N turns it on and off.
- The mouse points where it should in a window, also after the window moves or changes, and while the screen is filled.
- Hotkeys (keyboard shortcuts) for fill the screen, the border and moving the window. They can be changed in `MUAFixPack.ini`; see the README.
- When the resolution is changed in Options, the window keeps its place and size until the first picture at the new resolution, so it doesn't jump around.

## Controllers
- Xbox controllers use the game's own Xbox 360 layout and icons.
- Xbox button icons work with the texs files from Marvel Mods' [Official Characters Pack (OCP)](https://marvelmods.com/forum/index.php/topic,11037.0.html) (`XboxIcons=1`, the default) and with the game's original texs files (`XboxIcons=0`).
- DualShock 4 and DualSense work with the automatic layout, and buttons can be set by hand. The touchpad works as Back.
- PlayStation button icons, using the texs files from the OCP (not included; `PlayStationIcons=1`, the default).
- Marvel Mods' Third Person Modes works with Xbox and PlayStation controllers.

## Known issues
- With an Xbox and a PlayStation controller connected at the same time, the game shows Xbox button icons at this time (the game's own rule: one set of icons for everyone).
- Switching between windowed and fullscreen while the game runs isn't possible at this time; fullscreen is chosen in `MUAFixPack.ini`.
- Marvel Mods' MUA Patch: its remastered shadow quality setting doesn't work with MUAFixPack at this time. Shadows appear fuzzy, possibly due to shaders. The game's shadows look good without it.

## Files
- `MUAFixPack.asi` - the plugin
- `MUAFixPack.ini` - settings (with the Official Characters Pack)
- `MUAFixPack_BaseGame.ini` - settings for the game without the Official Characters Pack (rename to `MUAFixPack.ini`: remove `_BaseGame` from the name)
- `MUAFixPack.cpp` - source code
