@echo off
call ..\..\setenv_devkitPro_PSPr15.bat
:loop
cls
echo %TIME% > _CompileTime
call make_libs_ins.bat LibImgBMP
call make_libs_ins.bat LibImgJpeg
call make_libs_ins.bat LibImgPNG
call make_libs_ins.bat LibImgPSD
call make_libs_ins.bat LibSndFLAC
call make_libs_ins.bat LibSndG721
call make_libs_ins.bat LibSndGME
call make_libs_ins.bat LibSndM4A
call make_libs_ins.bat LibSndMDX
call make_libs_ins.bat LibSndMIDI
call make_libs_ins.bat LibSndMOD
call make_libs_ins.bat LibSndMP3
call make_libs_ins.bat LibSndOGG
call make_libs_ins.bat LibSndTTA
call make_libs_ins.bat LibSndWAV
call make_libs_ins.bat LibSndWMA
call make_libs_ins.bat libzlib
echo %TIME% >> _CompileTime
pause
goto loop
