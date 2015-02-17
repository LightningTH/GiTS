.org @BiosEnd

'get rid of the stack push
pop a

'get a directory listing
ldimmb a, 3
out a, 65

xor c, c

WaitForDirOpen:
	in d, 65
	cjne c, d, @WaitForDirOpen

'print the header
ldimmb a, 5
call @CmdAdvanceBuffer

ldimm a, @HeaderStr
call @WriteString

ldimm a, @FileData
ldimmb b, 4
ldimmb e, 6

xor o, o

FileLoop:
	'dir is opened, get files
	out a, 66
	out b, 65

FileLoop_Name:
	in d, 65
	cjne c, d, @FileLoop_Name

	'got a return, see if we got a name
	in d, 66
	cje c, d, @FileLoopDone

	'get the file stats
	out a, 66
	out e, 65

FileLoop_Stats:
	in d, 65
	cjne c, d, @FileLoop_Stats

	'if directory, ignore
	ldimm j, @FileIsDir
	rb0b j, j
	cjne j, c, @FileLoop

	'file count
	inc o

	'file size
	ldimm l, @FileSize
	rb0 l, l
	ldimm f, @FileSizeData
	rb0 m, f
	add m, l
	cja m, l, @FileLoop_StoreSizeLow
	cje m, l, @FileLoop_StoreSizeLow

	'increase the high word
	ldimm g, @FileSizeDataHigh
	rb0 d, g
	inc d
	wb0 d, g

FileLoop_StoreSizeLow:
	wb0 m, f

	'got the stats, print them out
	ldimmb d, 2
	ldimm f, 0x30
	ldimm j, @FileMonth
	ldimm h, @Slash
	ldimm i, @LineReturn

	'print month
	rb0b g, j
	push g
	push d
	push f
	push c
	call @PrintNumber
	pop k

	'slash
	push h
	call @WriteString

	'print day
	inc j
	rb0b g, j
	push g
	push d
	push f
	push c
	call @PrintNumber
	pop k

	'slash
	push h
	call @WriteString

	'print year
	dec j
	sub j, d
	add d, d
	rb0 g, j
	push g
	push d
	push f
	push c
	call @PrintNumber
	pop k

	'spaces
	ldimm k, @Spaces
	push k
	call @WriteString

	'print hour
	add j, d
	ldimmb d, 2
	rb0b g, j
	ldimmb k, 12
	cja g, k, @HourPM

	ldimm l, @AM
	jmp @PrintHour

HourPM:
	ldimm l, @PM
	sub g, k

PrintHour:
	push g
	push d
	push f
	push c
	call @PrintNumber
	pop k

	'spaces
	ldimm k, @Colon
	push k
	call @WriteString

	'print minute
	inc j
	rb0b g, j
	push g
	push d
	push f
	push c
	call @PrintNumber
	pop k

	'AM/PM
	push l
	call @WriteString

	ldimm l, @Spaces13
	push l
	call @WriteString

	ldimm f, @TempNumBuf
	ldimm d, 0x20
	ldimmb g, 10
	push f
	push d
	push g
	call @memset

	ldimm f, @FileSize
	rb0 f, f
	push f
	ldimmb g, 5
	push g
	push d
	push c
	call @PrintNumber
	pop l

	ldimm l, @OneSpace
	push l
	call @WriteString

	ldimm l, @Filename
	ldimmb f, 12
	xor m, m
	add l, f
	wb0b m, l
	sub l, f
	push l
	call @WriteString

	'make note of file count

	'line return
	push i
	call @WriteString

	jmp @FileLoop

FileLoopDone:

'close the directory
ldimm a, 5
out a, 65

ldimmb a, 2
call @CmdAdvanceBuffer

'do the footer
ldimm a, @Spaces13
push a
call @WriteString

'file count
ldimm f, 0x20
ldimmb a, 3
push o
push a
push f
push c
call @PrintNumber
pop a

ldimm a, @FileStr
push a
call @WriteString

ldimm a, @FileSizeData
ldimm b, @FileSizeDataHigh
rb0 a, a
rb0 b, b
push b
push a
ldimmb a, 10
push a
ldimm a, 20h
push a
call @PrintLargeNumber

ldimm a, @FooterStr
push a
call @WriteString

'return
ret

PrintLargeNumber:
	pop d
	pop c
	pop b
	pop a

	'a is high word
	'b is low word
	'c is maximum size
	'd is padding byte
	xor e, e

PrintLargeNumber_Loop:
	'f = rem
	'g = quotient
	'h = dividend high
	'i = dividend low
	'j = divisor
	'k = counter
	'l = 1
	'm = 31
	'n = scratch

	mov h, a
	mov i, b
	xor f, f
	xor g, g
	ldimmb j, 10
	ldimm k, 32
	ldimmb l, 1
	ldimm m, 15

PrintLargeNumber_Loop32:
	'rem = rem << 1 | (high bit of dividend)
	'dividend <<= 1
	shl f, l
	mov n, h
	shr n, m
	or f, n
	shl h, l
	mov n, i
	shr n, m
	or h, n
	shl i, l

	'quotient <<= 1
	shl g, l

	'if(rem > divisor)
	'	q |= 1
	'	rem -= divisor
	'else
	'	q |= 0
	cjb f, j, @PrintLargeNumber_DontAdjustRemainder

	sub f, j
	or g, l

PrintLargeNumber_DontAdjustRemainder:
	dec k
	cjne k, e, @PrintLargeNumber_Loop32

	'instead of looping for the full number, we shouldn't
	'have that large of a number so just print the two parts

	'print the number
	dec c
	push g
	push c
	push d
	push e
	call @PrintNumber
	pop o

	'print the last digit
	ldimmb c, 1
	push f
	push c
	push d
	push e
	call @PrintNumber
	pop o

	ret

Slash:
	db "/", 00h

Spaces:
	db "  ", 00h

OneSpace:
	db " ", 00h

Colon:
	db ":", 00h

AM:
	db " AM", 00h

PM:
	db " PM", 00h

Spaces13:
	db "             ", 00h

FileCount:
	dw 0

FileSizeDataHigh:
	dw 0

FileSizeData:
	dw 0

FileData:
Filename:
	db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

FileSize:
	dw 0

FileYear:
	dw 0

FileMonth:
	db 0

FileDay:
	db 0

FileHour:
	db 0

FileMin:
	db 0

FileIsDir:
	db 0

HeaderStr:
	db " Volume in drive C is unknown", 0ah
	db " Volume Serial Number is 1337-7331", 0ah, 0ah
	db " Directory of C:\", 0ah, 0ah, 00h

FileStr:
	db " File(s)     ", 0

FooterStr:
	db " bytes", 0ah
	db "               0 Dir(s)               0 bytes free", 0ah, 00

DirInfo:
	dw ffffh
	db "Display a directory listing", 0
