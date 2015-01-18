import sys, os
import subprocess
import struct

subprocess.check_output(["./paq/paq8l", "-d", "answer.paq8l"])
data = open("a","r").read()

Stage2Bits = ""
for i in xrange(2, len(data), 2):
	val = struct.unpack(">H", data[i:i+2])[0]
	if val > 0x00FF:
		Stage2Bits += '1'
	else:
		Stage2Bits += '0'
Stage2 = ""
for i in xrange(0, len(Stage2Bits), 8):
	Stage2 += chr(int(Stage2Bits[i:i+8], 2))

print len(Stage2)
open("answer2.paq8l","w").write(Stage2)

subprocess.check_output(["./paq/paq8l", "-d", "answer.paq8l"])
data = open("b","r").read()

AnswerBits = ""
for i in xrange(2, len(data), 2):
	val = struct.unpack(">H", data[i:i+2])[0]
	if val == 0x0020:
		AnswerBits += '1'
	elif val == 0x2004:
		AnswerBits += '0'

Answer = ""
for i in xrange(0, len(AnswerBits), 8):
	Answer += chr(int(AnswerBits[i:i+8], 2))
print Answer
