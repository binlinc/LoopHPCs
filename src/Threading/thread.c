
#include "thread.h"
#include "dbg\debug.h"
#include "simple-pebs.h"
#include "LBR\LBR.h"
#include<NDIS.H>


extern PDRIVER_OBJECT drv;
extern PVOID64  g_pSemaphore; 


static KEVENT event;

/* delay execution for a while */
void delay_exec(INT64 interval)
{	
	LARGE_INTEGER _interval;
	LARGE_INTEGER			timeout;
	_interval.QuadPart=interval;
	
	//Callers of KeDelayExecutionThread must be running at IRQL <= APC_LEVEL.
	KeDelayExecutionThread(KernelMode,FALSE,&_interval);	

}

/* generic thread controller */
void control_thread(int action, int core)
{
	debug("control_thread");

	switch(action)
	{
	case LOAD_BTS:
		delay_exec(DELAY_INTERVAL);
		ThreadStart((void*)core);
		break;
	case UNLOAD_BTS:
		delay_exec(DELAY_INTERVAL);
		ThreadStop((void*)core);


		break;
	default:
		debug("Invalid Option to control_thread");
		break;
	}
}

void thread_attach_to_core(unsigned int id)
{
	KAFFINITY mask;
#pragma warning( disable : 4305 )
#pragma warning( disable : 4334 )
	mask=1<<id;
	KeSetSystemAffinityThread(mask);
}
unsigned long g_OldDS=0;

VOID ThreadStart(_In_ PVOID StartContext)
{
	unsigned int id;

#pragma warning( disable : 4305 )
	id=(int)StartContext;

		DbgPrint("\nCore%d ThreadStart",id);
	thread_attach_to_core(id);

#ifdef _PEB_
	/* set up bts */
	setup_PEBDS();
	enable_PEB();
#endif


#ifdef _LBR_
		enable_LBR();
#endif

	 return;

}

VOID ThreadStop(_In_ PVOID StartContext)
{
	unsigned int id;
	#pragma warning( disable : 4305 )
	id=(int)StartContext;
	DbgPrint("\nCore%d ThreadStop",id);
	thread_attach_to_core(id);

#ifdef _PEB_
	disable_PEB();
	clean_PEBDS();	
#endif

#ifdef _LBR_
		disable_LBR();
#endif


	KeRevertToUserAffinityThread();
	return;
}