#include "dynacommon.h"
#include <stdio.h>
#include <memory.h>

void ResetDynarecVars()
{
	CurLabel = 0;
	CurInstructionCount = 0;
	KnownActive = 1;
	memset(KnownRegs, 0, sizeof(KnownRegs));
	memset(RegAssignment, 0xff, sizeof(RegAssignment));
	memset(LabelData, 0xff, sizeof(LabelData));
	memset(UnknownLabelData, 0, sizeof(UnknownLabelData));
}

void ResetDynarec()
{
	memset(DynaLookup, 0, ((64*1024) >> 1) * sizeof(DynaFunc));
	DynaRecMemPtr = DynaMemory;

	ResetDynarecVars();
}

void Rec_DoAdd(int r1, int r2)
{
	CreateInstruction(OP_ADD(r1, r2));
	return;
}

void Rec_DoSub(int r1, int r2)
{
	CreateInstruction(OP_SUB(r1, r2));
	return;
}

void Rec_DoMul(int r1, int r2)
{
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_CLAIMREG(X86_REG_EAX));
	CreateInstruction(OP_CLAIMREG(X86_REG_EDX));
	CreateInstruction(OP_XOR(X86_REG_EDX, X86_REG_EDX));
	CreateInstruction(OP_MOV(X86_REG_EAX, r1));
	CreateInstruction(OP_MUL(r2));
	CreateInstruction(OP_FREEREG(X86_REG_EDX));
	CreateInstruction(OP_ASSIGNREG(r1, X86_REG_EAX));
	CreateInstruction(OP_ACTIVATEKNOWN());
	return;
}

void Rec_DoDiv(int r1, int r2)
{
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_CLAIMREG(X86_REG_EAX));
	CreateInstruction(OP_CLAIMREG(X86_REG_EDX));
	CreateInstruction(OP_XOR(X86_REG_EDX, X86_REG_EDX));
	CreateInstruction(OP_MOV(X86_REG_EAX, r1));
	CreateInstruction(OP_DIV(r2));
	CreateInstruction(OP_FREEREG(X86_REG_EDX));
	CreateInstruction(OP_ASSIGNREG(r1, X86_REG_EAX));
	CreateInstruction(OP_ACTIVATEKNOWN());
	return;
}

void Rec_DoMod(int r1, int r2)
{
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_CLAIMREG(X86_REG_EAX));
	CreateInstruction(OP_CLAIMREG(X86_REG_EDX));
	CreateInstruction(OP_XOR(X86_REG_EDX, X86_REG_EDX));
	CreateInstruction(OP_MOV(X86_REG_EAX, r1));
	CreateInstruction(OP_DIV(r2));
	CreateInstruction(OP_ASSIGNREG(r1, X86_REG_EDX));
	CreateInstruction(OP_FREEREG(X86_REG_EAX));
	CreateInstruction(OP_ACTIVATEKNOWN());
	return;
}

void Rec_DoAnd(int r1, int r2)
{
	CreateInstruction(OP_AND(r1, r2));
	return;
}

void Rec_DoOr(int r1, int r2)
{
	CreateInstruction(OP_OR(r1, r2));
	return;
}

void Rec_DoXor(int r1, int r2)
{
	CreateInstruction(OP_XOR(r1, r2));
	return;
}

void Rec_DoNot(int r1, int r2)
{
	CreateInstruction(OP_NOT(r1));
	return;
}

void Rec_DoNeg(int r1, int r2)
{
	CreateInstruction(OP_NEG(r1));
	return;
}

void Rec_DoInc(int r1, int r2)
{
	CreateInstruction(OP_INC(r1));
	return;
}

void Rec_DoDec(int r1, int r2)
{
	CreateInstruction(OP_DEC(r1));
	return;
}

void Rec_DoRol(int r1, int r2)
{
	CreateInstruction(OP_CLAIMREG(X86_REG_ECX));
	CreateInstruction(OP_MOV(X86_REG_ECX, r2));
	CreateInstruction(OP_ROL(r1, X86_REG_ECX));
	CreateInstruction(OP_ASSIGNREG(r2, X86_REG_ECX));
	return;
}

void Rec_DoRor(int r1, int r2)
{
	CreateInstruction(OP_CLAIMREG(X86_REG_ECX));
	CreateInstruction(OP_MOV(X86_REG_ECX, r2));
	CreateInstruction(OP_ROR(r1, X86_REG_ECX));
	CreateInstruction(OP_ASSIGNREG(r2, X86_REG_ECX));
	return;
}

void Rec_DoShl(int r1, int r2)
{
	CreateInstruction(OP_CLAIMREG(X86_REG_ECX));
	CreateInstruction(OP_MOV(X86_REG_ECX, r2));
	CreateInstruction(OP_SHL(r1, X86_REG_ECX));
	CreateInstruction(OP_ASSIGNREG(r2, X86_REG_ECX));
	return;
}

void Rec_DoShr(int r1, int r2)
{
	CreateInstruction(OP_CLAIMREG(X86_REG_ECX));
	CreateInstruction(OP_MOV(X86_REG_ECX, r2));
	CreateInstruction(OP_SHR(r1, X86_REG_ECX));
	CreateInstruction(OP_ASSIGNREG(r2, X86_REG_ECX));
	return;
}

void Rec_DoMov(int r1, int r2)
{
	CreateInstruction(OP_MOV(r1, r2));
	return;
}

void Rec_DoLdImm(int r1, int r2)
{
	CreateInstruction(OP_MOV_IMM(r1, *(short *)(&ProgramMemory[Registers[PC] + 2])));
	Registers[PC] += 2;
	return;
}

void Rec_DoLdImmb(int r1, int r2)
{
	CreateInstruction(OP_MOV_IMM(r1, r2));
	return;
}

void Rec_DoJmp(int r1, int r2)
{
	unsigned short JmpPos;

	if(r1 == PC)
	{
		CreateInstruction(OP_MOV_IMM(PC, *(short *)(&ProgramMemory[Registers[PC] + 2]) + Registers[PC]));
	}
	else
	{
		CreateInstruction(OP_MOV(PC, r1));
	}

	IsBranch = 1;
}

void Rec_DoCJne(int r1, int r2)
{
	int LabelID, LabelID2;
	unsigned short JmpPos;

	LabelID = GetNewLabel();
	LabelID2 = GetNewLabel();

	//force a register for pc
	CreateInstruction(OP_GETTEMPREG(0));

	CreateInstruction(OP_CMP(r1, r2));
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_JNE(LabelID));

	CreateInstruction(OP_MOV_IMM(TEMPREG, Registers[PC] + 4));

	CreateInstruction(OP_JMP(LabelID2));

	CreateInstruction(OP_ADDLABEL(LabelID));

	JmpPos = *(short *)(&ProgramMemory[Registers[PC] + 2]) + Registers[PC];
	CreateInstruction(OP_MOV_IMM(TEMPREG, JmpPos));

	CreateInstruction(OP_ADDLABEL(LabelID2));
	CreateInstruction(OP_ASSIGNREG(PC, TEMPREG));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoCJe(int r1, int r2)
{
	int LabelID, LabelID2;
	unsigned short JmpPos;

	LabelID = GetNewLabel();
	LabelID2 = GetNewLabel();

	//force a register for pc
	CreateInstruction(OP_GETTEMPREG(0));

	CreateInstruction(OP_CMP(r1, r2));
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_JE(LabelID));

	CreateInstruction(OP_MOV_IMM(TEMPREG, Registers[PC] + 4));

	CreateInstruction(OP_JMP(LabelID2));

	CreateInstruction(OP_ADDLABEL(LabelID));

	JmpPos = *(short *)(&ProgramMemory[Registers[PC] + 2]) + Registers[PC];
	CreateInstruction(OP_MOV_IMM(TEMPREG, JmpPos));

	CreateInstruction(OP_ADDLABEL(LabelID2));
	CreateInstruction(OP_ASSIGNREG(PC, TEMPREG));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoCJa(int r1, int r2)
{
	int LabelID, LabelID2;
	unsigned short JmpPos;

	LabelID = GetNewLabel();
	LabelID2 = GetNewLabel();

	//force a register for pc
	CreateInstruction(OP_GETTEMPREG(0));

	CreateInstruction(OP_CMP(r1, r2));
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_JA(LabelID));

	CreateInstruction(OP_MOV_IMM(TEMPREG, Registers[PC] + 4));

	CreateInstruction(OP_JMP(LabelID2));

	CreateInstruction(OP_ADDLABEL(LabelID));

	JmpPos = *(short *)(&ProgramMemory[Registers[PC] + 2]) + Registers[PC];
	CreateInstruction(OP_MOV_IMM(TEMPREG, JmpPos));

	CreateInstruction(OP_ADDLABEL(LabelID2));
	CreateInstruction(OP_ASSIGNREG(PC, TEMPREG));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoCJb(int r1, int r2)
{
	int LabelID, LabelID2;
	unsigned short JmpPos;

	LabelID = GetNewLabel();
	LabelID2 = GetNewLabel();

	//force a register for pc
	CreateInstruction(OP_GETTEMPREG(0));

	CreateInstruction(OP_CMP(r1, r2));
	CreateInstruction(OP_DEACTIVATEKNOWN());
	CreateInstruction(OP_JB(LabelID));

	CreateInstruction(OP_MOV_IMM(TEMPREG, Registers[PC] + 4));

	CreateInstruction(OP_JMP(LabelID2));

	CreateInstruction(OP_ADDLABEL(LabelID));

	JmpPos = *(short *)(&ProgramMemory[Registers[PC] + 2]) + Registers[PC];
	CreateInstruction(OP_MOV_IMM(TEMPREG, JmpPos));

	CreateInstruction(OP_ADDLABEL(LabelID2));
	CreateInstruction(OP_ASSIGNREG(PC, TEMPREG));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoPush(int r1, int r2)
{
	int EndLabel;

	EndLabel = GetNewLabel();

	CreateInstruction(OP_GETTEMPREG(0));	
	CreateInstruction(OP_MOV_FROM_MEMPTR(TEMPREG, &StackSize));

	CreateInstruction(OP_CMP_IMM(TEMPREG, 0x400 - (16*2)));
	CreateInstruction(OP_JAE(EndLabel));

	CreateInstruction(OP_INC(TEMPREG));
	CreateInstruction(OP_MOV_TO_MEMPTR(&StackSize, TEMPREG));

	CreateInstruction(OP_MOVZX_32(TEMPREG, TEMPREG));
	CreateInstruction(OP_SHL_IMM_32(TEMPREG, 1));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, (unsigned int)StackPos-2));
	CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, r1));

	CreateInstruction(OP_FREETEMPREG(0));
	CreateInstruction(OP_ADDLABEL(EndLabel));
}

void Rec_DoPop(int r1, int r2)
{
	int EndLabel;

	EndLabel = GetNewLabel();

	CreateInstruction(OP_GETTEMPREG(0));	
	CreateInstruction(OP_MOV_FROM_MEMPTR(TEMPREG, &StackSize));

	CreateInstruction(OP_CMP_IMM(TEMPREG, 0));

	CreateInstruction(OP_JBE(EndLabel));
	
	CreateInstruction(OP_DEC(TEMPREG));
	CreateInstruction(OP_MOV_TO_MEMPTR(&StackSize, TEMPREG));

	CreateInstruction(OP_MOVZX_32(TEMPREG, TEMPREG));
	CreateInstruction(OP_SHL_IMM_32(TEMPREG, 1));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, (unsigned int)StackPos));
	CreateInstruction(OP_MOV_FROM_REGMEMPTR(r1, TEMPREG));

	CreateInstruction(OP_FREETEMPREG(0));
	CreateInstruction(OP_ADDLABEL(EndLabel));
}

void Rec_DoBankRead0(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, ProgramMemory));
	CreateInstruction(OP_MOV_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankRead1(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, VideoMemory));
	CreateInstruction(OP_MOV_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankRead2(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, &VideoMemory[0x10000]));
	CreateInstruction(OP_MOV_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite0(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, ProgramMemory));
	CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite1(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, VideoMemory));
	CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite2(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, &VideoMemory[0x10000]));
	CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankRead0Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, ProgramMemory));
	CreateInstruction(OP_MOVZX_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankRead1Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, VideoMemory));
	CreateInstruction(OP_MOVZX_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankRead2Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, &VideoMemory[0x10000]));
	CreateInstruction(OP_MOVZX_FROM_REGMEMPTR(r1, TEMPREG));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite0Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, ProgramMemory));
	CreateInstruction(OP_MOVB_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite1Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, VideoMemory));
	CreateInstruction(OP_MOVB_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoBankWrite2Byte(int r1, int r2)
{
	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOVZX_32(TEMPREG, r2));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, &VideoMemory[0x10000]));
	CreateInstruction(OP_MOVB_TO_REGMEMPTR(TEMPREG, r1));
	CreateInstruction(OP_FREETEMPREG(0));
}

void Rec_DoHalt(int r1, int r2)
{
	CreateInstruction(OP_MOV_TO_MEMPTR_IMM(&HaltCPU, 1));
	IsBranch = 1;
}

void Rec_DoCall(int r1, int r2)
{
	int i;
	int HaltLabel, EndLabel;

	if(r1 == PC)
		Registers[PC] += 4;
	else
		Registers[PC] += 2;

	HaltLabel = GetNewLabel();
	EndLabel = GetNewLabel();

	CreateInstruction(OP_DEACTIVATEKNOWN());

	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOV_FROM_MEMPTR_32(TEMPREG, &CallStackPos));
	CreateInstruction(OP_CMP_IMM_32(TEMPREG, (unsigned int)MemoryEnd - 32));
	CreateInstruction(OP_JGE(HaltLabel));

	CreateInstruction(OP_GETTEMPREG(1));
	CreateInstruction(OP_MOV_IMM(TEMPREG+1, Registers[PC]));
	CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, TEMPREG+1));
	CreateInstruction(OP_ADD_IMM_32(TEMPREG, 2));
	CreateInstruction(OP_FREETEMPREG(1));

	for(i = 1; i < 16; i++)
	{
		CreateInstruction(OP_MOV_TO_REGMEMPTR(TEMPREG, i));
		CreateInstruction(OP_ADD_IMM_32(TEMPREG, 2));
	}

	CreateInstruction(OP_MOV_TO_MEMPTR_32(&CallStackPos, TEMPREG));

	CreateInstruction(OP_GETTEMPREG(1));

	if(r1 == PC)
	{
		CreateInstruction(OP_MOV_IMM(TEMPREG+1, *(unsigned short *)(&ProgramMemory[Registers[PC]-2]) & 0xfffe));
	}
	else
	{
		CreateInstruction(OP_MOV(TEMPREG+1, r1));
		CreateInstruction(OP_AND_IMM(TEMPREG+1, 0xfffe));
	}

	CreateInstruction(OP_JMP(EndLabel));

	CreateInstruction(OP_ADDLABEL(HaltLabel));
	CreateInstruction(OP_MOV_TO_MEMPTR_IMM(&HaltCPU, 1));

	CreateInstruction(OP_ADDLABEL(EndLabel));
	CreateInstruction(OP_ASSIGNREG(PC, TEMPREG+1));
	CreateInstruction(OP_FREETEMPREG(0));
	CreateInstruction(OP_FREETEMPREG(1));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoRet(int r1, int r2)
{
	int i;
	int HaltLabel, EndLabel;

	HaltLabel = GetNewLabel();
	EndLabel = GetNewLabel();

	CreateInstruction(OP_DEACTIVATEKNOWN());

	CreateInstruction(OP_GETTEMPREG(0));
	CreateInstruction(OP_MOV_FROM_MEMPTR_32(TEMPREG, &CallStackPos));
	CreateInstruction(OP_CMP_IMM_32(TEMPREG, (unsigned int)BeginCallStackPos + 32));
	CreateInstruction(OP_JL(HaltLabel));

	for(i = 15; i >= 0; i--)
	{
		CreateInstruction(OP_SUB_IMM_32(TEMPREG, 2));
		CreateInstruction(OP_MOV_FROM_REGMEMPTR(i, TEMPREG));
	}

	CreateInstruction(OP_MOV_TO_MEMPTR_32(&CallStackPos, TEMPREG));
	CreateInstruction(OP_JMP(EndLabel));

	CreateInstruction(OP_ADDLABEL(HaltLabel));
	CreateInstruction(OP_MOV_TO_MEMPTR_IMM(&HaltCPU, 1));

	CreateInstruction(OP_ADDLABEL(EndLabel));

	CreateInstruction(OP_FREETEMPREG(0));

	CreateInstruction(OP_ACTIVATEKNOWN());

	IsBranch = 1;
}

void Rec_DoIn(int r1, int r2)
{
	short PortNum = *(short *)(&ProgramMemory[Registers[PC]+2]);
	if(PortNum >= 60 && PortNum <= 66)
	{
		PortNum -= 60;
		CreateInstruction(OP_MOV_FROM_MEMPTR(r1, &Ports[PortNum]));
	}

	Registers[PC] += 2;
}

void Rec_DoOut(int r1, int r2)
{
	short PortNum = *(short *)(&ProgramMemory[Registers[PC]+2]);
	if(PortNum >= 60 && PortNum <= 66)
	{
		PortNum -= 60;
		CreateInstruction(OP_MOV_TO_MEMPTR(&Ports[PortNum], r1));
	}

	Registers[PC] += 2;
}

void Rec_DoFreeze(int r1, int r2)
{
	//CreateInstruction(OP_INT3());
	CreateInstruction(OP_MOV_TO_MEMPTR_IMM(&PauseCPU, 1));
	CreateInstruction(OP_MOV_IMM(PC, Registers[PC]+2));
	IsBranch = 1;
	Registers[PC] += 2;
}

void Rec_DoNothing(int r1, int r2)
{
	return 0;
}

void Rec_DoUnknown(int r1, int r2)
{
	UnknownInstruction = 1;
	IsBranch = 1;
}

InstructFunc DynaRecInstructionList[256];
InstructFuncArr DynaRecInstructionArr[] =
{
	{0xfd, Rec_DoUnknown},		//first entry is default for all instructions
	{0xff, Rec_DoHalt},
	{0x00, Rec_DoAdd},
	{0x01, Rec_DoSub},
	{0x02, Rec_DoMul},
	{0x03, Rec_DoDiv},
	{0x04, Rec_DoMod},
	{0x05, Rec_DoAnd},
	{0x06, Rec_DoOr},
	{0x07, Rec_DoXor},
	{0x08, Rec_DoNeg},
	{0x09, Rec_DoInc},
	{0x0a, Rec_DoDec},
	{0x0b, Rec_DoNot},
	{0x10, Rec_DoRol},
	{0x11, Rec_DoRor},
	{0x12, Rec_DoShl},
	{0x13, Rec_DoShr},
	{0x20, Rec_DoMov},
	{0x21, Rec_DoLdImm},
	{0x22, Rec_DoLdImmb},
	{0x30, Rec_DoJmp},
	{0x31, Rec_DoCJne},
	{0x32, Rec_DoCJe},
	{0x33, Rec_DoCJb},
	{0x34, Rec_DoCJa},
	{0x40, Rec_DoPush},
	{0x41, Rec_DoPop},
	{0x50, Rec_DoBankRead0},
	{0x51, Rec_DoBankRead1},
	{0x52, Rec_DoBankRead2},
	{0x53, Rec_DoBankWrite0},
	{0x54, Rec_DoBankWrite1},
	{0x55, Rec_DoBankWrite2},
	{0x56, Rec_DoBankRead0Byte},
	{0x57, Rec_DoBankRead1Byte},
	{0x58, Rec_DoBankRead2Byte},
	{0x59, Rec_DoBankWrite0Byte},
	{0x5a, Rec_DoBankWrite1Byte},
	{0x5b, Rec_DoBankWrite2Byte},
	{0x60, Rec_DoCall},
	{0x61, Rec_DoRet},
	{0x62, Rec_DoIn},
	{0x63, Rec_DoOut},
	{0xf0, Rec_DoNothing},
	{0xf1, Rec_DoNothing},
	{0xfe, Rec_DoFreeze},
	{0,0}
};

void InitDynarec()
{
	int i;
	for(i = 0; i < 256; i++)
		DynaRecInstructionList[i] = DynaRecInstructionArr[0].Func;

	for(i = 0; DynaRecInstructionArr[i].Func; i++)
		DynaRecInstructionList[DynaRecInstructionArr[i].Position] = DynaRecInstructionArr[i].Func;
}
