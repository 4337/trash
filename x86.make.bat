"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.16.27023\bin\HostX86\x86\ml.exe" /coff /c "Z:\work\Juche client\Stubs\x86\x86.asm" /Fo "Z:\work\Juche client\Stubs\x86\x86.obj"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.16.27023\bin\HostX86\x86\link.exe" /SECTION:.text,RWE /SUBSYSTEM:CONSOLE /OUT:x86_stub.exe x86.obj

del x86.obj

pause