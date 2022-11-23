PUBLIC IsVmxSupported
PUBLIC EnableVmx
PUBLIC DisableVmx
PUBLIC IsVmxEnabled

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

END