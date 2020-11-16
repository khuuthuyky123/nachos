// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32
#define MAXINT 2147483647
#define MININT -2147483648
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
	int i;
	int ch;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1];
	if (kernelBuf == NULL)
	{
		return kernelBuf;
	}
	memset(kernelBuf, 0, limit + 1);
	for (i = 0; i < limit; ++i)
	{
		machine->ReadMem(virtAddr + i, 1, &ch);
		kernelBuf[i] = ch;
		if (ch == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int ch = 0;
	do
	{
		ch = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, ch);
		i++;
	} while (i < len && ch != 0);
	return i;
}

void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, counter);
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	machine->WriteRegister(NextPCReg, counter + 4);
}
char* Int2Str(int num);
int Str2Int(char* buffer, int length,int* error)
{
	int num = 0;
	int curPos = 0;
	int startPos = 0;
	int sign = 0;
	char* limit;
	*error = 0;
	
	if (buffer[0]=='-')
	{
		startPos = 1;
		curPos = 1;
		sign = 1;
	}
	else 
		if (buffer[0]=='+')
		{
			startPos = 1;
			curPos = 1;
			sign = 0;
		}

	while (buffer[curPos]>='0' && buffer[curPos]<='9')
		curPos++;
	
	if (curPos < length)
	{
		*error = 2;
	}
	else
	{
		if ((startPos==0 && length>10 && sign==0) || (startPos==1 && length>11 && sign==1))
			*error =1;
		else 
		{
			if ((startPos==1 && length==11) || (startPos==0 && length==10))
			{
				if (sign)
					limit = Int2Str(MININT);
				else 
					limit = Int2Str(MAXINT);
				
				for (curPos=startPos ; curPos<length ; curPos++)
				{
					if ((buffer[curPos]-'0')>(limit[curPos+(sign-length+10)*1]-'0'))
						*error = 1;
				}
				delete limit;
			}
			if (*error ==0)
			{
				for (curPos=startPos ; curPos<length ; curPos++)
				{
					num = num*10 +(int)(buffer[curPos]-48);
				}
			}
		}
	}

	if (*error!=0) 
		return 0;
	if (sign)
		return (num*(-1));
	return num;
}

char* Int2Str(int num)
{
	int length =0;
	int sign = 0;
	char* str1 = new char[256];
	if (num==MININT)
	{
		delete str1;
		char* str = new char[12];
		str[0]='-'; str[1]='2'; str[2]='1'; str[3]='4'; str[4] = '7'; str[5] = '4'; str[6]= '8'; 
		str[7]='3'; str[8]='6'; str[9]='4'; str[10]='8'; str[11]=0;
		return str;
	}
	if (num<0)
	{
		length = 1;
		sign = 1;
		str1[0]='-';
		num *=-1;
	}
	else 
		if (num==0)
		{	
			delete str1;
			char* str = new char[2];
			str[0]='0';
			str[1]=0;
			return str;
		}
		
	while (num!=0)
	{
		str1[length++] = (num%10+'0');
		num=num/10;
		if (num==0) 
		break;
	}
	char* str = new char[length+1];
	str[length]=0;

	if (sign==1)
		str[0]=str1[0];

	for (int i = sign; i<length; i++)
	{
		str[i]=str1[length-1 +sign - i];
	}

	delete str1;
	return str;
		
}

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException: // No valid translation found
	{
		printf("\n PageFaultException: No valid translation found\n");
		interrupt->Halt();
		break;
	}
	case ReadOnlyException: // Write attempted to page marked "read-only"
	{
		printf("\n ReadOnlyException: Write attempted to page marked \"read-only\"");
		interrupt->Halt();
		break;
	}
	case BusErrorException: // Translation resulted in an invalid physical address
	{
		printf("\n BusErrorException: Translation resulted in an invalid physical address");
		interrupt->Halt();
		break;
	}
	case AddressErrorException: // Unaligned reference or one that was beyond the end of the address space
	{
		printf("\n AddressErrorException: Unaligned reference or one that was beyond the end of the address space");
		interrupt->Halt();
		break;
	}
	case OverflowException: // Integer overflow in add or sub.
	{
		printf("\n OverflowException: Integer overflow in add or sub ");
		interrupt->Halt();
		break;
	}
	case IllegalInstrException: // Unimplemented or reserved instr.
	{
		printf("\n IllegalInstrException: Unimplemented or reserved instr ");
		interrupt->Halt();
		break;
	}
	case NumExceptionTypes:
	{
		printf("\n NumExceptionTypes\n");
		interrupt->Halt();
		break;
	}
	case SyscallException:
	{
		switch (type)
		{
		case SC_Halt:
		{
			DEBUG('a', "Shutdown, initiated by user program.\n");
			printf("\nShutdown, initiated by user program.\n");
			interrupt->Halt();
			break;
		}
		case SC_Create:
		{
			int virtAddr;
			char *filename;
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");

			// Lấy tham số tên tập tin từ thanh ghi r4
			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");

			// MaxFileLength là = 32
			filename = User2System(virtAddr, MaxFileLength + 1);

			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1); // trả về lỗi cho chương
				// trình người dùng
				delete filename;

				break;
			}

			DEBUG('a', "\n Finish reading filename.");
			//DEBUG('a',"\n File name : '"<<filename<<"'");
			// Create file with size = 0
			// Dùng đối tượng fileSystem của lớp OpenFile để tạo file,
			// việc tạo file này là sử dụng các thủ tục tạo file của hệ điều
			// hành Linux, chúng ta không quản ly trực tiếp các block trên
			// đĩa cứng cấp phát cho file, việc quản ly các block của file
			// trên ổ đĩa là một đồ án khác

			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error create file '%s'", filename);
				machine->WriteRegister(2, -1);
				delete filename;

				break;
			}

			machine->WriteRegister(2, 0); // trả về cho chương trình

			// người dùng thành công
			delete filename;
			break;
		}
		case SC_Exit:
			break;

		case SC_Exec:
			break;

		case SC_Join:
			break;

		case SC_Open:
			break;

		case SC_Read:
			break;

		case SC_Write:
			break;

		case SC_Close:
			break;

		case SC_Seek:
			break;

		case SC_Fork:
			break;

		case SC_Yield:
			break;

		case SC_ReadString:{
			int virtAddr, length;
			char *buffer;
			// Lay tham so ten tap tin to thanh ghi r4
			virtAddr = machine->ReadRegister(4);

			// Lay do dai toi da cua chuoi nhap vao thu thanh ghi r5
			length = machine->ReadRegister(5);

			//Copy chuoi tu User Space sang System Space
			buffer = User2System(virtAddr, length);

			//Doc chuoi
			gSynchConsole->Read(buffer, length);
			System2User(virtAddr, length, buffer);
			delete buffer;
			break;
		}

		case SC_PrintString:
		{
			int virtAddr, length;
			char *buffer;
			length = 0;
			// Lay tham so ten tap tin to thanh ghi r4
			virtAddr = machine->ReadRegister(4);

			//Copy chuoi tu User Space sang System Space
			buffer = User2System(virtAddr, 255);

			//Tinh do dai that cua chuoi
			while (buffer[length] != 0)
				length++;

			//In chuoi
			gSynchConsole->Write(buffer, length + 1);
			// nguoi dung thanh cong
			delete buffer;
			break;
		}
		
		case SC_ReadInt:
		{
			int *error = new int;
			char *buffer= new char[256];
			int length = gSynchConsole->Read(buffer,255);
			int num = Str2Int(buffer,length,error);
			if (*error==1)
				printf("\nSo nguyen nhap vao phai nam trong khoang [%d,%d]\n",MININT,MAXINT);
			if (*error==2)
				printf("\nChuoi so nguyen vua nhap co chua ki tu. Vui long nhap so nguyen 4 bytes hop le\n");
			machine->WriteRegister(2,num);
			delete buffer;
			delete error;
			break;
		}
		
		case SC_PrintInt:
		{	
			int num;
			int length=0;
			char *buffer;
			num = machine->ReadRegister(4);
			buffer = Int2Str(num);
			while (buffer[length] != 0)
				length++;
			gSynchConsole->Write(buffer, length+1);
			delete buffer;
			break;
		}

		case SC_ReadChar:
		{
		//Input: Khong co
			//Output: Duy nhat 1 ky tu (char)
			//Cong dung: Doc mot ky tu tu nguoi dung nhap
			int maxBytes = 255;
			char* buffer = new char[255];
			int numBytes = gSynchConsole->Read(buffer, maxBytes);

			if(numBytes > 1) //Neu nhap nhieu hon 1 ky tu thi khong hop le
			{
				printf("Chi duoc nhap duy nhat 1 ky tu!");
				DEBUG('a', "\nERROR: Chi duoc nhap duy nhat 1 ky tu!");
				machine->WriteRegister(2, 0);
			}
			else if(numBytes == 0) //Ky tu rong
			{
				printf("Ky tu rong!");
				DEBUG('a', "\nERROR: Ky tu rong!");
				machine->WriteRegister(2, 0);
			}
			else
			{
				//Chuoi vua lay co dung 1 ky tu, lay ky tu o index = 0, return vao thanh ghi R2
				char c = buffer[0];
				machine->WriteRegister(2, c);
			}

			delete buffer;
			//IncreasePC(); // error system
			//return;
			break;
		}

		case SC_PrintChar:
		{
			// Input: Ki tu(char)
			// Output: Ki tu(char)
			// Cong dung: Xuat mot ki tu la tham so arg ra man hinh
			char c = (char)machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
			gSynchConsole->Write(&c, 1); // In ky tu tu bien c, 1 byte
			//IncreasePC();
			break;

		}
		
		default:
		{
			printf("\n Unexpected user mode exception (%d %d)", which, type);
			break;
		}
		}
		IncreasePC();
	}
	}
}
