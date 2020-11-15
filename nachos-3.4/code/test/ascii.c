#include "syscall.h"
#include "copyright.h"

int main()
{
    int i = 0;
    int j = 0;
    PrintString("\n\t|-----------------------------------------------|");
    PrintString("\n\t|   Cac ky tu co the in duoc cua bang ma ASCII\t|");
    PrintString("\n\t|\t\t\t\t\t\t|\n");
    for (i = 32; i<127 ; i++)
    {
        PrintString("\t|\t");
        for (j=i;j<i+15 && j<127;j++)
        {
            PrintChar((char)(j));
            PrintChar(' ');
        }
        PrintString("  \t|\n");
        i = j;
    }
    PrintString("\t|-----------------------------------------------|\n");
    return 0;
}