@echo off
setlocal

echo Tab to Space Converter
echo Target: Current directory and all subdirectories
echo Space count: 4 (fixed)
echo Target files: .h .hpp .cpp

echo.
echo Starting conversion...

for %%E in (h hpp cpp) do (
    for /r "." %%F in (*.%%E) do (
        echo Processing: %%F
        powershell -Command "(Get-Content '%%F') -replace '\t', '    ' | Set-Content '%%F'"
    )
)

echo.
echo Conversion complete.
echo All C++ source files have been converted to use 4-space indentation.
pause