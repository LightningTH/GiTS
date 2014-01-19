#!/usr/bin/python

import Image, ImageDraw, ImageFont
import StringIO
import sys
import os
import hashlib, random, struct, zlib

fontpixels = Image.open("BigFont.png")

def txt2img(text,bg="#ffffff",fg="#000000",font="Ubuntu-R.ttf",FontSize=50):
	font_dir = "/usr/share/fonts/truetype/ubuntu-font-family/"
	font_size = FontSize
	fnt = ImageFont.truetype(font_dir+font, font_size)
	lineWidth = 40
	img = Image.new('RGBA', (320, 200), "#000000")
	imgbg = Image.new('RGBA', img.size, "#000000") # make an entirely black image
	mask = Image.new('L',img.size,"#000000")       # make a mask that masks out all
	draw = ImageDraw.Draw(img)                     # setup to draw on the main image
	drawmask = ImageDraw.Draw(mask)                # setup to draw on the mask
	drawmask.line((0, lineWidth/2, img.size[0],lineWidth/2),
		  fill="#999999", width=40)        # draw a line on the mask to allow some bg through
	img.paste(imgbg, mask=mask)                    # put the (somewhat) transparent bg on the main

	textlines = text.split("\n")
	(linewidth, lineheight) = draw.textsize(textlines[0], font=fnt)
	linewidth = linewidth / len(textlines[0])
	startpos = (200 - (lineheight*len(textlines))) / 2
	for i in xrange(len(textlines)):
		draw.text(((320 - (linewidth*len(textlines[i])))/2,startpos + (i*lineheight)), textlines[i], font=fnt, fill=bg)      # add some text to the main
	del draw 

	img.save("aif.jpg")
	return img

def getaifdata(img, skipcount):
	img_buf = list(img.getdata())

	c = 0
	d = 0
	data = [""]*200
	for i in img_buf:
		(r,g,b,a) = i
		if r != 0:
			data[d] += "*"
		else:
			data[d] += " "
		c += 1
		if c == 320:
			d += 1
			c = 0

	outdata = "APERTURE IMAGE FORMAT (c) 1985\n\n\r%d\n\n\r" % (skipcount)

	i = 199
	startline = 199
	dataline = ""
	while(data[i] != ""):
		dataline += data[i]
		data[i] = ""
		i -= skipcount

		if i < 0:
			startline -= 1
			i = startline

	c = 1
	x = 0
	for i in xrange(len(dataline) - 1):
		if (x == 0) and (dataline[i] == dataline[i+1]):
			c += 1
		else:
			outdata += chr(c + 32)
			c = 1
			x = 0

		if(c == (126-32)):
			outdata += chr(c + 32)
			c = 0
			x = 1

	if (x == 0) and (dataline[len(dataline) - 2] == dataline[len(dataline) - 1]):
		c += 1
	else:
		outdata += chr(c + 32)
		c = 0
		
	outdata += chr(c + 32)
	return outdata

def writechar(img, char, imgx, imgy, rgb):
	char = ord(char) - 32
	down = int(char / 16)
	across = char % 16
	for y in xrange(16):
		line = ""
		for x in xrange(16):
			(r,g,b) = fontpixels.getpixel(((across*16)+x,(down*16+y)))
			if b != 0:
				(r, g, b) = img.getpixel(((imgx+x),(imgy+y)))
				if rgb == 0:
					r = 0xff
				elif rgb == 1:
					g = 0xff
				else:
					b = 0xff 
				img.putpixel(((imgx+x),(imgy+y)), (r, g, b))
	return

def writestring(img, data, x, y, rgb):
	(imgsizex, imgsizey) = img.size
	for c in data:
		writechar(img, c, x, y, rgb)
		x += 16
		if x >= imgsizex:
			x = 0
			y += 16

if __name__ == "__main__":

	answer = "Cave Johnson\nhates\nBlack Mesa"

	#get the image and convert to aif format
	aifdata = getaifdata(txt2img(answer), 42)
	open("answer.aif","w").write(aifdata)

	#create a new image for the sstv format
	img = Image.new('RGB', (640, 480), "#000000")
	
	#parse up and write the data
	aifdata = aifdata.split("\n\n\r")

	writestring(img, aifdata[0], 0, 0, 0)

	rgb = 0
	charsperline = 640/16
	#account for the header
	y = 1
	for i in xrange(0, len(aifdata[2]), charsperline):
		writestring(img, aifdata[2][i:i+charsperline], 0, y*16, rgb)
		if((y+1)*16 >= 480):
			rgb += 1
			y = 0
		else:
			y += 1

	img.save("answer.bmp")

