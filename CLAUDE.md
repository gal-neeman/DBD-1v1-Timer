# CLAUDE.md

This is a C++ overlay built on Win32, Direct2D/DirectWrite, and XInput. The JsonCpp amalgamation is vendored in `third_party/`, which is on the include path via `$(ProjectDir)third_party`, so include it as `<json/json.h>`. See README.md for features.

## Build
- Open `DBD 1v1 Timer.sln`, which uses MSVC toolset v145. From a Developer PowerShell:
  `msbuild "DBD 1v1 Timer.sln" /p:Configuration=Release /p:Platform=x64`
- There are no tests or linter. Verify a change by building and running the exe.
- Sources are listed explicitly in `DBD 1v1 Timer.vcxproj` and its `.filters` file, so add new .cpp files there.
- Libraries are linked with `#pragma comment(lib, ...)` in `Program.cpp`, not in project settings.
- Only `DBD 1v1 Timer1.rc` is compiled; `DBD 1v1 Timer.rc` is unused. The `.rc` files are UTF-16, so grep and sed silently find nothing; decode with `iconv -f UTF-16 -t UTF-8` first. The compiled `.rc` loads its bitmaps from absolute paths outside the repo (`...\dbd1v1\concept\*.bmp`).
- `IDB_MOUSE` and `IDB_CONTROLLER` in `globals.h` hardcode the values of `IDB_BITMAP1` and `IDB_BITMAP2` from `resource.h`. Keep them in sync.
- The global keyboard and mouse hooks are skipped when a debugger is attached, so hotkeys don't work under the VS debugger.

## How it fits together
- **All input becomes one message.** Two sources feed `HotkeyManager::execute`:
  - the low-level hooks in `Program.cpp`;
  - `ControllerManager`, which polls controller index 0 on its own thread. It maps XInput and PlayStation button bits to `CONTROLLER_*` codes (5000–5015) and delivers them with `SendMessage(CONTROLLER_INPUT)`.

  `HotkeyManager::execute` then `PostMessage`s `HOTKEY_HIT` to `MainWindow::handleHotKey`. While the settings window is open, controller input goes to it instead, for rebinding.
- **Timing is single-threaded:** `SetTimer` fires every 16 ms; each `WM_TIMER` calls `Timer::updateTime()` on both timers, then redraws.
- **Windows** derive from the CRTP template `BaseWindow<T>` (`BaseWindow.h`). Each window type needs a unique `className()`. `RegisterClass` failures are ignored, so a duplicate name silently reuses the other class's window procedure.
- **Hotkey capture in Settings:** `SettingsWindow::ctrlWndProc` subclasses every control. While a binding is active, it forwards mouse clicks to the settings window, which is how mouse buttons can be bound.
- **Exactly two timers is assumed in every layer:**
  - `timer1`/`timer2`;
  - `KEY_TIMER1`/`KEY_TIMER2`;
  - the `timer1Key`/`conTimer1Key` settings and their JSON keys;
  - the last-seconds logic in `MainWindow::handlePainting`.

## Deliberate choices that look wrong
- The D2D render target is `D2D1_RENDER_TARGET_TYPE_SOFTWARE` with `D2D1_PRESENT_OPTIONS_IMMEDIATELY` on purpose: ReShade hooks hardware render targets, which causes stutter. Don't switch it back to hardware as an optimization.
- `wWinMain` writes a registry AppCompat flag that disables fullscreen optimizations. This is also a stutter fix.
- Transparency is a color key on pure black: when transparent, the window is cleared to `RGB(0,0,0)`. Palette index 20 is `RGB(1,1,1)` so the "black" color isn't keyed out. Never put pure black in the palette (`PALETTE` in `ResourceUtils.h`).
- `optionClickThrough` is never saved and is reset to false on load, so a user can't lock themselves out of the window.

## Settings
- The settings file is `Settings.json` in the **working directory** (`SETTINGS_FILE_NAME`). The `settings.json` in the repo is only a sample.
- `settingsFromJson` validates each group of fields separately (hotkeys, bool options, numeric options, colors). If a group is invalid, the whole group falls back to defaults. The fallback values there repeat the struct defaults in `globals.h` as literals, so change both together.
- Colors are stored as indexes 0–24 into `PALETTE`, not as RGB values.
- `appSettings` is the only runtime source of settings. The file is read once, by `loadSettingsFromFile` in `wWinMain` before the main window is created, and after that is only written, by `applySettings`.
- **Adding a setting:**
  1. Add the field and its default to `SettingsStruct` in `globals.h`.
  2. Update `settingsToJson` and `settingsFromJson`, including validation and a fallback.
  3. Add a `CID_*` constant and the control in `SettingsWindow`: `initializeTextControls`, `initializeButtonControls`, `applyInitialControlTextValue`, and `handleControlCommand`. If the value comes from a text field, also read it in the OK handler.
  4. Read the setting through `appSettings`.

## Version bump
Update these places:
- `DBD 1v1 Timer1.rc`: FILEVERSION, PRODUCTVERSION, and the `FileVersion`/`ProductVersion` strings.
- `APP_VERSION` in `globals.h`, which the Settings window title is built from.

## Workflow
- Issues live in the Linear project "DBD 1v1 Timer", through the Linear server in `.mcp.json`. Commands in `.claude/commands/`: `/create-feature`, `/create-bug`, `/pr`, `/cr`.
- Branch names are `feature/<issue>/<kebab-name>` or `bugs/<issue>/<kebab-name>`. Release branches are `versions/x.y.z`. PRs target `master`.
- Commit messages are short, lowercase, and imperative (for example "fix validation bug for colors").
- Planned work lives in `TODO`.
