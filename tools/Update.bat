@echo off
cd ..
git pull
git submodule update --init --recursive
echo All Updated
pause