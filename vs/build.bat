rem compile obj

cl /c /EHsc ..\lib\*.cpp /I ..\include\
cl /c /EHsc ..\unicode\*.cpp /I ..\include\

rem static lib

lib /OUT:reflex.lib *.obj

rem dynamic lib

LINK /DLL /OUT:reflex.dll *.obj

rem compile reflex

cl /c /EHsc ..\src\*.cpp /I ..\include\

rem reflex

cl  reflex.obj reflex.lib

rem reflex

reflex -V
