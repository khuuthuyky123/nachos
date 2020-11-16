/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int A[1024];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp;
    //PrintString("Nhap vao so luong phan tu cua array:");
    int n = ReadInt();
    for (i = 0; i < n; i++)
	{
	//PrintString("Nhap phan tu:");
	int t = ReadInt();
        A[i] = t;
	}

    /* then sort! */
    for (i = 0; i < n-1; i++)
        for (j = i; j < (n-1 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
    // then print A[]
    //PrintString("array sau khi sort:");
    for (i = 0; i <  n; i++)
	PrintInt(A[i])

    return 0;
}

	
