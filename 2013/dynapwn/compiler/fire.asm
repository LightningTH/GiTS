.org @BiosEnd

pop a

'based off of an old dos fire demo
'instead of 320x100 we are doing 640x90

call @Fire_BlankVideo

'blank out the flame buffer area
xor a, a
out a, 61
ldimm o, @flames
ldimm b, 57600
ldimmb c, 2

Fire_BlankFireBuffer_Loop:
	wb0 a, o
	add o, c
	sub b, c
	cjne a, b, @Fire_BlankFireBuffer_Loop

	ldimm n, 80
	ldimm o, 640

	'spots
	xor a, a
do_fire:
	ldimm a, 500

	ldimm b, @randseed
	ldimm d, 62e9h
	ldimm e, 62h
	ldimm f, 3619h
	ldimm i, ffh
	ldimm l, @flames
	ldimm j, 50560
	add l, j

	rb0 j, b
	add b, c
	rb0 k, b

put_spots:
	add j, d
	mov g, j
	and g, i
	add j, e
	mov h, g
	add g, e
	and g, i
	add k, f

	cja g, h, @put_spots_noinc

	inc k

put_spots_noinc:
	mov g, k
	mod g, o

	mov h, l
	add h, g
	rb0b g, h
	dec g
	wb0b g, h
	dec a
	cjne a, m, @put_spots

	'save the random values
	wb0 k, b
	sub b, c
	wb0 j, b

	'a = flames + 1 + flw
	'b = screen + 1
	'c = 2
	'd = flh - 2
	'e = flw - 2
	'm = 0
	'n = flh
	'o = flw
	ldimm a, @flames
	add a, o
	inc a
	ldimm b, 12801
	ldimm d, 78
	xor m, m

avg_col:
	ldimm e, 638

avg_row:
	mov f, a
	sub f, o
	rb0b f, f

	dec a
	rb0b g, a
	add f, g

	add a, c
	rb0b g, a
	add f, g

	mov g, a
	add g, o
	dec g
	rb0b g, g
	add f, g

	shr f, c
	wb2b f, b

	inc b
	dec e
	cjne e, m, @avg_row

	add a, c
	add b, c
	dec d
	cjne d, m, @avg_col

	'copy screen to flame buffer
	ldimm a, @flames
	add a, c
	ldimm b, 12802
	ldimm d, 12799

flames_copy_loop:
	rb2 e, b
	wb0 e, a
	add a, c
	add b, c
	rb2 e, b
	wb0 e, a
	add a, c
	add b, c
	dec d
	cjne d, m, @flames_copy_loop

	'refresh the screen
	ldimmb a, 1
	out a, 64

	'test for key press
	in a, 61
	out m, 61

	cje a, m, @do_fire

	'reset the screen buffer before returning
	ldimm a, @ScreenBuffer
	ldimm b, @ScreenBufferPtr
	wb0 a, b
	ldimm b, @ScreenBufferEnd
	xor c, c
	ldimm d, @ScreenBufferUpdatePos
	wb0 c, d
	ldimmb d, 2

ResetScreenBuffer_Loop:
	wb0 c, a
	add a, d
	cjb a, b, @ResetScreenBuffer_Loop

	call @Fire_BlankVideo
	ret

Fire_BlankVideo:
	xor a, a
	xor b, b
	ldimmb c, 2

Fire_BlankVideo_Loop:
	wb1 a, b
	wb2 a, b
	add b, c
	cjne a, b, @Fire_BlankVideo_Loop

	ret

randseed:
	dw 0, 0
flames:

flamesinfo:
	dw ffffh
	db "Display fire on the screen", 0

