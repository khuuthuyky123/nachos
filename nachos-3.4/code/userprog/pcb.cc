#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

PCB::PCB(int id)
{
	joinsem= new Semaphore("JoinSem",0);
	exitsem= new Semaphore("ExitSem",0);
	//mutex= new Semaphore("Mutex",1);
	multex = new Semaphore("multex",1);

	pid= id;
	exitcode= 0;
	numwait= 0;
	if(id)
		parentID= currentThread->processID;
	else
		parentID= -1;
	thread= NULL;
	JoinStatus= -1;

}

PCB::~PCB()
{
	if(joinsem != NULL)
		delete joinsem;
	if(exitsem != NULL)
		delete exitsem;
	// if(mutex != NULL)
	// 	delete mutex;
	if(multex != NULL)
		delete multex;
}

//------------------------------------------------------------------
int PCB::GetID()
{
	return pid;
}

int PCB::GetNumWait()
{
	return numwait;
}

int PCB::GetExitCode()
{
	return exitcode;	
}

void PCB::SetExitCode(int ec)
{
	exitcode= ec;
}

void PCB::IncNumWait()
{
	multex->P();
	numwait++;
	multex->V();
}

void PCB::DecNumWait()
{
	multex->P();
	if(numwait>0)
		numwait--;
	multex->V();
}
void PCB::SetFileName(char* fn){ strcpy(FileName,fn);}

char* PCB::GetNameThread()
{
	return thread->getName();
}

//-------------------------------------------------------------------
void PCB::JoinWait()
{
	//JoinStatus= parentID;
	//IncNumWait();
	joinsem->P();
}

void PCB::JoinRelease()
{
	//DecNumWait();
	joinsem->V();
}

void PCB::ExitWait()
{
	exitsem->P();
}

void PCB::ExitRelease()
{
	exitsem->V();
}

//------------------------------------------------------------------
int PCB::Exec(char *filename, int pID)
{
	//mutex->P();
	multex->P();
	thread= new Thread(filename);
	if(thread == NULL)
	{
		printf("\nLoi: Khong tao duoc tien trinh moi !!!\n");
		//mutex->V();
		multex->V();
		return -1;
	}
	thread->processID= pID;
	parentID = currentThread->processID;
	thread->Fork(MyStartProcess,pID);
	//mutex->V();
	multex->V();
	return pID;
}


//*************************************************************************************
void MyStartProcess(int pID)
{
	char *filename= pTab->GetName(pID);
	AddrSpace *space= new AddrSpace(filename);
	if(space == NULL)
	{
		printf("\nLoi: Khong du bo nho de cap phat cho tien trinh !!!\n");
		return; 
	}
	currentThread->space= space;

	space->InitRegisters();		// set the initial register values
	space->RestoreState();		// load page table register

	machine->Run();			// jump to the user progam
	ASSERT(FALSE);			// machine->Run never returns;
						// the address space exits
						// by doing the syscall "exit"
}
