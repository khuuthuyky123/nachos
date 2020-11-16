/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

	/* size of physical memory; with code, we'll run out of space!*/


int
main()
{
    int A[100];
    int i, j, tmp ,t;
    int n; 
    char choose;
    PrintString("Nhap vao so luong phan tu cua array: ");
    n = ReadInt();
    for (i = 0; i < n; i++)
	{
        PrintString("Nhap phan tu: ");
        A[i] = ReadInt();
            //A[i] = t;
	}

    PrintString("Sort tang dan hay giam dan: \n1.Tang dan\n2.Giam dan\nLua chon: ");
    choose = ReadChar();
    
    if (choose == '1')
    {
    
        for (i = 0; i < n-1; i++)
            for (j = i; j < n; j++)
                if (A[j] < A[i]) 
                {	/* out of order -> need to swap ! */
                    tmp = A[j];
                    A[j] = A[i];
                    A[i] = tmp;
                }
    }
    else if (choose == '2')
        {
            for (i = 0; i < n-1; i++)
                for (j = i; j < n; j++)
                    if (A[j] > A[i]) 
                    {	/* out of order -> need to swap ! */
                        tmp = A[j];
                        A[j] = A[i];
                        A[i] = tmp;
                    }
        }
    // then print A[]
    PrintString("Array sau khi sort: ");
    for (i = 0; i < n; i++)
    {
	    PrintInt(A[i]);
        PrintChar(' ');
    }
    return 0;
}
