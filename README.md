# Reject Cookies for OBS

[![Build](https://github.com/Jayconius/obs-reject-cookies/actions/workflows/build.yml/badge.svg)](https://github.com/Jayconius/obs-reject-cookies/actions/workflows/build.yml)

An OBS Studio plugin that automatically clicks **Reject** on Twitch's "Cookies and Advertising Choices"
banner in OBS's built-in Twitch docks (Chat, Stats, Activity Feed, Stream Info). No more pulling docks out
to reach a button that's cut off.

Unofficial; not affiliated with OBS or Twitch.

## Install (Windows, 64-bit)

1. Download the `obs-reject-cookies-<version>-windows-x64.zip` file from the [latest release](../../releases/latest).
2. Close OBS.
3. Extract the zip into your OBS folder (e.g. `C:\Program Files\obs-studio`, or your portable folder),
   so that `obs-reject-cookies.dll` ends up in `obs-plugins\64bit`.
4. Start OBS. Banners are rejected as soon as they appear.

Uninstall: delete `obs-plugins\64bit\obs-reject-cookies.dll`.

## How it works

The plugin finds OBS's browser docks (using the same interface OBS uses for its own docks) and adds a small
script to each page once, and again if the dock navigates or reloads. On `twitch.tv` pages the script watches
for the consent banner and clicks its reject button the moment it appears. The button is found by its position
in the banner rather than its label, so it should work in any Twitch language (only English has been tested so far). Other sites are left alone. It collects no
data and makes no network requests.

## Notes

- Requires OBS 31 or newer on Windows (64-bit), with the built-in browser module (`obs-browser`), which
  is included and enabled by default. Tested on OBS 32.2.1.
- Windows only. Linux and macOS ports are welcome as forks or pull requests.
- It depends on Twitch's current banner markup (`data-a-target="consent-banner-accept"`), so it may need
  an update if Twitch changes it.

## Build

Requires Visual Studio with C++ tools and CMake. See `CMakePresets.json`:

```powershell
cmake --preset windows-x64
cmake --build build_x64 --config RelWithDebInfo
```

Release zips are built by GitHub Actions from the tagged source (see `.github/workflows/build.yml`).

## License

GPL-2.0-or-later (the plugin links against OBS's GPL libraries). `src/browser-panel.hpp` is from
[obs-browser](https://github.com/obsproject/obs-browser) (GPL-2.0).
