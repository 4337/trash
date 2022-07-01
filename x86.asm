.386
.model flat,stdcall
option casemap:none

option prologue: none                        
option epilogue: none


ldr_load_lib proto stdcall 

.data 

MAGIC_ADDR dd 031333370h
MAGIC_PATH_SIZE dd 0110h * 02h / 04h

.code

ldr_load_lib proc stdcall public
  
    PUSHFD
	PUSHAD
	
	CALL @f
	db 256 * 2 / 4 dup(0bah,0ddh,0beh,0ffh)
	@@: POP edx
	
	CALL @f
	db 031h, 033h, 033h, 070h
	@@: POP ECX
	
	MOV ECX, DWORD PTR[ECX]
	
	PUSH ebx
	PUSH edx
	PUSH 0
	PUSH 0
    CALL ECX
	
	POPAD
	POPFD
	INT 0cch

ldr_load_lib endp 

start:
     call ldr_load_lib
end start

option prologue:PrologueDef
option epilogue:EpilogueDef


