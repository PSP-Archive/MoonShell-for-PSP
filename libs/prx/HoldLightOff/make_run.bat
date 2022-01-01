@echo off
call setenv_devkitPro_PSPr15.bat

:loop
cls
del HoldLightOff.elf
del HoldLightOff.prx
del main.o
make -j 2
if exist HoldLightOff.elf goto run

echo !!! Compile error !!!
pause
goto loop

:run
del HoldLightOff.elf
copy HoldLightOff.prx ..\..\..\psplink\p.prx
pause
goto loop
