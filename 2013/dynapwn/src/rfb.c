#include <memory.h>
#include "dynacommon.h"
#include <zlib.h>
#include <strings.h>
#include <fcntl.h>

#define RFBVERSION "RFB 003.007\n"
#define CHALLENGENAME "DynaPwn"

#pragma pack(push, 1)
typedef struct PixelDataStruct
{
	unsigned char Padding[3];
	unsigned char BitsPerPixel;
	unsigned char Depth;
	unsigned char BigEndian;
	unsigned char TrueColor;
	unsigned short Red;
	unsigned short Green;
	unsigned short Blue;
	unsigned char RedShift;
	unsigned char GreenShift;
	unsigned char BlueShift;
	unsigned char Padding2[3];
} PixelDataStruct;
#pragma pack(pop)

PixelDataStruct PixelData;
 
z_stream strm;

unsigned short bswap16(unsigned short Data)
{
	return (Data >> 8) | (Data << 8);
}

int InitRFB()
{
	unsigned char Buffer[256];

	memset(&PixelData, 0, sizeof(PixelData));

	/* allocate deflate state */
	memset(&strm, 0, sizeof(strm));
	if(deflateInit2(&strm, 9, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
		return -1;

	//send the protocol version
	sendAll(fd, RFBVERSION, 12);

	//get the client version
	if(recv(fd, Buffer, 12, 0) != 12)
		return -1;

	Buffer[12] = 0;

	if(memcmp(Buffer, RFBVERSION, 12))
		return -1;

	//tell it no security
	sendAll(fd, "\x01\x01", 2);

	if(readAll(fd, Buffer, 1) != 1)
		return -1;

	if(Buffer[0] != 1)
	{
		sendAll(fd, "\x00\x00\x00\x01", 4);
		return -1;
	}

	//get the client init
	if(readAll(fd, Buffer, 1) != 1)
		return -1;

	//respond with server init
	PixelData.BitsPerPixel = 8;
	PixelData.Red = 255;
	PixelData.Green = 255;
	PixelData.Blue = 255;
	PixelData.RedShift = 5;
	PixelData.GreenShift = 0;
	PixelData.BlueShift = 3;
	memset(Buffer, 0, sizeof(Buffer));
	Buffer[0] = DISPLAYWIDTH >> 8;
	Buffer[1] = DISPLAYWIDTH & 0xFF;
	Buffer[2] = DISPLAYHEIGHT >> 8;
	Buffer[3] = DISPLAYHEIGHT & 0xFF;
	Buffer[4] = 32;
	Buffer[5] = 24;
	Buffer[6] = 1;
	Buffer[7] = 1;
	Buffer[9] = 255;
	Buffer[11] = 255;
	Buffer[13] = 255;
	Buffer[14] = 0;
	Buffer[15] = 6;
	Buffer[16] = 11;
	Buffer[23] = sizeof(CHALLENGENAME) - 1;
	memcpy(&Buffer[24], CHALLENGENAME, sizeof(CHALLENGENAME) - 1);

	sendAll(fd, Buffer, 24+sizeof(CHALLENGENAME)-1);
	return 0;
}

int CheckEncodings()
{
	unsigned int Buffer[100];
	unsigned short EncodeCount;

	//figure out if the zlib encoding is allowed, if not, fail

	//get rid of padding
	if(readAll(fd, Buffer, 1) != 1)
		return -1;

	//get the encode count
	if(readAll(fd, &EncodeCount, 2) != 2)
		return -1;

	EncodeCount = (EncodeCount >> 8) | (EncodeCount << 8);
	if(EncodeCount > 100)
		return -1;

	//receive all of the encodings
	readAll(fd, Buffer, EncodeCount*4);
	while(EncodeCount)
	{
		EncodeCount--;
		if(Buffer[EncodeCount] == 0x06000000)
			return 0;		//found zlib encoding
	}

	//no zlib encoding, fail
	return -1;
}

//make sure our out buffer size is slightly bigger than the input buffer
//incase there is no compression at all, set to 101% + a few bytes for header
#define OUTSIZE ((((DISPLAYWIDTH*DISPLAYHEIGHT)+((DISPLAYWIDTH*DISPLAYHEIGHT)/100))*4)+100)
int FrameBufferUpdateRequest(int SkipRead)
{
	//times 4 to allow for 32bits per pixel if so requested
	unsigned char Input[DISPLAYWIDTH*DISPLAYHEIGHT*4];
	unsigned char Output[OUTSIZE+20];
	unsigned int BytesCompressed;
	int i;
	unsigned int r, g, b, fcolor;

	//recv the full request
	if(!SkipRead)
	{
		if(readAll(fd, Output, 9) != 9)
			return -1;
	}

	//if no video update request then ignore the buffer update
	if(Ports[4] == 0)
	{
		PendingFrameBufferUpdate = 1;
		return 0;
	}

	//first part of the video memory
	for(i = 0; i < DISPLAYWIDTH*(DISPLAYHEIGHT/2); i++)
	{
		r = ((VideoMemory[i]) & 0x07) * (PixelData.Red / 7);
		g = ((VideoMemory[i] >> 3) & 0x07) * (PixelData.Green / 7);
		b = ((VideoMemory[i] >> 6) & 0x03) * (PixelData.Blue / 3);
		fcolor = (r << PixelData.RedShift) | (g << PixelData.GreenShift) | (b << PixelData.BlueShift);

		switch(PixelData.BitsPerPixel)
		{
			case 8:
				*(unsigned char *)(&Input[i]) = (unsigned char)fcolor;
				break;
			case 16:
				if(PixelData.BigEndian)
					fcolor = bswap16(fcolor);

				*(unsigned short *)(&Input[i*2]) = (unsigned short)fcolor;
				break;
			case 32:
				if(PixelData.BigEndian)
					fcolor = __builtin_bswap32(fcolor);

				*(unsigned int *)(&Input[i*4]) = fcolor;
				break;
		};
	}

	//account for the bank change
	for(;i < DISPLAYWIDTH*DISPLAYHEIGHT; i++)
	{
		r = (VideoMemory[i+(0x10000-((0x10000/DISPLAYWIDTH)*DISPLAYWIDTH))] & 0x07) * (PixelData.Red / 7);
		g = ((VideoMemory[i+(0x10000-((0x10000/DISPLAYWIDTH)*DISPLAYWIDTH))] >> 3) & 0x07) * (PixelData.Green / 7);
		b = ((VideoMemory[i+(0x10000-((0x10000/DISPLAYWIDTH)*DISPLAYWIDTH))] >> 6) & 0x03) * (PixelData.Blue / 3);
		fcolor = (r << PixelData.RedShift) | (g << PixelData.GreenShift) | (b << PixelData.BlueShift);

		switch(PixelData.BitsPerPixel)
		{
			case 8:
				*(unsigned char *)(&Input[i]) = (unsigned char)fcolor;
				break;
			case 16:
				if(PixelData.BigEndian)
					fcolor = bswap16(fcolor);

				*(unsigned short *)(&Input[i*2]) = (unsigned short)fcolor;
				break;
			case 32:
				if(PixelData.BigEndian)
					fcolor = __builtin_bswap32(fcolor);

				*(unsigned int *)(&Input[i*4]) = fcolor;
				break;
		};
	}

	strm.avail_in = DISPLAYWIDTH * DISPLAYHEIGHT * (PixelData.BitsPerPixel / 8);
	strm.next_in = Input;

	//keep compressing the stream until nothing is left
	strm.avail_out = OUTSIZE;
	strm.next_out = &Output[20];
	strm.data_type = Z_BINARY;

	if(deflate(&strm, Z_SYNC_FLUSH) != Z_OK)
		return -1;

	//calcualte how many bytes were output
	BytesCompressed = OUTSIZE - strm.avail_out;

	memset(Output, 0, 20);
	Output[3] = 1;
	Output[8] = DISPLAYWIDTH >> 8;
	Output[9] = DISPLAYWIDTH & 0xff;
	Output[10] = DISPLAYHEIGHT >> 8;
	Output[11] = DISPLAYHEIGHT & 0xff;
	Output[15] = 6;
	*(unsigned int *)(&Output[16]) = __builtin_bswap32(BytesCompressed);
	sendAll(fd, Output, BytesCompressed + 20);

	//set the flag indicating video update
	Ports[4] = 0;
	PendingFrameBufferUpdate = 0;
	return 0;
}

int KeyEvent()
{
	unsigned char Buffer[200];
	if(readAll(fd, Buffer,7) != 7)
		return -1;

	alarm(120);

	//if key down then pass it along
	if(Buffer[0] && (Buffer[6] < 0x7f))
		Ports[1] = Buffer[6];
	PauseCPU = 0;
	return 0;
}

int PointerEvent()
{
	char Buffer[200];
	if(readAll(fd, Buffer,5) != 5)
		return -1;

	alarm(120);

	//store the x and y coords
	//top bits of y is also mouse button down flag
	//as y is 0 to 203
	Ports[2] = ((unsigned short)Buffer[2] << 8) | Buffer[1];
	Ports[3] = ((unsigned short)Buffer[0] << 8) | Buffer[3];
	PauseCPU = 0;
	return 0;
}

int SetPixelFormat()
{
	if(readAll(fd, &PixelData,sizeof(PixelData)) != 19)
		return -1;

	PixelData.Red = bswap16(PixelData.Red);
	PixelData.Green = bswap16(PixelData.Green);
	PixelData.Blue = bswap16(PixelData.Blue);
	return 0;
}

int ClientCutText()
{
	char Buffer[65536];
	char OutBuffer[65536];

	unsigned int DataLen;
	int RecvLen;
	z_stream appstrm;

	if(readAll(fd, Buffer, 3) != 3)
		return -1;

	if(readAll(fd, &DataLen, 4) != 4)
		return -1;

	DataLen = __builtin_bswap32(DataLen);
	if((DataLen > sizeof(Buffer)) || !AllowFileUpload || (Ports[6] == 0))
	{
		//just receive then exit
		while(DataLen)
		{
			if(DataLen > sizeof(Buffer))
				RecvLen = sizeof(Buffer);
			else
				RecvLen = DataLen;

			if(readAll(fd, Buffer, RecvLen) != RecvLen)
				return -1;

			DataLen -= RecvLen;
		};
		return 0;
	}

	//reset the flag just incase
	AllowFileUpload = 0;

	if(readAll(fd, Buffer, DataLen) != DataLen)
		return -1;

	//decompress the data, if it is too large, bail and don't let it execute
	memset(&appstrm, 0, sizeof(appstrm));
	if(inflateInit(&appstrm) != Z_OK)
		return -1;

	appstrm.avail_in = DataLen;
	appstrm.next_in = Buffer;

	//Ports[6] is where to upload to so make sure we don't go outside of our boundary
	appstrm.avail_out = (unsigned int)65535-(unsigned int)(Ports[6]);
	appstrm.next_out = OutBuffer;
	appstrm.data_type = Z_BINARY;

	if(inflate(&appstrm, Z_FINISH) != Z_STREAM_END)
		return -1;

	memcpy(&ProgramMemory[(unsigned short)Ports[6]], OutBuffer, (65535-(unsigned short)Ports[6]) - appstrm.avail_out);
	inflateEnd(&appstrm);

	//indicate we uploaded data
	Ports[6] = 0;

	//reset the dynarec buffers if in use due to possibly overwriting used data
	ResetDynarec();
	return 0;
}

int RFBSendFile(unsigned int FileHandle)
{
	unsigned int Buffer[2];
	unsigned int DataLen;
	unsigned char *DataPtr;
	unsigned char CompressBuffer[65536];
	z_stream appstrm;

	///int FileHandle = 0;

	Buffer[0] = 3;
	if(FileHandle == -1)
		return 0;

	DataLen = lseek(FileHandle, 0, SEEK_END);
	lseek(FileHandle, 0, SEEK_SET);
	DataPtr = (unsigned char *)malloc(DataLen);
	read(FileHandle, DataPtr, DataLen);
	lseek(FileHandle, 0, SEEK_SET);

	//compress the data, if it is too large, bail and don't let it execute
	memset(&appstrm, 0, sizeof(appstrm));
	if(deflateInit(&appstrm, 9) != Z_OK)
	{
		free(DataPtr);
		return -1;
	}

	appstrm.avail_in = DataLen;
	appstrm.next_in = DataPtr;

	appstrm.avail_out = sizeof(CompressBuffer);
	appstrm.next_out = &CompressBuffer;
	appstrm.data_type = Z_BINARY;
	if(deflate(&appstrm, Z_FINISH) != Z_STREAM_END)
	{
		free(DataPtr);
		return -1;
	}

	DataLen = sizeof(CompressBuffer) - appstrm.avail_out;
	Buffer[1] = __builtin_bswap32(DataLen + 4);
	sendAll(fd, Buffer, sizeof(Buffer));
	sendAll(fd, "zlib", 4);
	sendAll(fd, CompressBuffer, DataLen);

	deflateEnd(&appstrm);
	free(DataPtr);

	return 0;
}

int HandleRFB()
{
	unsigned char Cmd;
	unsigned char Buffer[4096];

	//get a byte and see what function to pass to
	if(recv(fd, &Cmd, 1, 0) != 1)
		return -1;

	switch(Cmd)
	{
		case 0:
			return SetPixelFormat();
		case 2:
			return CheckEncodings();
		case 3:
			return FrameBufferUpdateRequest(0);
		case 4:
			return KeyEvent();
		case 5:
			return PointerEvent();

		case 6:
			return ClientCutText();

		default:
			return recv(fd, Buffer, 4096, 0);
	};

	return 0;
}
