.org @BiosEnd

'get the file to type
pop m

'attempt to open the file
ldimmb b, 7
out m, 66
out b, 65

'wait for 66 to change
Download_OpenWait:
	in b, 66
	cje m, b, @Download_OpenWait

'see if we got a file
xor c, c
dec c
cje b, c, @Download_InvalidFile

'got a valid file, get the file size
ldimm a, @GetFileSizeEnd
wb0 b, a
out a, 66
ldimmb f, 11
out f, 65

xor o, o

Download_WaitForSize:
	in c, 65
	cjne c, o, @Download_WaitForSize

	in c, 66

	'reset the file pointer
	ldimmb e, 4
	add e, a
	wb0 o, e
	out a, 66
	out f, 65

Download_WaitForSize2:
	in f, 65
	cjne f, o, @Download_WaitForSize2

	'size in c, print our message

	ldimm a, @DownloadMsg
	push a
	call @WriteString

	push m
	call @WriteString

	ldimm a, @DownloadMsg2
	push a
	call @WriteString

	push c
	ldimm d, 7
	push d
	push d
	push d
	call @PrintNumber

	ldimm a, @DownloadMsg3
	push a
	call @WriteString

	call @UpdateVideo

	'send it
	out b, 66
	ldimmb g, 1
	out g, 65

	'close the file
	out b, 66
	ldimmb d, 8
	out d, 65

	'return from the program
	ret


Download_InvalidFile:
	ldimm a, @InvalidFileMsg
	push a
	call @WriteString
	ret

InvalidFileMsg:
	db "Invalid file", 0ah, 00h

DownloadMsg:
	db "Sending file ", 00h

DownloadMsg2:
	db ", ", 00h

DownloadMsg3:
	db " bytes", 0ah, 00h

GetFileSizeEnd:
	dw 0, 0, 2

DownloadInfo:
	dw ffffh
	db "Download a specified file", 0

