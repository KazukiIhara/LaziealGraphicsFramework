@echo off
cd /d "%~dp0" || exit /b 1
cd /d ".\Premake\" || exit /b 1
premake5  vs2026
pause