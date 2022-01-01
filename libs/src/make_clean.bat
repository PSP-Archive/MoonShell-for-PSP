call ..\..\setenv_devkitPro_PSPr15.bat
:loop
call make_clean_ins.bat LibImgBMP
call make_clean_ins.bat LibImgJpeg
call make_clean_ins.bat LibImgPNG
call make_clean_ins.bat LibImgPSD
call make_clean_ins.bat LibSndFLAC
call make_clean_ins.bat LibSndG721
call make_clean_ins.bat LibSndGME
call make_clean_ins.bat LibSndM4A
call make_clean_ins.bat LibSndMDX
call make_clean_ins.bat LibSndMIDI
call make_clean_ins.bat LibSndMOD
call make_clean_ins.bat LibSndMP3
call make_clean_ins.bat LibSndOGG
call make_clean_ins.bat LibSndTTA
call make_clean_ins.bat LibSndWAV
call make_clean_ins.bat LibSndWMA
call make_clean_ins.bat libzlib
pause
rem goto loop
