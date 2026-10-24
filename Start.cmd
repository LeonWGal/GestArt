@echo off
if exist "%~dp0bin\GestArtUltimate.exe" (
  start "" "%~dp0bin\GestArtUltimate.exe" %*
) else if exist "%~dp0release\bin\GestArtUltimate.exe" (
  start "" "%~dp0release\bin\GestArtUltimate.exe" %*
) else (
  echo Build GestArt Ultimate first. See README.md.
  pause
)
