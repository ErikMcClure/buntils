@ECHO OFF
md "..\Packages\bss-util"

:: So, uh, apparently XCOPY deletes empty subdirectories in your destination.
XCOPY "*.cpp" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.c" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "test\*.h" "..\Packages\bss-util\test" /S /C /I /R /Y
XCOPY "test\*.rc" "..\Packages\bss-util\test" /S /C /I /R /Y
XCOPY "test\*.ico" "..\Packages\bss-util\test" /S /C /I /R /Y
XCOPY "*.vcxproj" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.filters" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "*.sln" "..\Packages\bss-util" /S /C /I /R /Y
XCOPY "bss-util\*.rc" "..\Packages\bss-util\bss-util" /C /I /R /Y

md "..\Packages\bss-util\include"
md "..\Packages\bss-util\doc"
md "..\Packages\bss-util\bin"
md "..\Packages\bss-util\bin32"
md "..\Packages\bss-util\test"

XCOPY "LICENSE.txt" "..\Packages\bss-util" /C /I /R /Y
XCOPY "README.md" "..\Packages\bss-util" /C /I /R /Y
XCOPY "include\*.h" "..\Packages\bss-util\include" /S /C /I /R /Y
XCOPY "doc\*.txt" "..\Packages\bss-util\doc" /S /C /I /R /Y
XCOPY "bin\bss-util*.dll" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin\bss-util*.lib" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin\bss-util*.pdb" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin32\bss-util*.dll" "..\Packages\bss-util\bin32" /C /I /Y
XCOPY "bin32\bss-util*.lib" "..\Packages\bss-util\bin32" /C /I /Y
XCOPY "bin32\bss-util*.pdb" "..\Packages\bss-util\bin32" /C /I /Y
XCOPY "bin\test.exe" "..\Packages\bss-util\bin" /C /I /Y
XCOPY "bin32\test.exe" "..\Packages\bss-util\bin32" /C /I /Y

Pause