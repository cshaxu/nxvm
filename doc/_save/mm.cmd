@echo off

rem NXVM Makefile Builder
rem Copyright 2012-2014 Neko.

cd.>"list.txt"

for /f "delims=" %%i in ('dir /a:-d /b src\*.c') do (
echo src/%%i>>"list.txt"
)

for /f "delims=" %%i in ('dir /a:-d /b src\device\*.c') do (
echo src/device/%%i>>"list.txt"
)

for /f "delims=" %%i in ('dir /a:-d /b src\device\debug\*.c') do (
echo src/device/debug/%%i>>"list.txt"
)

for /f "delims=" %%i in ('dir /a:-d /b src\device\qdx\*.c') do (
echo src/device/qdx/%%i>>"list.txt"
)

for /f "delims=" %%i in ('dir /a:-d /b src\platform\*.c') do (
echo src/platform/%%i>>"list.txt"
)

for /f "delims=" %%i in ('dir /a:-d /b src\platform\linux\*.c') do (
echo src/platform/linux/%%i>>"list.txt"
)

mmakfile.exe /l "list.txt" 1>"makefile"
del /f "list.txt"
