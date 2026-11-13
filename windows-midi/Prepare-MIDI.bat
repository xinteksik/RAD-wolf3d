@echo off
setlocal DisableDelayedExpansion
chcp 65001 >nul
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Prepare-MIDI.ps1" -GameDirectory "%~1"
set "result=%errorlevel%"
if not defined RADWOLF_NO_PAUSE pause
exit /b %result%
