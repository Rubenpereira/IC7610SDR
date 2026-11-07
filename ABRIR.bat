@echo off
cd /d "%~dp0pronto"
if not exist IC7610SDR.exe (
  echo Nao achei pronto\IC7610SDR.exe - rode COMPILAR.bat primeiro.
  pause
  exit /b 1
)
start "" IC7610SDR.exe
