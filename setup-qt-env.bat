@echo off
REM Qt Environment Setup Script for EKiosk Project
REM This script sets up environment variables for Qt development

echo Setting up Qt environment variables for EKiosk...

REM Set Qt5 directory (adjust path as needed for your system)
if exist "C:\Qt\5.15.2\msvc2019_64" (
  setx QT5_DIR "C:\Qt\5.15.2\msvc2019_64"
  echo QT5_DIR set to: C:\Qt\5.15.2\msvc2019_64
  ) else if exist "C:\Qt\Qt5.6.3\5.6.3\msvc2015" (
  setx QT5_DIR "C:\Qt\Qt5.6.3\5.6.3\msvc2015"
  echo QT5_DIR set to: C:\Qt\Qt5.6.3\5.6.3\msvc2015
  ) else (
  echo Warning: Common Qt5 installation paths not found.
  echo Please set QT5_DIR manually to your Qt5 installation directory.
)

REM Set Qt6 directory (adjust path as needed for your system)
if exist "C:\Qt\6.10.1\msvc2022_64" (
  setx QT6_DIR "C:\Qt\6.10.1\msvc2022_64"
  echo QT6_DIR set to: C:\Qt\6.10.1\msvc2022_64
  ) else (
  echo Qt6 installation not found at default location.
)

echo.
echo Environment variables set. Please restart your terminal/command prompt.
echo You can verify the variables with: echo %%QT5_DIR%% or echo %%QT6_DIR%%
echo.
echo For VS Code CMake extension, you can now use the default preset.
pause
