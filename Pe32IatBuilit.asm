Pe32IatHnds struct
            _pe32iat_hnd dd 0
            _pe32iat_cnt dd 0
Pe32IatHnds ends

.data 

iHnd Pe32IatHnds <0,0>

.code

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

COMMENT * 
+ Pe32IatBuilit (Re-builit Pe32 Import Table) ofc 
+ its POC code, the real code of this function is neverendig secret :D
+ IN : mem  = ptr to PE Image Memory *mem = 'MZ'
+      hnds = ptr to Pe32IatHnds structure (is needed for free)
+ RETURN : 1 = SUCCESS | 0 = ERRROR
+ AUTHOR : tao 2013-08-02 21:55
*

Pe32IatBuilit proc stdcall mem:ptr dword,hnds:ptr Pe32IatHnds

mov ebx,dword ptr[esp + 04h]
mov edx,ebx
assume ebx:ptr IMAGE_DOS_HEADER
add ebx,dword ptr[ebx].e_lfanew

assume ebx:ptr IMAGE_NT_HEADERS
add edx,dword ptr[ebx].OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT * sizeof(IMAGE_DATA_DIRECTORY)]

.if (dword ptr[ebx].OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT * sizeof(IMAGE_DATA_DIRECTORY) + 04h] > 0)
    
 assume edx:ptr IMAGE_IMPORT_DESCRIPTOR
 xor ecx,ecx
 .while ((edx != 0) && (dword ptr[edx].Name1 != 0))
                
  push edx                                            
  push ecx                                            
 
  mov  ebx,dword ptr[esp + 0ch] 
  add  ebx,dword ptr[edx].Name1
  push ebx
  call LoadLibraryA
  or   eax,eax
  je   @f
             
  push eax
                
  mov  edi,dword ptr[esp + 014h]

  mov  eax,dword ptr[edi + 04h]
  inc  eax
  shl  eax,2

  push eax
  push dword ptr[edi]
  call crt_realloc
  add  esp,08h
  or   eax,eax
  je   @f
             
  mov  dword ptr[edi],eax 
  inc  dword ptr[edi + 04h]

  pop  ebx
  pop  ecx
                
  mov  dword ptr[eax + ecx * 04h],ebx 

  pop  edx

  mov  edi,dword ptr[esp + 04h]
  mov  esi,edi

  .if (dword ptr[edx].OriginalFirstThunk != 0) 
    
   add edi,dword ptr[edx].OriginalFirstThunk       
   add esi,dword ptr[edx].FirstThunk              
  
  .else 
                    
   add edi,dword ptr[edx].FirstThunk               
   mov esi,edi                                     
  
  .endif

  .while dword ptr[edi] != 0
                   
   push eax
   push ecx
   push edx

   .if (dword ptr[edi] &  IMAGE_ORDINAL_FLAG32)

    mov  edi,dword ptr[edi]
    and  edi,0ffffh
    push edi
    push dword ptr[eax + ecx * 04h]
    call GetProcAddress 

   .else

    mov ebx,dword ptr[esp + 010h]
    add ebx,dword ptr[edi]
    assume ebx:ptr IMAGE_IMPORT_BY_NAME
    lea  ebx,dword ptr[ebx].Name1 

    push ebx
    push dword ptr[eax + ecx * 04h]
    call GetProcAddress 

   .endif

   mov dword ptr[esi],eax

   pop edx
   pop ecx
   pop eax

   cmp dword ptr[esi],0
   je  @f
   
   add esi,4
   add edi,4

  .endw

  inc ecx
  add edx,sizeof IMAGE_IMPORT_DESCRIPTOR
 
 .endw

 mov eax,1
 ret 08h

.endif
            
@@: xor eax,eax
ret 08h

Pe32IatBuilit endp

OPTION PROLOGUE:PrologueDef
OPTION EPILOGUE:EpilogueDef