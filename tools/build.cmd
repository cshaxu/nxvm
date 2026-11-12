@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "build_result=1"
set "build_pause=1"
set "build_app=%~1"
if /i "%~2"=="--no-pause" set "build_pause=0"
if not "%~2"=="" if /i not "%~2"=="--no-pause" goto usage
if not "%~3"=="" goto usage
if not "%build_app%"=="" goto run

echo Select a product to build and publish for x86 and x64:
echo   1. NXVM
echo   2. My5160
echo   3. My5170
echo   4. MyDeskPro386
echo   5. MyPC110
echo   6. MyNES
echo   7. MyRaidenII
echo   8. All products
echo   0. Exit
choice /c 123456780 /n /m "Product: "
if errorlevel 255 goto finish
if errorlevel 9 exit /b 0
if errorlevel 8 (
    set "build_app=all"
    goto run
)
if errorlevel 7 (
    set "build_app=myraidenii"
    goto run
)
if errorlevel 6 (
    set "build_app=mynes"
    goto run
)
if errorlevel 5 (
    set "build_app=mypc110"
    goto run
)
if errorlevel 4 (
    set "build_app=mydeskpro386"
    goto run
)
if errorlevel 3 (
    set "build_app=my5170"
    goto run
)
if errorlevel 2 (
    set "build_app=my5160"
    goto run
)
if errorlevel 1 (
    set "build_app=nxvm"
    goto run
)
goto finish
:run
where.exe cmake.exe >nul 2>&1
if errorlevel 1 (
    echo Missing required tool on PATH: cmake.exe
    goto finish
)
cmake.exe "-DPRODUCT=%build_app%" -P "%~dp0..\cmake\BuildProducts.cmake"
set "build_result=%errorlevel%"
if not "%build_result%"=="0" echo Build failed. Some products or architectures may not be updated.
goto finish
:usage
echo Usage: build.cmd [nxvm^|my5160^|my5170^|mydeskpro386^|mypc110^|mynes^|myraidenii^|all] [--no-pause]
:finish
if "%build_pause%"=="1" pause
exit /b %build_result%
