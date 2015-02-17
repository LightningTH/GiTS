'Boot app for the cpu
'logo is 16x24

ldimm a, @ScreenBufferPtr
ldimm b, @ScreenBuffer
wb0 b, a

ldimm c, @BootString
push c
call @WriteString

call @UpdateVideo

'x, y, logo ptr
call @WriteLogo

'go get the password
ldimmb o, 1
call @MainLoop
pop a

push a
call @rstrip

ldimm b, @BootPassword
push a
push b
call @strcmp
pop a

xor b, b
cje a, b, @ContinueBoot

'halt due to invalid password
halt

ContinueBoot:
	ldimm c, @BootStringStarting
	push c
	call @WriteString

	'write out the prompt
	ldimm d, @Prompt
	push d
	call @WriteString

	call @UpdateVideo
	xor o, o

MainLoop:
	xor n, n
	xor a, a
	out a, 60

	'update cursor
	call @BlinkCursor

	'signal screen refresh
	call @RefreshScreen

MainLoop2:
	frz
	in c, 61
	out a, 61
	cjne c, a, @Main_CharTyped

	in b, 60
	cje a, b, @MainLoop2

	inc n
	ldimm m, 50
	cjne n, m, @MainLoop2
	jmp @MainLoop

Main_CharTyped:
	ldimm d, @TempChar
	wb0 c, d
	push d
	call @WriteString

	ldimm d, 0x0d
	cje c, d, @Main_HandleCommand

Main_ContinueAfterCmd:
	call @UpdateVideo
	jmp @MainLoop

Main_HandleCommand:
	'make sure the screen is updated as we'll be modifying the buffer possibly during the rstrip
	call @UpdateVideo

	xor a, a
	ldimm d, @Cmds

	'set e to the start of the command
	ldimm e, @ScreenBufferPtr
	rb0 e, e

	'see if we are in the boot mode
	cje o, a, @Main_HandleCommand_Normal

	'in boot mode, finalise the e setup then push it and return
	ldimm f, 65
	sub e, f
	push e
	ret

Main_HandleCommand_Normal:
	ldimm f, 76
	sub e, f

	'copy the command into our buffer then start work on it
	ldimm f, @CmdBuffer
	ldimm g, 32
	push f
	push a
	push g
	call @memset

	push f
	push e
	push g
	call @memcpy

	'null terminate
	ldimm e, @CmdBuffer
	ldimm f, 31
	add f, e
	wb0b a, f

	'strip the command
	push e
	call @rstrip

	ldimmb f, 2

Main_HandleCommand_Loop:
	rb0 b, d
	add d, f
	add d, f

	'if the end of the list then be done
	cje a, b, @Main_Handle_CmdUnknown

	'check for a space, if found, null it
	mov g, e
	dec g
	ldimm h, 0x20

Main_HandleCommand_SpaceCheck:
	inc g
	rb0b i, g

	'if null then done
	cje a, i, @Main_Handle_Command_DoneSpace
	cjne i, h, @Main_HandleCommand_SpaceCheck

	'got a space, null and continue
	wb0b a, g

Main_Handle_Command_DoneSpace:
	
	'see if it matches
	push b
	push e
	call @strcmp

	pop b

	cjne a, b, @Main_HandleCommand_Loop

	'command matches, process it
	sub d, f
	rb0 d, d

	call d

Main_FinishCmd:
	ldimm d, @Prompt
	push d
	call @WriteString
	jmp @Main_ContinueAfterCmd

Main_Handle_CmdUnknown:
	call @CmdUnknown
	jmp @Main_FinishCmd

CmdDirStr:
	db "dir", 00h

CmdTypeStr:
	db "type", 00h

CmdHelpStr:
	db "help", 00h

CmdExitStr:
	db "exit", 00h

CmdDownloadStr:
	db "download", 00h

CmdUploadStr:
	db "upload", 00h

'our hidden mode to swap to dynarec
CmdDynaStr:
	db "dyna", 00h

Cmds:
	dw @CmdDirStr, @CmdDir
	dw @CmdHelpStr, @CmdHelp
	dw @CmdTypeStr, @CmdType
	dw @CmdExitStr, @CmdExit
	dw @CmdDownloadStr, @CmdDownload
	dw @CmdUploadStr, @CmdUpload
	dw @CmdDynaStr, @CmdDyna
	dw 0000h

CmdType:
	'setup f to point to the first param after the command
	ldimmb f, 4
	add f, e

	'restore the space between the command and param
	ldimm a, 0x20
	wb0b a, f
	inc f

	ldimmb a, 2

	ldimm b, @FileList
	xor d, d

CmdType_Loop:
	rb0 c, b
	add b, a

	rb0 g, b
	add b, a

	'if end of list then error about unknown file
	cje d, c, @CmdTypeUnknownFile

	'see if the file matches
	push f
	push c
	call @strcmp

	pop c
	cjne c, d, @CmdType_Loop

	'found the file, print it's data
	push g
	call @WriteString

	ldimmb g, 0x0d
	ldimm a, @TempChar
	wb0 g, a
	jmp @CmdPrintData

CmdTypeUnknownFile:
	ldimm a, @CmdTypeFilesUnknown
	jmp @CmdPrintData
	ret

CmdHelp:
	ldimmb a, 7
	call @CmdAdvanceBuffer

	ldimm a, @CmdHelpLines
	jmp @CmdPrintData
	ret

CmdExit:
	halt

CmdDir:
	ldimmb a, 11
	call @CmdAdvanceBuffer

	ldimm a, @CmdDirLines
	jmp @CmdPrintData

CmdUnknown:
	ldimmb a, 2
	call @CmdAdvanceBuffer

	ldimm a, @CmdUnknownLines
	jmp @CmdPrintData

CmdDownload:
	'setup f to point to the first param after the command
	ldimmb f, 8
	add f, e

	'restore the space between the command and param
	ldimm a, 0x20
	wb0b a, f
	inc f

	ldimm b, @FileList
	xor d, d

	ldimm a, 4

CmdDownload_Loop:
	rb0 c, b
	add b, a

	'if end of list then error about unknown file
	cje d, c, @CmdTypeUnknownFile

	'see if the file matches
	push f
	push c
	call @strcmp

	pop c
	cjne c, d, @CmdDownload_Loop

	'found the file, print that we are sending it
	ldimm a, @CmdDownloadMsg
	push a
	call @WriteString

	push f
	call @WriteString

	ldimm a, @CmdDownloadMsg_NewLine
	push a
	call @WriteString

	out f, 66
	ldimmb g, 1
	out g, 65

	ldimmb g, 0x0d
	ldimm a, @TempChar
	wb0 g, a
	jmp @CmdPrintData

CmdDownloadMsg:
	db "Sending file ", 00h

CmdDownloadMsg_NewLine:
	db 0ah, 00h

CmdUpload:
	'Indicate we are waiting
	ldimm a, @CmdUploadMsg
	push a
	call @WriteString

	'show were to upload to and request update
	ldimm e, @BiosEnd
	out e, 66
	ldimmb b, 2
	out b, 65

	'wait around up to 5 seconds for input
	xor b, b
	xor c, c
	ldimm d, 499

	call @UpdateVideo
	call @RefreshScreen

CmdUpload_Loop:
	frz

	'if key press, abort
	in a, 61
	out b, 61
	cjne a, b, @CmdUpload_Abort

	'if we have upload indication then handle it
	in a, 66
	cje a, b, @CmdUpload_Execute

	'check our timer
	in a, 60
	out b, 60
	cje a, b, @CmdUpload_Loop

	'timer kicked off
	inc c
	cja c, d, @CmdUpload_Timeout
	jmp @CmdUpload_Loop

CmdUpload_Execute:
	ldimm a, @CmdUploadMsgDone
	push a
	call @WriteString
	call @UpdateVideo
	call @RefreshScreen

	'execute the provided code
	call e

	ldimm a, @CmdUploadMsgExecuteDone
	jmp @CmdUpload_Loop

CmdUpload_Timeout:
	ldimm a, @CmdUploadMsg_Timeout
	jmp @CmdPrintData

CmdUpload_Abort:
	ldimm a, @CmdUploadMsg_Abort
	jmp @CmdPrintData

CmdUploadMsg:
	db "Waiting on file to execute (5 second timeout)...", 0ah, 00h

CmdUploadMsg_Timeout:
	db "Timeout while waiting", 0ah, 00h

CmdUploadMsg_Abort:
	db "Upload aborted", 0ah, 00h

CmdUploadMsgDone:
	db "Executing file", 0ah, 00h

CmdUploadMsgExecuteDone:
	db "Finished file execution", 0ah, 00h

CmdDyna:
	ldimmb a, 4
	call @CmdAdvanceBuffer

	ldimm a, @CmdDynaMsg
	push a
	call @WriteString
	dyna

	ldimm a, @CmdDynaMsgDone
	jmp @CmdPrintData

CmdDynaMsg:
	db "Enabling Dynarec mode", 0ah, "WARNING! System may become unstable!", 0ah, 00h

CmdDynaMsgDone:
	db "Now running in dynarec mode", 0ah, 00h

CmdPrintData:
	push a
	call @WriteString
	ret

CmdAdvanceBuffer:
	ldimm b, @ScreenBufferPtr
	rb0 c, b
	'pop a
	push c
	push a
	call @AdvanceBuffer

	pop a
	wb0 a, b
	ret

FileList:
	dw @FileListKey, @KeyFileData
	dw @FileListApp, 0
	dw @FileListBios, 0
	dw 0

FileListKey:
	db "key", 00h

FileListApp:
	db "app.exe", 00h

FileListBios:
	db "bios.bin", 00h

CmdTypeFilesUnknown:
	db "Invalid file", 0ah, 00h

CmdUnknownLines:
	db "Unknown command", 0ah, 00h

CmdHelpLines:
	db "DIR            Display a directory listing", 0ah
	db "DOWNLOAD       Download a specified file", 0ah
	db "EXIT           Close the terminal session", 0ah
	db "HELP           Provides a list of known commands", 0ah
	db "TYPE           Display the contents of a text file", 0ah
        db "UPLOAD         Upload and execute a file", 0ah, 00h

CmdDirLines:
	db " Volume in drive C is unknown", 0ah
	db " Volume Serial Number is 1337-7331", 0ah, 0ah
	db " Directory of C:\", 0ah, 0ah
	db "09/10/2012  11:04 PM                39 KEY", 0ah
	db "09/10/2012  11:10 PM             32134 APP.EXE", 0ah
	db "09/10/2012  11:10 PM              5084 BIOS.BIN", 0ah
	db "               2 File(s)          37257 bytes", 0ah
	db "               0 Dir(s)               0 bytes free", 0ah, 00

KeyFileData:
	db "This is not the key you are looking for", 00

rstrip:
	pop a

	push a
	call @strlen

	pop b

	'a is the string
	'b is the length

	'remove all spaces from the right side
	add a, b
	ldimm c, 0x20
	xor d, d
	dec a

rstrip_loop:
	rb0b f, a

	cjne f, c, @rstrip_done
	wb0b d, a
	dec a

	jmp @rstrip_loop

rstrip_done:
	ret

memcpy:
	pop c
	pop b
	pop a

	xor e, e

memcpy_loop:
	rb0b d, b
	wb0b d, a

	dec c
	inc b
	inc a
	cjne c, e, @memcpy_loop

memcpy_done:
	ret

memset:
	pop c
	pop b
	pop a

	xor e, e

memset_loop:
	wb0b b, a

	dec c
	inc a
	cjne c, e, @memset_loop

memset_done:
	ret

strlen:
	pop a
	mov b, a
	xor c, c

strlen_loop:
	rb0b e, a
	inc a

	cjne e, c, @strlen_loop

	sub a, b
	dec a
	push a
	ret

strcmp:
	pop a
	pop b

	xor c, c

strcmp_loop:
	rb0b e, a
	rb0b f, b

	inc a
	inc b
	cjne e, f, @strcmp_nomatch

	'see if we found a null byte indicating done
	cje e, c, @strcmp_done
	jmp @strcmp_loop

strcmp_nomatch:
	inc c

strcmp_done:
	push c
	ret

UpdateVideo:
	'take the buffer and write it to the video area
	ldimm a, @ScreenBuffer
	ldimm b, @ScreenBufferEnd

	ldimm c, 80
	ldimmb d, 8
	ldimm e, 0xff
	xor f, f

	'screen y coord
	ldimm o, @ScreenBufferUpdatePos
	rb0 n, o

	'adjust a to the last line to write then get our value again	
	mul n, c
	add a, n
	rb0 n, o

UpdateVideoLoopY:
	mov g, a
	xor m, m

UpdateVideoLoopX:
	rb0b h, g
	inc g
	cje h, f, @UpdateVideo_EndOfLine

	'write out h
	push m
	push n
	push h
	call @WriteChar
	inc m
	jmp @UpdateVideoLoopX

UpdateVideo_EndOfLine:
	'if a character was written then update our last line
	cje m, f, @UpdateVideo_EmptyLine
	wb0 n, o

UpdateVideo_EmptyLine:
	'move to the next line
	add a, c
	inc n
	cjne a, b, @UpdateVideoLoopY

UpdateVideoDone:
	ret

RefreshScreen:
	ldimmb a, 1
	out a, 64
	ret

WriteLogo:
	ldimm a, 16
	ldimm b, 24
	ldimm c, @BootLogo
	add c, b
	add c, b
	xor d, d
	ldimm e, 0x37
	ldimmb n, 1

WriteLogoLoopY:
	dec b
	dec c
	dec c

	push a
	rb0 f, c

WriteLogoLoopX:
	dec a
	mov g, f
	and g, n
	shr f, n

	ldimm m, 640
	mul m, b
	add m, a

	cje g, n, @WriteLogo_BitSet

	wb1b d, m
	jmp @WriteLogo_BitDone

WriteLogo_BitSet:
	wb1b e, m

WriteLogo_BitDone:
	cjne a, d, @WriteLogoLoopX

	pop a
	cjne b, d, @WriteLogoLoopY
	ret

WriteString:
	'params are str ptr
	'writes the string to the console buffer
	'and advances the console buffer if need be

	pop a

	ldimm b, 80
	ldimm c, @ScreenBufferEnd
	ldimm m, @ScreenBufferPtr
	ldimm e, 0xff

	xor f, f
	rb0 d, m

	'd is where we are pointing to in the screen buffer

WriteString_Loop:
	rb0 n, a
	and n, e
	inc a

	'n is the current char
	'if null, finish
	cje n, f, @WriteString_Done

	'if a new line, advance the buffer if need be
	ldimm g, 0x0a
	cje n, g, @WriteString_AdvanceLine

	'if a new line, advance the buffer if need be
	ldimm g, 0x0d
	cje n, g, @WriteString_AdvanceLine

	'if a backspace then indicate a space and backup one character
	ldimm g, 0x08
	cje n, g, @WriteString_HandleBackspace

	'make sure the character isn't invalid
	'we allow 0x20 to 0x7f
	ldimm g, 0x20
	cjb n, g, @WriteString_Loop
	ldimm g, 0x7f
	cja n, g, @WriteString_Loop

	'normal character
	wb0b n, d
	inc d
	jmp @WriteString_Loop

WriteString_HandleBackspace:
	'make sure the prompt isn't overwritten
	ldimm h, @ScreenBufferUpdatePos
	rb0 h, h
	ldimm i, 80
	mul h, i
	ldimm i, @ScreenBuffer
	mov j, d
	sub j, i

	sub j, h
	
	ldimm g, 5
	cjb j, g, @WriteString_Loop

	dec d
	ldimm g, 0x2020
	wb0 g, d

	jmp @WriteString_Loop

WriteString_AdvanceLine:
	'make sure that the cursor isn't visible
	ldimm h, 0x20
	wb0 h, d

	'advance the buffer by 1 line if required
	ldimmb g, 1
	push d
	push g

	call @AdvanceBuffer
	pop d

	ldimm g, @ScreenBuffer

	'ptr += (80 - ((ptr - start) % 80))
	mov h, d
	sub h, g
	mod h, b
	mov g, b
	sub g, h
	add d, g

	jmp @WriteString_Loop

WriteString_Done:
	'store our new position
	wb0 d, m

	'calculate the new cursor position
	ldimm g, @ScreenBuffer
	sub d, g
	mov i, d
	div i, b
	mod d, b
	push d
	push i
	call @SetCursorPos

	ret

AdvanceBuffer:
	pop b
	pop o

	xor n, n

	'o is the current position in the screen buffer
	'b is how many lines desired at the end of the screen buffer

	ldimm c, @ScreenBuffer
	ldimm d, 80

	mov e, o
	sub e, c
	div e, d

	'e is the current line we are on, see if we need to make room
	add e, b
	ldimm b, 25

	cjb e, b, @AdvanceBuffer_Done

	'need to make room, e will be the number to move
	sub e, b
	inc e

	ldimm a, @ScreenBufferEnd
	ldimm b, 80
	mul e, b
	mov n, e

	add e, c

	ldimm f, 2

	'c is beginning of buffer
	'e is where to start moving from

AdvanceBuffer_Loop:
	rb0 g, e
	wb0 g, c
	add e, f
	add c, f

	'see if the original is at the end
	cjb e, a, @AdvanceBuffer_Loop

	'at the end of the original, blank out the rest
	xor g, g

AdvanceBuffer_BlankLoop:
	wb0 g, c
	add c, f
	cjb c, a, @AdvanceBuffer_BlankLoop

	call @BlankVideo

	ldimm a, @ScreenBufferUpdatePos
	wb0 g, a

AdvanceBuffer_Done:
	'calculate and push the new location
	sub o, n
	push o
	ret

'blank out the video area
BlankVideo:
	ldimm a, ff00h
	ldimm b, 2
	xor c, c
	xor d, d

BlankVideo_Bank1:
	wb1 c, d
	add d, b
	cjne a, d, @BlankVideo_Bank1

	'start again
	xor d, d

BlankVideo_Bank2:
	wb2 c, d
	add d, b
	cjne a, d, @BlankVideo_Bank2
	ret

'get the cursor x/y position
GetCursorPos:
	ldimm c, @CursorPointerX
	rb0 a, c

	ldimm c, @CursorPointerY
	rb0 b, c

	'x then y
	push a
	push b
	ret
	
'set the cursor x/y position
SetCursorPos:
	pop b
	pop a

	ldimm c, @CursorPointerX
	wb0 a, c

	ldimm c, @CursorPointerY
	wb0 b, c
	ret

WriteChar:
	'params x, y, character
	'writes a character to the video buffer
	pop e
	pop b
	pop a

	ldimmb g, 8
	mul a, g
	mul b, g

	xor c, c
	ldimm d, 0xff

	'adjust the character to line up
	'mul by 8 for character size then add the needed offset
	ldimm f, 32
	sub e, f
	mul e, g
	ldimm f, @Characters
	add e, f

	'e is now proper memory position
	add g, b

	'g is the end of the y array
	'b is the beginning of the y array

	ldimmb i, 1

WriteCharYLoop:
	'get a byte of the character
	rb0b f, e
	inc e

	'f is the byte to work on, h is the end point of x
	ldimmb h, 8
	add h, a

WriteCharXLoop:
	dec h
	mov j, f
	and j, i
	shr f, i

	'push x and y
	push h
	push b

	'j is the flag if the forecolor or backcolor should be used
	cje j, i, @WriteCharForeColor

	'backcolor
	push c
	jmp @WriteCharPixel

WriteCharForeColor:
	push d

WriteCharPixel:
	call @WritePixel

	cjne a, h, @WriteCharXLoop

	inc b
	cjne b, g, @WriteCharYLoop

WriteCharDone:
	ret

WritePixel:
	'x, y, pixelcolor
	pop c
	pop b
	pop a

	'see if x is over 640
	ldimm d, 639
	cja a, d, @WritePixelDone

	'see if y is above 204
	ldimm d, 203
	cja b, d, @WritePixelDone

	'determine which bank to write to and the proper memory location
	ldimm d, 101
	ldimm e, 640
	cja b, d, @WritePixelBank2

	mul e, b
	add e, a

	'write the pixel out
	wb1b c, e	
	jmp @WritePixelDone

WritePixelBank2:
	sub b, d
	dec b
	mul e, b
	add e, a

	wb2b c, e	

WritePixelDone:
	ret

BlinkCursor:
	ldimm a, @CursorOn
	rb0 b, a
	ldimm c, 0x20

	'a indicates if the cursor is on
	call @GetCursorPos
	pop f
	pop e

	cje b, c, @BlinkCursor_TurnOn

	ldimm b, 0x20
	jmp @BlinkCursor_Done

BlinkCursor_TurnOn:
	ldimm b, 0x5f

BlinkCursor_Done:
	wb0 b, a

	'params x, y, character
	push e
	push f
	push b
	call @WriteChar
	ret

BootString:
db "   Lightning BIOS v1.0", 0Ah, "   Copyright 2012", 0Ah, 0Ah, 0Ah
db "Main CPU: GITS 4546", 0Ah, "Memory Test: 64K OK", 0Ah, 0Ah
db "Boot Password: ", 00h

BootStringStarting:
	db "Starting...", 0Ah, 0Ah, 00h

BootPassword:
	db "BootMeUp!!!", 00h

Prompt:
db "C:\>", 00h

CmdBuffer:
	db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
	db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

'cursor pointer in x/y coord
CursorPointerX:
dw 0

CursorPointerY:
dw 0

CursorOn:
dw 0

'current position we are on for the buffer
ScreenBufferPtr:
dw 0

TempChar:
dw 0

ScreenBuffer:
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d
dw 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d, 0d

ScreenBufferEnd:

ScreenBufferUpdatePos:
dw 0

BootLogo:
dw 0000000000000011b
dw 0000000000000111b
dw 0000000000011110b
dw 0000000001111110b
dw 0000000011111100b
dw 0000000011111100b
dw 0000000111111000b
dw 0000000111111000b
dw 0000001111111111b
dw 0000001111111111b
dw 0000011111111110b
dw 0000011111111100b
dw 0000000001111000b
dw 0000000011111000b
dw 0000000011110000b
dw 0000000111100000b
dw 0000001111000000b
dw 0000001110000000b
dw 0000011110000000b
dw 0000011100000000b
dw 0000011000000000b
dw 0000110000000000b
dw 0000110000000000b
dw 0001100000000000b

Characters:
' 
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'!
db 00110000b
db 01111000b
db 01111000b
db 00110000b
db 00110000b
db 00000000b
db 00110000b
db 00000000b

'"
db 01101100b
db 01101100b
db 01101100b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'#
db 01101100b
db 01101100b
db 11111110b
db 01101100b
db 11111110b
db 01101100b
db 01101100b
db 00000000b

'$
db 00110000b
db 01111100b
db 11000000b
db 01111000b
db 00001100b
db 11111000b
db 00110000b
db 00000000b

'%
db 00000000b
db 11000110b
db 11001100b
db 00011000b
db 00110000b
db 01100110b
db 11000110b
db 00000000b

'&
db 00111000b
db 01101100b
db 00111000b
db 01110110b
db 11011100b
db 11001100b
db 01110110b
db 00000000b

''
db 01100000b
db 01100000b
db 11000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'(
db 00011000b
db 00110000b
db 01100000b
db 01100000b
db 01100000b
db 00110000b
db 00011000b
db 00000000b

')
db 01100000b
db 00110000b
db 00011000b
db 00011000b
db 00011000b
db 00110000b
db 01100000b
db 00000000b

'*
db 00000000b
db 01100110b
db 00111100b
db 11111111b
db 00111100b
db 01100110b
db 00000000b
db 00000000b

'+
db 00000000b
db 00110000b
db 00110000b
db 11111100b
db 00110000b
db 00110000b
db 00000000b
db 00000000b

',
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00110000b
db 00110000b
db 01100000b

'-
db 00000000b
db 00000000b
db 00000000b
db 11111100b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'.
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00110000b
db 00110000b
db 00000000b

'/
db 00000110b
db 00001100b
db 00011000b
db 00110000b
db 01100000b
db 11000000b
db 10000000b
db 00000000b

'0
db 01111100b
db 11000110b
db 11001110b
db 11011110b
db 11110110b
db 11100110b
db 01111100b
db 00000000b

'1
db 00110000b
db 01110000b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 11111100b
db 00000000b

'2
db 01111000b
db 11001100b
db 00001100b
db 00111000b
db 01100000b
db 11001100b
db 11111100b
db 00000000b

'3
db 01111000b
db 11001100b
db 00001100b
db 00111000b
db 00001100b
db 11001100b
db 01111000b
db 00000000b

'4
db 00011100b
db 00111100b
db 01101100b
db 11001100b
db 11111110b
db 00001100b
db 00011110b
db 00000000b

'5
db 11111100b
db 11000000b
db 11111000b
db 00001100b
db 00001100b
db 11001100b
db 01111000b
db 00000000b

'6
db 00111000b
db 01100000b
db 11000000b
db 11111000b
db 11001100b
db 11001100b
db 01111000b
db 00000000b

'7
db 11111100b
db 11001100b
db 00001100b
db 00011000b
db 00110000b
db 00110000b
db 00110000b
db 00000000b

'8
db 01111000b
db 11001100b
db 11001100b
db 01111000b
db 11001100b
db 11001100b
db 01111000b
db 00000000b

'9
db 01111000b
db 11001100b
db 11001100b
db 01111100b
db 00001100b
db 00011000b
db 01110000b
db 00000000b

':
db 00000000b
db 00110000b
db 00110000b
db 00000000b
db 00000000b
db 00110000b
db 00110000b
db 00000000b

';
db 00000000b
db 00110000b
db 00110000b
db 00000000b
db 00000000b
db 00110000b
db 00110000b
db 01100000b

'<
db 00011000b
db 00110000b
db 01100000b
db 11000000b
db 01100000b
db 00110000b
db 00011000b
db 00000000b

'=
db 00000000b
db 00000000b
db 11111100b
db 00000000b
db 00000000b
db 11111100b
db 00000000b
db 00000000b

'>
db 01100000b
db 00110000b
db 00011000b
db 00001100b
db 00011000b
db 00110000b
db 01100000b
db 00000000b

'?
db 01111000b
db 11001100b
db 00001100b
db 00011000b
db 00110000b
db 00000000b
db 00110000b
db 00000000b

'@
db 01111100b
db 11000110b
db 11011110b
db 11011110b
db 11011110b
db 11000000b
db 01111000b
db 00000000b

'A
db 00110000b
db 01111000b
db 11001100b
db 11001100b
db 11111100b
db 11001100b
db 11001100b
db 00000000b

'B
db 11111100b
db 01100110b
db 01100110b
db 01111100b
db 01100110b
db 01100110b
db 11111100b
db 00000000b

'C
db 00111100b
db 01100110b
db 11000000b
db 11000000b
db 11000000b
db 01100110b
db 00111100b
db 00000000b

'D
db 11111000b
db 01101100b
db 01100110b
db 01100110b
db 01100110b
db 01101100b
db 11111000b
db 00000000b

'E
db 11111110b
db 01100010b
db 01101000b
db 01111000b
db 01101000b
db 01100010b
db 11111110b
db 00000000b

'F
db 11111110b
db 01100010b
db 01101000b
db 01111000b
db 01101000b
db 01100000b
db 11110000b
db 00000000b

'G
db 00111100b
db 01100110b
db 11000000b
db 11000000b
db 11001110b
db 01100110b
db 00111110b
db 00000000b

'H
db 11001100b
db 11001100b
db 11001100b
db 11111100b
db 11001100b
db 11001100b
db 11001100b
db 00000000b

'I
db 01111000b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 01111000b
db 00000000b

'J
db 00011110b
db 00001100b
db 00001100b
db 00001100b
db 11001100b
db 11001100b
db 01111000b
db 00000000b

'K
db 11100110b
db 01100110b
db 01101100b
db 01111000b
db 01101100b
db 01100110b
db 11100110b
db 00000000b

'L
db 11110000b
db 01100000b
db 01100000b
db 01100000b
db 01100010b
db 01100110b
db 11111110b
db 00000000b

'M
db 11000110b
db 11101110b
db 11111110b
db 11111110b
db 11010110b
db 11000110b
db 11000110b
db 00000000b

'N
db 11000110b
db 11100110b
db 11110110b
db 11011110b
db 11001110b
db 11000110b
db 11000110b
db 00000000b

'O
db 00111000b
db 01101100b
db 11000110b
db 11000110b
db 11000110b
db 01101100b
db 00111000b
db 00000000b

'P
db 11111100b
db 01100110b
db 01100110b
db 01111100b
db 01100000b
db 01100000b
db 11110000b
db 00000000b

'Q
db 01111000b
db 11001100b
db 11001100b
db 11001100b
db 11011100b
db 01111000b
db 00011100b
db 00000000b

'R
db 11111100b
db 01100110b
db 01100110b
db 01111100b
db 01101100b
db 01100110b
db 11100110b
db 00000000b

'S
db 01111000b
db 11001100b
db 11100000b
db 01110000b
db 00011100b
db 11001100b
db 01111000b
db 00000000b

'T
db 11111100b
db 10110100b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 01111000b
db 00000000b

'U
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 11111100b
db 00000000b

'V
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 01111000b
db 00110000b
db 00000000b

'W
db 11000110b
db 11000110b
db 11000110b
db 11010110b
db 11111110b
db 11101110b
db 11000110b
db 00000000b

'X
db 11000110b
db 11000110b
db 01101100b
db 00111000b
db 00111000b
db 01101100b
db 11000110b
db 00000000b

'Y
db 11001100b
db 11001100b
db 11001100b
db 01111000b
db 00110000b
db 00110000b
db 01111000b
db 00000000b

'Z
db 11111110b
db 11000110b
db 10001100b
db 00011000b
db 00110010b
db 01100110b
db 11111110b
db 00000000b

'[
db 01111000b
db 01100000b
db 01100000b
db 01100000b
db 01100000b
db 01100000b
db 01111000b
db 00000000b

'\
db 11000000b
db 01100000b
db 00110000b
db 00011000b
db 00001100b
db 00000110b
db 00000010b
db 00000000b

']
db 01111000b
db 00011000b
db 00011000b
db 00011000b
db 00011000b
db 00011000b
db 01111000b
db 00000000b

'^
db 00010000b
db 00111000b
db 01101100b
db 11000110b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'_
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 11111111b

'`
db 00110000b
db 00110000b
db 00011000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

'a
db 00000000b
db 00000000b
db 01111000b
db 00001100b
db 01111100b
db 11001100b
db 01110110b
db 00000000b

'b
db 11100000b
db 01100000b
db 01100000b
db 01111100b
db 01100110b
db 01100110b
db 11011100b
db 00000000b

'c
db 00000000b
db 00000000b
db 01111000b
db 11001100b
db 11000000b
db 11001100b
db 01111000b
db 00000000b

'd
db 00011100b
db 00001100b
db 00001100b
db 01111100b
db 11001100b
db 11001100b
db 01110110b
db 00000000b

'e
db 00000000b
db 00000000b
db 01111000b
db 11001100b
db 11111100b
db 11000000b
db 01111000b
db 00000000b

'f
db 00111000b
db 01101100b
db 01100000b
db 11110000b
db 01100000b
db 01100000b
db 11110000b
db 00000000b

'g
db 00000000b
db 00000000b
db 01110110b
db 11001100b
db 11001100b
db 01111100b
db 00001100b
db 11111000b

'h
db 11100000b
db 01100000b
db 01101100b
db 01110110b
db 01100110b
db 01100110b
db 11100110b
db 00000000b

'i
db 00110000b
db 00000000b
db 01110000b
db 00110000b
db 00110000b
db 00110000b
db 01111000b
db 00000000b

'j
db 00001100b
db 00000000b
db 00001100b
db 00001100b
db 00001100b
db 11001100b
db 11001100b
db 01111000b

'k
db 11100000b
db 01100000b
db 01100110b
db 01101100b
db 01111000b
db 01101100b
db 11100110b
db 00000000b

'l
db 01110000b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 00110000b
db 01111000b
db 00000000b

'm
db 00000000b
db 00000000b
db 11001100b
db 11111110b
db 11111110b
db 11010110b
db 11000110b
db 00000000b

'n
db 00000000b
db 00000000b
db 11111000b
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 00000000b

'o
db 00000000b
db 00000000b
db 01111000b
db 11001100b
db 11001100b
db 11001100b
db 01111000b
db 00000000b

'p
db 00000000b
db 00000000b
db 11011100b
db 01100110b
db 01100110b
db 01111100b
db 01100000b
db 11110000b

'q
db 00000000b
db 00000000b
db 01110110b
db 11001100b
db 11001100b
db 01111100b
db 00001100b
db 00011110b

'r
db 00000000b
db 00000000b
db 11011100b
db 01110110b
db 01100110b
db 01100000b
db 11110000b
db 00000000b

's
db 00000000b
db 00000000b
db 01111100b
db 11000000b
db 01111000b
db 00001100b
db 11111000b
db 00000000b

't
db 00010000b
db 00110000b
db 01111100b
db 00110000b
db 00110000b
db 00110100b
db 00011000b
db 00000000b

'u
db 00000000b
db 00000000b
db 11001100b
db 11001100b
db 11001100b
db 11001100b
db 01110110b
db 00000000b

'v
db 00000000b
db 00000000b
db 11001100b
db 11001100b
db 11001100b
db 01111000b
db 00110000b
db 00000000b

'w
db 00000000b
db 00000000b
db 11000110b
db 11010110b
db 11111110b
db 11111110b
db 01101100b
db 00000000b

'x
db 00000000b
db 00000000b
db 11000110b
db 01101100b
db 00111000b
db 01101100b
db 11000110b
db 00000000b

'y
db 00000000b
db 00000000b
db 11001100b
db 11001100b
db 11001100b
db 01111100b
db 00001100b
db 11111000b

'z
db 00000000b
db 00000000b
db 11111100b
db 10011000b
db 00110000b
db 01100100b
db 11111100b
db 00000000b

'{
db 00011100b
db 00110000b
db 00110000b
db 11100000b
db 00110000b
db 00110000b
db 00011100b
db 00000000b

'|
db 00011000b
db 00011000b
db 00011000b
db 00000000b
db 00011000b
db 00011000b
db 00011000b
db 00000000b

'}
db 11100000b
db 00110000b
db 00110000b
db 00011100b
db 00110000b
db 00110000b
db 11100000b
db 00000000b

'~
db 01110110b
db 11011100b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b
db 00000000b

BiosEnd:
