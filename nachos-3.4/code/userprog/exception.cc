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

int Str2Int(char* buffer, int length)
{
	int num = 0;
	for (int i =0 ; i<length;i++)
	{
		num = num*10 +(int)(buffer[i]-48);
	}
	return num;
}

char* Int2Str(int num)
{
	int length =0;
	char* str1 = new char[256];
	while (num!=0)
	{
		str1[length++] = (num%10+'0');
		num=num/10;
		if (num==0) 
		break;
	}
	char* str = new char[length+1];
	str[length]=0;
	for (int i = 0; i<length; i++)
	{
		str[i]=str1[length-1-i];
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

		case SC_Close:{
			int fileId = machine->ReadRegister(4);
			if (0 <= fileId && fileId <= 10){
				if (fileSystem->open_file[fileId]){
					delete fileSystem->open_file[fileId];
					fileSystem->open_file[fileId] = NULL;
					machine->WriteRegister(2, 0);
					break;
				}
			}
			machine->WriteRegister(2, -1);
			break;
		}

		case SC_Seek:{
			int pos = machine->Readregister(4);
			int id = machine->Readregister(5);

			//Kiem tra su ton tai cua file
			if (fileSystem->open_file[id] == NULL){
				printf("\nKhong the seek vi file khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			
			//Kiem tra id file nam trong bang mo ta file khong
			if (id < 0 || id > 10){
				printf("\nKhong the seek vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			} 
			
			//Kiem tra co goi Seek tren console khong
			if (id == 0 || id == 1){
				printf("\nKhong the seek tren file console.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}

			//Neu pos = -1  thi gan pos = Length, neu khong thi giu nguyen pos
			if (pos == -1){
				pos = fileSystem->open_file[id]->Length();
			}

			//Kiem tra su hop le cua pos
			if (pos > fileSystem->open_file[id]->Length() || pos < 0){
				prinf("\nKhong the seek file den vi tri nay.");
				machine->WriteRsgister(2, -1);
			}
			//Tra ve vi tri di chuyen thuc su trong file
			else{
				fileSystem->open_file[id]->Seek(pos);
				machine->WriteRegister(2, pos);
			}
			IncreasePC();
			break;
		}
			

		case SC_Fork:
			break;

		case SC_Yield:
			break;

		case SC_ReadString:{
			int virtAddr, length;
			char* buffer;
			// Lay tham so ten tap tin to thanh ghi r4
			virtAddr = machine->ReadRegister(4);
			
			// Lay do dai toi da cua chuoi nhap vao thu thanh ghi r5
			length = machine->ReadRegister(5);
			
			//Copy chuoi tu User Space sang System Space
			buffer = User2System(virtAddr, length);
			
			//Doc chuoi
			gSynchConsole->Read(buffer, length);
			delete buffer;
			IncreasePC();
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
			char *buffer= new char[256];
			int length = gSynchConsole->Read(buffer,255);
			int num = Str2Int(buffer,length);
			machine->WriteRegister(2,num);
			delete buffer;
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
			break;

		case SC_PrintChar:
			break; 
		
		case SC_Ascii:{
			for (int i = 0; i <256; i++){
				printf("%c ", char(i));
			}
			break;
		}
			

		case SC_Help:
			break;

		case SC_Sort:
			break;
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
