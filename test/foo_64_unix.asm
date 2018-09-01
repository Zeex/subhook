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
  lea rdi, [rel message]
  %ifdef USE_PLT
    call puts wrt ..plt
  %else
    call puts
  %endif
  pop rbp
  ret
