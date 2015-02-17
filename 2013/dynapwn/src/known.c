#include "dynacommon.h"

int KnownActive;

void Known_Add(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value += InVal;
}

void Known_Sub(KnownRegStruct *OutReg, unsigned short InVal)
{
	//"oops"
	int OutVal;
	//__asm__("int $3");
	OutVal = (short)OutReg->Value;
	*(int *)(&OutReg->Value) = ((int)OutVal) - InVal;
}

void Known_Mul(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value *= InVal;
}

void Known_Div(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value /= InVal;
}

void Known_Mod(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value %= InVal;
}

void Known_And(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value &= InVal;
}

void Known_Or(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value |= InVal;
}

void Known_Xor(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value ^= InVal;
}

void Known_Not(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = !InVal;
}

void Known_Neg(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = -InVal;
}

void Known_Rol(KnownRegStruct *OutReg, unsigned short InVal)
{
	InVal = InVal & 15;
	OutReg->Value = (OutReg->Value >> InVal) | (OutReg->Value << (16 - InVal));
}

void Known_Ror(KnownRegStruct *OutReg, unsigned short InVal)
{
	InVal = InVal & 15;
	OutReg->Value = (OutReg->Value << InVal) | (OutReg->Value >> (16 - InVal));
}

void Known_Shl(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = (OutReg->Value >> InVal);
}

void Known_Shr(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = (OutReg->Value << InVal) & 0xffff;
}

void Known_Dec(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = (InVal - 1) & 0xffff;
}

void Known_Inc(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = (InVal + 1) & 0xffff;
}

void Known_Move(KnownRegStruct *OutReg, unsigned short InVal)
{
	OutReg->Value = InVal;
}

int Known_Activate(InstructionInfoStruct *CurInstruction)
{
	KnownActive = 1;
	return 0;
}

int Known_Deactivate(InstructionInfoStruct *CurInstruction)
{
	KnownActive = 0;

	//write out all known values
	WriteKnownRegs();
	return 0;
}

struct KnownRegStruct KnownRegs[16];

int CheckKnownRegs(InstructionInfoStruct *InstructInfo)
{
	int FoundOut = 0;

	if(!KnownActive)
		return 0;

	//look up and see if we know the value of the register and if so, handle it
	//otherwise find an empty register to use or empty a register if required
	InstructionInfoStruct TempInstruction;
	TempInstruction.OutReg = 0xffff;
	
	int i, x;

	if(InstructInfo->Flags & FLAG_INREG)
	{
		for(i=0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
		{
			if(KnownRegs[i].InUse && (KnownRegs[i].Register == InstructInfo->InReg))
			{
				//out is unknown but in is known, mark instruction as value instead of register
				InstructInfo->Flags = (InstructInfo->Flags & ~FLAG_INREG) | FLAG_VALUE;
				InstructInfo->InVal = KnownRegs[i].Value;
				break;
			}
		}
	}
	
	//now check the out
	if(InstructInfo->Flags & FLAG_OUTREG)
	{
		for(i=0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
		{
			if(KnownRegs[i].InUse && (KnownRegs[i].Register == InstructInfo->OutReg))
			{
				FoundOut = 1;
			
				//if we have an in value then just use it, otherwise add an instruction
				//to write the out value out, unless the instruction is a move
				if(InstructInfo->Flags & FLAG_VALUE)
				{
					//find the function to call
					if(InstructInfo->CacheInstruction)
					{
						InstructInfo->CacheInstruction(&KnownRegs[i], InstructInfo->InVal);
						InstructInfo->ASMInstruction = ASM_Nop;
						return 1;
					}
					else
					{
						//no known function, write out as long as it isn't a move
						if(InstructInfo->ASMInstruction != ASM_Move)
						{
							TempInstruction.OutReg = InstructInfo->OutReg;
							TempInstruction.InVal = KnownRegs[i].Value;
							TempInstruction.Flags = FLAG_OUTREG | FLAG_VALUE;
							TempInstruction.ASMInstruction = ASM_Move;
						}
						KnownRegs[i].InUse = 0;
					}
				}
				else
				{
					//in value is unknown but out is known so write out's value as long as it isn't a move
					if(InstructInfo->ASMInstruction != ASM_Move)
					{
						TempInstruction.OutReg = InstructInfo->OutReg;
						TempInstruction.InVal = KnownRegs[i].Value;
						TempInstruction.Flags = FLAG_OUTREG | FLAG_VALUE;
						TempInstruction.ASMInstruction = ASM_Move;
					}
					KnownRegs[i].InUse = 0;
				}
			
				break;
			}
		}

		//if out isn't assigned but we have an in value and out is a move then assign
		if(!FoundOut && InstructInfo->Flags & FLAG_VALUE && InstructInfo->ASMInstruction == ASM_Move)
		{
			for(i = 0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
			{
				//if empty, assign
				if(!KnownRegs[i].InUse)
				{
					KnownRegs[i].InUse = 1;
					KnownRegs[i].Register = InstructInfo->OutReg;
					KnownRegs[i].Value = InstructInfo->InVal;

					//make sure that it isn't in the assigned area
					for(x = 0; x < (sizeof(RegAssignment) / sizeof(RegAssignmentStruct)); x++)
					{
						//save it off if required
						if(RegAssignment[x].Register == InstructInfo->OutReg)
						{
							RegAssignment[x].Register = 0xffff;
							RegAssignment[x].Modified = 0;
						}
					}

					//ignore instruction
					InstructInfo->ASMInstruction = ASM_Nop;
					return 1;
				}
			}
		}
	
		//if temp is not assigned
		if(TempInstruction.OutReg != 0xffff)
		{
			//compile the instruction after locating a register for out
			AssignRegs(&TempInstruction);
			DynaRecMemPtr += ASM_Move(&TempInstruction);
		}
	}
	
	return 0;
}

int WriteKnownRegs()
{
	int i;
	InstructionInfoStruct NewInstruction;

	//write all used entries out to the proper locations
	for(i=0; i < (sizeof(KnownRegs) / sizeof(KnownRegStruct)); i++)
	{
		if(KnownRegs[i].InUse)
		{
			NewInstruction.InVal = KnownRegs[i].Value;
			NewInstruction.OutMemPos = &Registers[(unsigned int)KnownRegs[i].Register];
			NewInstruction.Flags = FLAG_VALUE | FLAG_MEMORYOUT;
			DynaRecMemPtr += ASM_Move(&NewInstruction);
			KnownRegs[i].InUse = 0;
		}
	}

	return 0;
}
