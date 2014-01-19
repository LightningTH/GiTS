import Image, ImageDraw, ImageFont, ImageOps
import StringIO
import sys
import os
import hashlib, random, struct, zlib

Framrate = 30
FramesPerCursor = [20, 10]	#total/on count

def DoBorders():
	global size, fnt

	#data = "-"*49 + "  " + "-"*49 + " \n"
	data = ""
	for i in xrange(0, 15):
		data += "|" + " "*48 + "||" + " "*49 + "|\n"

	data += "|" + " "*48 + "||" + " "*49 + "|\n"

	for i in xrange(0, 13):
		data += "|" + " "*48 + "|\n"

	text = data.split("\n")
	(textx, texty) = fnt.getsize(text[0])
	texty += 5
	img = Image.new('L', size, 0)
	draw = ImageDraw.Draw(img)

	y = 9
	draw.text((6, y), "-"*49 + "  " + "-"*49, font=fnt, fill="#ffffff")
	y += texty

	for Entry in text:
		draw.text((6, y), Entry, font=fnt, fill="#ffffff")
		draw.text((7, y), Entry, font=fnt, fill="#ffffff")
		y += texty

	draw.text((6, y), "-"*49, font=fnt, fill="#ffffff")

	draw = ImageDraw.Draw(img)
	draw.text((7, texty*16 + 15), " "*51 + "-"*49, font=fnt, fill="#ffffff")

	#draw.text((7, texty*16 + 15), " "*51 + "-"*49, font=fnt, fill="#ffffff")
	#draw.text((8, texty*16 + 15), " "*51 + "-"*49, font=fnt, fill="#ffffff")
	del draw 

	return img

def DoLyrics(img, lyrics, sidetext):
	global size, fnt

	lyrics = lyrics.split("\n")
	(textx, texty) = fnt.getsize("X")

	lyrics += [""]*(30-len(lyrics))

	sidetext = sidetext.split("\n")
	sidetext = [""]*(15-len(sidetext)) + sidetext

	texty += 5
	draw = ImageDraw.Draw(img)                     # setup to draw on the main image

	y = 11 + texty - 5
	for Entry in lyrics:
		draw.text((6+textx + 3, y), Entry, font=fnt, fill="#ffffff")
		draw.text((7+textx + 3, y), Entry, font=fnt, fill="#ffffff")
		y += texty


	y = 11 + texty + 5
	for Entry in sidetext:
		draw.text((6+textx*52, y), Entry, font=fnt, fill="#ffffff")
		draw.text((7+textx*52, y), Entry, font=fnt, fill="#ffffff")
		y += texty


	del draw 
	
	return img

picfnt = ImageFont.truetype("./cour.ttf", 40)
def AddPic(img, FrameNum):
	global fnt, picfnt, Pics, FrameOffset

	if(FrameNum < (Pics[0][0] - FrameOffset)):
		return img

	if(FrameNum >= (Pics[1][0] - FrameOffset)):
		Pics.pop(0)

	picdata = Pics[0][1].split("\n")
	picdata.pop(0)
	(textx, texty) = fnt.getsize("X")
	(pictextx, pictexty) = picfnt.getsize("X")

	
	texty += 5
	picimg = Image.new('L', (pictextx*41 + 1, pictexty*20), 0)
	draw = ImageDraw.Draw(picimg)                     # setup to draw on the main image

	y = 0 #+ texty*16
	for Entry in picdata:
		x = 0
		for Char in Entry:
			draw.text((x, y), Char, font=picfnt, fill="#ffffff")
			if (((ord(Char) | 0x20) > 0x61) and ((ord(Char) | 0x20) < 0x7a)):
				draw.text((x, y+1), Char, font=picfnt, fill="#ffffff")
				draw.text((1+x, y), Char, font=picfnt, fill="#ffffff")
				draw.text((1+x, y+1), Char, font=picfnt, fill="#ffffff")
			x += pictextx

		y += pictexty

	del draw 

	picimg = picimg.resize((410, 360), Image.BICUBIC)
	picimg2 = Image.new('L', size, 0)
	(picimgx, picimgy) = picimg.size
	img.paste(picimg, (textx*56 + 3, texty*17 + 2))

	#draw = ImageDraw.Draw(img)
	#draw.text((7, texty*16 + 15), " "*51 + "-"*49, font=fnt, fill="#ffffff")
	#draw.text((8, texty*16 + 15), " "*51 + "-"*49, font=fnt, fill="#ffffff")
	#del draw

	return img

def WritePic(img, FrameNum):
	imgsolid = Image.new("RGB", (1024, 768), "#9a7e2a")
	bg = Image.new("RGB", (1024, 768), 0)
	imgmask = Image.eval(img, lambda p: 255*(int(p > 60)))
	#imgmask.save("mask.png", "PNG")
	#img.save("orig.png", "PNG")
	imgcomp = Image.composite(imgsolid, bg, imgmask)
	imgcomp.save("images/frame%03d.png" % (FrameNum), "PNG")

def AddLyricCursor(img, FrameNum, X, Y):
	global FramesPerCursor, size, fnt

	if(FrameNum % FramesPerCursor[0]) < FramesPerCursor[1]:
		(textx, texty) = fnt.getsize("X")

		texty += 5
		y = 11 + texty - 5 + (texty * Y) + 2

		draw = ImageDraw.Draw(img)                     # setup to draw on the main image
		draw.text((6 + (X * textx) + 3, y), "_", font=fnt, fill="#ffffff")
		draw.text((7 +  (X * textx) + 3, y), "_", font=fnt, fill="#ffffff")
	
	return img

def AddLine(img, StartFrame, EndFrame, LineNum, VerseLine, StartPos = 0):
	global size, fnt

	#when called we should be on the startframe

	#if the line is empty then just do the frames we have
	if len(VerseLine) == 0:
		for CurFrame in xrange(StartFrame, EndFrame + 1):
			imgdata = img.copy()
			imgdata = AddLyricCursor(imgdata, CurFrame, 1, LineNum)
			imgdata = AddCredits(imgdata, CurFrame)
			imgdata = AddPic(imgdata, CurFrame)
			WritePic(imgdata, CurFrame)
		return

	#calculate number of frames per char for the line
	FramesPerChar = float(EndFrame - StartFrame) / float(len(VerseLine))
	#print "Frames per char: %f" % (FramesPerChar)

	(textx, texty) = fnt.getsize("X")

	"""
	sidetext = sidetext.split("\n")
	sidetext = [""]*(15-len(sidetext)) + sidetext
	"""

	texty += 5
	y = 11 + texty - 5 + (texty * LineNum)
	x = 6 + textx + 3 + (StartPos * textx)

	for CurFrame in xrange(StartFrame, EndFrame + 1):
		#print "Frame %d" % (CurFrame)
		imgdata = img.copy()
		draw = ImageDraw.Draw(imgdata)                     # setup to draw on the main image

		NumChars = int((CurFrame - StartFrame) / FramesPerChar)
		draw.text((x, y), VerseLine[0:NumChars], font=fnt, fill="#ffffff")
		draw.text((x + 1, y), VerseLine[0:NumChars], font=fnt, fill="#ffffff")
		imgdata = AddLyricCursor(imgdata, CurFrame, NumChars + 1 + StartPos, LineNum)
		imgdata = AddCredits(imgdata, CurFrame)
		imgdata = AddPic(imgdata, CurFrame)
		WritePic(imgdata, CurFrame)

	"""
	y = 11 + texty + 5
	for Entry in sidetext:
		draw.text((6+textx*52, y), Entry, font=fnt, fill="#ffffff")
		draw.text((7+textx*52, y), Entry, font=fnt, fill="#ffffff")
		y += texty
	"""

	del draw 

def AddCreditCursor(img, FrameNum, X, Y):
	global FramesPerCursor, size, fnt

	if(FrameNum % FramesPerCursor[0]) >= FramesPerCursor[1]:
		(textx, texty) = fnt.getsize("X")

		texty += 5
		y = 11 + texty - 5 + (texty * Y) + 4

		draw = ImageDraw.Draw(img)                     # setup to draw on the main image
		draw.text((6 + ((X + 52) * textx), y), "_", font=fnt, fill="#ffffff")
		draw.text((7 +  ((X + 52) * textx), y), "_", font=fnt, fill="#ffffff")
	
	return img

def AddCredits(img, FrameNum):
	global size, fnt, Credits, FrameOffset


	(textx, texty) = fnt.getsize("X")

	FrameVal = FrameNum - (1744 - FrameOffset)
	if(FrameVal < 0):
		return AddCreditCursor(img, FrameNum, 0, 14)

	CreditLines = [""]*19 + Credits.split("\n")

	draw = ImageDraw.Draw(img)
	y = 11 + texty + 5

	CurLine = 0
	while(FrameVal >= (len(CreditLines[CurLine]) * 2)):
		FrameVal = FrameVal - (len(CreditLines[CurLine]) * 2)

		CurLine += 1
		if CurLine >= len(CreditLines):
			FrameNum = len(CreditLines[-1]) * 2
			CurLine = len(CreditLines) - 1
			break

	for Entry in xrange(CurLine - 18, CurLine):
		draw.text((6+textx*52, y), CreditLines[Entry], font=fnt, fill="#ffffff")
		draw.text((7+textx*52, y), CreditLines[Entry], font=fnt, fill="#ffffff")
		y += texty

	draw.text((6+textx*52, y), CreditLines[CurLine][0:FrameVal / 2], font=fnt, fill="#ffffff")
	draw.text((7+textx*52, y), CreditLines[CurLine][0:FrameVal / 2], font=fnt, fill="#ffffff")

	del draw
	if(len(CreditLines[CurLine][0:FrameVal / 2].strip()) == 0):
		return AddCreditCursor(img, FrameNum, 0, 14)	
	else:
		return AddCreditCursor(img, FrameNum, FrameVal / 2, 14)


PortalPic = """
              .,-:;//;:=,
          . :H@@@MM@M#H/.,+%;,
       ,/X+ +M@@M@MM%=,-%HMMM@X/,
     -+@MM; $M@@MH+-,;XMMMM@MMMM@+-
    ;@M@@M- XM@X;. -+XXXXXHHH@M@M#@/.
  ,%MM@@MH ,@%=            .---=-=:=,.
  =@#@@@MX .,              -%HX$$%%%+;
 =-./@M@M$                  .;@MMMM@MM:
 X@/ -$MM/                    .+MM@@@M$
,@M@H: :@:                    . =X#@@@@-
,@@@MMX, .                    /H- ;@M@M=
.H@@@@M@+,                    %MM+..%#$.
 /MMMM@MMH/.                  XM@MH; =;
  /%+%$XHH@$=              , .H@@@@MX,
   .=--------.           -%H.,@@@@@MX,
   .%MM@@@HHHXX$$$%+- .:$MMX =M@@MM%.
     =XMMM@MM@MM#H;,-+HMM@M+ /MMMX=
       =%@M@M#@$-.=$@MM@@@M; %M%=
         ,:+$+-,/H#MMMMMMM@= =,
               =++%%%%+/:-.
"""

HazardPic = """
             =+$HM####@H%;,
          /M###############M$,
          ,@################+
           .H##############+
             X############/
              $##########/
               %########/
                /X/;;+X/

                 -XMMX-
                ,######,
#############X  .M####M.  X#############
##############-   -//-   -##############
X##############%,       ,+#############X
-##############X         X#############-
 %############%           %###########%
  %##########;             ;#########%
   ;#######M=               =M######;
    .+M###@.                 .@###M+.
       :XM.                   .MX:
"""

ProtonPic = """
                 =/;;/-
                +:    //
               /;      /;
              -X        H.
.//;;;:;;-,   X=        :+   .-;:=;:;%;.
M-       ,=;;;#:,      ,:#;;:=,       ,@
:%           :%.=/++++/=.$=           %=
 ,%;         %/:+/;,,/++:+/         ;+.
   ,+/.    ,;@+,        ,%H;,     ,/+,
      ;+;;/= @.  .H##X   -X :///+;
      ;+=;;;.@,  .XM@$.  =X.//;=%/.
   ,;:      :@%=        =$H:     .+%-
 ,%=         %:-///==///-//         =%,
;+           :%-;;;:;;;;-X-           +:
@-      .-;;;;M-        =M/;;;-.      -X
 :;;::;;-.    %-        :+    ,-;;-;:==
              ,X        H.
               ;/      %=
                //    +;
                 ,////,
"""

BrokenHeartPic = """
                          .,---.
                        ./XM#MMMX;,
                      -%##########M%,
                     -@######%  $###@=
      .,--,         -H#######$   $###M:
   ,;$M###MMX;     .;##########$;HM###X=
 ,/@##########H=      ;################+
-+#############M/,      %##############+
%M###############=      /##############:
M################      .M#############;.
@###############M      ,@###########M:.
X################,      -$=X#######@:
/@##################%-     +######$-
.;##################X     .X#####+,
 .;H################/     -X####+.
   ,;X##############,       .MM/
      ,:+$H@M#######M#$-    .$$=
           .,-=;+$@###X:    ;/=.
                  .,/X$;   .::,
                      .,    ..
"""

ExplosionPic = """
            .+
             /M;
              H#@:              ;,
              -###H-          -@/
               %####$.  -;  .%#X
                M#####+;#H :M#M.
..          .+/;%#########X###-
 -/%H%+;-,    +##############/
    .:$M###MH$%+############X  ,--=;-
        -/H#####################H+=.
           .+#################X.
         =%M####################H;.
            /@###############+;;/%%;,
         -%###################$.
       ;H######################M=
    ,%#####MH$%;+#####M###-/@####%
  :$H%+;=-      -####X.,H#   -+M##@-
 .              ,###;    ;      =$##+
                .#H,               :XH,
                 +                   .;-
"""

FirePic = """
                     -$-
                    .H##H,
                   +######+
                .+#########H.
              -$############@.
            =H###############@  -X:
          .$##################:  @#@-
     ,;  .M###################;  H###;
   ;@#:  @###################@  ,#####:
 -M###.  M#################@.  ;######H
 M####-  +###############$   =@#######X
 H####$   -M###########+   :#########M,
  /####X-   =########%   :M########@/.
    ,;%H@X;   .$###X   :##MM@%+;:-
                 ..
  -/;:-,.              ,,-==+M########H
 -##################@HX%%+%%$%%%+:,,
    .-/H%%%+%%$H@###############M@+=:/+:
/XHX%:#####MH%=    ,---:;;;;/%%XHM,:###$
$@#MX %+;-
"""

CheckmarkPic = """
                                     :X-
                                  :X###
                                ;@####@
                              ;M######X
                            -@########$
                          .$##########@
                         =M############-
                        +##############$
                      .H############$=.
         ,/:         ,M##########M;.
      -+@###;       =##########M;
   =%M#######;     :#########M/
-$M###########;   :#########/
 ,;X###########; -########$.
     ;H#########+#######M=
       ,+##############+
          /M#########@-
            ;M######%
              +####:
                $M-
"""

BlackMesaPic = """
           .-;+$XHHHHHHX$+;-.
        ,;X@@X%/;=----=:/%X@@X/,
      =$@@%=.               =+H@X:
    -XMX:                     =XMX=
   /@@:                         =H@+
  %@X,                           .$@$
 +@X.                              $@%
-@@,                               .@@=
%@%                                 +@$
H@:                                 :@H
H@:          :HHHHHHHHHHHHHHHHX,    =@H
%@%          ;@M@@@@@@@@@@@@@@@H-   +@$
=@@,         :@@@@@@@@@@@@@@@@@@@= .@@:
 +@X         :@@@@@@@@@@@@@M@@@@@@:%@%
  $@$,       ;@@@@@@@@@@@@@@@M@@@@@@$.
   +@@HHHHHHHH@@@@@@@@@@@@@@@@@@@@@+
    =X@@@@@@@@@@@@@@@@@@@@@@@@@@@X=
      :$@@@@@@@@@@@@@@@@@@@@@@@$:
        ,;$@@@@@@@@@@@@@@@@@X/-
           -;+$XXHHHHHX$+;-
"""

CakePic = """
            ,:/+/-
            /M/              .,-=;//;-
       .:/= ;MH/,    ,=/+%$XH@MM#@:
      -$##@+$###@H@MMM#######H:.    -/H#
 .,H@H@ X######@ -H#####@+-     -+H###@X
  .,@##H;      +XM##M/,     =%@###@X;-
X%-  :M##########$.    .:%M###@%:
M##H,   +H@@@$/-.  ,;$M###@%,          -
M####M=,,---,.-%%H####M$:          ,+@##
@##################@/.         :%H##@$-
M###############H,         ;HM##M$=
#################.     =$M##M$=
################H..;XM##M$=          .:+
M###################@%=           =+@MH%
@################M/.          =+H#X%=
=+M##############M,       -/X#X+;,
  .;XM##########H=    ,/X#H+:,
     .=+HM######M+/+HM@+=.
         ,:/%XM####H/.
              ,.:=-.
"""

GladosPic = """
       #+ @      # #              M#@
      .X  X.%##@;# #   +@#######X. @#%
   ,==.   ,######M+  -#####%M####M-    #
  :H##M%:=##+ .M##M,;#####/+#######% ,M#
 .M########=  =@#@.=#####M=M#######=  X#
 :@@MMM##M.  -##M.,#######M#######. =  M
             @##..###:.    .H####. @@ X,
   ############: ###,/####;  /##= @#. M
           ,M## ;##,@#M;/M#M  @# X#% X#
.%=   ######M## ##.M#:   ./#M ,M #M ,#$
##/         $## #+;#: #### ;#/ M M- @# :
#+ #M@MM###M-;M #:$#-##$H# .#X @ + $#. #
      ######/.: #%=# M#:MM./#.-#  @#: H#
+,.=   @###: /@ %#,@  ##@X #,-#@.##% .@#
#####+;/##/ @##  @#,+       /#M    . X,
   ;###M#@ M###H .#M-     ,##M  ;@@; ###
   .M#M##H ;####X ,@#######M/ -M###$  -H
    .M###%  X####H  .@@MM@;  ;@#M@
      H#M    /@####/      ,++.  / ==-,
                -/;   +X@MMH@#H  #####$-
"""

TVPic = """
           @@              @@
            \\            //
              \\        //
               \\      //
                 \\  //
                  \\//
  %##################################%
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+
  +##                              ##+  
  +##                              ##+
  +##                              ##+
  %##################################%
"""

Pics = [
	[2163, PortalPic],
	[2530, HazardPic],
	[2594, PortalPic],
	[2828, ProtonPic],
	[2947, PortalPic],
	[3565, BrokenHeartPic],
	[3710, ExplosionPic],
	[3911, FirePic],
	[4083, CheckmarkPic],
	[4384, ExplosionPic],
	[4448, ProtonPic],
	[4509, PortalPic],
	[5293, BlackMesaPic],
	[5593, CakePic],
	[5715, GladosPic],
	[5773, TVPic],
	[5836, PortalPic],
	[5952, ProtonPic],
	[6012, ExplosionPic],
	[6073, PortalPic],
	[99999999, PortalPic]
]

#layout (repeated)
#<string>, num frames to write string in, num frames of pause until next line

Verse1 =  """
Forms FORM-29827281-12:
Test Assessment Report


This is a challenge.
I'm giving advice here:
MOVE ALONG.
It's hard to overstate
my difficulty.
Ghost in the Shellcode
We do what we want
because you can't.
Many lols for all of us.
Except the ones who will fail.

But there's no sense pouring
over all of the lines.
You just keep on trying
till you run out of time.
And some hacking gets done.
And you have some good fun.
But you're one of the ones
who will fail.
"""

Verse1StartFrame = 1468

#first frame char is seen
#last frame text is seen
#<repeat pattern>
Verse1Pattern = [
1471, 1530,
1531, 1590,
1591, 1591,
1591, 1591,
1681, 1741,
1799, 1861,
1867, 1918,
1950, 2018,
2023, 2100,
2167, 2217,
2282, 2327,
2335, 2380,
2433, 2522,
2532, 2583,
2585, 2593,
2596, 2651,
2653, 2705,
2712, 2770,
2772, 2823,
2830, 2888,
2890, 2947,
2949, 2992,
2996, 3038,
3040, 3082
]

Verse2 = """
Forms FORM-55551-5:
Personnel File Addendum:

Dear <<Subject Name Here>>,

Please don't be angry.
I'm being so sincere right now.
Even though you don't know
how to solve me.
So tear me to pieces.
And run every piece through IDA Pro.
As you work I laugh because
this is so futile for you!
Now these points I'm worth
make a beautiful score.
But you can't have them
unless you do a lot more.
You could be in the lead
Think how great that would be.
But you're one of the ones
who will fail.
"""

Verse2StartFrame = 3083
Verse2Pattern = [
3085, 3102,
3103, 3135,
3140, 3155,
3157, 3226,
3227, 3227,
3229, 3285,
3357, 3457,
3509, 3610,
3612, 3651,
3712, 3761,
3831, 3938,
3989, 4074,
4085, 4133,
4148, 4208,
4210, 4267,
4268, 4327,
4329, 4385,
4387, 4449,
4450, 4510,
4511, 4556,
4557, 4611,
]

Verse3 = """
Forms FORM-55551-6:
Personnel File Addendum Addendum:

One last thing:

Go ask for more hints.
Check meta data then despair.
Maybe go find someone else
to help you.
Maybe PPP\0
.\0
.\0
.
THAT WAS A JOKE.\0
 THEY'RE STUMPED.
Anyway, these points are great.
But you won't get them of course.
Look at me still talking
when there's hacking to do.
When I see the scores,
it makes me glad I'm not you.
There's reversing to be done.
And level 16 to be run.
For the people who still
haven't failed.
"""

Verse3StartFrame = 4656
Verse3Pattern = [
4659, 4673,
4673, 4712,
4713, 4730,
4735, 4798,
4799, 4799,
4800, 4858,
4912, 5026,
5073, 5173,
5176, 5211,
5280, 5334,
5349, 5350,	#dot
5366, 5367,	#dot
5381, 5382,	#dot
5401, 5442,	#pause after JOKE.
5474, 5507,
5555, 5646,
5648, 5704,
5718, 5774,
5775, 5837,
5838, 5888,
5889, 5953,
5955, 6013,
6015, 6074,
6075, 6123,
6124, 6176
]

Verse4 = """


PS: And believe me you are
gonna fail.
PPS: We're all still laughing and you're
gonna fail.
PPPS: \0
The clock is ticking and you're
gonna fail.

FINAL THOUGHT:
Somebody solved it but you're
gonna fail.

FINAL THOUGHT PS:
You're wasting hours but you're
gonna fail.


GONNA FAIL
"""

Verse4StartFrame = 6182
Verse4Pattern = [
6187, 6187,
6187, 6187,
6188, 6244,
6246, 6279,
6303, 6365,
6367, 6398,
6422, 6428,
6429, 6485,
6486, 6517,
6517, 6520,
6522, 6552,
6555, 6602,
6605, 6640,
6641, 6641,
6643, 6666,
6669, 6723,
6724, 6758,
6759, 6770,
6771, 6784,
6785, 6819
]

Verse5 = ""
Verse5StartFrame = 6825
Verse5Pattern = []

FinalFrame = 6928

Verses = [
[Verse1, Verse1StartFrame, Verse1Pattern],
[Verse2, Verse2StartFrame, Verse2Pattern],
[Verse3, Verse3StartFrame, Verse3Pattern],
[Verse4, Verse4StartFrame, Verse4Pattern],
[Verse5, Verse5StartFrame, Verse5Pattern],
["", FinalFrame, ""]
]


Credits = """
>LIST PERSONNEL
 
 
Lightning
Psifertex
Cluster Fuzz
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
 
Voices:
Phoenix Kiana
 
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

size = (1024, 768) #(textx + 20, (texty * len(text)))
fnt = ImageFont.truetype("./cour.ttf", 16)

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
	img = DoBorders()
	img = DoLyrics(img, PrevLines, "")
	for i in xrange(CurFrame, VerseStartFrame):
		imgdata = img.copy()
		imgdata = AddLyricCursor(img.copy(), i - FrameOffset, LineStart + 1, Y)
		imgdata = AddCredits(imgdata, i - FrameOffset)
		imgdata = AddPic(imgdata, i - FrameOffset)
		WritePic(imgdata, i - FrameOffset)

	CurFrame = VerseStartFrame

	LineStart = 0
	Y = 0
	for CurLineFrames in xrange(0, len(VersePattern), 2):
		StartFrame = VersePattern[CurLineFrames]
		EndFrame = VersePattern[CurLineFrames+1]

		print "Frame %d to %d - %s" % (StartFrame - FrameOffset, EndFrame - FrameOffset, VerseData[CurLineFrames / 2])

		#generate the border and current lyrics
		img = DoBorders()
		img = DoLyrics(img, AllLines, "")

		for i in xrange(CurFrame, StartFrame):
			imgdata = img.copy()
			imgdata = AddLyricCursor(img.copy(), i - FrameOffset, LineStart + 1, Y)
			imgdata = AddCredits(imgdata, i - FrameOffset)
			imgdata = AddPic(imgdata, i - FrameOffset)
			WritePic(imgdata, i - FrameOffset)

		#start adding each char
		CurLine = VerseData[CurLineFrames / 2]
		if(len(VerseData[CurLineFrames / 2]) and VerseData[CurLineFrames / 2][-1] == '\0'):
			CurLine = CurLine[0:-1]
			HaveNull = True
		else:
			HaveNull = False

		AddLine(img.copy(), StartFrame - FrameOffset, EndFrame - FrameOffset, Y, CurLine, LineStart)

		#add in our line to the current line list
		AllLines += CurLine
		if(not HaveNull):
			AllLines += "\n"
			LineStart = 0
			Y += 1
		else:
			LineStart += len(CurLine)

		CurFrame = EndFrame
