@ECHO OFF
md "..\Packages\buntils"

:: So, uh, apparently XCOPY deletes empty subdirectories in your destination.
XCOPY "*.cpp" "..\Packages\buntils" /S /C /I /R /Y
XCOPY "*.c" "..\Packages\buntils" /S /C /I /R /Y
XCOPY "test\*.h" "..\Packages\buntils\test" /S /C /I /R /Y
XCOPY "test\*.rc" "..\Packages\buntils\test" /S /C /I /R /Y
XCOPY "test\*.ico" "..\Packages\buntils\test" /S /C /I /R /Y
XCOPY "*.vcxproj" "..\Packages\buntils" /S /C /I /R /Y
XCOPY "*.filters" "..\Packages\buntils" /S /C /I /R /Y
XCOPY "*.sln" "..\Packages\buntils" /S /C /I /R /Y
XCOPY "buntils\*.rc" "..\Packages\buntils\buntils" /C /I /R /Y

md "..\Packages\buntils\include"
md "..\Packages\buntils\bin"
md "..\Packages\buntils\bin32"
md "..\Packages\buntils\test"

XCOPY "*.md" "..\Packages\buntils" /C /I /Y
XCOPY "LICENSE*" "..\Packages\buntils" /C /I /Y
XCOPY "include\*.h" "..\Packages\buntils\include" /S /C /I /R /Y
XCOPY "bin\buntils*.dll" "..\Packages\buntils\bin" /C /I /Y
XCOPY "bin\buntils*.lib" "..\Packages\buntils\bin" /C /I /Y
XCOPY "bin\buntils*.pdb" "..\Packages\buntils\bin" /C /I /Y
XCOPY "bin32\buntils*.dll" "..\Packages\buntils\bin32" /C /I /Y
XCOPY "bin32\buntils*.lib" "..\Packages\buntils\bin32" /C /I /Y
XCOPY "bin32\buntils*.pdb" "..\Packages\buntils\bin32" /C /I /Y
XCOPY "bin\test.exe" "..\Packages\buntils\bin" /C /I /Y
XCOPY "bin32\test.exe" "..\Packages\buntils\bin32" /C /I /Y

Pause