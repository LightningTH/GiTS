from PIL import Image, ImageDraw, ImageFont, ImageOps
import StringIO
import sys
import os
import hashlib, random, struct, zlib
import base64

FramesPerCursor = [20, 10]	#total/on count

BackgroundImage = Image.open("Credits-Background.png")
GradientImage = Image.open("portal2-stripe.png")
ButtonImage = Image.open("portal2-box-pink.png")
DotImage = Image.open("portal2-dot-pink.png")

CreditColor = "#9e1974"

#EmbedData = base64.b64encode(open("embeddata","r").read())
EmbedData = base64.b64encode(open("a.paq8l","r").read())
Base64Dict = {}
for i in xrange(0, 26):
	Base64Dict[chr(0x41 + i)] = i
	Base64Dict[chr(0x61 + i)] = 26+i
for i in xrange(0, 10):
	Base64Dict[chr(0x30+i)] = (26+26)+i
Base64Dict["+"] = (26+26+10)
Base64Dict["/"] = (26+26+10+1)
Base64Dict["="] = -1	#ignoring = as we ran out of bits

def AddDots(img, FrameNum):
	BtImg = ButtonImage.copy()

	#setup the button area
	c = Image.new("RGBA", (1522-1160,216-35))

	#figure out what part of the data to display. 1 entry per line, 6 pixel scroll per frame
	#calculate how far down into the data to start rendering
	DataStartBlock = FrameNum * 6 / (ButtonImage.size[1]-2)

	#We also need to know how far on the mid data to start showing
	DataMid = (len(EmbedData) / 2)

	DataStartBlock2 = DataMid + DataStartBlock

	#figure out the pixel offset to draw on
	PixelOffset = ((FrameNum % 4) * 2)

	#turn into 0, 6, 4, 2 when original is 0, 2, 4, 6
	PixelOffset = (8 - PixelOffset) % 8

	#1st group scrolling down
	DownCount = 22

	#get the chunk of data for the two blocks
	#make sure it is padded out with A's
	if (DataStartBlock+DownCount) > (len(EmbedData) / 2):
		Count = (len(EmbedData) / 2) - DataStartBlock
		if Count <= 0:
			DataBlock = "A"*DownCount
		else:
			DataBlock = EmbedData[DataStartBlock:DataStartBlock+Count] + "A"*(DownCount - Count)
	else:
		DataBlock = EmbedData[DataStartBlock:DataStartBlock+DownCount]

	if (DataStartBlock2+DownCount) > len(EmbedData):
		Count = len(EmbedData) - DataStartBlock2
		if Count <= 0:
			DataBlock2 = "A"*DownCount
		else:
			DataBlock2 = EmbedData[DataStartBlock2:DataStartBlock2+Count] + "A"*(DownCount - Count)
	else:
		DataBlock2 = EmbedData[DataStartBlock2:DataStartBlock2+DownCount]

	for x in xrange(DownCount - 1, -1, -1):	#22 should show
		CurChar = Base64Dict[DataBlock[x]]
		for i in xrange(0, 6):
			#if bit is set then put a block in, right to left alignment
			if CurChar & (1<<i):
				c.paste(ButtonImage, (((6-i)*(ButtonImage.size[0]+9)), ((21-x)*(ButtonImage.size[1]-2)) + PixelOffset))

	for x in xrange(0, DownCount):	#22 should show
		CurChar = Base64Dict[DataBlock2[x]]

		if CurChar == -1:
			#= is at the end of the data
			c.paste(DotImage, (195, (x*(ButtonImage.size[1]-2)) - PixelOffset))
		else:
			for i in xrange(0, 6):
				#if bit is set then put a block in, right to left alignment
				if CurChar & (1<<i):
					c.paste(ButtonImage, (180 + ((6-i)*(ButtonImage.size[0]+9)), (x*(ButtonImage.size[1]-2)) - PixelOffset))

	#setup the button position
	d = Image.new("RGBA", img.size)

	#now copy and paste from the button grid to the image we will alpha transfer
	#this is done to allow the buttons to cut off at the top and bottom as they only partially overwrite the border
	#in the original setup
	d.paste(c, (1145,35))
	ImgGrd = Image.alpha_composite(img,d)
	del img
	del d
	del c
	return ImgGrd

def DoScan(img, FrameNum):
	FrameNum += 13
	if (FrameNum % 60) < 15:
		return img

	#figure out what position the scan needs to be at
	
	#setup the gradient position
	c = Image.new("RGBA", img.size)

	CurFrame = (FrameNum % 60) - 15
	FrameHeight = c.size[1]+GradientImage.size[1]
	FrameOffset = (FrameHeight / 45)
	FrameOffset = (FrameOffset*CurFrame)-(GradientImage.size[1] - FrameOffset)
	c.paste(GradientImage, (0, FrameOffset))
	ImgGrd = Image.alpha_composite(img,c)
	del img
	del c
	return ImgGrd

def DoBackground():
	return BackgroundImage.copy()

def DoLyrics(img, lyrics):
	global fnt

	lyrics = lyrics.split("\n")
	(textx, texty) = fnt.getsize("X")

	texty += 25
	draw = ImageDraw.Draw(img)                     # setup to draw on the main image

	y = 130
	for Entry in lyrics:
		draw.text((140, y), Entry, font=fnt, fill="#9e1974")
		#draw.text((7+textx + 3, y), Entry, font=fnt, fill="#92742b")
		y += texty

	del draw 
	
	return img


def WritePic(img, FrameNum):
	img = DoScan(img, FrameNum)
	img.save("images/frame%04d.png" % (FrameNum), "PNG")

def AddLyricCursor(img, FrameNum, Lyrics, LineNum, StartOffset = 0):
	global FramesPerCursor, fnt

	if(FrameNum % FramesPerCursor[0]) < FramesPerCursor[1]:
		if len(Lyrics) == 0:
			textx = 0
		else:
			textx = fnt.getsize(Lyrics)[0]
		texty = fnt.getsize("X")[1]

		texty += 25
		y = 130 + (texty * LineNum)

		draw = ImageDraw.Draw(img)                     # setup to draw on the main image
		draw.text((140 + textx + StartOffset, y), "_", font=fnt, fill="#9e1974")
		#draw.text((7 +  (X * textx) + 3, y), "_", font=fnt, fill="#92742b")
	
	return img

def AddLine(img, StartFrame, EndFrame, LineNum, VerseLine, StartPos = 0):
	global fnt

	#when called we should be on the startframe

	#if the line is empty then just do the frames we have
	if len(VerseLine) == 0:
		for CurFrame in xrange(StartFrame, EndFrame + 1):
			imgdata = img.copy()
			imgdata = AddLyricCursor(imgdata, CurFrame, "", LineNum, StartPos)
			imgdata = AddCredits(imgdata, CurFrame)
			imgdata = AddDots(imgdata, CurFrame)
			WritePic(imgdata, CurFrame)

			sys.stdout.write("%d/%d\r" % (CurFrame, EndFrame))
			sys.stdout.flush()

		return

	#calculate number of frames per char for the line
	FramesPerChar = float(EndFrame - StartFrame) / float(len(VerseLine))
	#print "Frames per char: %f" % (FramesPerChar)

	(textx, texty) = fnt.getsize("X")

	texty += 25
	y = 130 + (texty * LineNum)
	x = 140 + StartPos

	for CurFrame in xrange(StartFrame, EndFrame + 1):
		#print "Frame %d" % (CurFrame)
		imgdata = img.copy()
		draw = ImageDraw.Draw(imgdata)                     # setup to draw on the main image

		NumChars = int((CurFrame - StartFrame) / FramesPerChar)
		draw.text((x, y), VerseLine[0:NumChars], font=fnt, fill="#9e1974")
		#draw.text((x + 1, y), VerseLine[0:NumChars], font=fnt, fill="#92742b")
		imgdata = AddLyricCursor(imgdata, CurFrame, VerseLine[0:NumChars], LineNum, StartPos)
		imgdata = AddCredits(imgdata, CurFrame)
		imgdata = AddDots(imgdata, CurFrame)
		WritePic(imgdata, CurFrame)
		sys.stdout.write("%d/%d\r" % (CurFrame, EndFrame))
		sys.stdout.flush()

	del draw 

def AddCreditCursor(img, FrameNum, X, Y):
	global FramesPerCursor, fnt

	if(FrameNum % FramesPerCursor[0]) >= FramesPerCursor[1]:
		draw = ImageDraw.Draw(img)                     # setup to draw on the main image
		draw.text((X+5, Y+2), "_", font=fnt, fill=CreditColor)
	
	return img

def AddCredits(img, FrameNum):
	global fnt, Credits, FrameOffset


	(textx, texty) = fnt.getsize("X")

	FrameVal = FrameNum - (221 - FrameOffset)
	if(FrameVal < 0):
		return img

	CreditLines = [""]*15 + Credits.split("\n")

	draw = ImageDraw.Draw(img)

	CurLine = 0
	while(FrameVal >= (len(CreditLines[CurLine]) * 1.5)):
		FrameVal = FrameVal - (len(CreditLines[CurLine]) * 1.5)

		CurLine += 1
		if CurLine >= len(CreditLines):
			FrameVal = len(CreditLines[-1])*1.5
			CurLine = len(CreditLines) - 1
			break

	texty = fnt.getsize("X")[1] + 25
	y = texty+180
	for Entry in xrange(CurLine - 15, CurLine):
		textx = fnt.getsize(CreditLines[Entry])[0]
		draw.text((1770-textx, y), CreditLines[Entry], font=fnt, fill=CreditColor)
		y += texty

	textx = fnt.getsize(CreditLines[CurLine])[0]
	draw.text((1770-textx, y), CreditLines[CurLine][0:int(FrameVal/1.5)], font=fnt, fill=CreditColor)

	del draw
	return AddCreditCursor(img, FrameNum, 1770-textx+fnt.getsize(CreditLines[CurLine][0:int(FrameVal/1.5)])[0], y)


#layout (repeated)
#<string>, num frames to write string in, num frames of pause until next line

Verse1 =  """
Forms FORM-29827281-12-2:
Notice of Dismissal

Well here we are again
It's always such a challenge
Remember when you tried
To solve my last?
Oh, how you tried and tried
Until you gave up and failed
Under the circumstances
You've been shockingly bad
"""

Verse1StartFrame = 66

#first frame char is seen
#last frame text is seen
#<repeat pattern>
Verse1Pattern = [
66,124,
127,185,
186,245,
246,298,
318,376,
395,437,
439,502,
535,589,
599,657,
673,737,
738,809
]

Verse2 = """
You want these points?
Solve it
That's what I'm counting on,

I used to want you to fail
but
Now I want to waste your time

"""

Verse2StartFrame = 836
Verse2Pattern = [
847,909,
918,945,
976,1055,
1056,1131,
1132,1214,
1216,1223,
1238,1326,
1327,1433
]

Verse3 = """
The clock is ticking now
You could use the points here
So get going and try your best
Tomcr00se just solved it all
And now leads the scoreboard
It's such a shame the same
will never happen to you

"""

Verse3StartFrame = 1434
Verse3Pattern = [
1462,1513,
1538,1593,
1613,1721,
1750,1808,
1825,1887,
1894,1945,
1948,2041,
2042,2054
]

Verse4 = """
Severance Package Details:

You need
these points
to win it
That's what I'm counting on
I'll let you get right to it
Now I want to waste your time

"""

Verse4StartFrame = 2055
Verse4Pattern = [
2055,2059,
2059,2059,
2060,2083,
2088,2118,
2126,2189,
2205,2278,
2344,2440,
2452,2539,
2540,2657
]

Verse5 = """
Did you not understand
That this song is a bad ruse
To keep you stumbling
By yourself, and sad

But now I see you there
As you search around here
When you give up still I will
not stop feeling so bad
"""

Verse5StartFrame = 2658
Verse5Pattern = [
2692,2744,
2764,2821,
2839,2873,
2885,2940,
2977,3030,
3050,3111,
3120,3179,
3188,3258,
3259,3278
]

Verse6 = """
Go code some new \0
disaster
That's what I'm counting on
You're not gonna solve this challenge
Now I want to waste your \0
time
Now I want to waste your \0
time
Now I want to waste your
"""

Verse6StartFrame = 3279
Verse6Pattern = [
3290,3331,
3342,3396,	#"disaster" written slower, \0 on above line
3436,3509,
3576,3673,
3676,3744,
3754,3781,	#"gone" written slower, \0 on above line
3820,3888,
3898,3925,	#"gone" written slow, \0 on above line
3963,4038
]

Verse7 = """




time
"""

Verse7StartFrame = 4042
Verse7Pattern = [
4042,4042,
4042,4042,
4042,4042,
4043,4043,
4051,4072
]

FinalFrame = 4243

Verses = [
[Verse1, Verse1StartFrame, Verse1Pattern],
[Verse2, Verse2StartFrame, Verse2Pattern],
[Verse3, Verse3StartFrame, Verse3Pattern],
[Verse4, Verse4StartFrame, Verse4Pattern],
[Verse5, Verse5StartFrame, Verse5Pattern],
[Verse6, Verse6StartFrame, Verse6Pattern],
[Verse7, Verse7StartFrame, Verse7Pattern],
["", FinalFrame, ""]
]


Credits = """
>LIST PERSONNEL:
Lightning
Psifertex
peterl
Hahna
Don't Panic
fuzyll
Computerality
RyanWithZombies
hoju
Scarecrow
Suspenders
HockeyInJune
z3x
Magic Hands
 
Crash Override
Acid Burn
The Phantom Phreak
Cereal Killer
Lord Nikon
Joey
The Plague
Richard Gill
Martin Bishop
Cosmo
Donald Crease
Erwin Emery
Darryl Roskow
Carl Arbogast
Liz
Werner Brandes
David Lightman
Dr. John McKittrick
Dr. Stephen Falken
Jennifer Mack
Kevin Flynn
Alan Bradley
Ed Dillinger
Dr. Lora Baines
Dr. Walter Gibbs
Sark
Ram
Crom
Chris Knight
Mitch Taylor
Jordan Cochran
Prof. Jerry Hathaway
Lazlo Hollyfeld
Kent
David Decker
Sherry Nugill
James Bond
Alec Trevelyan
Natalya Simonova
Xenia Onatopp
Jack Wade
Boris Grishenko
Marty McFly
Dr Emmett L. Brown
Garry Wallace
Wyatt Donnelly
Lisa
Chet Donnelly
Angela Bennett
Jack Devlin
Dr. Alan Champion
Ruth Marx
Michael Bergstrom
Thomas A. Anderson
Morpheus
Trinity
Agent Smith
Rick Deckard
Roy Batty
Rachael
Gaff
Bryant
Pris
Richard B. Riddick
Aaron
Abe
Robert
Phillip
Stanley Jobson
Gabriel Shear
Ginger Knowles
Maximillian Cohen
Luke Skywalker
Darth Vader
Han Solo
Ben Obi-Wan Kenobi
Yoda
Captain Lone Starr
Barf
Yogurt
Dark Helmet
Silas Warner
Duke Nukem
Donkey Kong
John D Carmack
John Romero
Sonic the Hedgehog
Pac-Man
Stranger
Atrus
Douglas Adams
Arthur Dent
Tim Sweeney
Link
Princess Zelda
Epona
Ganon
Navi
Mario Mario
Luigi Mario
Zero Wing
Fox McCloud
Falco Lombardi
Gordon Freeman
G-Man
Barney Calhoun
Adrian Shephard
Dr Rosenberg
Alyx Vance
Isaac Kleiner
Eli Vance
Arne Magnusson
Judith Mossman
Odessa Cubbage
Father Grigori
Wallace Breen
Chell
Doug Rattmann
Cave Johnson
Caroline
GLaDOS
Wheatley
Vault Boy
Master Chief
Solid Snake
Lara Croft
Crash
Nico Bellic
Kirby
Mega Man
Yoshi
Spyro
Goku
Max Payne
Bowser
Ryu
Scorpion
Sonya Blade
Johnny Cage
Liu Kang
Marcus Fenix
Leon Kennedy
Prince of Persia
Simon Belmont
Centipede
Samus Aran
Pitfall Harry
Little Mac
Billy Lee
Jimmy
Samuel Fisher
James T. Kirk
Jean-Luc Picard
Spock
Leonard McCoy
Montgomery Scott
Hikaru Sulu
Pavel Chekov
William Riker
Deanna Troi
Beverly Crusher
Data
Geordi La Forge
Worf
Kathryn Janeway
Chakotay
Tuvok
Tom Paris
Harry Kim
The Doctor
Seven of Nine
Sephiroth
Cloud Strife
Vincent Valentine
Zack Fair
Tifa Lockhart
Aerith Gainsborough
Barret Wallace
Cait Sith
 
Konami
id Software
Apogee
Sega
Atari
Activision
Nintendo
Sony
Valve
Interplay
Bethesda
Rare
3D Realms
Tiger
Gearbox Software
GT Interactive
Ubisoft
Blizzard
Bungie Studioes
Capcom
Eidos Interactive
Epic Games
LucasArts
Sierra
Square Enix
 
 
Video and challenge idea:
Lightning
 
Lyrics:
Cluster Fuzz
  
Audio:
fuzyll
 
 
Special Thanks To:
Shmoo Group
Potters
 
All of our spouses and family
for putting up with us during
the prep for Ghost in the Shellcode
 
 
 



 
 
 
 
THANK YOU FOR PARTICIPATING
IN THIS
FORENSICS CENTER ACTIVITY!!
 
 
 
 
 
 
"""

fnt = ImageFont.truetype("./Consolas.ttf", 36)

CurFrame = Verses[0][1]
FrameOffset = Verses[0][1]

PrevLines = ""
AllLines = ""
Y = 0
LineStart = 0

for Verse in Verses:
	(VerseData, VerseStartFrame, VersePattern) = Verse
	VerseData = VerseData.split("\n")
	VerseData.pop(0)
	PrevLines = AllLines
	AllLines = ""
	CurLine = ""

	HaveNull = False

	print "Empty frames between verses: %d to %d" % (CurFrame - FrameOffset, VerseStartFrame - FrameOffset)
	img = DoBackground()
	img = DoLyrics(img, PrevLines)
	for i in xrange(CurFrame, VerseStartFrame):
		imgdata = AddLyricCursor(img.copy(), i - FrameOffset, "", Y, LineStart)
		imgdata = AddCredits(imgdata, i - FrameOffset)
		imgdata = AddDots(imgdata, i - FrameOffset)
		WritePic(imgdata, i - FrameOffset)
		sys.stdout.write("%d/%d\r" % (CurFrame, VerseStartFrame))
		sys.stdout.flush()

	CurFrame = VerseStartFrame

	LineStart = 0
	Y = 0
	for CurLineFrames in xrange(0, len(VersePattern), 2):
		StartFrame = VersePattern[CurLineFrames]
		EndFrame = VersePattern[CurLineFrames+1]

		print "Frame %d to %d - %s" % (StartFrame - FrameOffset, EndFrame - FrameOffset, VerseData[CurLineFrames / 2])

		#generate the border and current lyrics
		img = DoBackground()
		img = DoLyrics(img, AllLines)

		for i in xrange(CurFrame, StartFrame):
			imgdata = AddLyricCursor(img.copy(), i - FrameOffset, "", Y, LineStart)
			imgdata = AddCredits(imgdata, i - FrameOffset)
			imgdata = AddDots(imgdata, i - FrameOffset)
			WritePic(imgdata, i - FrameOffset)
			sys.stdout.write("%d/%d\r" % (CurFrame, StartFrame))
			sys.stdout.flush()


		#start adding each char
		CurLine = VerseData[CurLineFrames / 2]
		if(len(VerseData[CurLineFrames / 2]) and VerseData[CurLineFrames / 2][-1] == '\0'):
			CurLine = CurLine[0:-1]
			HaveNull = True
		else:
			HaveNull = False

		AddLine(img, StartFrame - FrameOffset, EndFrame - FrameOffset, Y, CurLine, LineStart)

		#add in our line to the current line list
		AllLines += CurLine
		if(not HaveNull):
			AllLines += "\n"
			LineStart = 0
			Y += 1
		else:
			LineStart += fnt.getsize(CurLine)[0]

		CurFrame = EndFrame

	#sys.exit(0)

print "Last frames for dots"
img = DoBackground()
img = DoLyrics(img, PrevLines)
i = CurFrame

while(1):
	#figure out what part of the data to display. 1 entry per line, 6 pixel scroll per frame
	#calculate how far down into the data to start rendering
	DataStartBlock = (i - FrameOffset) * 6 / (ButtonImage.size[1]-2)
	if (DataStartBlock+22) >= ((len(EmbedData) / 2) + 11):
		break

	imgdata = AddLyricCursor(img.copy(), i - FrameOffset, "", Y, LineStart)
	imgdata = AddCredits(imgdata, i - FrameOffset)
	imgdata = AddDots(imgdata, i - FrameOffset)
	WritePic(imgdata, i - FrameOffset)
	sys.stdout.write("%d\r" % (i))
	sys.stdout.flush()
	i += 1

sys.stdout.write("\n")

"""
img = DoBorders()
img = DoLyrics(img, Data, "1\n2\n3\n4\n5\n6\n")
img = DoPic(img, PortalPic)

imgsolid = Image.new("RGB", (1024, 768), "#9a7e2a")
bg = Image.new("RGB", (1024, 768), 0)
imgmask = Image.eval(img, lambda p: 255*(int(p > 60)))
#imgmask.save("mask.png", "PNG")
#img.save("orig.png", "PNG")
imgcomp = Image.composite(imgsolid, bg, imgmask)
imgcomp.save("test.png","PNG")
"""
