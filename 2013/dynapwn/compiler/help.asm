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

FileLoop:
	ldimm a, @Filename
	ldimmb b, 4
	ldimm f, @ComExt

	'dir is opened, get files
	out a, 66
	out b, 65

FileLoop_Name:
	'm is name length
	in m, 65
	cjne c, m, @FileLoop_Name

	'got a return, see if we got a name
	in m, 66
	cje c, m, @FileLoopDone

	'see if the name ends with .com, if so then open and display info
	mov e, a
	add e, m
	sub e, b

	push e
	push f
	call @strcmp
	pop g

	'if not zero then get another file
	cjne g, c, @FileLoop

	'attempt to open the file
	ldimmb b, 7
	out a, 66
	out b, 65

	'wait for 66 to change
Help_OpenWait:
	'file handle in o
	in o, 66
	cje a, o, @Help_OpenWait

	'see if we got a file
	mov e, c
	dec e
	cje o, e, @FileLoop

'got a valid file, get the file size
	ldimm e, @GetFileSizeEnd
	wb0 o, e
	out e, 66
	ldimmb f, 11
	out f, 65

Help_WaitForSize:
	in e, 65
	cjne e, c, @Help_WaitForSize

	'file size into n
	in n, 66
	ldimm l, 100
	ldimmb f, 2

	'set the file pointer 100 bytes from the end
	ldimm e, @SetFilePos
	wb0 o, e
	add e, f

	'if the file is too short then reset l
	cjb n, l, @ContinueSetLow
	sub n, l
	jmp @ContinueSet

ContinueSetLow:
	mov l, n
	xor n, n
	
ContinueSet:
	wb0 n, e
	sub e, f
	out e, 66
	ldimmb g, 11
	out g, 65

Help_WaitForSize2:
	in e, 65
	cjne e, c, @Help_WaitForSize2

	'read the last l bytes of the file then scan for 0xffff
	ldimm e, @ReadBytes
	add f, f
	wb0 o, e
	add e, f
	wb0 l, e
	sub e, f
	out e, 66
	ldimmb g, 9
	out g, 65

Help_WaitForRead:
	in g, 65
	cjne g, c, @Help_WaitForRead

	'close the file
	out o, 66
	ldimmb f, 8
	out f, 65

	'scan for the 0xffff identifier
	ldimm b, @FileData
	xor f, f
	dec f
	mov d, b
	add d, l
	
Help_FindHelpInfo:
	dec d
	rb0 e, d

	cje e, f, @Help_FoundHelpInfo
	cjne b, d, @Help_FindHelpInfo

	'no luck, set d properly
	ldimm d, @HelpUnknownCmd
	jmp @Help_PrintLine

Help_FoundHelpInfo:
	inc d
	inc d

Help_PrintLine:
	'filename is a .com, remove .com part
	ldimmb f, 4
	mov e, a
	add e, m
	sub e, f
	wb0b c, e

	'write the name
	push a
	call @WriteString

	ldimm g, @Spaces15
	add g, m
	sub g, f
	push g
	call @WriteString

	push d
	call @WriteString

	ldimm g, @Newline
	push g
	call @WriteString
	jmp @FileLoop

FileLoopDone:

	'close the directory
	ldimm a, 5
	out a, 65

	'return
	ret

Spaces15:
	db "               ", 0

ComExt:
	db ".com", 0

Filename:
	db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

GetFileSizeEnd:
	dw 0, 0, 2

SetFilePos:
	dw 0, 0, 0

ReadBytes:
	dw 0, @FileData, 100

HelpUnknownCmd:
	db "Unkown command", 0

HelpInfo:
	dw ffffh
	db "Display a list of known commands", 0

FileData:
