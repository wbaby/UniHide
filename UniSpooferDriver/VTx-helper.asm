;x64 ABI is described here https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention

PUBLIC IsVmxSupported
PUBLIC EnableVmx
PUBLIC DisableVmx
PUBLIC IsVmxEnabled

EXTERN VmExitHandler:PROC
EXTERN VmResumeExec:PROC

.code _text

; char IsVmxSupported();
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

; char EnableVmx();
EnableVmx PROC
	xor rax, rax
	mov rax, cr4

	or rax, 2000h ; bit 13
	mov cr4, rax

	mov rax, 1
	ret
EnableVmx ENDP

; char DisableVmx();
DisableVmx PROC
	xor rax, rax
	mov rax, cr4

	xor rax, 2000h ; bit 13
	mov cr4, rax

	mov rax, 1
	ret
DisableVmx ENDP

; char IsVmxEnabled();
IsVmxEnabled PROC
	mov eax, 1
	mov rcx, cr4
	bt rcx, 13
	jc done
	dec eax
done:
	ret
IsVmxEnabled ENDP

; void VmxSaveAndLaunch(DWORD64 rsp, DWORD64 rbp);
VmxSaveAndLaunch PROC
	mov [rcx], rsp	;save rsp
	mov [rdx], rbp	;save rbp

	mov rax, _end
	push rax

	vmlaunch
_end:
	ret
VmxSaveAndLaunch ENDP

; void VmxRestore(DWORD64 rsp, DWORD64 rbp);
VmxRestore PROC
	vmxoff 

	mov rsp, rcx	;restore rsp
	mov rbp, rdx	;restore rbp

	ret
VmxRestore ENDP

; DWORD64 GetGdtBase();
GetGdtBase PROC
	LOCAL GDTR[10]:BYTE
	sgdt GDTR					; save global descriptor table register in local GDTR
	mov rax, qword ptr GDTR[2]	; the first 2 bytes indicate the GDT size
	ret
GetGdtBase ENDP
; DWORD32 GetGdtLimit();
GetGdtLimit PROC
	LOCAL GDTR[10]:BYTE
	sgdt GDTR					; save global descriptor table register in local GDTR
	mov ax, word ptr GDTR[0]	; the first 2 bytes indicate the GDT size
	ret
GetGdtLimit ENDP
; DWORD64 GetIdtBase();
GetIdtBase PROC
	LOCAL IDTR[10]:BYTE
	sidt IDTR					; save interrupt descriptor table in local IDTR
	mov rax, qword ptr IDTR[2]	; the first 2 bytes indicate the GDT size
	ret
GetIdtBase ENDP
; DWORD32 GetIdtLimit();
GetIdtLimit PROC
	LOCAL IDTR[10]:BYTE
	sidt IDTR					; save interrupt descriptor table in local IDTR
	mov ax, word ptr IDTR[0]	; the first 2 bytes indicate the GDT size
	ret
GetIdtLimit ENDP
; USHORT GetCs();
GetCs PROC
	mov rax, cs
	ret
GetCs ENDP
; USHORT GetDs();
GetDs PROC
	mov rax, ds
	ret
GetDs ENDP
; USHORT GetEs();
GetEs PROC
	mov rax, es
	ret
GetEs ENDP
; USHORT GetSs();
GetSs PROC
	mov rax, ss
	ret
GetSs ENDP
; USHORT GetFs();
GetFs PROC
	mov rax, fs
	ret
GetFs ENDP
; USHORT GetGs();
GetGs PROC
	mov rax, gs
	ret
GetGs ENDP
; USHORT GetRflags();
GetRflags PROC
	PUSHFQ
	pop rax
	ret
GetRflags ENDP
; USHORT GetLdtr();
GetLdtr PROC
	sldt rax
	ret
GetLdtr ENDP
; USHORT GetTr();
GetTr PROC
	str rax
	ret
GetTr ENDP

; void VmExitWrapper();
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