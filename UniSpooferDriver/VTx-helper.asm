PUBLIC IsVmxSupported
PUBLIC EnableVmx
PUBLIC DisableVmx
PUBLIC IsVmxEnabled

EXTERN VmExitHandler:PROC
EXTERN VmResumeExec:PROC

.code _text

; char __fastcall IsVmxSupported();
IsVmxSupported PROC
	mov rax, 1
	cpuid
	mov rax, 1
	bt ecx, 5 ; test bit 5 for VMX support https://www.felixcloutier.com/x86/cpuid#fig-3-7
	jc done
	dec rax
done:
	ret
IsVmxSupported ENDP

; char __fastcall EnableVmx();
EnableVmx PROC
	xor rax, rax
	mov rax, cr4

	or rax, 2000h ; bit 13
	mov cr4, rax

	mov rax, 1
	ret
EnableVmx ENDP

; char __fastcall DisableVmx();
DisableVmx PROC
	xor rax, rax
	mov rax, cr4

	xor rax, 2000h ; bit 13
	mov cr4, rax

	mov rax, 1
	ret
DisableVmx ENDP

; char __fastcall IsVmxEnabled();
IsVmxEnabled PROC
	mov eax, 1
	mov rcx, cr4
	bt rcx, 13
	jc done
	dec eax
done:
	ret
IsVmxEnabled ENDP

; void __fastcall SaveVmxState(DWORD64 rsp, DWORD64 rbp);
SaveVmxState PROC
	xor eax, eax
	mov [rcx], rsp	;save rsp
	mov [rdx], rbp	;save rbp
	ret
SaveVmxState ENDP

; void __fastcall VmxRestore(DWORD64 rsp, DWORD64 rbp);
VmxRestore PROC
	vmxoff 

	mov rsp, rcx	;restore rsp
	mov rbp, rdx	;restore rbp

	add rsp, 8		;align stack pointer

	mov rax, 1		;ret value

	ret
VmxRestore ENDP

; DWORD64 __fastcall GetGdtBase();
GetGdtBase PROC
	LOCAL GDTR[10]:BYTE
	sgdt GDTR					; save global descriptor table register in local GDTR
	mov rax, qword ptr GDTR[2]	; the first 2 bytes indicate the GDT size
	ret
GetGdtBase ENDP
; DWORD32 __fastcall GetGdtLimit();
GetGdtLimit PROC
	LOCAL GDTR[10]:BYTE
	sgdt GDTR					; save global descriptor table register in local GDTR
	mov ax, word ptr GDTR[0]	; the first 2 bytes indicate the GDT size
	ret
GetGdtLimit ENDP
; DWORD64 __fastcall GetIdtBase();
GetIdtBase PROC
	LOCAL IDTR[10]:BYTE
	sidt IDTR					; save interrupt descriptor table in local IDTR
	mov rax, qword ptr IDTR[2]	; the first 2 bytes indicate the GDT size
	ret
GetIdtBase ENDP
; DWORD32 __fastcall GetIdtLimit();
GetIdtLimit PROC
	LOCAL IDTR[10]:BYTE
	sidt IDTR					; save interrupt descriptor table in local IDTR
	mov ax, word ptr IDTR[0]	; the first 2 bytes indicate the GDT size
	ret
GetIdtLimit ENDP
; USHORT __fastcall GetCs();
GetCs PROC
	mov rax, cs
	ret
GetCs ENDP
; USHORT __fastcall GetDs();
GetDs PROC
	mov rax, ds
	ret
GetDs ENDP
; USHORT __fastcall GetEs();
GetEs PROC
	mov rax, es
	ret
GetEs ENDP
; USHORT __fastcall GetSs();
GetSs PROC
	mov rax, ss
	ret
GetSs ENDP
; USHORT __fastcall GetFs();
GetFs PROC
	mov rax, fs
	ret
GetFs ENDP
; USHORT __fastcall GetGs();
GetGs PROC
	mov rax, gs
	ret
GetGs ENDP
; USHORT __fastcall GetRflags();
GetRflags PROC
	PUSHFQ
	pop rax
	ret
GetRflags ENDP
; USHORT __fastcall GetLdtr();
GetLdtr PROC
	sldt rax
	ret
GetLdtr ENDP
; USHORT __fastcall GetTr();
GetTr PROC
	str rax
	ret
GetTr ENDP

; void __fastcall VmExitWrapper();
VmExitWrapper PROC
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rbp
	push rbp
	push rbx
	push rdx
	push rcx
	push rax

	mov rcx, rsp
	sub rsp, 28h		; make space on the stack

	call VmExitHandler
	add rsp, 28h		; restore rsp

	pop rax
	pop rcx
	pop rdx
	pop rbx
	pop rbp
	pop rbp
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15

	sub rsp, 100h ; to avoid error in future functions (?)

	jmp VmResumeExec
VmExitWrapper ENDP
END