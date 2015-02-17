#ifndef DYNACOMMON_HEADER
#define DYNACOMMON_HEADER


//640*204 = just shy of 128k of memory
#define DISPLAYWIDTH 640
#define DISPLAYHEIGHT 204

typedef void (*DynaFunc)(void);

extern short StackSize;
extern void *MemoryAlloc;
extern void *MemoryEnd;
extern short *StackPos;
extern unsigned short *Registers;
extern unsigned char *ProgramMemory;
extern unsigned char *DynaMemory;
extern unsigned char *VideoMemory;
extern DynaFunc *DynaLookup;
extern unsigned char *DynaRecMemPtr;
extern unsigned char *StartDynaRecMemPtr;
extern unsigned char *CallStackPos;
extern unsigned char *BeginCallStackPos;

//stack is first so that a function can push data onto the stack before
//returning as a memcpy is done across the rest of the registers
#define PC 0
#define INSTRUCTION_MASK 0xff

typedef void (*InstructFunc)(int Reg1, int Reg2);
extern InstructFunc InterpretterInstructionList[256];
extern InstructFunc DynaRecInstructionList[256];

extern int HandleRFB();
extern int RFBSendFile(unsigned int FileHandle);
extern int BiosSize;
extern int AllowFileUpload;

extern unsigned short Ports[7];
extern int IsBranch;
extern int IsDynarec;
extern int KnownActive;
extern int UnknownInstruction;
extern int PendingFrameBufferUpdate;
extern void CompileDynarec();
extern void RunDynarec(int PCVal);
extern void InitDynarec();
extern void InitInterpretter();
extern void ResetDynarecVars();
extern void ResetDynarec();

extern short PauseCPU;
extern short HaltCPU;

#define X86_REG_EAX	0x8000
#define X86_REG_ECX	0x8001
#define X86_REG_EDX	0x8002
#define X86_REG_EBX	0x8003
//#define STACKSIZE	0x8100
//#define STACK		0x8200	//anything between 0x100 and 0x200 is STACK-val
//#define CALLSTACKPOS	0x0400
#define REG_R0		0x0000
#define REG_R1		0x0001
#define REG_R2		0x0002
#define REG_R3		0x0003
#define REG_R4		0x0004
#define REG_R5		0x0005
#define REG_R6		0x0006
#define REG_R7		0x0007
#define REG_R8		0x0008
#define REG_R9		0x0009
#define REG_R10		0x000a
#define REG_R11		0x000b
#define REG_R12		0x000c
#define REG_R13		0x000d
#define REG_R14		0x000e
#define REG_R15		0x000f
#define TEMPREG		0x8010
#define LABEL		0xf000

#define FLAG_OUTREG	0x0001
#define FLAG_INREG	0x0002
#define FLAG_VALUE	0x0004
#define FLAG_MEMORYIN	0x0008
#define FLAG_MEMORYOUT	0x0010
#define FLAG_INX86REG	0x0020
#define FLAG_OUTX86REG	0x0040
#define FLAG_X86MASK	0x007C

typedef struct KnownRegStruct KnownRegStruct;
typedef struct InstructionInfoStruct InstructionInfoStruct;

typedef void (*CacheFuncPtr)(KnownRegStruct *OutReg, unsigned short InVal);
typedef int (*ASMFuncPtr)(InstructionInfoStruct *CurInstruction);

typedef struct InstructFuncArr
{
	int Position;
	InstructFunc Func;
} InstructFuncArr;

struct KnownRegStruct
{
	unsigned short Register;
	unsigned short InUse;
	unsigned short Value;
};

struct InstructionInfoStruct
{
	union
	{
		unsigned short InReg;
		unsigned short InVal;
		unsigned short GotoLabelID;
		void * InMemPos;
	};
	union
	{
		unsigned short OutReg;
		unsigned short LabelID;
		void * OutMemPos;
	};
	int Flags;
	ASMFuncPtr ASMInstruction;
	CacheFuncPtr CacheInstruction;
	unsigned char *LabelPos;
};

struct RegAssignmentStruct
{
	unsigned short Register;
	unsigned short Modified;
} RegAssignmentStruct;

struct UnknownLabelDataStruct
{
	unsigned short LabelID;
	unsigned short Position;
} UnknownLabelDataStruct;

extern unsigned short LabelData[256];
extern struct UnknownLabelDataStruct UnknownLabelData[256];
extern struct KnownRegStruct KnownRegs[16];
extern int CurLabel;
extern int CurInstructionCount;
extern struct InstructionInfoStruct *InstructionList;
extern struct RegAssignmentStruct RegAssignment[4];
extern unsigned short fd;

extern int WriteKnownRegs();

extern int CreateInstruction(ASMFuncPtr FuncPtr, unsigned int RegIn, unsigned int RegOut, int Flags, CacheFuncPtr CacheFunc);

extern int ASM_Move(InstructionInfoStruct *CurInstruction);
extern int ASM_Move_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Move_FromRegMemPtr(InstructionInfoStruct *CurInstruction);
extern int ASM_Move_ToRegMemPtr(InstructionInfoStruct *CurInstruction);
extern int ASM_MoveB_ToRegMemPtr(InstructionInfoStruct *CurInstruction);
extern int ASM_MoveZX_FromRegMemPtr(InstructionInfoStruct *CurInstruction);
extern int ASM_MoveZX_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Nop(InstructionInfoStruct *CurInstruction);
extern int ASM_Add(InstructionInfoStruct *CurInstruction);
extern int ASM_Add_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Sub(InstructionInfoStruct *CurInstruction);
extern int ASM_Sub_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Or(InstructionInfoStruct *CurInstruction);
extern int ASM_Mul(InstructionInfoStruct *CurInstruction);
extern int ASM_Div(InstructionInfoStruct *CurInstruction);
extern int ASM_Mod(InstructionInfoStruct *CurInstruction);
extern int ASM_And(InstructionInfoStruct *CurInstruction);
extern int ASM_Rol(InstructionInfoStruct *CurInstruction);
extern int ASM_Ror(InstructionInfoStruct *CurInstruction);
extern int ASM_Shl(InstructionInfoStruct *CurInstruction);
extern int ASM_Shl_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Shr(InstructionInfoStruct *CurInstruction);
extern int ASM_Or(InstructionInfoStruct *CurInstruction);
extern int ASM_Xor(InstructionInfoStruct *CurInstruction);
extern int ASM_Not(InstructionInfoStruct *CurInstruction);
extern int ASM_Neg(InstructionInfoStruct *CurInstruction);
extern int ASM_Inc(InstructionInfoStruct *CurInstruction);
extern int ASM_Dec(InstructionInfoStruct *CurInstruction);
extern int ASM_Cmp(InstructionInfoStruct *CurInstruction);
extern int ASM_Cmp_32(InstructionInfoStruct *CurInstruction);
extern int ASM_Jxx(InstructionInfoStruct *CurInstruction);
extern int ASM_Int3(InstructionInfoStruct *CurInstruction);

extern int GetTempReg(InstructionInfoStruct *CurInstruction);
extern int FreeTempReg(InstructionInfoStruct *CurInstruction);

extern int AddLabel(InstructionInfoStruct *CurInstruction);

extern void Known_Move(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Add(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Sub(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Mul(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Div(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Mod(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_And(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Or(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Xor(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Not(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Neg(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Inc(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Dec(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Rol(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Ror(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Shl(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Shl_32(KnownRegStruct *OutReg, unsigned short InVal);
extern void Known_Shr(KnownRegStruct *OutReg, unsigned short InVal);

extern int Known_Activate(InstructionInfoStruct *CurInstruction);
extern int Known_Deactivate(InstructionInfoStruct *CurInstruction);

extern int ClaimCPUReg(InstructionInfoStruct *CurInstruction);
extern int FreeCPUReg(InstructionInfoStruct *CurInstruction);
extern int AssignCPUReg(InstructionInfoStruct *CurInstruction);
extern int AssignRegs(InstructionInfoStruct *InstructInfo);

#define OP_CLAIMREG(REG) ClaimCPUReg, REG, 0, 0, 0
#define OP_FREEREG(REG) FreeCPUReg, REG, 0, 0, 0
#define OP_ASSIGNREG(REG1, REG2) AssignCPUReg, REG1, REG2, 0, 0
#define OP_MOV(REG1, REG2) ASM_Move, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Move
#define OP_MOV_FROM_MEMPTR_32(REG1, REG2) ASM_Move_32, REG1, REG2, FLAG_OUTREG | FLAG_MEMORYIN, 0
#define OP_MOV_TO_MEMPTR_32(REG1, REG2) ASM_Move_32, REG1, REG2, FLAG_MEMORYOUT | FLAG_INREG, 0
#define OP_MOV_TO_MEMPTR(REG1, REG2) ASM_Move, REG1, REG2, FLAG_MEMORYOUT | FLAG_INREG, Known_Move
#define OP_MOV_TO_REGMEMPTR(REG1, REG2) ASM_Move_ToRegMemPtr, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOVB_TO_REGMEMPTR(REG1, REG2) ASM_MoveB_ToRegMemPtr, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOV_TO_MEMPTR_IMM(REG1, IMMVAL) ASM_Move, REG1, IMMVAL, FLAG_MEMORYOUT | FLAG_VALUE, Known_Move
#define OP_MOVZX_FROM_REGMEMPTR(REG1, REG2) ASM_MoveZX_FromRegMemPtr, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOVZX_32(REG1, REG2) ASM_MoveZX_32, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOV_FROM_MEMPTR(REG1, REG2) ASM_Move, REG1, REG2, FLAG_OUTREG | FLAG_MEMORYIN, Known_Move
#define OP_MOV_FROM_REGMEMPTR(REG1, REG2) ASM_Move_FromRegMemPtr, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOV_IMM(REG1, IMMVAL) ASM_Move, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, Known_Move
#define OP_ADD(REG1, REG2) ASM_Add, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Add
#define OP_ADD_IMM(REG1, IMMVAL) ASM_Add, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, Known_Add
#define OP_ADD_IMM_32(REG1, IMMVAL) ASM_Add_32, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, 0
#define OP_SUB(REG1, REG2) ASM_Sub, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Sub
#define OP_SUB_IMM(REG1, IMMVAL) ASM_Sub, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, Known_Sub
#define OP_SUB_IMM_32(REG1, IMMVAL) ASM_Sub_32, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, 0
#define OP_MUL(REG1) ASM_Mul, REG1, REG1, FLAG_OUTREG | FLAG_INREG, 0
#define OP_DIV(REG1) ASM_Div, REG1, REG1, FLAG_OUTREG | FLAG_INREG, 0
#define OP_MOD(REG1, REG2) ASM_Mod, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_AND(REG1, REG2) ASM_And, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_And
#define OP_AND_IMM(REG1, IMMVAL) ASM_And, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, Known_And
#define OP_OR(REG1, REG2) ASM_Or, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Or
#define OP_XOR(REG1, REG2) ASM_Xor, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Xor
#define OP_NOT(REG1) ASM_Not, REG1, REG1, FLAG_OUTREG | FLAG_INREG, Known_Not
#define OP_NEG(REG1) ASM_Neg, REG1, REG1, FLAG_OUTREG | FLAG_INREG, Known_Neg
#define OP_INC(REG1) ASM_Inc, REG1, REG1, FLAG_OUTREG | FLAG_INREG, Known_Inc
#define OP_DEC(REG1) ASM_Dec, REG1, REG1, FLAG_OUTREG | FLAG_INREG, Known_Dec
#define OP_CMP(REG1, REG2) ASM_Cmp, REG1, REG2, FLAG_OUTREG | FLAG_INREG, 0
#define OP_CMP_IMM(REG1, IMMVAL) ASM_Cmp, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, 0
#define OP_CMP_IMM_32(REG1, IMMVAL) ASM_Cmp_32, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, 0
#define OP_INT3() ASM_Int3, 0, 0, 0, 0

#define OP_ROL(REG1, REG2) ASM_Rol, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Rol
#define OP_ROR(REG1, REG2) ASM_Ror, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Ror
#define OP_SHL(REG1, REG2) ASM_Shl, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Shl
#define OP_SHL_IMM(REG1, IMMVAL) ASM_Shl, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, Known_Shl
#define OP_SHL_IMM_32(REG1, IMMVAL) ASM_Shl_32, REG1, IMMVAL, FLAG_OUTREG | FLAG_VALUE, 0
#define OP_SHR(REG1, REG2) ASM_Shr, REG1, REG2, FLAG_OUTREG | FLAG_INREG, Known_Shr

#define OP_JMP(LabelID) ASM_Jxx, LabelID, 0xe9, FLAG_VALUE, 0
#define OP_JL(LabelID) ASM_Jxx, LabelID, 0x8c0f, FLAG_VALUE, 0
#define OP_JAE(LabelID) ASM_Jxx, LabelID, 0x830f, FLAG_VALUE, 0
#define OP_JGE(LabelID) ASM_Jxx, LabelID, 0x8d0f, FLAG_VALUE, 0
#define OP_JBE(LabelID) ASM_Jxx, LabelID, 0x860f, FLAG_VALUE, 0
#define OP_JB(LabelID) ASM_Jxx, LabelID, 0x820f, FLAG_VALUE, 0
#define OP_JE(LabelID) ASM_Jxx, LabelID, 0x840f, FLAG_VALUE, 0
#define OP_JA(LabelID) ASM_Jxx, LabelID, 0x870f, FLAG_VALUE, 0
#define OP_JNE(LabelID) ASM_Jxx, LabelID, 0x850f, FLAG_VALUE, 0
#define OP_ADDLABEL(LabelID) AddLabel, LabelID, 0, FLAG_VALUE, 0
#define OP_GETTEMPREG(ID) GetTempReg, ID, 0, 0, 0
#define OP_FREETEMPREG(ID) FreeTempReg, ID, 0, 0, 0
#define OP_ACTIVATEKNOWN() Known_Activate, 0, 0, 0, 0
#define OP_DEACTIVATEKNOWN() Known_Deactivate, 0, 0, 0, 0

#endif
