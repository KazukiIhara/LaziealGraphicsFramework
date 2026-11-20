@echo off
setlocal EnableDelayedExpansion

if "%~1"=="" exit /b 1
if "%~2"=="" exit /b 1

set "sourceDir=%~dp0..\..\Externals\Effekseer"
set "buildDir=%sourceDir%\..\..\..\..\..\..\..\generated\effekseer\%~2"
set "installDir=%sourceDir%\install_%~2"
set "cmakeCommand="
set "visualStudioVersion="
set "vswhereResult=%~dp0LaziealEffekseerVswhere_%RANDOM%_%RANDOM%.txt"

if /i "%~1"=="Visual Studio 17 2022" set "visualStudioVersion=[17.0,18.0)"
if /i "%~1"=="Visual Studio 18 2026" set "visualStudioVersion=[18.0,19.0)"

if defined visualStudioVersion (
    set "vswhere=!ProgramFiles(x86)!\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!vswhere!" (
        "!vswhere!" -latest -products * -version "!visualStudioVersion!" -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe >"!vswhereResult!" 2>nul
        if not errorlevel 1 if exist "!vswhereResult!" set /p "cmakeCommand="<"!vswhereResult!"
        if exist "!vswhereResult!" del /q "!vswhereResult!"
    )
)

if not defined cmakeCommand for /f "delims=" %%i in ('where cmake 2^>nul') do if not defined cmakeCommand set "cmakeCommand=%%i"

if not defined cmakeCommand (
    echo CMake was not found. Install the CMake component for %~1 or add a compatible CMake to PATH.
    exit /b 1
)

setlocal DisableDelayedExpansion
set "cmakeHelp=%~dp0LaziealEffekseerCMakeHelp_%RANDOM%_%RANDOM%.txt"
"%cmakeCommand%" --help >"%cmakeHelp%" 2>nul
if errorlevel 1 (
    if exist "%cmakeHelp%" del /q "%cmakeHelp%"
    echo Failed to run CMake: %cmakeCommand%
    exit /b 1
)

findstr /l /c:"%~1" "%cmakeHelp%" >nul
if errorlevel 1 (
    del /q "%cmakeHelp%"
    echo The selected CMake does not support the "%~1" generator: %cmakeCommand%
    exit /b 1
)
del /q "%cmakeHelp%"

if not exist "%installDir%\lib\Effekseer.lib" goto BUILD
if not exist "%installDir%\lib\Effekseerd.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererCommon.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererCommond.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererLLGI.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererLLGId.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererDX12.lib" goto BUILD
if not exist "%installDir%\lib\EffekseerRendererDX12d.lib" goto BUILD
if not exist "%installDir%\lib\LLGI.lib" goto BUILD
if not exist "%installDir%\lib\LLGId.lib" goto BUILD
if not exist "%installDir%\include\EffekseerRendererLLGI\Common.h" goto BUILD
findstr /c:"../EffekseerRendererCommon/EffekseerRendererCommon/" "%installDir%\include\EffekseerRendererLLGI\Common.h" >nul
if not errorlevel 1 goto BUILD
exit /b 0

:BUILD
"%cmakeCommand%" -S "%sourceDir%" -B "%buildDir%" -G "%~1" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%installDir%" ^
    -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DBUILD_TEST=OFF ^
    -DBUILD_GL=OFF ^
    -DBUILD_VULKAN=OFF ^
    -DBUILD_DX9=OFF ^
    -DBUILD_DX11=OFF ^
    -DBUILD_DX12=ON ^
    -DUSE_OPENAL=OFF ^
    -DUSE_DSOUND=OFF ^
    -DUSE_XAUDIO2=OFF ^
    -DCMAKE_DEBUG_POSTFIX=d
if errorlevel 1 exit /b 1

"%cmakeCommand%" --build "%buildDir%" --config Debug --target INSTALL
if errorlevel 1 exit /b 1

"%cmakeCommand%" --build "%buildDir%" --config Release --target INSTALL
exit /b %errorlevel%
