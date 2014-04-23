@echo off
mkdir cpp\console
mkdir cpp\vmachine\system
copy *.? cpp\*.?
copy console\*.? cpp\console\*.?
copy vmachine\*.? cpp\vmachine\*.?
copy vmachine\system\*.? cpp\vmachine\system\*.?
ren cpp\*.c cpp\*.cpp
cd cpp
ren *.c *.cpp
cd console
ren *.c *.cpp
cd ..\vmachine
ren *.c *.cpp
cd system
ren *.c *.cpp
cd..
cd..
