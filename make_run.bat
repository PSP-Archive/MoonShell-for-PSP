@echo off
call setenv_devkitPro_PSPr15.bat

:loop
cls
del EBOOT.elf
del EBOOT.PBP
del EBOOT.prx
make -j 2
if exist EBOOT.PBP goto run

echo !!! Compile error !!!
pause
goto loop

:run
del EBOOT.elf

cd psplink

del EBOOT.PBP
copy ..\EBOOT.PBP
pspsh.exe -e "cd ms0:"
pspsh.exe -e "rm ./a.pbp"
pspsh.exe -e "copy host0:EBOOT.PBP ./a.pbp"
pspsh.exe -e "./a.prx"
pspsh.exe
cd ..

rem pause
goto loop
