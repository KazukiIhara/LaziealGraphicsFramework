@echo off
setlocal EnableExtensions

if "%~1"=="" goto :usage

set "converter="
for %%C in (
    "%~dp0..\..\..\generated\outputs\Develop\x64\texconv.exe"
    "%~dp0..\..\..\generated\outputs\Release\x64\texconv.exe"
    "%~dp0..\..\..\generated\outputs\Debug\x64\texconv.exe"
) do (
    if not defined converter if exist "%%~fC" set "converter=%%~fC"
)

if not defined converter (
    for /f "delims=" %%C in ('where texconv.exe 2^>nul') do (
        if not defined converter set "converter=%%C"
    )
)

if not defined converter (
    echo [FAILED] texconv.exe was not found.
    echo Build the Texconv project first, or add texconv.exe to PATH.
    pause
    exit /b 1
)

set "result=0"
for %%F in (%*) do (
    if /I "%%~xF"==".hdr" (
        if exist "%%~dpnF.dds" (
            echo [SKIP] DDS already exists: %%~dpnF.dds
        ) else (
            echo [CONVERT] %%~fF
            rem %%~dpF ends with a backslash, so append a dot to keep the closing quote intact.
            "%converter%" -f R16G16B16A16_FLOAT -m 0 -dx10 -o "%%~dpF." "%%~fF"
            if errorlevel 1 set "result=1"
        )
    ) else (
        echo [SKIP] Not an HDR file: %%~fF
        set "result=1"
    )
)

pause
exit /b %result%

:usage
echo Drag one or more .hdr files onto this .bat file.
echo Existing .dds files are skipped.
pause
exit /b 1
