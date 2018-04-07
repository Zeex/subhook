extern puts
global foo

section .text

message:
  db 'foo() called', 0

foo:
  nop
  push message
  call puts
  add esp, 4
  ret
