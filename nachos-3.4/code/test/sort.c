/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int
main()
{
    int A[100];
    int i, j, tmp ,t;
    int n; 
    char choose;
    // Yeu cau nhap so luong phan tu cua mang tu ban phim
    PrintString("Nhap vao so luong phan tu n cua array (n<=100). n = ");
    n = ReadInt();
    if (n>100 || n <=0)
    {
        PrintString("\nGia tri khong hop le. Thoat chuong trinh...\n");
        return 0;
    }
    for (i = 0; i < n; i++) // Yeu cau nguoi dung nhap tung phan tu
	{
        PrintString("Nhap phan tu A[");
        PrintInt(i);
        PrintString("] = ");
        A[i] = ReadInt();
	}
    choose = '0';
    while ( choose != '1' && choose != '2' ) // Yeu cau nguoi dung chon cach sap xep 1.Tang 2.Giam, neu nhap sai se yeu cau nhap lai
    {
        PrintString("\nSort tang dan hay giam dan: \n1.Tang dan\n2.Giam dan\nLua chon: ");
        choose = ReadChar();
        if ( choose != '1' && choose != '2' )
                PrintString("Ban nhap sai\n");
    }
    
    if (choose == '1') // Sap xep mang tang dan
    {
    
        for (i = 0; i < n - 1; i++)
            for (j = n - 1; j > i; j--)
                if (A[j] < A[i]) 
                {	/* out of order -> need to swap ! */
                    tmp = A[j];
                    A[j] = A[i];
                    A[i] = tmp;
                }
    }
    else if (choose == '2') // Sap xep mang giam dan
        {
            for (i = 0; i < n-1; i++)
                for (j = n - 1; j > i; j--)
                    if (A[j] > A[i]) 
                    {	/* out of order -> need to swap ! */
                        tmp = A[j];
                        A[j] = A[i];
                        A[i] = tmp;
                    }
        }
 
    // then print A[]
    PrintString("\nMang sau khi sort: ");
    for (i = 0; i < n; i++)
    {
	    PrintInt(A[i]);
        PrintChar(' ');
    }
    return 0;
}