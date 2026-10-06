# Reject Cookies for OBS

An OBS Studio plugin that automatically clicks **Reject** on Twitch's "Cookies and Advertising Choices"
banner in OBS's built-in Twitch docks (Chat, Stats, Activity Feed, Stream Info). No more pulling docks out
to reach a button that's cut off.

Unofficial; not affiliated with OBS or Twitch.

## Install (Windows, 64-bit)

1. Download `obs-reject-cookies-1.0.0-windows-x64.zip` from the [latest release](../../releases/latest).
2. Close OBS.
3. Extract the zip into your OBS folder (e.g. `C:\Program Files\obs-studio`, or your portable folder),
   so that `obs-reject-cookies.dll` ends up in `obs-plugins\64bit`.
4. Start OBS. Banners are rejected within a couple of seconds of a dock loading.

Uninstall: delete `obs-plugins\64bit\obs-reject-cookies.dll`.

## How it works

Every 2 seconds the plugin runs a tiny script in each of OBS's browser docks, using the same interface
OBS uses for its own docks. The script does nothing unless the page is on `twitch.tv` and the consent banner is
showing; then it clicks the "Reject" button. It collects no data and makes no network requests.

## Notes

- Tested on OBS 32.2.1 (Windows). Needs the OBS browser module (`obs-browser`) to be enabled.
- It depends on Twitch's current banner markup (`data-a-target="consent-banner-accept"` and a button labelled
  "Reject"), so it may need an update if Twitch changes it.

## Build

Requires Visual Studio with C++ tools and CMake. See `CMakePresets.json`:

```powershell
cmake --preset windows-x64
cmake --build build_x64 --config RelWithDebInfo
```

`legacy-script/` holds an earlier external-script version that needed OBS launched with a debug port. It is
superseded by the plugin.

## License

GPL-2.0-or-later (the plugin links against OBS's GPL libraries). `src/browser-panel.hpp` is from
[obs-browser](https://github.com/obsproject/obs-browser) (GPL-2.0).
