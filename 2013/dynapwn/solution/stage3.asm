.org @Stage2End

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3
call @DummyFunc3

call @Dummy2
call @PushData
jmp @Continue3

Dummy2:
	ldimm a, 207
	ldimm b, 4096
	xor c, c
	xor d, d
	add c, a
	add d, b
	ret

PushData:
	ldimm a, 0x5b61
	ldimm b, 0xb70f
	ldimm c, 0x448b
	ldimm d, 0x2233
	ldimm e, 0x6811
	ldimm f, 0x1000
	ldimm g, 0x0000
	ldimm h, 0x4468
	ldimm i, 0x2233
	ldimm j, 0x5111
	ldimm k, 0xc381
	ldimm l, 0x7788
	ldimm m, 0x5566
	ldimm n, 0xd3ff
	ldimm o, 0xaabb
	call @ModifiedFunc3
	ret

ModifiedFunc3:
	mov o, o
	push a
	pop a
	push b
	pop b
	push c
	pop c
	push e
	mov a, a
	mov b, b
	mov c, c
	inc c
	inc c
	dec c
	dec c
	inc o

	call @DummyFunc3

DummyFunc3:
ret

Continue3:
	ldimm a, 0xffff
	ldimm b, 0x358b
	ldimm c, 0xcccc
	sub a, b
	call @PushData
	call @Dummy2
