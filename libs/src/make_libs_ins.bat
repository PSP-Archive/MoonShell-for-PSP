cd %1
del %1.a
del ..\..\%1.a
make -j 2
copy %1.a ..\..
del %1.a
cd ..
