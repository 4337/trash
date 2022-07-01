.386
.model flat,stdcall
option casemap:none

option prologue: none                        
option epilogue: none


ldr_load_lib proto stdcall 

.data 

MAGIC_ADDR dd 031333370h
MAGIC_PATH_SIZE dd (0100h * 02h / 04h) - 04h

.code

ldr_load_lib proc stdcall public
  
    PUSHFD
	PUSHAD
	
	CALL @f
	dw 01111h      ;//length
	dw 02222h      ;//length
	dd 066666666h  ;//Buffer ptr
	db (256 * 2 / 4) - 4 dup(0bah,0ddh,0beh,0ffh)  ;//Buffer
	@@: POP EDX
	
	LEA EAX, DWORD PTR[EDX + 8]
	MOV DWORD PTR[EDX + 4], EAX 
	
	CALL @f
	db 031h, 033h, 033h, 070h
	@@: POP ECX
	
	MOV ECX, DWORD PTR[ECX]
	
    SUB ESP, 08h
    
	PUSH ESP   ;PHANDLE
	PUSH EDX   ;ModuleFileName
	PUSH 0     ;Flags
	PUSH 0     ;PathToFile
    CALL ECX
	
	ADD ESP,08h
	
	POPAD
	POPFD
	INT 0cch

ldr_load_lib endp 

start:
     call ldr_load_lib
end start

option prologue:PrologueDef
option epilogue:EpilogueDef


