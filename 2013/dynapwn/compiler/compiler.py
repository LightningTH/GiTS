import sys, os, struct

def GetReg(RegID, CurLineNum, CurLine):
	if(RegID == 'pc'):
		return 0

	Reg = ord(RegID) - ord('a') + 1
	if(Reg < 0 or Reg >= 16):
		print "Invalid register %s on line %d: %s" % (RegID, CurLineNum, CurLine)
		sys.exit(0)
	return Reg

Cmds = dict()
Cmds["add"] = 0x00
Cmds["sub"] = 0x01
Cmds["mul"] = 0x02
Cmds["div"] = 0x03
Cmds["mod"] = 0x04
Cmds["and"] = 0x05
Cmds["or"] = 0x06
Cmds["xor"] = 0x07
Cmds["neg"] = 0x08
Cmds["inc"] = 0x09
Cmds["dec"] = 0x0a
Cmds["not"] = 0x0b
Cmds["rol"] = 0x10
Cmds["ror"] = 0x11
Cmds["shl"] = 0x12
Cmds["shr"] = 0x13
Cmds["mov"] = 0x20
Cmds["ldimm"] = 0x21
Cmds["ldimmb"] = 0x22
Cmds["jmp"] = 0x30
Cmds["cjne"] = 0x31
Cmds["cje"] = 0x32
Cmds["cjb"] = 0x33
Cmds["cja"] = 0x34
Cmds["push"] = 0x40
Cmds["pop"] = 0x41
Cmds["rb0"] = 0x50
Cmds["rb1"] = 0x51
Cmds["rb2"] = 0x52
Cmds["wb0"] = 0x53
Cmds["wb1"] = 0x54
Cmds["wb2"] = 0x55
Cmds["rb0b"] = 0x56
Cmds["rb1b"] = 0x57
Cmds["rb2b"] = 0x58
Cmds["wb0b"] = 0x59
Cmds["wb1b"] = 0x5a
Cmds["wb2b"] = 0x5b
Cmds["call"] = 0x60
Cmds["ret"] = 0x61
Cmds["in"] = 0x62
Cmds["out"] = 0x63
Cmds["dump"] = 0xf0
Cmds["dyna"] = 0xf1
Cmds["frz"] = 0xfe
Cmds["halt"] = 0xff
Cmds["db"] = 0x100
Cmds["dw"] = 0x100

#array of various commands by opcode so the text compare can allow for multiple names
LabelCmds = [0x30, 0x31, 0x32, 0x33, 0x34, 0x60]
ImmCmds = [0x21, 0x62, 0x63]
ImmByteCmds = [0x22]
BankCmds = [0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b]
OneParamCmds = [0x08, 0x09, 0x0a, 0x0b, 0x40, 0x41]
ZeroParamCmds = [0x61, 0xf0, 0xf1, 0xfe, 0xff]
StorageCmds = [0x100]

ASMCode = open(sys.argv[1], 'rb').read().split("\n")
CurLineNum = 0
OutData = ""
KnownLabels = dict()
UnknownLabels = []	#format is (LabelName, Byte Position, IsOffsetFlag)

LastCmdIsStorage = False
DataOffset = 0

#if we have a map file then parse it up
#map data format is
#label address
#where address is a hex value
if len(sys.argv) == 4:
	MapData = open(sys.argv[3], 'rb').read().split("\n")
	for Entry in MapData:
		Line = Entry.split(" ")
		if len(Line) == 2:
			KnownLabels[Line[0]] = ("map file %s" % (sys.argv[3]), int(Line[1][2:], 16) | 0x80000000)

for Entry in ASMCode:
	CurLineNum = CurLineNum + 1
	CurLine = Entry.strip()

	#if a comment then skip
	if len(CurLine) == 0 or CurLine[0] == '\'':
		continue

	#see if we have an offset to start at
	if CurLine[0:4] == ".org":
		NumEntry = CurLine[5:]
		try:
			if(NumEntry[0:1] == '@'):
				#try looking up a value
				(LineNum, DataOffset) = KnownLabels[NumEntry[1:]]
				DataOffset = DataOffset & 0xffff
			elif(NumEntry[-1] == 'h'):
				#hex
				DataOffset = int(NumEntry[0:-1], 16)
			elif(NumEntry[-1] == 'd'):
				#decimal
				DataOffset = int(NumEntry[0:-1], 10)
			else:
				#assume decimal
				DataOffset = int(NumEntry, 10)
		except Exception, ex:
			print "Unknown number %s on line %d: %s" % (NumEntry, CurLineNum, CurLine)
			sys.exit(0)
		continue

	#see if it is a define for a label
	if CurLine[0:7] == ".define":
		LineData = CurLine.split(" ")
		if len(LineData) != 3:
			print "Invalid #define on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		#figure out the number
		NumEntry = LineData[2]
		try:
			if(NumEntry[-1] == 'h'):
				#hex
				DefinePos = int(NumEntry[0:-1], 16)
			elif(NumEntry[-1] == 'd'):
				#decimal
				DefinePos = int(NumEntry[0:-1], 10)
			else:
				#assume decimal
				DefinePos = int(NumEntry, 10)
		except Exception, ex:
			print ex
			print "Unknown number %s on line %d: %s" % (NumEntry, CurLineNum, CurLine)
			sys.exit(0)

		#set our flag that this label is special
		DefinePos |= 0x80000000

		#now see if the label is already defined
		if(LineData[1] in KnownLabels):
			(DefLine, ByteOffset) = KnownLabels[LineData[1]]
			print "Label %s already defined at line %s" % (LineData[1], str(DefLine))
			sys.exit(0)

		#add the label
		KnownLabels[LineData[1]] = (CurLineNum, DefinePos)
		continue

	#See if it is a label
	if CurLine[-1] == ":":
		if(LastCmdIsStorage):
			LastCmdIsStorage = False
			if(len(OutData) % 2):
				OutData = OutData + "\x00"

		if(CurLine[0:-1] in KnownLabels):
			(DefLine, ByteOffset) = KnownLabels[CurLine[0:-1]]
			print "Label %s already defined at line %d" % (CurLine[0:-1], DefLine)
			sys.exit(0)

		KnownLabels[CurLine[0:-1]] = (CurLineNum, len(OutData))
		#print "Label %s at 0x%04x" % (CurLine[0:-1], len(OutData) + DataOffset)
		continue

	#must be a command, parse it
	CurCmd = CurLine.split(" ")
	if(CurCmd[0] not in Cmds):
		print "Error on line %d: %s" % (CurLineNum, CurLine)
		sys.exit(0)

	if(Cmds[CurCmd[0]] in StorageCmds):
		LastCmdIsStorage = True

		#could be a string, number, or combination
		if(CurCmd[0] == "db"):
			#walk the line as it may contain a string
			i = CurLine.find("db")+2
			while(i < len(CurLine)):
				if(CurLine[i] == ' ' or CurLine[i] == 0x09 or CurLine[i] == ','):
					i += 1;
					continue;
				elif(CurLine[i] == '"'):
					#find the rest of the string
					EndOfStr = CurLine[i+1:].find("\"")
					if(EndOfStr == -1):
						print "Failed to find end of string on line %d: %s" % (CurLineNum, CurLine)
						sys.exit(0)

					OutData = OutData + CurLine[i+1:i+1+EndOfStr]
					i += EndOfStr + 2
				else:
					#has to be numeric

					NumEntry = CurLine[i:].find(",")
					if(NumEntry != -1):
						NumEntry = CurLine[i:i+NumEntry]
					else:
						NumEntry = CurLine[i:]

					try:
						if(NumEntry[-1] == 'b'):
							#bit
							OutData = OutData + chr(int(NumEntry[0:-1], 2))
						elif(NumEntry[-1] == 'h'):
							#hex
							OutData = OutData + chr(int(NumEntry[0:-1], 16))
						elif(NumEntry[-1] == 'd'):
							#decimal
							OutData = OutData + chr(int(NumEntry[0:-1], 10))
						else:
							#assume decimal
							OutData = OutData + chr(int(NumEntry, 10))
					except Exception, ex:
						print ex
						print "Unknown number %s on line %d: %s" % (NumEntry, CurLineNum, CurLine)
						sys.exit(0)
					i += len(NumEntry)

		else:
			if(len(OutData) % 2):
				OutData = OutData + "\x00"

			#must be a number, convert
			CurCmd.pop(0)
			for NumEntry in CurCmd:
				NumEntry = NumEntry.strip()
				if(NumEntry[-1] == ','):
					NumEntry = NumEntry[0:-1].strip()

				if(NumEntry[0] == '@'):
					#add the unknown label
					UnknownLabels.append((NumEntry[1:], len(OutData), False))
					OutData = OutData + "\x00\x00"
				else:
					try:
						if(NumEntry[-1] == 'b'):
							#bit
							OutData = OutData + struct.pack("<H", int(NumEntry[0:-1], 2))
						elif(NumEntry[-1] == 'h'):
							#hex
							OutData = OutData + struct.pack("<H", int(NumEntry[0:-1], 16))
						elif(NumEntry[-1] == 'd'):
							#decimal
							OutData = OutData + struct.pack("<H", int(NumEntry[0:-1], 10))
						else:
							#assume decimal
							OutData = OutData + struct.pack("<H", int(NumEntry, 10))
					except:
						print "Unknown number %s on line %d: %s" % (NumEntry, CurLineNum, CurLine)
						sys.exit(0)

		continue

	elif(LastCmdIsStorage):
		#if the last command was storage then realign the data
		LastCmdIsStorage = False
		if(len(OutData) % 2):
			OutData = OutData + "\x00"

	#handle normal commands
	if(Cmds[CurCmd[0]] in LabelCmds):
		#account for the label position
		LabelIsOffset = True
		JustRegister = False

		if(CurCmd[0] in ["call","jmp"]):
			if(CurCmd[0] == "call"):
				LabelIsOffset = False

			#must have 1 param and be a label
			if(len(CurCmd) != 2):
				print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
				sys.exit(0)

			if(CurCmd[1][0] != "@"):
				Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)
				JustRegister = True
			else:
				UnknownLabels.append((CurCmd[1][1:], len(OutData) + 2, LabelIsOffset))
				Reg1 = 0
			Reg2 = 0
		else:
			#must have 3 params
			if(len(CurCmd) != 4):
				print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
				sys.exit(0)

			#figure out the registers
			Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)
			Reg2 = GetReg(CurCmd[2][0], CurLineNum, CurLine)

			#add the unknown label
			UnknownLabels.append((CurCmd[3][1:], len(OutData) + 2, LabelIsOffset))

		#add in the command and space for the label position
		OutData = OutData + chr(Cmds[CurCmd[0]]) + chr(Reg1 | Reg2 << 4)
		if(JustRegister == 0):
			OutData = OutData + "\x00\x00"

	elif(Cmds[CurCmd[0]] in ImmCmds):
		if(len(CurCmd) != 3):
			print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		#figure out the register
		Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)

		OutData = OutData + chr(Cmds[CurCmd[0]]) + chr(Reg1)

		#see if it is a label request
		if(CurCmd[2][0] == '@'):
			#label reference, add it to the unknown for later fill in
			UnknownLabels.append((CurCmd[2][1:], len(OutData), False))
			OutData = OutData + "\x00\x00"

		else:
			#store the immediate
			Number = CurCmd[2].strip()
			if(Number[-1] == 'h' or Number[0:2] == "0x"):
				try:
					if(Number[-1] == 'h'):
						Number = Number[0:-1]
					else:
						Number = Number[2:]

					ImmVal = int(Number, 16)
				except:
					print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
					sys.exit(0)
			elif(Number[-1] == 'b'):
				try:
					ImmVal = int(Number[0:-1], 2)
				except:
					print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
					sys.exit(0)
			else:
				if(Number[-1] == 'd'):
					Number = Number[0:-1]

				try:
					ImmVal = int(Number, 10)
				except:
					print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
					sys.exit(0)

			if(ImmVal > 0xffff):
				print "Immediate too large on line %d: %s" % (CurLineNum, CurLine)
				sys.exit(0)
			OutData = OutData + struct.pack("<H", ImmVal)

	elif(Cmds[CurCmd[0]] in ImmByteCmds):
		if(len(CurCmd) != 3):
			print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		#figure out the register
		Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)

		#store the immediate
		Number = CurCmd[2].strip()
		if(Number[-1] == 'h' or Number[0:2] == "0x"):
			try:
				if(Number[-1] == 'h'):
					Number = Number[0:-1]
				else:
					Number = Number[2:]

				ImmVal = int(Number, 16)
			except:
				print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
				sys.exit(0)
		elif(Number[-1] == 'b'):
			try:
				ImmVal = int(Number[0:-1], 2)
			except:
				print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
				sys.exit(0)
		else:
			if(Number[-1] == 'd'):
				Number = Number[0:-1]

			try:
				ImmVal = int(Number, 10)
			except:
				print "Invalid immediate %s on line %d: %s" % (CurCmd[2], CurLineNum, CurLine)
				sys.exit(0)

		if(ImmVal > 0xf):
			print "Immediate too large on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		OutData = OutData + chr(Cmds[CurCmd[0]]) + chr(Reg1 | (ImmVal << 4))

	elif(Cmds[CurCmd[0]] in ZeroParamCmds):
		if(len(CurCmd) != 1):
			print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		OutData = OutData + chr(Cmds[CurCmd[0]]) + "\x00"

	elif(Cmds[CurCmd[0]] in OneParamCmds):
		if(len(CurCmd) != 2):
			print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		#figure out the registers
		Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)

		OutData = OutData + chr(Cmds[CurCmd[0]]) + chr(Reg1)
	else:
		#must have 2 params
		if(len(CurCmd) != 3):
			print "Invalid params on line %d: %s" % (CurLineNum, CurLine)
			sys.exit(0)

		#figure out the registers
		Reg1 = GetReg(CurCmd[1][0], CurLineNum, CurLine)
		Reg2 = GetReg(CurCmd[2][0], CurLineNum, CurLine)

		OutData = OutData + chr(Cmds[CurCmd[0]]) + chr(Reg1 | Reg2 << 4)

#cycle though all unknown labels and fill them in
for (CurLabel, CurPos, IsOffset) in UnknownLabels:
	if(CurLabel not in KnownLabels):
		print "Unable to locate label %s" % (CurLabel)
		sys.exit(0)

	(KnownLabelLineNum, KnownLabelPos) = KnownLabels[CurLabel]
	if(IsOffset):
		OffsetPos = (KnownLabelPos - CurPos) & 0xffff
		OutData = OutData[0:CurPos] + struct.pack("<H", (OffsetPos + 2) & 0xffff) + OutData[CurPos+2:]
	else:
		if(KnownLabelPos & 0x80000000):
			OutData = OutData[0:CurPos] + struct.pack("<H", KnownLabelPos & 0xffff) + OutData[CurPos+2:]
		else:
			OutData = OutData[0:CurPos] + struct.pack("<H", (KnownLabelPos + DataOffset) & 0xffff) + OutData[CurPos+2:]

open(sys.argv[2],"wb").write(OutData)
print "Wrote %d bytes" % (len(OutData))

MapData = ""
for Entry in KnownLabels:
	(Line, Position) = KnownLabels[Entry];
	try:
		Line = int(Line)
		MapData = MapData + "%s 0x%04x\n" % (Entry, Position+DataOffset)
	except:
		continue

if len(MapData):
	open(sys.argv[1][:-4] + ".map","wb").write(MapData)

