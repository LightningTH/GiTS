.org 2h

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @DummyFunc
call @DummyFunc
call @DummyFunc
call @DummyFunc

call @ModifiedFunc
jmp @Continue

DummyFunc:
ret

ModifiedFunc:
	add a, a
	or a, a
	or a, a
	or a, a
	or a, a
	or a, c
	or e, d
	inc a
	inc a
	inc a
	or f, b
	push f
	ret

Continue:
	ldimm a, 0xffff
	ldimm b, 0x8d90
	ldimm c, 0x9090
	sub a, b
	call @ModifiedFunc

'get the value, write to the video memory and report an update
'then request another upload. 0x0000 indicates success so can't upload there
	pop a
	xor b, b
	wb1 a, b

	ldimmb a, 1
	out a, 64

	ldimm a, @Stage2End
	out a, 66
	ldimmb c, 2
	out c, 65

WaitForUpload:
	xor c, c
	in b, 66
	cjne c, b, @WaitForUpload

'jump to End
	jmp a

Stage2End:
