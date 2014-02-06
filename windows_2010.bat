:: go to folder with premake4.exe
cd premake

:: build solution in 2010 since premake doesn't natively support 2012 yet
premake4 --file=premake4.lua vs2010

START "" ..\build\SLN.SLN

:END
