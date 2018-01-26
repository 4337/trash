pe32align proc near stdcall uses ebx edx \      
                            s_size:DWORD,\
                            fm_align:DWORD
  xor edx,edx
  mov eax,s_size
  mov ebx,fm_align
  div ebx
  inc eax
  mov ebx,fm_align
  mul ebx 
  ret 08h
pe32align endp