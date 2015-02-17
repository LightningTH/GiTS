#include "dynacommon.h"
#include <fcntl.h>

void Int_DoPush(int r1, int unused)
{
	if(StackSize >= ((0x400 - (16*2)) / 2))
		return;

	StackPos[StackSize] = Registers[r1];
	StackSize++;
	return;
}

void Int_DoPop(int r1, int unused)
{
	if(StackSize <= 0)
		return;

	StackSize--;
	Registers[r1] = StackPos[StackSize];
	return;
}

void Int_DoAdd(int r1, int r2)
{
	Registers[r1] = Registers[r1] + Registers[r2];
	return;
}

void Int_DoSub(int r1, int r2)
{
	Registers[r1] = Registers[r1] - Registers[r2];
	return;
}

void Int_DoMul(int r1, int r2)
{
	Registers[r1] = Registers[r1] * Registers[r2];
	return;
}

void Int_DoDiv(int r1, int r2)
{
	Registers[r1] = Registers[r1] / Registers[r2];
	return;
}

void Int_DoMod(int r1, int r2)
{
	Registers[r1] = Registers[r1] % Registers[r2];
	return;
}

void Int_DoAnd(int r1, int r2)
{
	Registers[r1] = Registers[r1] & Registers[r2];
	return;
}

void Int_DoOr(int r1, int r2)
{
	Registers[r1] = Registers[r1] | Registers[r2];
	return;
}

void Int_DoShl(int r1, int r2)
{
	Registers[r1] = Registers[r1] << Registers[r2];
	return;
}

void Int_DoShr(int r1, int r2)
{
	Registers[r1] = Registers[r1] >> Registers[r2];
	return;
}

void Int_DoRol(int r1, int r2)
{
	unsigned short RolAmount = Registers[r2] & 0xf;

	Registers[r1] = ((Registers[r1] << RolAmount) & 0xffff) | ((Registers[r1] >> (16-RolAmount)) & 0xffff);
	return;
}

void Int_DoRor(int r1, int r2)
{
	unsigned short RorAmount = Registers[r2] & 0xf;
	Registers[r1] = ((Registers[r1] >> RorAmount) & 0xffff) | ((Registers[r1] << (16-RorAmount)) & 0xffff);
	return;
}

void Int_DoXor(int r1, int r2)
{
	Registers[r1] = Registers[r1] ^ Registers[r2];
	return;
}

void Int_DoNot(int r1, int r2)
{
	Registers[r1] = !Registers[r1];
	return;
}

void Int_DoNeg(int r1, int r2)
{
	Registers[r1] = -Registers[r1];
	return;
}

void Int_DoInc(int r1, int r2)
{
	Registers[r1] = Registers[r1] + 1;
	return;
}
void Int_DoDec(int r1, int r2)
{
	Registers[r1] = Registers[r1] - 1;
	return;
}

void Int_DoMov(int r1, int r2)
{
	Registers[r1] = Registers[r2];
	return;
}

void Int_DoBankRead0(int r1, int r2)
{
	Registers[r1] = *(unsigned short *)(&ProgramMemory[Registers[r2]]);
	return;
}

void Int_DoBankRead1(int r1, int r2)
{
	Registers[r1] = *(unsigned short *)(&VideoMemory[Registers[r2] + (0*0x10000)]);
	return;
}

void Int_DoBankRead2(int r1, int r2)
{
	Registers[r1] = *(unsigned short *)(&VideoMemory[Registers[r2] + (1*0x10000)]);
	return;
}

void Int_DoBankWrite0(int r1, int r2)
{
	*(unsigned short *)(&ProgramMemory[Registers[r2]]) = Registers[r1];
	return;
}

void Int_DoBankWrite1(int r1, int r2)
{
	*(unsigned short *)(&VideoMemory[Registers[r2] + (0*0x10000)]) = Registers[r1];
	return;
}

void Int_DoBankWrite2(int r1, int r2)
{
	*(unsigned short *)(&VideoMemory[Registers[r2] + (1*0x10000)]) = Registers[r1];
	return;
}

void Int_DoBankRead0Byte(int r1, int r2)
{
	Registers[r1] = *(unsigned char *)(&ProgramMemory[Registers[r2]]);
	return;
}

void Int_DoBankRead1Byte(int r1, int r2)
{
	Registers[r1] = *(unsigned char *)(&VideoMemory[Registers[r2] + (0*0x10000)]);
	return;
}

void Int_DoBankRead2Byte(int r1, int r2)
{
	Registers[r1] = *(unsigned char *)(&VideoMemory[Registers[r2] + (1*0x10000)]);
	return;
}

void Int_DoBankWrite0Byte(int r1, int r2)
{
	*(unsigned char *)(&ProgramMemory[Registers[r2]]) = Registers[r1] & 0xff;
	return;
}

void Int_DoBankWrite1Byte(int r1, int r2)
{
	*(unsigned char *)(&VideoMemory[Registers[r2] + (0*0x10000)]) = Registers[r1] & 0xff;
	return;
}

void Int_DoBankWrite2Byte(int r1, int r2)
{
	*(unsigned char *)(&VideoMemory[Registers[r2] + (1*0x10000)]) = Registers[r1] & 0xff;
	return;
}
void Int_DoJmp(int r1, int r2)
{
	if(r1 == PC)
		Registers[PC] = Registers[PC] + *(short *)(&ProgramMemory[Registers[PC]+2]);
	else
		Registers[PC] = Registers[r1];

	IsBranch = 1;
}

void Int_DoCJne(int r1, int r2)
{
	if(Registers[r1] != Registers[r2])
		Registers[PC] = Registers[PC] + *(short *)(&ProgramMemory[Registers[PC]+2]);
	else
		Registers[PC] += 4;
	IsBranch = 1;
}

void Int_DoCJe(int r1, int r2)
{
	if(Registers[r1] == Registers[r2])
		Registers[PC] = Registers[PC] + *(short *)(&ProgramMemory[Registers[PC]+2]);
	else
		Registers[PC] += 4;
	IsBranch = 1;
}

void Int_DoCJa(int r1, int r2)
{
	if((unsigned short)Registers[r1] > (unsigned short)Registers[r2])
		Registers[PC] = Registers[PC] + *(short *)(&ProgramMemory[Registers[PC]+2]);
	else
		Registers[PC] += 4;
	IsBranch = 1;
}

void Int_DoCJb(int r1, int r2)
{
	if((unsigned short)Registers[r1] < (unsigned short)Registers[r2])
		Registers[PC] = Registers[PC] + *(short *)(&ProgramMemory[Registers[PC]+2]);
	else
		Registers[PC] += 4;
	IsBranch = 1;
}

void Int_LdImm(int r1, int r2)
{
	Registers[r1] = *(short *)(&ProgramMemory[Registers[PC]+2]);
	Registers[PC] += 2;
}

void Int_LdImmb(int r1, int r2)
{
	Registers[r1] = r2;
}

void Int_DoIn(int r1, int r2)
{
	short PortNum = *(short *)(&ProgramMemory[Registers[PC]+2]);
	if(PortNum >= 60 && PortNum <= 66)
		Registers[r1] = Ports[PortNum - 60];
	Registers[PC] += 2;
}

void Int_DoOut(int r1, int r2)
{
	short PortNum = *(short *)(&ProgramMemory[Registers[PC]+2]);
	if(PortNum == 60 || PortNum == 61 || PortNum == 64 || PortNum == 65 || PortNum == 66)
		Ports[PortNum - 60] = Registers[r1];
	Registers[PC] += 2;
}

void Int_DoCall(int r1, int r2)
{
	if((unsigned int)CallStackPos+32 >= (unsigned int)MemoryEnd)
	{
		HaltCPU = 1;
		return;
	}

	if(r1 == PC)
		Registers[PC] += 4;
	else
		Registers[PC] += 2;

	memcpy(CallStackPos, &Registers[PC], 32);
	CallStackPos += 32;
	if(r1 == PC)
		Registers[PC] = *(unsigned short *)(&ProgramMemory[Registers[PC]-2]) & 0xfffe;
	else
		Registers[PC] = Registers[r1] & 0xfffe;
	IsBranch = 1;
}

void Int_DoRet(int r1, int r2)
{
	unsigned short CurPC;
	IsBranch = 1;

	if((unsigned int)CallStackPos-32 < (unsigned int)BeginCallStackPos)
	{
		HaltCPU = 1;
		return;
	}

	CallStackPos -= 32;
	memcpy(&Registers[PC], CallStackPos, 32);
}

void Int_DoNop(int r1, int r2)
{
	return;
}

void Int_DoFreeze(int r1, int r2)
{
	PauseCPU = 1;
	IsBranch = 1;
	Registers[PC] += 2;
}

void Int_DoHalt(int r1, int r2)
{
	HaltCPU = 1;
	IsBranch = 1;
}

void Int_Dyna(int r1, int r2)
{
	IsDynarec = 1;
	IsBranch = 1;
	Registers[PC] += 2;
}

#include <sys/time.h>
void Int_Dump(int r1, int r2)
{
	int i;
	int TempStackPos;

	for(i = 0; i < 16; i++)
	{
		if(i == 0)
			printf("PC: %04x  ", Registers[PC]);
		else
		{
			if(i % 4 == 0)
				printf("\n");

			printf("r%c: %04x ", 0x40+i, Registers[i]); 
		}
	}
	printf("\nStack:\n");
	for(i = 0; i < StackSize; i++)
	{
		printf("%03d: %04x\n", i, StackPos[i]);
	}
	printf("\n");

	printf("Call Stack:\n");
	if((CallStackPos-32) >= (unsigned int)BeginCallStackPos)
	{
		for(i = 32; (CallStackPos-i) >= (unsigned int)BeginCallStackPos; i+=32)
		{
			printf("ret PC: %04x\n", *(unsigned short *)(CallStackPos-i));
		}
	}

	/*
	char filename[200];
	sprintf(filename, "data-%04x.bin", Registers[PC]);
	
	int f = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if(f != -1)
	{
		write(f, VideoMemory, 128*1024);
		close(f);
	}

	sprintf(filename, "data-%04x-app.bin", Registers[PC]);
	
	f = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if(f != -1)
	{
		write(f, ProgramMemory, 64*1024);
		close(f);
	}
	*/
}


InstructFunc InterpretterInstructionList[256];
InstructFuncArr InterpretterInstructArr[] =
{
	{0xff, Int_DoHalt},		//first entry is default for all instructions
	{0x00, Int_DoAdd},
	{0x01, Int_DoSub},
	{0x02, Int_DoMul},
	{0x03, Int_DoDiv},
	{0x04, Int_DoMod},
	{0x05, Int_DoAnd},
	{0x06, Int_DoOr},
	{0x07, Int_DoXor},
	{0x08, Int_DoNeg},
	{0x09, Int_DoInc},
	{0x0a, Int_DoDec},
	{0x0b, Int_DoNot},
	{0x10, Int_DoRol},
	{0x11, Int_DoRor},
	{0x12, Int_DoShl},
	{0x13, Int_DoShr},
	{0x20, Int_DoMov},
	{0x21, Int_LdImm},
	{0x22, Int_LdImmb},
	{0x30, Int_DoJmp},
	{0x31, Int_DoCJne},
	{0x32, Int_DoCJe},
	{0x33, Int_DoCJb},
	{0x34, Int_DoCJa},
	{0x40, Int_DoPush},
	{0x41, Int_DoPop},
	{0x50, Int_DoBankRead0},
	{0x51, Int_DoBankRead1},
	{0x52, Int_DoBankRead2},
	{0x53, Int_DoBankWrite0},
	{0x54, Int_DoBankWrite1},
	{0x55, Int_DoBankWrite2},
	{0x56, Int_DoBankRead0Byte},
	{0x57, Int_DoBankRead1Byte},
	{0x58, Int_DoBankRead2Byte},
	{0x59, Int_DoBankWrite0Byte},
	{0x5a, Int_DoBankWrite1Byte},
	{0x5b, Int_DoBankWrite2Byte},
	{0x60, Int_DoCall},
	{0x61, Int_DoRet},
	{0x62, Int_DoIn},
	{0x63, Int_DoOut},
	{0xf0, Int_Dump},
	{0xf1, Int_Dyna},
	{0xfe, Int_DoFreeze},
	{0,0}
};

void InitInterpretter()
{
	int i;
	for(i = 0; i < 256; i++)
		InterpretterInstructionList[i] = InterpretterInstructArr[0].Func;

	for(i = 0; InterpretterInstructArr[i].Func; i++)
		InterpretterInstructionList[InterpretterInstructArr[i].Position] = InterpretterInstructArr[i].Func;
}
