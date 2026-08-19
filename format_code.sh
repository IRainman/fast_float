#!/bin/bash
echo "Formatting all C/C++ files recursively..."

# Find and format files using an optimized execution loop
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.c" \) -exec echo "Formatting: {}" \; -exec clang-format -style=file -i {} \;

echo "Formatting complete!"

# Pause analog: waits for a single keypress (-n 1) and hides the input (-s)
echo "Press any key to continue . . ."
read -n 1 -s -r