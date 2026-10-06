# Usage: .\install.ps1 -ObsPath "D:\obs-studio\bin\64bit\obs64.exe"
param([string]$ObsPath = "C:\Program Files\obs-studio\bin\64bit\obs64.exe")
if (-not (Test-Path $ObsPath)) { throw "obs64.exe not found at $ObsPath" }
if (-not (Get-Command node -ErrorAction SilentlyContinue)) { throw "Node.js 22+ is required (https://nodejs.org)" }
Set-Content -Path "$PSScriptRoot\obs-path.txt" -Value $ObsPath
$lnk = Join-Path ([Environment]::GetFolderPath('Desktop')) 'OBS (auto-reject cookies).lnk'
$s = (New-Object -ComObject WScript.Shell).CreateShortcut($lnk)
$s.TargetPath = "$PSScriptRoot\launch-obs.vbs"
$s.IconLocation = "$ObsPath,0"
$s.WorkingDirectory = $PSScriptRoot
$s.Save()
Write-Host "Done. Start OBS from the shortcut: $lnk"
