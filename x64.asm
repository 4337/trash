public main

ldr_load_lib proto 

.data 

.code

option prologue: none                        
option epilogue: none

ldr_load_lib proc
    
	ip: 
	INT 03h
    PUSHFQ
	PUSH RAX
	PUSH RBX
	PUSH RCX
	PUSH RDX
	PUSH RBP
	PUSH RSI
	PUSH RDI
	PUSH R8
	PUSH R9
	PUSH R10
	PUSH R11
	PUSH R12
	PUSH R13
	PUSH R14
	PUSH R15
	PUSH RSP
	
	CALL @f
	dw 01111h      ;//Length
	dw 02222h      ;//MaximumLength
    NOP
    NOP
    NOP
    NOP	;//wyrównanie 
	dq 06666666666666666h  ;//Buffer ptr
	db (80 * 2) dup(090h)  ;//Buffer
	@@: POP R8    ;path
	
    LEA RAX, QWORD PTR[R8 + 010h]
	MOV QWORD PTR[R8 + 08h], RAX 
	
	CALL @f
	db 031h, 033h, 033h, 070h, 031,033h,033h,070h
	@@: POP R10
	
	MOV R10, QWORD PTR[R10]
	
    SUB RSP, 010h
	
    MOV R9, RSP   ;handle
	XOR RCX, RCX  ;path2file
	XOR RDX, RDX  ;flags
	CALL R10

	ADD RSP,010h
	
	POP RSP
	POP R15
	POP R14
	POP R13
	POP R12
	POP R10
	POP R9
	POP R8
	POP RDI
	POP RSI
	POP RBP
	POP RDX
	POP RCX
	POP RBX
	POP RAX
	POPFQ
	JMP ip   ;restore RIP

ldr_load_lib endp 

option prologue:PrologueDef
option epilogue:EpilogueDef

main proc
     call ldr_load_lib
main endp

end
 
