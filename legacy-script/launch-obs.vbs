' Starts OBS with the browser debug port, plus the cookie-reject watcher (both hidden).
' OBS path comes from obs-path.txt (written by install.ps1), else the default install path.
Set fso = CreateObject("Scripting.FileSystemObject")
Set sh = CreateObject("WScript.Shell")
here = fso.GetParentFolderName(WScript.ScriptFullName)

obs = "C:\Program Files\obs-studio\bin\64bit\obs64.exe"
cfg = here & "\obs-path.txt"
If fso.FileExists(cfg) Then
  Set f = fso.OpenTextFile(cfg, 1)
  line = Trim(f.ReadLine())
  f.Close
  If line <> "" Then obs = line
End If

args = "--remote-debugging-port=9222"
root = fso.GetParentFolderName(fso.GetParentFolderName(fso.GetParentFolderName(obs)))
If fso.FileExists(root & "\portable_mode.txt") Then args = "--portable " & args

sh.CurrentDirectory = fso.GetParentFolderName(obs)
sh.Run """" & obs & """ " & args, 1, False
sh.CurrentDirectory = here
sh.Run "node reject-cookies.mjs", 0, False
