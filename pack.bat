@ECHO OFF
md "..\Packages\bss-util"

:: So, uh, apparently XCOPY deletes empty subdirectories in your destination.
XCOPY "*.cpp" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.c" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.vcxproj" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.filters" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.sln" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "bss-util\*.rc" "..\Packages\bss-util\bss-util" /C /I /R /Y

md "..\Packages\bss-util\include"
md "..\Packages\bss-util\doc"
md "..\Packages\bss-util\bin"
md "..\Packages\bss-util\test"

XCOPY "include\*.h" "..\Packages\bss-util\include" /S /C /I /R /Y
DEL "..\Packages\bss-util\include\bss_sort.h"
DEL "..\Packages\bss-util\include\cIntervalTree.h"
DEL "..\Packages\bss-util\include\cHuffmanTree.h"
XCOPY "doc\*.txt" "..\Packages\bss-util\doc" /S /C /I /R /Y
XCOPY "bin\*.dll" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin\*.lib" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin\*.pdb" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin\blank.txt" "..\Packages\bss-util\bin\" /C /I /Y
DEL "..\Packages\bss-util\bin\test_d.*"
DEL "..\Packages\bss-util\bin\test.*"
DEL "..\Packages\bss-util\bin\test64.*"
DEL "..\Packages\bss-util\bin\test64_d.*"
XCOPY "bin\*.exe" "..\Packages\bss-util\bin\" /C /I /Y

Pause