#include "ptable.h"
#include "system.h"

PTable::PTable(int size)
{
	if (size < 0)
		return;
	int i = 0;
	psize = size;
	bm = new BitMap(size);
	bmsem = new Semaphore("bmsem",1);
		
	for(i = 0 ; i < MAXPROCESS ; ++i)
		pcb[i] = 0;
	bm->Mark(0);
	pcb[0] = new PCB(0);
	pcb[0]->SetFileName("./test/scheduler");
	pcb[0]->parentID = -1;

}

PTable::~PTable()
{
	int i=0;
	if(bm!=NULL)
		delete bm;
	if(bmsem!=NULL)
		delete bmsem;
	for(i=0; i<MAXPROCESS; i++)
		if(pcb[i]!=NULL)
			delete pcb[i];
}

//--------------------------------------------------------------------

int PTable::ExecUpdate(char* filename)
{
	bmsem->P();			//chi nap 1 tien trinh vao mot thoi diem
	if(filename == NULL)
	{
		printf("\nPTable::Exec : Can't not execute name is NULL.\n");
		bmsem->V();
		return -1;
	}	

//Kiem tra chuong trinh duoc goi co la chinh no khong
	if(!strcmp(filename,"./test/scheduler") || !strcmp(filename,currentThread->getName()))
	{
		printf("\nLoi: khong duoc phep goi exce chinh no !!!\n");
		bmsem->V();
		return -1;
	}

	//Kiem tra con slot trong khong
	int ID= GetFreeSlot();
	if(ID==-1)
	{
		printf("\nLoi: Da vuot qua 10 tien trinh !!!\n");
		bmsem->V();
		return -1;
	}

//Kiem tra file co ton tai tren may khong

	OpenFile *executable = fileSystem->Open(filename);

	if (executable == NULL) 
	{
		printf("\nUnable to open file %s\n", filename);
		bmsem->V();
		return -1;
    }
	delete executable;			// close file

	pcb[ID]= new PCB(ID);
	bm->Mark(ID);
	int pID= pcb[ID]->Exec(filename,ID);

	bmsem->V();
	return pID;
}

int PTable::ExitUpdate(int ec)
{
//Kiem tra pID co ton tai khong
	int pID= currentThread->processID;
	if(!IsExist(pID))
	{
		printf("\nLoi: Tien trinh khong ton tai !!!\n");
		return -1;
	}
//////////////////////////////////////////////////////////////

//Neu la main process thi Halt()
	if(pID==0)
	{
		currentThread->FreeSpace();
		interrupt->Halt();
		return 0;
	}
/////////////////////////////////////////////////////////////

	pcb[pID]->SetExitCode(ec);
	
	pcb[pcb[pID]->parentID]->DecNumWait();
	pcb[pID]->JoinRelease();
	pcb[pID]->ExitWait();
	Remove(pID);	
	return ec;
}

int PTable::JoinUpdate(int pID)
{
	
	if(pID <= 0 || pID > 9)
	{
		printf("\nLoi: Khong ton tai process: id = %d\n",pID);
		return -1;
	}

	if (pcb[pID] == NULL)
	{
		printf("Loi: Khong ton tai process id nay!");
		return -1;
	}

//kiem tra tien trinh dang chay co la cha cua tien trinh can join hay khong
	if(currentThread->processID != pcb[pID]->parentID)
	{
		printf("\nLoi: Ko duoc phep join vao tien trinh khong phai cha cua no !!!\n");
		return -1;
	}
/////////////////////////////////////////////////////////////////////////////////////////////
	
	pcb[pcb[pID]->parentID]->IncNumWait();
	pcb[pID]->JoinWait(); 	//doi den khi tien trinh con ket thuc

	int ec = pcb[pID]->GetExitCode();

	if(ec != 0)
	{
		printf("\nProcess exit with exitcode EC = %d ",ec);
		return -1;
	}

	pcb[pID]->ExitRelease();	//cho phep tien trinh con ket thuc
	
	return 0;
}

void PTable::Remove(int pID)
{
	if(pID<0 || pID>9)
		return;
	if(bm->Test(pID))
	{
		bm->Clear(pID);
		delete pcb[pID];
	}
}

//----------------------------------------------------------------------------------------------
int PTable::GetFreeSlot()
{
	return bm->Find();
}

bool PTable::IsExist(int pID)
{
	if(pID<0 || pID>9)
		return 0;
	return bm->Test(pID);
}

char* PTable::GetName(int pID)
{
	if(pID>=0 && pID<10 && bm->Test(pID))
		return pcb[pID]->GetNameThread();
}
