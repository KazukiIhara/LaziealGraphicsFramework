@echo off
setlocal
cd /d "%~dp0"

pushd "Premake"
premake5.exe vs2022
set "result=%errorlevel%"
popd

exit /b %result%
