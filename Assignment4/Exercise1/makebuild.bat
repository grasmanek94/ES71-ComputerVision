@ECHO OFF
cd /d "%~dp0"
rd /s /q build
mkdir build
cd build
cmake .. -G "Visual Studio 14 Win64"
pause