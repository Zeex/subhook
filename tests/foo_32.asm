extern puts
global foo

section .rodata

message:
  db 'foo() called', 0

section .text

;; Long nop macros for nasm/yasm borrowed from nasm-utils:
;; https://github.com/travisdowns/nasm-utils
%define nop1 nop                                                     ; just a nop, included for completeness
%define nop2 db 0x66, 0x90                                           ; 66 NOP
%define nop3 db 0x0F, 0x1F, 0x00                                     ;    NOP DWORD ptr [EAX]
%define nop4 db 0x0F, 0x1F, 0x40, 0x00                               ;    NOP DWORD ptr [EAX + 00H]
%define nop5 db 0x0F, 0x1F, 0x44, 0x00, 0x00                         ;    NOP DWORD ptr [EAX + EAX*1 + 00H]
%define nop6 db 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00                   ; 66 NOP DWORD ptr [EAX + EAX*1 + 00H]
%define nop7 db 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00             ;    NOP DWORD ptr [EAX + 00000000H]
%define nop8 db 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00       ;    NOP DWORD ptr [EAX + EAX*1 + 00000000H]
%define nop9 db 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 ; 66 NOP DWORD ptr [EAX + EAX*1 + 00000000H]

foo:
  push ebp
  mov ebp, esp
  sub esp, 4 ; align the stack to a 16-byte boundary
  push message
  call puts
  mov esp, ebp
  pop ebp
  ret
