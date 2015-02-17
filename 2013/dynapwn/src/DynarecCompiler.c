#include "dynacommon.h"

//overflow value on the math to affect register to be 0xffff resulting in writing to dynarec memory
int CurLabel;
int CurInstructionCount;
unsigned char *DynaRecMemPtr;
unsigned char *CurrentDynaPos;

struct RegAssignmentStruct RegAssignment[4];
unsigned short LabelData[256];
struct UnknownLabelDataStruct UnknownLabelData[256];
unsigned char *StartDynaRecMemPtr;

int ClaimCPUReg(InstructionInfoStruct *CurInstruction)
{
	int ClaimReg, i;
	InstructionInfoStruct NewInstruction;

	ClaimReg = CurInstruction->OutReg & 0x7FFF;

	//save it off if required
	if(RegAssignment[ClaimReg].Modified && ((RegAssignment[ClaimReg].Register & 0x8000) == 0))
	{
		NewInstruction.InReg = ClaimReg;
		NewInstruction.OutMemPos = &Registers[RegAssignment[ClaimReg].Register];
		NewInstruction.Flags = FLAG_INX86REG | FLAG_MEMORYOUT;
		DynaRecMemPtr += ASM_Move(&NewInstruction);
	}

	RegAssignment[ClaimReg].Register = CurInstruction->OutReg;
	RegAssignment[ClaimReg].Modified = 0;
	return 0;
}

int FreeCPUReg(InstructionInfoStruct *CurInstruction)
{
	int i;
	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		if(RegAssignment[i].Register == CurInstruction->OutReg)
		{
			RegAssignment[i].Register = 0xffff;
			break;
		}
	}
	return 0;
}

int GetTempReg(InstructionInfoStruct *CurInstruction)
{
	unsigned int EmptyReg;
	int i;
	InstructionInfoStruct NewInstruction;

	EmptyReg = 0xffff;
	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		if(RegAssignment[i].Register == 0xffff)
		{
			EmptyReg = i;
			break;
		}
	}

	//if no register found, use the empty one
	if(EmptyReg == 0xffff)
	{
		//keep advancing if the register is already assigned
		for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
		{
			if((RegAssignment[i].Register & 0x8000) == 0)
			{
				EmptyReg = i;
				break;
			}
		}

		//error
		if(EmptyReg == 0xffff)
			return -1;

		//save it off if required
		if(RegAssignment[EmptyReg].Modified)
		{
			NewInstruction.InReg = EmptyReg;
			NewInstruction.OutMemPos = &Registers[RegAssignment[EmptyReg].Register];
			NewInstruction.Flags = FLAG_INX86REG | FLAG_MEMORYOUT;
			DynaRecMemPtr += ASM_Move(&NewInstruction);
		}
	}

	//assign register and fill it in
	RegAssignment[EmptyReg].Register = TEMPREG + CurInstruction->OutReg;
	RegAssignment[EmptyReg].Modified = 0;
	return 0;
}

int FreeTempReg(InstructionInfoStruct *CurInstruction)
{
	int i;
	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		if(RegAssignment[i].Register == (TEMPREG + CurInstruction->OutReg))
		{
			RegAssignment[i].Register = 0xffff;
			break;
		}
	}

	for(i=0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
	{
		if(KnownRegs[i].InUse && KnownRegs[i].Register == CurInstruction->OutReg)
		{
			KnownRegs[i].InUse = 0;
			break;
		}
	}

	return 0;
}

int AssignCPUReg(InstructionInfoStruct *CurInstruction)
{
	int i;
	//first, unassign if already assigned
	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		if(RegAssignment[i].Register == CurInstruction->OutReg)
		{
			RegAssignment[i].Register = 0xffff;
			RegAssignment[i].Modified = 0;
		}
	}

	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		if(RegAssignment[i].Register == CurInstruction->InReg)
		{
			RegAssignment[i].Register = CurInstruction->OutReg;
			break;
		}
	}

	for(i=0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
	{
		if(KnownRegs[i].InUse && KnownRegs[i].Register == CurInstruction->InReg)
		{
			KnownRegs[i].Register = CurInstruction->OutReg;
			break;
		}
	}

	return 0;
}

int GetNewLabel()
{
	CurLabel++;
	if(CurLabel >= (sizeof(LabelData) / sizeof(unsigned short)))
		return 0;

	return CurLabel + LABEL;
}

int AddLabel(InstructionInfoStruct *CurInstruction)
{
	//set the label position
	LabelData[CurInstruction->OutReg & ~LABEL] = (unsigned short)(DynaRecMemPtr - StartDynaRecMemPtr);
	return 0;
}

int CreateInstruction(ASMFuncPtr FuncPtr, unsigned int RegOut, unsigned int RegIn, int Flags, CacheFuncPtr CacheFunc)
{
	if(CurInstructionCount >= 256)
		return;
		
	InstructionList[CurInstructionCount].InMemPos = RegIn;
	InstructionList[CurInstructionCount].OutMemPos = RegOut;
	InstructionList[CurInstructionCount].ASMInstruction = FuncPtr;
	InstructionList[CurInstructionCount].Flags = Flags;
	InstructionList[CurInstructionCount].CacheInstruction = CacheFunc;
	
	CurInstructionCount++;
}

int AssignRegs(InstructionInfoStruct *InstructInfo)
{
	int i;
	unsigned int EmptyReg;
	InstructionInfoStruct NewInstruction;

	//see if the register being used is in the current list of assigned
	//if not, find one for in and out

	//check in first so it is assigned and filled in if required
	if(InstructInfo->Flags & FLAG_INREG)
	{
		EmptyReg = 0xffff;
		for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
		{
			if(RegAssignment[i].Register == InstructInfo->InReg)
			{
				InstructInfo->InReg = i;
				InstructInfo->Flags |= FLAG_INX86REG;
				break;
			}
			else if((RegAssignment[i].Register == 0xffff) && (EmptyReg == 0xffff))
				EmptyReg = i;
		}

		//if no register found, find and use an empty one, assuming that this isn't a move instruction
		//at which point we will just give it a memory address to read from and assign it an out
		if(i >= (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)))
		{
			if((InstructInfo->ASMInstruction == ASM_Move) && (InstructInfo->Flags & FLAG_OUTREG))
			{
				//a move instruction from this register into a register
				//just give the memory address for in, let the assign below figure out what register to go into
				InstructInfo->InMemPos = &Registers[InstructInfo->InReg];
				InstructInfo->Flags = InstructInfo->Flags | FLAG_MEMORYIN;
			}
			else
			{
				if(EmptyReg == 0xffff)
				{
					//if no register found, pick one based on the register being asked
					EmptyReg = InstructInfo->InReg % (sizeof(RegAssignment) / sizeof(RegAssignmentStruct));

					//if a temp register then advance one
					while(1)
					{
						if((RegAssignment[EmptyReg].Register & 0x8000) == 0)
						{
							//if it matches the out then we need to skip
							if(!(InstructInfo->Flags & FLAG_OUTREG) || (RegAssignment[EmptyReg].Register != InstructInfo->OutReg))
							{
								break;
							}
						}
						EmptyReg = (EmptyReg + 1)  % (sizeof(RegAssignment) / sizeof(RegAssignmentStruct));
					};

					//save it off if required
					if(RegAssignment[EmptyReg].Modified)
					{
						NewInstruction.InReg = EmptyReg;
						NewInstruction.OutMemPos = &Registers[RegAssignment[EmptyReg].Register];
						NewInstruction.Flags = FLAG_INX86REG | FLAG_MEMORYOUT;
						DynaRecMemPtr += ASM_Move(&NewInstruction);
					}
				}

				//assign register and fill it in
				RegAssignment[EmptyReg].Register = InstructInfo->InReg;
				InstructInfo->InReg = EmptyReg;
				InstructInfo->Flags |= FLAG_INX86REG;

				NewInstruction.OutReg = EmptyReg;
				NewInstruction.InMemPos = &Registers[RegAssignment[EmptyReg].Register];
				NewInstruction.Flags = FLAG_MEMORYIN | FLAG_OUTX86REG;
				DynaRecMemPtr += ASM_Move(&NewInstruction);
				RegAssignment[EmptyReg].Modified = 0;
			}
		}
	}

	if(InstructInfo->Flags & FLAG_OUTREG)
	{
		EmptyReg = 0xffff;
		for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
		{
			if(RegAssignment[i].Register == InstructInfo->OutReg)
			{
				InstructInfo->OutReg = i;
				InstructInfo->Flags |= FLAG_OUTX86REG;
				break;
			}
			else if((RegAssignment[i].Register == 0xffff) && (EmptyReg == 0xffff))
				EmptyReg = i;
		}

		//if no register found, use the empty one
		if(i >= (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)))
		{
			if(EmptyReg == 0xffff)
			{
				//if no register found, pick one at random
				EmptyReg = InstructInfo->OutReg % (sizeof(RegAssignment) / sizeof(RegAssignmentStruct));

				//if a temp register then advance one
				while(1)
				{
					if((RegAssignment[EmptyReg].Register & 0x8000) == 0)
					{
						//if it matches the in then we need to skip
						//we check the x86 id as it is reassigned above
						if(!(InstructInfo->Flags & FLAG_INREG) || (EmptyReg != InstructInfo->InReg))
						{
							break;
						}
					}
					EmptyReg = (EmptyReg + 1) % (sizeof(RegAssignment) / sizeof(RegAssignmentStruct));
				};

				//save it off if required
				if(RegAssignment[EmptyReg].Modified)
				{
					NewInstruction.InReg = EmptyReg;
					NewInstruction.OutMemPos = &Registers[RegAssignment[EmptyReg].Register];
					NewInstruction.Flags = FLAG_INX86REG | FLAG_MEMORYOUT;
					DynaRecMemPtr += ASM_Move(&NewInstruction);
				}
			}

			//assign register
			RegAssignment[EmptyReg].Register = InstructInfo->OutReg;
			InstructInfo->OutReg = EmptyReg;
			InstructInfo->Flags |= FLAG_OUTX86REG;

			//if the command is not a move then fill in the register
			if(InstructInfo->ASMInstruction != ASM_Move)
			{
				NewInstruction.OutReg = EmptyReg;
				NewInstruction.InMemPos = &Registers[RegAssignment[EmptyReg].Register];
				NewInstruction.Flags = FLAG_MEMORYIN | FLAG_OUTX86REG;
				DynaRecMemPtr += ASM_Move(&NewInstruction);
				RegAssignment[EmptyReg].Modified = 0;
			}
		}
	}
}

int WriteAssignedRegs()
{
	int i;
	InstructionInfoStruct NewInstruction;

	for(i = 0; i < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); i++)
	{
		//save it off if required
		if((RegAssignment[i].Register & 0x8000) == 0 && RegAssignment[i].Modified)
		{
			NewInstruction.InReg = i;
			NewInstruction.OutMemPos = &Registers[RegAssignment[i].Register];
			NewInstruction.Flags = FLAG_INX86REG | FLAG_MEMORYOUT;
			DynaRecMemPtr += ASM_Move(&NewInstruction);
		}
	}

	return 0;
}

int ApplyLabels()
{
	int i;

	for(i = 0; i < (sizeof(UnknownLabelData) / sizeof(UnknownLabelDataStruct)); i++)
	{
		if(UnknownLabelData[i].LabelID == 0)
			break;

		//if there is a label, update the position data
		if(LabelData[UnknownLabelData[i].LabelID] != 0xffff)
		{
			*(int *)(StartDynaRecMemPtr+UnknownLabelData[i].Position) = ((int)LabelData[UnknownLabelData[i].LabelID] - (int)UnknownLabelData[i].Position - 4);
		}
	}

	return 0;
}

int CompileInstructions()
{
	int Counter;
	int i;
	ASMFuncPtr Func;

	//if we are close to the end then restart at the beginning
	if((DynaRecMemPtr + 100) >= VideoMemory)
	{
		//reset to the beginning of the buffer
		DynaRecMemPtr = DynaMemory;
		memset(DynaLookup, 0, ((64*1024) >> 1) * sizeof(DynaFunc));
	}

	StartDynaRecMemPtr = DynaRecMemPtr;

	//int 3
	//*(unsigned char *)DynaRecMemPtr = 0xcc;
	//DynaRecMemPtr++;

	//pusha
	*(unsigned char *)DynaRecMemPtr = 0x60;
	DynaRecMemPtr++;

	//printf("CurInstructionCount: %d\n", CurInstructionCount);
	for(Counter = 0; Counter < CurInstructionCount; Counter++)
	{
		if(CheckKnownRegs(&InstructionList[Counter]) == 0)
			AssignRegs(&InstructionList[Counter]);
		Func = InstructionList[Counter].ASMInstruction;

		//call the instruction and add how many bytes it added
		i = Func(&InstructionList[Counter]);
		if(i == -1)
			return -1;

		DynaRecMemPtr += i;

		//if we are close to the end, abort
		if((DynaRecMemPtr + 10) >= VideoMemory)
		{
			//fail
			return -1;
		}
	}

	//at this point all labels have been processed through, apply them
	ApplyLabels();
	WriteAssignedRegs();
	WriteKnownRegs();

	//if we are close to the end, abort
	if((DynaRecMemPtr + 2) >= VideoMemory)
	{
		//fail
		return -1;
	}

	//popa
	*(unsigned char *)DynaRecMemPtr = 0x61;
	DynaRecMemPtr++;

	//ret
	*(unsigned char *)DynaRecMemPtr = 0xc3;
	DynaRecMemPtr++;

	DynaLookup[Registers[PC] >> 1] = StartDynaRecMemPtr;
	return 0;
}
