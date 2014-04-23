@echo off
if exist "cpp" echo y | rd /s cpp
mkdir cpp\console
mkdir cpp\vmachine\system
copy *.h cpp\*.h
copy console\*.h cpp\console\*.h
copy vmachine\*.h cpp\vmachine\*.h
copy vmachine\system\*.h cpp\vmachine\system\*.h
copy *.c cpp\*.c
copy console\*.c cpp\console\*.c
copy vmachine\*.c cpp\vmachine\*.c
copy vmachine\system\*.c cpp\vmachine\system\*.c
cd cpp
ren *.c *.cpp
cd console
ren *.c *.cpp
cd ..\vmachine
ren *.c *.cpp
cd system
ren *.c *.cpp
cd ..\..\..