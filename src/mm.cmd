@echo off
rem Make NXVM Makefile
rem Coded by Neko, 2012-2013. All rights reserved.
cd.>"list.txt"
for /f "delims=" %%i in ('dir /a:-d /b *.c') do (
echo %%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b console\*.c') do (
echo console/%%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b vmachine\*.c') do (
echo vmachine/%%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b vmachine\system\l*.c') do (
echo vmachine/system/%%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b vmachine\bios\*.c') do (
echo vmachine/system/%%i>>"list.txt"
)
mmakfile.exe /l "list.txt" 1>"makefile"