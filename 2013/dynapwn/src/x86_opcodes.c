#include "dynacommon.h"

#define ModRM(Mod, OutReg, InReg)	(((Mod)<<6) | ((OutReg) << 3) | (InReg))
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char

#define Instruction1Param(Name, ValByte, ModVal)										\
int ASM_##Name(InstructionInfoStruct *CurInstruction)										\
{																\
	switch(CurInstruction->Flags & FLAG_X86MASK)										\
	{															\
		case (FLAG_INX86REG | FLAG_OUTX86REG):										\
			*(u8 *)DynaRecMemPtr = 0x66;										\
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ModVal, CurInstruction->OutReg) << 8) | ValByte;		\
			RegAssignment[CurInstruction->OutReg].Modified = 1;							\
			return 3;												\
		default:													\
			return 0;												\
	}															\
}

#define Instruction1Param_AXFlagged(Name, ValByte, ModVal)										\
int ASM_##Name(InstructionInfoStruct *CurInstruction)										\
{																\
	switch(CurInstruction->Flags & FLAG_X86MASK)										\
	{															\
		case (FLAG_INX86REG | FLAG_OUTX86REG):										\
			*(u8 *)DynaRecMemPtr = 0x66;										\
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ModVal, CurInstruction->OutReg) << 8) | ValByte;		\
			RegAssignment[0].Modified = 1;							\
			return 3;												\
		default:													\
			return 0;												\
	}															\
}
#define Instruction2Params(Name, RegByte, ValByte, ValMod)										\
int ASM_##Name(InstructionInfoStruct *CurInstruction)											\
{																	\
	switch(CurInstruction->Flags & FLAG_X86MASK)											\
	{																\
		case (FLAG_INX86REG | FLAG_OUTX86REG):											\
			*(u8 *)DynaRecMemPtr = 0x66;											\
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, CurInstruction->InReg, CurInstruction->OutReg) << 8) | RegByte;	\
			RegAssignment[CurInstruction->OutReg].Modified = 1;								\
			return 3;													\
		case (FLAG_VALUE | FLAG_OUTX86REG):											\
			*(u8 *)DynaRecMemPtr = 0x66;											\
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ValMod, CurInstruction->OutReg) << 8) | ValByte;			\
			*(u16 *)(DynaRecMemPtr+3) = CurInstruction->InVal;								\
			RegAssignment[CurInstruction->OutReg].Modified = 1;								\
			return 5;													\
		default:														\
			return 0;													\
	};																\
}

#define Instruction2Params_32(Name, RegByte, ValByte, ValMod)										\
int ASM_##Name(InstructionInfoStruct *CurInstruction)											\
{																	\
	switch(CurInstruction->Flags & FLAG_X86MASK)											\
	{																\
		case (FLAG_INX86REG | FLAG_OUTX86REG):											\
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, CurInstruction->InReg, CurInstruction->OutReg) << 8) | RegByte;		\
			RegAssignment[CurInstruction->OutReg].Modified = 1;								\
			return 2;													\
		case (FLAG_VALUE | FLAG_OUTX86REG):											\
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, ValMod, CurInstruction->OutReg) << 8) | ValByte;				\
			*(u32 *)(DynaRecMemPtr+2) = (unsigned int)CurInstruction->InMemPos;						\
			RegAssignment[CurInstruction->OutReg].Modified = 1;								\
			return 6;													\
		default:														\
			return 0;													\
	};																\
}

#define Instruction2ParamsCL(Name, RegByte, ValByte, Val1Byte, ByteMod)								\
int ASM_##Name(InstructionInfoStruct *CurInstruction)										\
{																\
	switch(CurInstruction->Flags & FLAG_X86MASK)										\
	{															\
		case (FLAG_INX86REG | FLAG_OUTX86REG):										\
			*(u8 *)DynaRecMemPtr = 0x66;										\
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | RegByte;		\
			RegAssignment[CurInstruction->OutReg].Modified = 1;							\
			return 3;												\
		case (FLAG_VALUE | FLAG_OUTX86REG):										\
			*(u8 *)DynaRecMemPtr = 0x66;										\
			if(CurInstruction->InVal == 1)										\
			{													\
				*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | Val1Byte;	\
				RegAssignment[CurInstruction->OutReg].Modified = 1;						\
				return 3;											\
			}													\
			else													\
			{													\
				*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | ValByte;	\
				*(u8 *)(DynaRecMemPtr+3) = CurInstruction->InVal & 0xFF;					\
				RegAssignment[CurInstruction->OutReg].Modified = 1;						\
				return 4;											\
			}													\
		default:													\
			return 0;												\
	};															\
}

#define Instruction2ParamsCL_32(Name, RegByte, ValByte, Val1Byte, ByteMod)							\
int ASM_##Name(InstructionInfoStruct *CurInstruction)										\
{																\
	switch(CurInstruction->Flags & FLAG_X86MASK)										\
	{															\
		case (FLAG_INX86REG | FLAG_OUTX86REG):										\
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | RegByte;		\
			return 2;												\
		case (FLAG_VALUE | FLAG_OUTX86REG):										\
			if(CurInstruction->InVal == 1)										\
			{													\
				*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | Val1Byte;	\
				return 2;											\
			}													\
			else													\
			{													\
				*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, ByteMod, CurInstruction->OutReg) << 8) | ValByte;	\
				*(u8 *)(DynaRecMemPtr+2) = CurInstruction->InVal & 0xFF;					\
				return 3;											\
			}													\
		default:													\
			return 0;												\
	};															\
}
#define Instruction1ParamAdd(Name, ValByte)								\
int ASM_##Name(InstructionInfoStruct *CurInstruction)							\
{													\
	switch(CurInstruction->Flags & FLAG_X86MASK)							\
	{												\
		case (FLAG_INX86REG | FLAG_OUTX86REG):							\
			*(u16 *)DynaRecMemPtr = (ValByte + CurInstruction->OutReg) << 8 | 0x66;		\
			RegAssignment[CurInstruction->OutReg].Modified = 1;				\
			return 2;									\
		default:										\
			return 0;									\
	};												\
}

int ASM_Move(InstructionInfoStruct *CurInstruction)
{
	//output the bytes for a mov based on required setup
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, CurInstruction->InReg, CurInstruction->OutReg) << 8) | 0x89;
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 3;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x03, 0, CurInstruction->OutReg) << 8) | 0xC7;
			*(u16 *)(DynaRecMemPtr+3) = CurInstruction->InVal;
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 5;

		case (FLAG_VALUE | FLAG_MEMORYOUT):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, 0x00, 0x05) << 8) | 0xC7;
			*(u32 *)(DynaRecMemPtr+3) = (unsigned int)(CurInstruction->OutMemPos);
			*(u16 *)(DynaRecMemPtr+7) = (unsigned short)CurInstruction->InVal;
			return 9;

		case (FLAG_INX86REG | FLAG_MEMORYOUT):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, CurInstruction->InReg, 0x05) << 8) | 0x89;
			*(u32 *)(DynaRecMemPtr+3) = (unsigned int)(CurInstruction->OutMemPos);
			return 7;

		case (FLAG_OUTX86REG | FLAG_MEMORYIN):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, CurInstruction->OutReg, 0x05) << 8) | 0x8B;
			*(u32 *)(DynaRecMemPtr+3) = (unsigned int)(CurInstruction->InMemPos);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 7;

		default:
			return 0;
	};
}

int ASM_Move_32(InstructionInfoStruct *CurInstruction)
{
	//output the bytes for a mov based on required setup
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, CurInstruction->InReg, CurInstruction->OutReg) << 8) | 0x89;
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 2;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x03, 0, CurInstruction->OutReg) << 8) | 0xC7;
			*(u32 *)(DynaRecMemPtr+2) = CurInstruction->InMemPos;
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 6;

		case (FLAG_INX86REG | FLAG_MEMORYOUT):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x00, CurInstruction->InReg, 0x05) << 8) | 0x89;
			*(u32 *)(DynaRecMemPtr+2) = (unsigned int)(CurInstruction->OutMemPos);
			return 6;

		case (FLAG_OUTX86REG | FLAG_MEMORYIN):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x00, CurInstruction->OutReg, 0x05) << 8) | 0x8B;
			*(u32 *)(DynaRecMemPtr+2) = (unsigned int)(CurInstruction->InMemPos);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 6;

		default:
			return 0;
	};
}

int ASM_Move_FromRegMemPtr(InstructionInfoStruct *CurInstruction)
{
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, CurInstruction->OutReg, CurInstruction->InReg) << 8) | 0x8B;
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 3;

		default:
			return 0;
	};
}

int ASM_Move_ToRegMemPtr(InstructionInfoStruct *CurInstruction)
{
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, CurInstruction->InReg, CurInstruction->OutReg) << 8) | 0x89;
			return 3;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			*(u8 *)DynaRecMemPtr = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = (ModRM(0x00, 0, CurInstruction->OutReg) << 8) | 0xC7;
			*(u16 *)(DynaRecMemPtr+3) = CurInstruction->InVal;
			return 5;

		default:
			return 0;
	};
}

int ASM_MoveB_ToRegMemPtr(InstructionInfoStruct *CurInstruction)
{
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x00, CurInstruction->InReg, CurInstruction->OutReg) << 8) | 0x88;
			return 2;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = (ModRM(0x00, 0, CurInstruction->OutReg) << 8) | 0xC6;
			*(u8 *)(DynaRecMemPtr+2) = CurInstruction->InVal & 0xFF;
			return 3;

		default:
			return 0;
	};
}

int ASM_MoveZX_FromRegMemPtr(InstructionInfoStruct *CurInstruction)
{
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = 0xb60f;
			*(u8 *)(DynaRecMemPtr+2) = ModRM(0x00, CurInstruction->OutReg, CurInstruction->InReg);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 3;

		default:
			return 0;
	};
}

int ASM_MoveZX(InstructionInfoStruct *CurInstruction)
{
	int Ret;

	//output the bytes for a mov based on required setup
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u8 *)(DynaRecMemPtr) = 0x66;
			*(u16 *)(DynaRecMemPtr+1) = 0xb60f;
			*(u8 *)(DynaRecMemPtr+3) = ModRM(0x03, CurInstruction->OutReg, CurInstruction->InReg);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 4;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			//write the value to a register
			Ret = ASM_Move(CurInstruction);

			//now movzx the register to itself
			*(u8 *)(DynaRecMemPtr+Ret) = 0x66;
			*(u16 *)(DynaRecMemPtr+1+Ret) = 0xb60f;
			*(u8 *)(DynaRecMemPtr+3+Ret) = ModRM(0x03, CurInstruction->OutReg, CurInstruction->OutReg);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return Ret+4;

		default:
			return 0;
	};
}

int ASM_MoveZX_32(InstructionInfoStruct *CurInstruction)
{
	int Ret;

	//output the bytes for a mov based on required setup
	switch(CurInstruction->Flags & FLAG_X86MASK)
	{
		case (FLAG_INX86REG | FLAG_OUTX86REG):
			*(u16 *)(DynaRecMemPtr) = 0xb70f;
			*(u8 *)(DynaRecMemPtr+2) = ModRM(0x03, CurInstruction->OutReg, CurInstruction->InReg);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return 3;

		case (FLAG_VALUE | FLAG_OUTX86REG):
			//write the value to a register
			Ret = ASM_Move(CurInstruction);

			//now movzx the register to itself
			*(u16 *)(DynaRecMemPtr+Ret) = 0xb70f;
			*(u8 *)(DynaRecMemPtr+2+Ret) = ModRM(0x03, CurInstruction->OutReg, CurInstruction->OutReg);
			RegAssignment[CurInstruction->OutReg].Modified = 1;
			return Ret+3;
		default:
			return 0;
	};
}

int ASM_Nop(InstructionInfoStruct *CurInstruction)
{
	return 0;
}

int ASM_Int3(InstructionInfoStruct *CurInstruction)
{
	*(u8 *)(DynaRecMemPtr) = 0xcc;
	return 1;
}

Instruction2Params(Add,  0x01, 0x81, 0);
Instruction2Params(And,  0x21, 0x81, 4);
Instruction2Params(Cmp,  0x39, 0x81, 7);
Instruction2Params(Or,   0x09, 0x81, 1);
Instruction2Params(Sub,  0x29, 0x81, 5);
Instruction2Params(Xor,  0x31, 0x81, 6);

Instruction1Param(Not,  0xF7, 2);
Instruction1Param(Neg,  0xF7, 3);
Instruction1Param_AXFlagged(Mul,  0xF7, 4);
Instruction1Param_AXFlagged(Div,  0xF7, 6);

Instruction2ParamsCL(Rol, 0xD3, 0xC1, 0xD1, 0);
Instruction2ParamsCL(Ror, 0xD3, 0xC1, 0xD1, 1);
Instruction2ParamsCL(Shl, 0xD3, 0xC1, 0xD1, 4);
Instruction2ParamsCL(Shr, 0xD3, 0xC1, 0xD1, 5);

Instruction1ParamAdd(Inc, 0x40);
Instruction1ParamAdd(Dec, 0x48);

Instruction2Params_32(Add_32,  0x01, 0x81, 0);
Instruction2Params_32(Cmp_32,  0x39, 0x81, 7);
Instruction2Params_32(Sub_32,  0x29, 0x81, 5);
Instruction2ParamsCL_32(Shl_32, 0xD3, 0xC1, 0xD1, 4);

int ASM_Jxx(InstructionInfoStruct *CurInstruction)
{
	int OffsetCalc = 0;
	int i;

	//see if the location is known, otherwise add it to the unknown list
	if(LabelData[CurInstruction->OutReg & ~LABEL] != 0xffff)
	{
		OffsetCalc = LabelData[CurInstruction->OutReg & ~LABEL] - (DynaRecMemPtr - StartDynaRecMemPtr) + 2;
	}
	else
	{
		//label location is unknown, add it to our unknown list
		for(i = 0; i < (sizeof(UnknownLabelData) / sizeof(UnknownLabelDataStruct)); i++)
		{
			if(UnknownLabelData[i].LabelID == 0)
				break;
		}

		//if unknown is too full, then bail
		if(i >= (sizeof(UnknownLabelData) / sizeof(UnknownLabelDataStruct)))
			return 0;

		//got a location, fill it in
		UnknownLabelData[i].LabelID = CurInstruction->OutReg & ~LABEL;
		UnknownLabelData[i].Position = (unsigned short)(DynaRecMemPtr - StartDynaRecMemPtr) + 1;
		if(CurInstruction->InReg != 0xe9)
			UnknownLabelData[i].Position++;
	}

	//if jmp then just output one byte
	if(CurInstruction->InReg == 0xe9)
	{
		*(u8 *)(DynaRecMemPtr) = CurInstruction->InReg;
		i = 1;
	}
	else
	{
		*(u16 *)(DynaRecMemPtr) = CurInstruction->InReg;
		i = 2;
	}

	*(u32 *)(DynaRecMemPtr+i) = OffsetCalc;
	return 4+i;
}

