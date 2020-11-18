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

// Input: Địa chỉ của vùng nhớ trên user chứa dữ liệu
// 	  Kích cỡ tối đa vùng nhớ
// Output:Dữ liệu được đọc ở dạng chuỗi kí tự, trỏ đến bằng con trỏ char*
// Chức năng: Chuyển dữ liệu từ User Space sang Kernel Space
char *User2System(int virtAddr, int limit)
{
	// Input: virtAddr: khong gian dia chi User (int), 
	// limit: gioi han cua buffer(int)
	int i; // Khai bao bien dem
	int ch; // Khai bao ky tu 
	char *kernelBuf = NULL; //Khai bao chuoi 
	kernelBuf = new char[limit + 1]; //Can cho cho terminal

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

// Input: Địa chỉ của vùng nhớ trên user nhận dữ liệu
// 	  Kích cỡ bộ đệm
//	  Con trỏ tới bộ đệm
// Output:Dữ liệu được đọc trả về cho User có kích cỡ là int
// Chức năng: Chuyển dữ liệu từ Kernel Space sang User Space
int System2User(int virtAddr, int len, char *buffer)
{
	// Input: virtAddr: Khong gian vung nho User(int), 
	// len: gioi han cua buffer(int), 
	// buffer: bo nho dem buffer(char*)

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

// Input: 	num: số nguyên cần chuyển
// Output:	Chuỗi tương ứng
// Chức năng:	chuyển số thành chuỗi tương ứng, trỏ đến bằng con trỏ char
char* Int2Str(int num);

// Input: 	buffer: con trỏ đến bộ đệm char, 
//		length: chiều dài của bộ đệm, 
//		error: con trỏ đến mã lỗi khi chạy hàm
// Output:	số nguyên chuyển được
// Chức năng:	chuyển số thành chuỗi tương ứng, trỏ đến bằng con trỏ char
int Str2Int(char* buffer, int length,int* error)
{
	int num = 0;		// kết quả trả về
	int curPos = 0;		// vị trí hiện tại đang xét
	int startPos = 0; 	// vị trí đầu tiên của phần giá trị số
	int sign = 0;		// dấu của số.
				// '-' là 1. '+' là 0
	char* limit;		// giới hạn của số nguyên. Có thể là MAXINT hoặc MININT
	*error = 0;		// mã lỗi mặc định là 0
	
	// Nếu kí tự đầu tiên là '-' hoặc '+' thì cập nhật biến sign, curPos và StartPos.
	if (buffer[0]=='-')
	{
		startPos = 1; 	// bắt đầu đọc từ vị trí 1 do bỏ qua kí tự dấu
		curPos = 1;	// vị trí hiện tại là 1
		sign = 1;	// dấu là '-'
	}
	else 
		if (buffer[0]=='+')
		{
			startPos = 1; 	// bắt đầu đọc từ vị trí 1 do bỏ qua kí tự dấu
			curPos = 1;	// vị trí hiện tại là 1
			sign = 0;	// dấu là '+'
		}

	// Đọc đến hết chuỗi cập nhật curPos
	while (buffer[curPos]>='0' && buffer[curPos]<='9')
		curPos++;
	
	//Nếu curPos không ở cuối chuỗi nghĩa là đã có kí tự khác kí tự số. Mã lỗi = 2
	if (curPos < length)
	{
		*error = 2;
	}
	else
	{
		// Xét các trường hợp tràn số, số có các dạng:
		//
		// TH1_1:  21..231	-> dấu bằng 0, chiều dài chuỗi bằng 10 (là chiều dài MAXINT)
		// TH1_2:  21..231	-> dấu bằng 0, chiều dài chuỗi lớn hơn 10 (là chiều dài MAXINT)
		// TH2_1: -21..231	-> dấu bằng 1, chiều dài chuỗi bằng 11 (là chiều dài MININT)
		// TH2_2: -21..231	-> dấu bằng 1, chiều dài chuỗi lớn hơn 11 (là chiều dài MININT)
		// TH3_1: +21..312	-> dấu bằng 1, chiều dài chuỗi bằng 11 (là chiều dài MAXINT)
		// TH3_2: +21..312	-> dấu bằng 1, chiều dài chuỗi lớn hơn 11 (là chiều dài MAXINT)

		// TH1_2 và TH2_2
		if ((startPos==0 && length>10 && sign==0) || (startPos==1 && length>11 && sign==1))
			*error =1;
		else 
		{
			// TH2_1, TH1_1, TH3_1, TH3_2
			if ((startPos==1 && length==11) || (startPos==0 && length==10))
			{
				if (sign)
					limit = Int2Str(MININT);
				else 
					limit = Int2Str(MAXINT);

				// Tiến hành so sánh từng chữ số trong phần giá trị. 
				// Nếu kí tự của buffer lớn hơn kí tự của limit thì nghĩa là tràn số.
				for (curPos=startPos ; curPos<length ; curPos++)
				{
					if ((buffer[curPos]-'0')>(limit[curPos+(sign-length+10)*1]-'0'))
						*error = 1;
				}
				delete limit;
			}

			// Tới đây vẫn không có lỗi thì số hợp lệ. Tiến hành chuyển đổi
			//	Chuyển đổi bằng cách lấy từng kí tự của buffer chuyển thành số rồi cộng dồn lại vào kết quả
			if (*error ==0)
			{
				for (curPos=startPos ; curPos<length ; curPos++)
				{
					num = num*10 +(int)(buffer[curPos]-48);
				}
			}
		}
	}

	// Nếu có lỗi thì trả về 0
	if (*error!=0) 
		return 0;

	// Nếu không có lỗi thì trả về số tương ứng với dấu
	if (sign)
		return (num*(-1));
	return num;
}

// Input: 	num: số nguyên cần chuyển
// Output:	Chuỗi tương ứng
// Chức năng:	chuyển số thành chuỗi tương ứng, trỏ đến bằng con trỏ char
char* Int2Str(int num)
{
	int length =0;			// chiều dài của số
	int sign = 0;			// dấu của số. số dương sign = 0, số âm sign = 1
	char* str1 = new char[256];	// chuỗi lưu tạm ban đầu.
	
	// Xét trường hợp đặc biệt, số là MININT thì trả thẳng kết quả chuỗi của MININT
	if (num==MININT)
	{
		delete str1;
		char* str = new char[12];
		str[0]='-'; str[1]='2'; str[2]='1'; str[3]='4'; str[4] = '7'; str[5] = '4'; str[6]= '8'; 
		str[7]='3'; str[8]='6'; str[9]='4'; str[10]='8'; str[11]=0;
		return str;
	}
	
	// Nếu số <0 thì cập nhật lại sign, length, num, thêm kí tự '-' vào chuỗi kết quả
	if (num<0)
	{
		length = 1;
		sign = 1;
		str1[0]='-';
		num *=-1;
	}
	else 
		// Nếu số =0 thì trả thẳng ra chuỗi '0\0'
		if (num==0)
		{	
			delete str1;
			char* str = new char[2];
			str[0]='0';
			str[1]=0;
			return str;
		}
	
	// Lặp tách từng chữ số từ hàng đơn vị, chuyển thành kí tự và thêm vào mảng kết quả tạm. 
	while (num!=0)
	{
		str1[length++] = (num%10+'0');
		num=num/10;
		if (num==0) 
		break;
	}
	
	// Như vậy thì mảng tạm sẽ lưu ngược thứ tự với giá trị số. 
	// Ví dụ số là -678 thì mảng tạm sẽ là -876. 
	// Do đó phải đảo lại cho đúng
	char* str = new char[length+1];
	str[length]=0;
	
	//Gán dấu nếu là số âm
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
	
		// Hàm tạo file mẫu trong file hướng dẫn 
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

		// SystemCall ReadString
		case SC_ReadString:{
			/*
			Input: buffer: chuoi nhap vao (char*), length: do dai toi da cua chuoi nhap vao (int)
			Output: Khong co
			Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da
			*/

			int virtAddr, length;
			char *buffer;
			// Lay đia chi cua buffer tu thanh ghi r4
			virtAddr = machine->ReadRegister(4);

			// Lay do dai toi da cua chuoi nhap vao tu thanh ghi r5
			length = machine->ReadRegister(5);

			//Copy chuoi tu User Space sang Kernel Space
			buffer = User2System(virtAddr, length);

			gSynchConsole->Read(buffer, length); //Goi ham Read cua SynchConsole de doc chuoi

			//Copy chuoi tu Kernel Space sang User Space
			System2User(virtAddr, length, buffer); 
			delete buffer;
			IncreasePC();
			break;
		}

		//System Call PrintString
		case SC_PrintString:
		{
			/*
			Input: buffer: chuoi nhap vao (char*)
			Output: chuoi doc duoc tu buffer
			Cong dung: In ra mot chuoi voi tham so la buffer
			*/
			int virtAddr, length;
			char *buffer;
			length = 0;
			// Lay dia chi cua buffer tu thanh ghi r4
			virtAddr = machine->ReadRegister(4);

			//Copy chuoi tu User Space sang Kernel Space
			buffer = User2System(virtAddr, 255);

			//Tinh do dai that cua chuoi
			while (buffer[length] != 0)
				length++;

			//In chuoi bang cach goi ham Write cua SynchConsole
			gSynchConsole->Write(buffer, length + 1);
			delete buffer;
			IncreasePC();
			break;
		}
		
		// System Call ReadInt
		case SC_ReadInt:
		{	
			// 	Nguyên mẫu hàm: int ReadInt();
			// 		 Input:	Không có
			// 		Output: Số nguyên
			// 	     Chức năng:	Đọc số nguyên nhập từ bàn phím
			//
			int *error = new int; 		// Khởi tạo biến chứa mã lỗi
			char *buffer= new char[256];	// Khởi tạo bộ nhớ đệm
			int length = gSynchConsole->Read(buffer,255);	// đọc chuỗi từ Console vào bộ đệm, trả về chiều dài bộ đệm là length
			int num = Str2Int(buffer,length,error);		// Gọi hàm chuyển chuỗi thành số
	
			// mã lỗi 1 nghĩa là tràn số
			// mã lỗi 2 nghĩa là nhập vào không phải số
			if (*error==1)	
				printf("\nSo nguyen nhap vao phai nam trong khoang [%d,%d]\n",MININT,MAXINT);
			if (*error==2)
				printf("\nChuoi so nguyen vua nhap co chua ki tu. Vui long nhap so nguyen 4 bytes hop le\n");
			
			// ghi kết quả ra thanh ghi 2, là thanh ghi chứa địa chỉ trả về của hàm
			machine->WriteRegister(2,num);
	
			// Giải phóng các vùng nhớ cấp phát động
			delete buffer;
			delete error;
			break;
		}
		
		case SC_PrintInt:
		{	
			// 	Nguyên mẫu hàm: void PrintInt(int num);
			// 		 Input:	số nguyên
			// 		Output: không có
			// 	     Chức năng:	In số nguyên ra màn hình Console
			//
			int num;	// số nguyên 
			int length=0;	// chiều dài bộ đệm
			char *buffer;	// bộ đệm
			num = machine->ReadRegister(4); // đọc số nguyên từ thanh ghi
			buffer = Int2Str(num);		// Gọi hàm đổi số thành chuỗi
			
			// Tính chiều dài bộ đệm
			while (buffer[length] != 0)
				length++;
			
			// In bộ đệm ra Console
			gSynchConsole->Write(buffer, length+1);
			
			// Giải phóng bộ đệm
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
		// Tăng program counter để không bị loop.
		IncreasePC();
	}
	}
}
