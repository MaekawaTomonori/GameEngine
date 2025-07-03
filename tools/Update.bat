@echo off
cd ..
git pull
git submodule update --init --recursive

call tools/BuildLib.bat

echo All Updated
pause