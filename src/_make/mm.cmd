@echo off
rem Make Makefile
rem By Neko, 2012. All rights reserved.
cd..
cd.>"list.txt"
for /f "delims=" %%i in ('dir /a:-d /b *.c') do (
echo %%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b asm86\*.c') do (
echo asm86/%%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b vmachine\*.c') do (
echo vmachine/%%i>>"list.txt"
)
for /f "delims=" %%i in ('dir /a:-d /b vmachine\system\*.c') do (
echo vmachine/system/%%i>>"list.txt"
)
mmakfile.exe "list.txt" 1>"makefile"