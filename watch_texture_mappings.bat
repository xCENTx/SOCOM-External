@echo off
setlocal
cd /d "%~dp0"
set "VIEWER_PYTHON=python"
if defined WEAPON_VIEWER_PYTHON set "VIEWER_PYTHON=%WEAPON_VIEWER_PYTHON%"
if not defined WEAPON_VIEWER_PYTHON if exist "%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" set "VIEWER_PYTHON=%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
"%VIEWER_PYTHON%" -c "import PIL,numpy" >nul 2>&1
if errorlevel 1 (
 echo Python with Pillow and numpy is required. Install tools\requirements.txt.
 echo Set WEAPON_VIEWER_PYTHON to select another Python executable.
 pause
 exit /b 1
)
set "REPLACEMENTS="
set /p REPLACEMENTS=<assets\viewer-settings.txt
if not defined REPLACEMENTS (
 echo Set the replacement folder in the viewer first.
 pause
 exit /b 1
)
"%VIEWER_PYTHON%" tools\match_pcsx2_textures.py assets "%REPLACEMENTS%\..\dumps" "%REPLACEMENTS%" --watch
if errorlevel 1 pause
