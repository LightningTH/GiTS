.org @BiosEnd

'get the file to type
pop a

'attempt to open the file
ldimmb b, 7
out a, 66
out b, 65

'wait for 66 to change
Type_OpenWait:
	in b, 66
	cje a, b, @Type_OpenWait

'see if we got a file
xor c, c
dec c
cje b, c, @Type_InvalidFile

'got a valid file, get the file size then read in 4k bytes at a time to print
ldimm a, @GetFileSizeEnd
wb0 b, a
out a, 66
ldimmb f, 11
out f, 65

xor o, o

Type_WaitForSize:
	in c, 65
	cjne c, o, @Type_WaitForSize

	in c, 66

	'reset the file pointer
	ldimmb e, 4
	add e, a
	wb0 o, e
	out a, 66
	out f, 65

Type_WaitForSize2:
	in f, 65
	cjne f, o, @Type_WaitForSize2

	'setup the size per loop, our pointer, and store the file descriptor
	ldimm d, 4096
	ldimm e, @ReadDataInfo
	ldimmb f, 9
	ldimm g, @FileData
	wb0 b, e

Type_Loop:
	cjb c, d, @Type_Loop_Finish

	'read the data
	out e, 66
	out f, 65

	'wait for read to complete
Type_Loop_Read:
	in h, 65
	cje f, h, @Type_Loop_Read

	'got data, print it after null terminate
	add g, d
	wb0b o, g
	sub g, d
	push g
	call @WriteString
	call @UpdateVideo
	call @RefreshScreen
	sub c, d
	jmp @Type_Loop

Type_Loop_Finish:
	'read the last bit of data, store number of bytes left to read into the struct
	ldimmb h, 4
	add h, e
	wb0 c, h

	'read the data
	out e, 66
	out f, 65

	'wait for read to complete
Type_Loop_Read2:
	in h, 65
	cje f, h, @Type_Loop_Read2

	'got data, print it
	add g, c
	wb0b o, g
	sub g, c
	push g
	call @WriteString

	'close the file
	out b, 66
	dec f
	out f, 65

	'put in the newline
	ldimm g, @Newline
	push g
	call @WriteString

	'return from the program
	ret


Type_InvalidFile:
	ldimm a, @InvalidFileMsg
	push a
	call @WriteString
	ret

InvalidFileMsg:
	db "Invalid file", 0ah, 00h

GetFileSizeEnd:
	dw 0, 0, 2

ReadDataInfo:
	dw 0, @FileData, 4096

FileData:

TypeInfo:
	dw ffffh
	db "Display the contents of a file", 0

