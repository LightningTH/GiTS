.org @BiosEnd

'clean up the stack
pop a

CmdUpload:
	'Indicate we are waiting
	ldimm a, @CmdUploadMsg
	push a
	call @WriteString

	'show were to upload to and request update
	ldimm e, @UploadEnd
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

	'return from the upload app
	ldimm a, @CmdUploadMsgExecuteDone
	push a
	call @WriteString
	ret

CmdUpload_Timeout:
	ldimm a, @CmdUploadMsg_Timeout
	jmp @UploadPrintData

CmdUpload_Abort:
	ldimm a, @CmdUploadMsg_Abort

UploadPrintData:
	push a
	call @WriteString
	ret

CmdUploadMsg:
	db "Waiting on file to execute (5 second timeout)...", 0ah, 00h

CmdUploadMsg_Timeout:
	db "Timeout while waiting", 0ah, 00h

CmdUploadMsg_Abort:
	db "Upload aborted", 0ah, 00h

CmdUploadMsgDone:
	db "Executing program", 0ah, 00h

CmdUploadMsgExecuteDone:
	db "Finished program execution", 0ah, 00h

UploadEnd:

UploadInfo:
	dw ffffh
	db "Upload and execute a program", 0

