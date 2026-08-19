@echo off
echo Formatting all C/C++ files recursively...

for /r %%i in (*.cpp *.h *.hpp *.cc *.c) do (
    echo Formatting: %%i
    clang-format -i -style=file "%%i"
)

echo Formatting complete!
pause
