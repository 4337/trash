COMMENT *
+
+  29.07.2013 21:11
+  PE32 Relocation v 1.0
+  USES: ebx,edx,eax,ecx,esi,edi
+  IN: mem   = pe file handle *mem = MZ
+      delta = delta offset (adress of pe file in memory - OptionalHeader.ImageBase))
+  OUT: void
+  Autor: echo
*

IMG_SIZE_BASE_RELOC equ 08h

pe32_relocation proto stdcall mem:ptr dword,delta:dword

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

pe32_relocation proc stdcall public mem:ptr dword,delta:dword

mov  ebx,dword ptr[esp + 04h]
assume ebx:ptr IMAGE_DOS_HEADER
add ebx,dword ptr[ebx].e_lfanew
assume ebx:ptr IMAGE_NT_HEADERS  
               
mov edx,dword ptr[esp + 04h]
add edx,dword ptr[ebx].OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC * sizeof(IMAGE_DATA_DIRECTORY)]
assume edx:ptr IMAGE_BASE_RELOCATION
                
COMMENT * ADD: chck IMAGE_DATA_DIRECTORY.isize ||
               Relocation Table Size > 0  *

.while (dword ptr[edx].VirtualAddress > 0)
               
 mov ebx,dword ptr[esp + 04h]
 add ebx,dword ptr[edx].VirtualAddress
             
 mov eax,dword ptr[edx].SizeOfBlock
 mov ecx,02h                                            
 sub eax,IMG_SIZE_BASE_RELOC
 idiv cx
       
 lea esi,dword ptr[edx + IMG_SIZE_BASE_RELOC];           

 xor ecx,ecx

 .while (ecx < eax)

  movzx edi,word ptr[esi]                               
  mov eax,edi
  shr eax,0ch               

  .if (eax == IMAGE_REL_BASED_ABSOLUTE)     
           
   ;                 

  .elseif (eax == IMAGE_REL_BASED_HIGHLOW)

   and edi,0fffh
   mov  eax,dword ptr[esp + 08h]
   add dword ptr[ebx + edi],eax                         ;<- sems to be ok but can be fuckup - chck short

  .endif

  add esi,sizeof word
  inc ecx

 .endw
            
 add edx,dword ptr[edx].SizeOfBlock

.endw

ret 08h

pe32_relocation endp

OPTION PROLOGUE:PrologueDef
OPTION EPILOGUE:EpilogueDef