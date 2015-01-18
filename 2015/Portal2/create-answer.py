import struct
import os, sys
import subprocess
import base64

try:
	os.unlink("a")
except:
	pass

Answer = "G-man has issues"

SecondText = open("gman-quotes.txt", "r").read()
FirstText = open("gman-background.txt", "r").read()

AnswerBin = bin(int(Answer.encode('hex'),16))[2:]
if len(AnswerBin) % 8:
	AnswerBin = "0"*(8-(len(AnswerBin) % 8)) + AnswerBin

print "Answer: %d spaces required, %d found" % (len(AnswerBin), len(SecondText.split(" ")))
if len(AnswerBin) > len(SecondText.split(" ")):
	print "Second text does not have enough spaces."
	sys.exit(0)

UnicodeSecondText = "\xfe\xff"
for i,c in enumerate(SecondText):
	if (c == ' ') and len(AnswerBin):
		if AnswerBin[0] == '0':
			UnicodeSecondText += struct.pack(">H", 0x2004)
		else:
			UnicodeSecondText += struct.pack(">H", ord(c))
		AnswerBin = AnswerBin[1:]
	else:
		UnicodeSecondText += struct.pack(">H",ord(c))

open("b","w").write(UnicodeSecondText)
subprocess.check_output(["./paq/paq8l","-8","b"])

UnicodeSecondText = open("b.paq8l","r").read()
flagbin = bin(int(UnicodeSecondText.encode('hex'),16))[2:]
if len(flagbin) % 8:
	flagbin = "0"*(8-(len(flagbin) % 8)) + flagbin

print "Second Text: %d characters required, %d found" % (len(flagbin), len(FirstText))
if len(flagbin) > len(FirstText):
	print "First text not long enough"
	sys.exit(0)

coverlen = len(FirstText)
#flagbin = ('{:0^%d}' % coverlen).format(flagbin)
#flagbin = flagbin[::-1].zfill(coverlen)[::-1]

output = open('a','w')
output.write('\xfe\xff')
for i,c in enumerate(FirstText):
	if (i >= len(flagbin)) or (flagbin[i]=='0'):
		output.write(struct.pack('>H',ord(c)))
	else:
		if c == ' ':
			output.write('\x20\x5f') #spaces are weird
		else:
			output.write(struct.pack('>H',ord(c)+0xfee0))
output.close()

subprocess.check_output(["./paq/paq8l","-8","a"])

b64 = open("a.paq8l","r").read()
b64 = base64.b64encode(b64)
print "base64: %d" % (len(b64))

#25 dots on the first and last frame
FrameCount = ((len(b64) / 2) - 50) * 8 / 6
FrameCount = FrameCount - 66
print "Frames: %d, Current total: %d" % (FrameCount, 4243 - 66)
if FrameCount > (4243-66):
	print "%0.2f seconds needed to finish" % (float(FrameCount - (4243-66)) / 30.0)
