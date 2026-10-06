# obs-reject-cookies

Automatically clicks **Reject** on Twitch's "Cookies and Advertising Choices" banner in OBS Studio's
built-in Twitch docks (Chat, Stats, Activity Feed, Stream Info), so you don't have to pull each dock
out to reach the button.

Unofficial; not affiliated with OBS or Twitch.

## How it works

OBS's embedded browser (CEF) can expose Chrome's remote-debugging endpoint. `launch-obs.vbs` starts OBS with
`--remote-debugging-port=9222` and runs `reject-cookies.mjs` hidden. The script polls the endpoint, finds
Twitch docks showing the banner, and clicks Reject. It exits when OBS closes.

This isn't a native OBS plugin: the debug port has to be set when OBS starts, which a separate plugin can't do.

## Install

Requires Windows and [Node.js](https://nodejs.org) 22+ (built-in `WebSocket`/`fetch`).

```powershell
.\install.ps1 -ObsPath "C:\Program Files\obs-studio\bin\64bit\obs64.exe"
```

Portable installs are detected automatically (`portable_mode.txt`). Start OBS from the
"OBS (auto-reject cookies)" desktop shortcut. A normal OBS start won't have the debug port, so it won't work.

## Notes

- The debug port listens on `127.0.0.1` only, but any program on your PC can connect to it.
- Relies on Twitch's current banner markup (`data-a-target="consent-banner-accept"` and a button labelled
  "Reject"); it may need updating if Twitch changes it.
- Set `OBS_DEBUG_PORT` to use a different port (also edit the port in `launch-obs.vbs`).

## Uninstall

Delete the shortcut and this folder.

## License

MIT
