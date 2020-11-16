#include "syscall.h"
#include "copyright.h"

int main()
{
	//PrintString("Nhap vao 1 ki tu:");
	char a = ReadChar();
	//char a = 'x';
	//PrintString("Ki tu do la:");
	PrintChar(a);
	return 0;
}