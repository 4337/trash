GetK32 proc
assume fs:nothing
       cld
       push fs:[0]
       pop  eax
next_seh:
        xchg eax,esi 
        lodsd
        cmp  eax,0ffffffffh
        jne  next_seh  
        lodsd 
no_good_pe:
          dec eax
          xor ax,ax
          cmp word ptr[eax],05A4Dh
          jne no_good_pe        
        ret
GetK32 endp
