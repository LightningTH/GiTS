#include "dynacommon.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sched.h>

char* USER = "dynapwn";
int LPORT = 4546;

#include "common.h"

short StackSize;
void *MemoryAlloc;
void *MemoryEnd;
short *StackPos;
unsigned char *BeginCallStackPos;
unsigned char *CallStackPos;
unsigned short *Registers;
unsigned char *ProgramMemory;
unsigned char *DynaMemory;
unsigned char *VideoMemory;
int IsBranch;
unsigned short fd;
InstructionInfoStruct *InstructionList;
DynaFunc *DynaLookup;
short PauseCPU = 0;
short HaltCPU = 0;
int IsDynarec = 0;
int UnknownInstruction = 0;
unsigned short Ports[7] = {0, 0, 0, 0, 0, 0, 0};
int PendingFrameBufferUpdate = 0;
jmp_buf env;
int BiosSize = 0;
int AllowFileUpload = 0;

typedef void (*sighandler_t)(int);

static void alarmHandle(int sig)
{
	if(sig == SIGALRM)
		longjmp(env, sig);
}

int TenMSPassed(struct timeval *StartTime, struct timeval *CurTime)
{
	
	if(StartTime->tv_sec != CurTime->tv_sec)
		return 1;

	if((StartTime->tv_usec + 10000) <= CurTime->tv_usec)
		return 1;

	return 0;
}

DIR	*CurDirList = NULL;
typedef struct FileDataStruct
{
	unsigned short Size;
	unsigned short Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Min;
	unsigned char IsDir;
} FileDataStruct;

typedef struct FileReadStruct
{
	unsigned short fd;
	unsigned short Pos;
	unsigned short Size;
} FileReadStruct;

int HandleFileRequest(int Port5, int Port6)
{
	struct dirent *dir;
	struct stat filestat;
	struct tm *timedata;
	FileDataStruct *FileData;
	FileReadStruct *ReadData;
	char tempname[100];
	int namelen;
	unsigned char tempval;
	int fd;
	time_t filetime;

	switch(Port5)
	{
		case 1:
			//send file request
			return RFBSendFile(Ports[6]);

		case 2:
			//file upload request
			AllowFileUpload = 1;
			break;

		case 3:
			//open up the home directory
			if(CurDirList)
				closedir(CurDirList);

			CurDirList = opendir(".");
			Ports[6] = 0;
			if(CurDirList)
				Ports[6] = 1;
			break;

		case 4:
			//get a filename from the directory, store at Ports[6]
			namelen = 0;

			if((((unsigned int)Ports[6]) + sizeof(FileDataStruct)) > 65535)
			{
				Ports[6] = -1;
				break;
			}

			//erase the entry
			memset(&ProgramMemory[Ports[6]], 0, 12);

			if(CurDirList)
			{
				do
				{
					dir = readdir(CurDirList);
					if(!dir || dir->d_name[0] != '.')
						break;
				} while(dir);

				if(dir)
				{
					namelen = strlen(dir->d_name);
					if(namelen > 12)
						namelen = 12;

					//name
					memcpy(&ProgramMemory[Ports[6]], dir->d_name, namelen);
					Ports[6] = namelen;
				}
				else
					Ports[6] = 0;

			}
			else
				Ports[6] = 0;

			break;

		case 5:
			//close a directory listing
			if(CurDirList)
				closedir(CurDirList);
			CurDirList = 0;
			break;

		case 6:
			//get file stats based on filename
			if((((unsigned int)Ports[6]) + sizeof(FileDataStruct) + 12) > 65535)
			{
				Ports[6] = 0;
				break;
			}

			//null terminate the name just incase it isn't already terminated
			ProgramMemory[Ports[6]+12] = 0;
			if(stat((unsigned char *)(&ProgramMemory[Ports[6]]), &filestat) == 0)
			{
				filetime = filestat.st_mtime;
				timedata = gmtime(&filetime);

				FileData = (FileDataStruct *)&ProgramMemory[Ports[6] + 12];

				//setup the rest of the data
				FileData->Size = (filestat.st_size & 0xffff);
				FileData->Year = timedata->tm_year + 1900;
				FileData->Month = timedata->tm_mon + 1;
				FileData->Day = timedata->tm_mday;
				FileData->Hour = timedata->tm_hour;
				FileData->Min = timedata->tm_min;

				if(filestat.st_mode & S_IFDIR)
					FileData->IsDir = 1;
			}
			break;

		case 7:
			//open a file if it doesn't contain "key", otherwise give them an invalid file
			if(strstr(&ProgramMemory[Ports[6]], "key"))
			{
				strcpy(tempname, "/tmp/dynakeyXXXXXX");
				Ports[6] = mkstemp(tempname);
				unlink(tempname);

#define NOTKEYSTR "This is not the key you are looking for"
				write(Ports[6], NOTKEYSTR, strlen(NOTKEYSTR));
				lseek(Ports[6], 0, SEEK_SET);
			}
			else
			{
				//some filenames might take up all 12 bytes so temp null end it
				//then restore the value after the open
				tempval = ProgramMemory[Ports[6]+12];
				ProgramMemory[Ports[6]+12] = 0;

				fd = open(&ProgramMemory[Ports[6]], O_RDONLY);
				ProgramMemory[Ports[6]+12] = tempval;
				Ports[6] = (unsigned short)fd;
			}

			break;

		case 8:
			//close a file
			close(Ports[6]);
			break;

		case 9:
			//read a file
			if((((unsigned int)Ports[6]) + sizeof(FileReadStruct)) <= 65535)
			{
				ReadData = (FileReadStruct *)(&ProgramMemory[Ports[6]]);
				if(((unsigned int)(ReadData->Pos) + (unsigned int)(ReadData->Size)) > 65535)
				{
					Ports[6] = 0;
					break;
				}

				Ports[6] = (unsigned short)read(ReadData->fd, &ProgramMemory[ReadData->Pos], ReadData->Size);
			}
			else
				Ports[6] = 0;

			break;
		case 10:
			//get file stats
			if((((unsigned int)Ports[6]) + sizeof(FileDataStruct)) > 65535)
			{
				Ports[6] = 0;
				break;
			}
			
			if(fstat(*(unsigned short *)(&ProgramMemory[Ports[6]]), &filestat) == 0)
			{
				timedata = gmtime(&filestat.st_mtim);

				FileData = (FileDataStruct *)&ProgramMemory[Ports[6]];

				//setup the rest of the data
				FileData->Size = (filestat.st_size & 0xffff);
				FileData->Year = timedata->tm_year;
				FileData->Month = timedata->tm_mon;
				FileData->Day = timedata->tm_mday;
				FileData->Hour = timedata->tm_hour;
				FileData->Min = timedata->tm_min;

				if(filestat.st_mode & S_IFDIR)
					FileData->IsDir = 1;
			}

			Ports[6] = 0;
			break;

		case 11:
			if((((unsigned int)(Ports[6])) + 6) > 65535)
			{
				Ports[6] = 0xffff;
				break;
			}

			//do file seek
			Ports[6] = (unsigned short)lseek(*(unsigned short *)(&ProgramMemory[Ports[6]]), *(unsigned short *)(&ProgramMemory[Ports[6] + 2]), *(unsigned short *)(&ProgramMemory[Ports[6] + 4]));
			break;

		default:
			break;
	};

	return 0;
}

int handleConnection(int in_fd)
{
	short ProgramSize;
	int Alarmed;
	InstructFunc CallFunc;
	unsigned short Instruction;
	struct timeval SelectTimeout;
	fd_set	readfd;
	fd_set	activefd;
	struct timeval StartTime, CurTime;

	//sleep(10);
	memset(&SelectTimeout, 0, sizeof(struct timeval));
	SelectTimeout.tv_sec = 1;

	FD_ZERO(&readfd);

	//go have fun with file descriptors
	int FileHandle;
	FileHandle = open("/dev/urandom", O_RDONLY);
	if(FileHandle == -1)
		return 0;

	read(FileHandle, &fd, sizeof(fd));
	close(FileHandle);

	//make sure fd is 1000 or less for random fd assignment
	fd %= 0x400;
	dup2(in_fd, fd);
	close(in_fd);

	//fd = in_fd;
	FD_SET(fd, &readfd);

	if(InitRFB() == -1)
		return 0;

	Alarmed = 0;
	StackSize = 0;
	IsBranch = 0;
	AllowFileUpload = 0;
	MemoryAlloc = mmap(0, 384*1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	memset(MemoryAlloc, 0, 384*1024);
	MemoryEnd = MemoryAlloc + (384*1024);

	//memory map
	//16 registers			32 bytes
	//stack 			0x3E0 bytes
	//main program memory		64k
	//instruction list for dynarec	358*20 = 7160 bytes
	//page alignment		8 bytes
	//dynarec memory		180k
	//video memory			128k
	//call stack			4k

	Registers = MemoryAlloc;
	StackPos = MemoryAlloc + (16*2);
	ProgramMemory = (unsigned char *)StackPos + 0x400 - (16*2);

	//DataMemory = ProgramMemory + (32*1024);
	//InstructionList = (InstructionInfoStruct *)(DataMemory + (32*1024));
	InstructionList = (InstructionInfoStruct *)(ProgramMemory + (64*1024));
	DynaMemory = MemoryAlloc + (72*1024);
	DynaRecMemPtr = DynaMemory;
	VideoMemory = DynaMemory + (180*1024);
	CallStackPos = VideoMemory + (128*1024);
	BeginCallStackPos = CallStackPos;

	//make sure the dynarec code can only be written and exec'd
	mprotect((void *)DynaMemory, 180*1024, PROT_WRITE | PROT_EXEC);

	//setup alarm
	Alarmed = setjmp(env);
	if(Alarmed)
	{
		munmap(MemoryAlloc, 384*1024);

		//return
		return 0;
	}

	signal(SIGALRM, (sighandler_t)alarmHandle);
	alarm(120);

	int f;
	f = open("bios.bin", O_RDONLY);
	if(f == -1)
	{
		printf("Unable to locate bios.bin\n");
		return 0;
	}

	BiosSize = lseek(f, 0, SEEK_END);
	lseek(f, 0, SEEK_SET);
	read(f, ProgramMemory, BiosSize);
	close(f);

	//IsDynarec = 1;
	/*
	if(!IsDynarec)
		f = open("registers.bin", O_WRONLY | O_CREAT, 0755);
	else
		f = open("registers.bin", O_RDONLY);
	lseek(f, 0, SEEK_SET);
	*/

	UnknownInstruction = 0;

	InitDynarec();

	DynaLookup = malloc((((64*1024)+1) >> 1) * sizeof(DynaFunc));
	memset(DynaLookup, 0, ((64*1024) >> 1) * sizeof(DynaFunc));

	InitInterpretter();
	gettimeofday(&StartTime, 0);

	while(!HaltCPU)
	{
		//ports[0] is the timing interrupt
		gettimeofday(&CurTime, 0);
		if(TenMSPassed(&StartTime, &CurTime))
		{
			Ports[0] = 1;
			PauseCPU = 0;
			memcpy(&StartTime, &CurTime, sizeof(struct timeval));
		}
		

		activefd = readfd;
		SelectTimeout.tv_sec = 0;
		SelectTimeout.tv_usec = 0;
		if(select(fd+1, &activefd, 0, 0, &SelectTimeout) > 0)
		{
			if(HandleRFB() == -1)
				break;
		}

		//if the app indicates and update is ready and the client requested an update
		//then send it
		if(Ports[4] && PendingFrameBufferUpdate)
		{
			if(FrameBufferUpdateRequest(1) == -1)
				break;
		}

		//if the app wants to do file handling then process properly
		if(Ports[5])
		{
			if(HandleFileRequest(Ports[5], Ports[6]) == -1)
				break;

			Ports[5] = 0;
		}

		if(PauseCPU)
		{
			//sleep until the next ten ms tick, not accurate but it works
			usleep(10000);
		}
		else
		{
			if(IsDynarec)
			{
				//dynarec mode

				//if already compiled for the address, call it
				if(DynaLookup[(Registers[PC] >> 1)])
				{
					DynaLookup[(Registers[PC] >> 1)]();
				}
				else
				{
					//loop until we get a branch instruction for a compile block
					int CurPC = Registers[PC];
					ResetDynarecVars();
					IsBranch = 0;
					while(1)
					{
						Instruction = *(short *)(&ProgramMemory[Registers[PC] & ~1]);
						CallFunc = DynaRecInstructionList[Instruction & INSTRUCTION_MASK];
						Instruction >>= 8;
						CallFunc(Instruction & 0x0f, Instruction >> 4); 

						if(IsBranch)
							break;

						Registers[PC] += 2;
					}

					if(UnknownInstruction)
						return 0;

					//if the compile fails, try once more after resetting due to running out of space
					Registers[PC] = CurPC;
					if(CompileInstructions() == -1)
					{
						//try once more after a quick reset
						ResetDynarec();
						IsBranch = 0;
						while(1)
						{
							Instruction = *(short *)(&ProgramMemory[Registers[PC] & ~1]);
							CallFunc = DynaRecInstructionList[Instruction & INSTRUCTION_MASK];
							Instruction >>= 8;
							CallFunc(Instruction & 0x0f, Instruction >> 4); 

							if(IsBranch)
								break;

							Registers[PC] += 2;
						}

						if(UnknownInstruction)
							return 0;

						Registers[PC] = CurPC;
						if(CompileInstructions() == -1)
							return 0;
					}

					DynaLookup[(Registers[PC] >> 1)]();
				}
			}
			else
			{
				//interpreter mode
				IsBranch = 0;
				Instruction = *(short *)(&ProgramMemory[Registers[PC] & ~1]);
				CallFunc = InterpretterInstructionList[Instruction & INSTRUCTION_MASK];

				Instruction >>= 8;

				CallFunc(Instruction & 0x0f, Instruction >> 4);

				if(!IsBranch)
					Registers[PC] += 2;
			}
		}
	};

	close(f);
	munmap(MemoryAlloc, 320*1024);
	return 0;
}
