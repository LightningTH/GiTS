.org @UploadEnd

dyna

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
	ldimm b, 0x10
	add a, d
	add e, c
	add g, f
	add i, j
	add k, a
	add m, n
	shr d, b
	push d
	ret

Continue:
	ldimm a, 0xffff
	ldimm b, 0xc190
	ldimm c, 0x9090
	sub a, b
	call @ModifiedFunc

'get the value, write to the video memory and report an update
'then request another upload to 0x0002. 0x0000 indicates success so can't upload there
	pop a
	xor b, b
	wb1 a, b

	ldimmb a, 1
	out a, 64

	inc a
	out a, 66
	out a, 65

WaitForUpload:
	xor c, c
	in b, 66
	cjne c, b, @WaitForUpload

'jump to 0x0002
	jmp a


