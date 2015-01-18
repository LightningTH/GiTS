"""
	Edgy

	A puzzle solving challenge that requires some programming to solve

	Designed for Shmoocon 2014 by Lightning
"""

import SocketServer
import sys
import socket
import threading
import thread
import tempfile
import subprocess
import os
import random
import time
import zlib

def GenerateMap(size, repeat, MinePercentage, Seed = 0):
	Data = [" "]
	for i in xrange(size - 1):
		Data += [" "]

	#generate a line per entry, use list() to force uniqueness
	MapData = [list(Data)]
	for i in xrange(size - 1):
		MapData += [list(Data)]

	#setup the start location
	Center = int(size / 2)
	MapData[Center][Center] = '@'

	#generate a pattern
	MaxStepsPerLoop = int((size / 1.5) / repeat)

	if Seed == 0:
		r = random.SystemRandom()
	else:
		r = random.Random(Seed)

	StepCount = r.randrange(MaxStepsPerLoop, int(MaxStepsPerLoop * 1.5))

	#find a pattern, we'll loop until we are good as we don't want to stomp ontop of ourselves as that's wasteful
	BadDirection = 0
	while(1):
		#generate a new list
		Pattern = [list(Data)]
		for i in xrange(size - 1):
			Pattern += [list(Data)]

		if Seed == 0:
			r = random.SystemRandom()
		else:
			r = random.Random(Seed)

		StepPattern = [r.randint(0, 3)]
		x = Center
		y = Center

		#setup the first entry
		if(StepPattern[0] == 0):
			x -= 1
		elif(StepPattern[0] == 1):
			x += 1
		elif(StepPattern[0] == 2):
			y -= 1
		elif(StepPattern[0] == 3):
			y += 1
		Pattern[y][x] = '+'

		#mark the direction we will refuse to go which is opposite of the starting direction
		BadDirection = StepPattern[0] ^ 1

		#generate a pattern to use
		Counter = 0

		for i in xrange(len(StepPattern), StepCount):
			while(1):
				Counter += 1
				Dir = r.randint(0, 3)
				#0 - left
				#1 - right
				#2 - up
				#3 - down
				#don't want up then down or left then right or vice versa
				#so if bit 1 matches then make sure the direction matches
				#otherwise it is the opposite which isn't helpful
				if (((StepPattern[-1] & 2) != (Dir & 2)) or (StepPattern[-1] == Dir)) and (Dir != BadDirection):

					#see if going this direction results in touching an
					#area already touched
					TempX = x
					TempY = y
					if(Dir == 0):
						TempX -= 1
					elif(Dir == 1):
						TempX += 1
					elif(Dir == 2):
						TempY -= 1
					elif(Dir == 3):
						TempY += 1

					#check the spot, it should be empty
					if Pattern[TempX][TempY] == ' ':

						#now check around us, there should only be 1 hit otherwise we are wasting steps
						MatchAround = 0
						if ((TempY - 1) >= 0) and (Pattern[TempY - 1][TempX] != ' '):
							MatchAround += 1
						if ((TempY + 1) < size) and (Pattern[TempY + 1][TempX] != ' '):
							MatchAround += 1
						if ((TempX - 1) >= 0) and (Pattern[TempY][TempX - 1] != ' '):
							MatchAround += 1
						if ((TempX + 1) < size) and (Pattern[TempY][TempX + 1] != ' '):
							MatchAround += 1

						#we should only be coming from 1 direction and no other line near us
						if(MatchAround <= 1):
							StepPattern += [Dir]
							x = TempX
							y = TempY
							Pattern[y][x] = '+'
							break

				if Counter > (StepCount * 10):
					break

			if Counter > (StepCount * 10):
				break

		if len(StepPattern) >= StepCount:
			break

	#finally got a full pattern, map it out then mark it up
	#x and y are where we left off at, repeat the pattern until we hit the end
	while(1):
		for Dir in StepPattern:
			if(Dir == 0):
				x -= 1
			elif(Dir == 1):
				x += 1
			elif(Dir == 2):
				y -= 1
			elif(Dir == 3):
				y += 1

			if (x >= size) or (y >= size) or (x < 0) or (y < 0):
				break
			Pattern[y][x] = '+'

		if (x >= size) or (y >= size) or (x < 0) or (y < 0):
			break
			

	#now generate the mines
	for i in xrange(0, int((size*size)*MinePercentage)):
		RandX = r.randint(0, size-1)
		RandY = r.randint(0, size-1)
		if MapData[RandY][RandX] == ' ' and Pattern[RandY][RandX] == ' ':
			MapData[RandY][RandX] = 'x'

	RetData = "-"*(size+2) + '\n'
	for i in MapData:
		RetData += '|' + ''.join(i) + '|\n'
	RetData += "-"*(size+2) + '\n'

	return (RetData, MapData, int(MaxStepsPerLoop*1.5))

class startedgy(SocketServer.StreamRequestHandler):

	def TestMap(self, size, RawMapData, Attempt):
		#walk from the middle of the map
		Center = int(size / 2)
		AttemptPos = 0
		Attempt = Attempt.lower()
		x = Center
		y = Center
		LoopCount = 0
		while(1):
			if(Attempt[AttemptPos] == 'a'):
				x -= 1
			elif(Attempt[AttemptPos] == 'd'):
				x += 1
			elif(Attempt[AttemptPos] == 'w'):
				y -= 1
			elif(Attempt[AttemptPos] == 's'):
				y += 1
			else:
				self.wfile.write("Invalid direction\n")
				return False

			if (x < 0) or (y < 0) or (x >= size) or (y >= size):
				return True

			if(RawMapData[y][x] != ' '):
				self.wfile.write("You hit a mine at X/Y %d/%d\n" % (x, y))
				return False

			AttemptPos = (AttemptPos + 1) % len(Attempt)
			if(AttemptPos == 0):
				LoopCount += 1
				if(LoopCount >= 100):
					self.wfile.write("Got stuck in a loop, you did not reach the edge\n")
					return False

		return False
					
	def handle(self):

		#self.connection.settimeout(5)

		try:
			self.wfile.write("Password\n")
			Result = self.rfile.readline().strip()

			if(Result != "EdgesAreFun"):
				self.wfile.write("Invalid password\n")
				return True

			self.wfile.write("Welcome to Edgy, please solve the following puzzle with the WASD keys\nThe pattern provided will be repeated until the edge of the map is reached\n Example: If WDD is sent then it will repeat WDDWDDWDD... until an edge is reached\n\n")

			FixedArray = [
				("",
				[[7, 46], [7, 98], [9, 10], [9,21], [11, 2], [11, 12]]),
				("*Cracks knuckles* Now that you are warmed up, let's kick it up a notch\n",
				[[15, 15], [15, 40], [15, 43], [17,55], [17, 65], [17, 76], [19, 80], [19, 85]])
			]

			for (Msg, ArrEntry) in FixedArray:
				if len(Msg):
					self.wfile.write(Msg)

				for Data in ArrEntry:
					(MapData, RawMapData, MaxValues) = GenerateMap(Data[0], 2, 0.4, Data[1])
					self.wfile.write(MapData)
					self.wfile.write("You have a maximum of %d moves\n" % (MaxValues))
					Result = self.rfile.readline().strip()
					if(self.TestMap(Data[0], RawMapData, Result) == False):
						return True

			self.wfile.write("Show off, time to mix it up\n")
			for i in xrange(0, 13):
				MapSize = (int(i / 3) + 21) | 1
				MapLoop = 3
				(MapData, RawMapData, MaxValues) = GenerateMap(MapSize, MapLoop, 0.4)
				self.wfile.write(MapData)
				self.wfile.write("You have a maximum of %d moves\n" % (MaxValues))

				Result = self.rfile.readline().strip()
				if(self.TestMap(MapSize, RawMapData, Result) == False):
					return True

			#slowly increase the difficulty as now they just did 12 random boards from 21 to 29 in size
			#hopefully they are brute forcing it and just wasted a bunch of time
			for i in xrange(29, 199, 2):
				MapSize = i
				MapLoop = int((i / 50) + 4)
				(MapData, RawMapData, MaxValues) = GenerateMap(MapSize, MapLoop, 0.3)
				self.wfile.write(MapData)
				self.wfile.write("You have a maximum of %d moves\n" % (MaxValues))

				Result = self.rfile.readline().strip()
				if len(Result) > MaxValues:
					self.wfile.write("Too many moves returned")
					return True

				if(self.TestMap(MapSize, RawMapData, Result) == False):
					return True

			self.wfile.write("Fine then. You've managed to solve my puzzles\nOne more, I promise\n")
			MapSize = 1001
			(MapData, RawMapData, MaxValues) = GenerateMap(MapSize, 5, 0.35)

			OutData = zlib.compress(MapData, 9)
			self.wfile.write("Sending %d bytes of zlib data. I still expect the answer in text though\n" % (len(OutData)))
			self.wfile.write(OutData)
			self.wfile.write("You have a maximum of %d moves\n" % (MaxValues))

			Result = self.rfile.readline().strip()
			if len(Result) > MaxValues:
				self.wfile.write("Too many moves returned")
				return True

			if(self.TestMap(MapSize, RawMapData, Result) == False):
				return True
			
			self.wfile.write("I guess you've earned it, the key is:\n")
			self.wfile.write("EdsgerDijkstraWasOnTheRightPath\n")
			return True

		except Exception, ex:
			print ex
			pass

		return True

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
	allow_reuse_address = True

def daemonize():
	if os.fork() != 0:
		os._exit(0)
	 
	os.setsid()
	 
	if os.fork() != 0:
		os._exit(0)
	 
	os.chdir("/")
	os.umask(022)
	[os.close(i) for i in xrange(3)]
	os.open(os.devnull, os.O_RDWR)
	os.dup2(0, 1)
	os.dup2(0, 2)

if __name__ == "__main__":
	#base port for service
	BasePort = 4444

	#daemonize()

	#since we are threading make a general lock
	#ThreadLock = threading.Lock()
	
	#start the service
	server = ThreadedTCPServer(('', BasePort), startedgy)
	#t = threading.Thread(target=server.serve_forever)
	#t.setDaemon(False) # don't hang on exit
	print "[+] Edgy running!\n"
	#t.start()
	
	#server this forever
	try:
		server.serve_forever()
	except KeyboardInterrupt:
		pass
