#ifndef PTABLE_H
#define PTABLE_H

#include "bitmap.h"
#include "pcb.h"
#include "synch.h"

#define MAXPROCESS 10

class PTable
{
private:
	BitMap		*bm;
	PCB		*pcb[MAXPROCESS];
	int		psize;
	Semaphore	*bmsem;		//Dung de ngan chan truong hop nap 2 tien trinh cung luc

public:
	PTable(int size);		//Khoi tao size doi tuong pcb de luu size process. Gan gia tri ban dau la null. Nho khoi tao *bm va *bmsem de su dung
	~PTable();			//Huy doi tuong da tao
	int ExecUpdate(char* filename);		//return PID
	int ExitUpdate(int ec);
	int JoinUpdate(int pID);
	int GetFreeSlot();		//Tim slot trong de luu thong tin cho tien trinh moi
	bool IsExist(int pID); 	//Kiem tra co ton tai process ID nay khong
	void Remove(int pID); 	//Xoa mot processID ra khoi mang quan ly no, khi ma tien trinh nay da ket thuc
	char* GetName(int pID);	//Lay ten cua tien trinh co processID la pID
};
#endif