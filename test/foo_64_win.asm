bits 64

extern puts
global foo

section .data

message:
  db 'foo() called', 0

section .text

foo:
  nop
  push rbp
  mov rbp, rsp
  sub rsp, 32
  mov rcx, qword message
  call puts
  add rsp, 32
  pop rbp
  ret
