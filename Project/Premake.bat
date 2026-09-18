@echo off
cd /d "%~dp0" || exit /b 1
cd /d ".\Premake\" || exit /b 1
if exist "..\LaziealGraphicsFramework.sln" del /q "..\LaziealGraphicsFramework.sln"
premake5  vs2026
pause
