"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.16.27023\bin\HostX64\x64\ml64.exe" /c "Z:\work\Juche client\Stubs\x64\x64.asm" /Fo "Z:\work\Juche client\Stubs\x64.obj"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.16.27023\bin\HostX64\x64\link.exe" /ENTRY:main /SECTION:.text,RWE /SUBSYSTEM:CONSOLE /OUT:x64_stub.exe x64.obj

del x64.obj

pause