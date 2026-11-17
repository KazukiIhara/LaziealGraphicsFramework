@echo off
setlocal EnableDelayedExpansion

if "%~1"=="" exit /b 1
if "%~2"=="" exit /b 1

set "sourceDir=%~dp0..\..\Externals\Effekseer"
set "buildDir=%sourceDir%\..\..\..\..\..\..\..\generated\effekseer\%~2"
set "installDir=%sourceDir%\install_%~2"
set "cmakeCommand=cmake"

where cmake >nul 2>nul
if errorlevel 1 (
    set "vswhere=!ProgramFiles(x86)!\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!vswhere!" (
        echo CMake was not found on PATH and vswhere.exe is unavailable.
        exit /b 1
    )
    for /f "usebackq delims=" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`) do set "cmakeCommand=%%i"
    if "!cmakeCommand!"=="cmake" (
        echo Visual Studio CMake component was not found.
        exit /b 1
    )
)

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
