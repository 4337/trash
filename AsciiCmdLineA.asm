COMMENT *
 blabla v 1.0.0 beta
*

.model flat,stdcall
option casemap:none
option prologue: none                        
option epilogue: none

include include\windows.inc
include include\masm32.inc
include include\msvcrt.inc
include include\kernel32.inc
include .\te_asm_mem.inc
include TeAsmAsciiCmdLine.inc

COMMENT * unused
includelib lib\kernel32.lib
includelib lib\msvcrt.lib
*
includelib .\te_asm_mem.lib

COMMENT *
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
|       internal functions declaration        |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+ *

AsmLibCmdArgcCount  proto stdcall iCmd:ptr byte
AsmLibStripQuots    proto stdcall iQuotType:byte
AsmLibInternalAlloc proto stdcall
AsmLibInternalCopy  proto stdcall

.code 

COMMENT *
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
|       internal functions definitions        |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+ *

COMMENT *
+------------------------------------------------+
| AsmLibCmdArgcCount stdcall                     |
| (Function count command line arguments)        |
| IN : iCmd is a pointer to command line string  |
| OUT: EAX = count of cmd line arguments         |
+------------------------------------------------+ *

AsmLibCmdArgcCount proc stdcall private iCmd:ptr byte
 
                   push ecx
                   push ebx                                      ;ebx for iCmd
                   push edx                                      ;edx for "flag"
                   
                   mov  ebx,dword ptr[esp + 010h]                ;ebx = iCmd
                   sub  eax,eax                                  ;initialize eax,edx,ecx = 0
                   cdq  
                   mov  ecx,edx
                 
                   .while byte ptr[ebx] != 00h                   ;while(*iCmd != '\0')
                    .if (byte ptr[ebx] == 022h) && !(dl & 01h)   ;if(*iCmd == ") && (! flag & 1)
                     xor edx,02h                                 ;setup 2 bit in flag
                     .if (dl & 02h)                              ;if( flag & 2) - drugi bit ma wartosc 1 ?
                      xor ecx,ecx       
                     .endif
                    .elseif (byte ptr[ebx] == 027h) && \         ;if(*iCmd == ') && (! flag & 2)
                            !(dl & 02h)
                     xor edx,01h
                     .if (dl & 01h)
                      xor ecx,ecx
                     .endif
                    .elseif byte ptr[ebx] == 020h && \
                            byte ptr[ebx + 01h] != 00h && \
                            byte ptr[ebx + 01h] != 020h
                     .if (dl & 01h) || (dl & 02h)
                      inc ecx
                     .else
                      inc eax
                     .endif
                    .endif
                    inc ebx                                      ;iCmd++
                   .endw
                   
                   add  eax,ecx
                   inc  eax
              
                   pop  edx                   
                   pop  ebx
                   pop  ecx

                   ret 4
AsmLibCmdArgcCount endp

align 4

COMMENT *
+-------------------------------------------------------+
| AsmLibStripQuots   stdcall                            |
| (function strip quot's at begin and end of argument)  |
| IN:  iQuotType is a quot character eg: ' || " || ..   |
| OUT: EAX = argument len                               |
+-------------------------------------------------------+ *

AsmLibStripQuots proc stdcall private iQuotType:byte
     
                 xor  eax,eax                                  ;init and store
                 push ecx
                 push edx
                 push edi

                 mov  ecx,dword ptr[esp + 010h]                ;ecx = iQuotType
                 .if byte ptr[esi] == cl                       ;esi = str-current-ptr if(*str == iQuotType)
                  mov edx,esi                                  ;edx = str-current-ptr copy
                  inc edx
                  .while byte ptr[edx] != 00h                  ;while(*str-curr-ptr-copy != '\0')
                   .if byte ptr[edx] == cl                     ;if(*str-curr-ptr-copy == iQuotType)
                    inc ebx
                    mov esi,edx                                ;skip
                    mov byte ptr[esi],00h                      ;terminate
                    inc esi                                    ;and jmp over null byte (next arg)
                    jmp @f                                     ;break
                   .endif 
                    inc edx
                    inc eax
                   .endw
                 mov eax,-1                                   ;terminate at first occurence ' ' if is only one slash
                .endif
            
                @@: pop edi
                pop edx
                pop ecx

                ret 4
AsmLibStripQuots endp

align 4

COMMENT *
+-----------------------------------------------------+
| AsmLibInternalAlloc stdcll                          |
| (Function allocate memmory for cmd argument)        |
| IN:  [esp + 020h] = argument length                 |
| OUT: EAX = HEAP_PTR || NULL(0) if error             |
+-----------------------------------------------------+ *

AsmLibInternalAlloc proc stdcall private
                
                    push ecx                                      ;malloc change so safe him
                    
                    push dword ptr[esp + 020h]                    ;[esp+020h] = arg_len
                    inc  dword ptr[esp]                           ;1 for null byte
                    call crt_malloc
                    add  esp,4
                  
                    pop  ecx

                    ret
AsmLibInternalAlloc endp

align 4

COMMENT *
+----------------------------------------------------+
| AsmLibInternalCopy stdcall                         |
| (Function copy argument to the heap mem)           |
| IN: [esp+020h] = argument length                   |
|     ebx = begin of argument (first char)           |
|     eax = heap_ptr (dst)                           |
| OUT: eax = memcpy                                  |
+----------------------------------------------------+ *

AsmLibInternalCopy  proc stdcall private
        
                    push ecx                                      ;memcpy destroy
                    
                    push dword ptr[esp + 020h]                    ;[esp+020h] = arg_len
                    push ebx                                      ;ebx = begin of arg
                    push eax                                      ;eax = dst on the heap
                    call crt_memcpy
                    add  esp,0ch

                    mov  ecx,dword ptr[esp + 020h]
                    mov  byte ptr[eax + ecx],00h                  ;terminate (sometimes is not terminate)

                    pop  ecx

                    ret
AsmLibInternalCopy  endp

align 4

COMMENT *
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
|       exported functions definitions        |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+ 

+-----------------------------------------------------------+
| MyAsciiCmdLineToArgv stdcall                              |
| IN : iARgc = ptr to var that hold returned arrayofptr size|
| OUT: EAX = arrayofptr = argv                              |
+-----------------------------------------------------------+ *

MyAsciiCmdLineToArgv proc stdcall public iArgc:ptr byte

                     sub  esp,0ch                                  ;local vars (esp = argc,esp + 4 = argv,esp + 8 = arg_length)
                     
                     push esi                                      ;store regs
                     push ecx
                     push ebx
                     push edi 
                     
                     xor  edi,edi                                  ;init by 0

                     call GetCommandLine
                     test eax,eax 
                     je   @@err                                    ;somethiongs goes wrong
                     
                     mov  esi,eax                                  ;esi = commandline
                     
                     push eax
                     call AsmLibCmdArgcCount                       ;AsmLibCmdArgcCount(commandLine)
                     cmp  eax,1
                     jl   @@err                                    ;if(argc < 1) return 0                     

                     mov  dword ptr[esp + 010h],eax                ;store argc in local var

                     mov  ecx,sizeof dword                         ;ecx = ptr size
                     mul  ecx                                      ;argc * sizeof(PTR*)
                     
                     cmp  eax,dword ptr[esp + 010h]                ;is a int overflow
                     jle  @@err 

                     push eax
                     call crt_malloc                               ;malloc(argc * sizeof(PTR*)
                     add  esp,04h
                     test eax,eax
                     je   @@err                                    ;malloc fail

                     mov  dword ptr[esp + 014h],eax                ;store arrayofptr in local var

                     xor  ecx,ecx                                  ;ecx count correct alloc and copying
                     mov  ebx,esi                                  ;ebx = begin of cmd
                     mov  edi,eax                                  ;edi = arrayofptr ptr
                     
                     mov dword ptr[esp + 018h],0                   ;arg len = 0
                      
                     .while 1
 
                      push 027h
                      call AsmLibStripQuots                        ;strip '
                      add  dword ptr[esp + 018h],eax               ;and store length

                      push 022h                                    ;strip "
                      call AsmLibStripQuots                       
                      add  dword ptr[esp + 018h],eax               ;and store

                      .if byte ptr[esi] == 020h && \
                          byte ptr[esi + 01h] != 020h && \
                          byte ptr[esi + 01h] != 00h
                          
                          call AsmLibInternalAlloc
                          test eax,eax
                          je @@free
 
                          mov dword ptr[edi + ecx * 4],eax        ;store place for argument in arrayOfPtr

                          call AsmLibInternalCopy                 ;copy arg to heap in arrayofptr
                        
                          mov ebx,esi
                          inc ebx
                          inc ecx
                          mov dword ptr[esp + 018h],0            ;arg len = 0 (new arg)
                      
                      .elseif byte ptr[esi] == 00h               ;last arg

                       call AsmLibInternalAlloc
                       test eax,eax
                       je @@free 
                                                
                       mov dword ptr[edi + ecx * 4],eax        ;store place for argument in arrayOfPtr 

                       call AsmLibInternalCopy                 ;copy arg to heap in arrayofptr
                  
                       inc ecx
                       jmp @f                                  ;break
                       
                      .endif

                      inc dword ptr[esp + 018h]                ;arg_len++
                      inc esi                                  ;cmd_ine++

                     .endw

                     @@free: push ecx
                             push edi
                             call TeAsmFreeArrayOfPtr
                             add  esp,08h

                     @@err: xor ecx,ecx
               
                     @@: mov ebx,dword ptr[esp + 020h]         ;ebx = &iARgc
                     mov dword ptr[ebx],ecx 
                  
                     mov eax,edi                               ;eax = arrayofptr argv
                
                     pop edi
                     pop ebx
                     pop ecx
                     pop esi

                     add esp,0ch 
                   
                     ret 4
MyAsciiCmdLineToArgv endp

option prologue:PrologueDef
option epilogue:EpilogueDef

end