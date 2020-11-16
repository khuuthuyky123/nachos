#include "syscall.h"
#include "copyright.h"

int main()
{
    // int a = ReadInt();
    // PrintInt(a);

    int len = 300;
    char buffer[300];
    ReadString(buffer,len);
    PrintString(buffer);
    return 0;
}