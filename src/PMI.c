#include "simple-pebs.h"
#include "LBR\LBR.h"
#include "dbg\debug.h"
#include "checks\checks.h"
#include "list\list.h"
#include <stdio.h>


#define _Standard_PMI_

extern PTDS_BASE DS_BASE;

extern BOOLEAN  bSuspend ;


ULONG interval_Time=0;
ULONG PMI_In=0;
ULONG PMI_Out=0;

ULONG QuerySystemTime() 
{ 
	LARGE_INTEGER CurTime, Freq; 
	CurTime = KeQueryPerformanceCounter(&Freq); 
	return (ULONG)((CurTime.QuadPart *1000)/Freq.QuadPart); 
} 


UINT64 last=0;
void PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext) 
{
	LARGE_INTEGER pa;
	UINT32* APIC;
	int i;
	unsigned long long tmp;
	int iOldPMC0;
	UINT64 GLOBAL_STATUS;
	BOOLEAN flag[MAX_PMCx] ; 

#ifdef _Standard_PMI_

	__writemsr(MSR_IA32_PERF_GLOBAL_CTRL, 0);
	__writemsr(MSR_IA32_PEBS_ENABLE, 0);
	GLOBAL_STATUS=__readmsr(MSR_PERF_GLOBAL_STATUS);
	 __writemsr(IA32_PERF_GLOBAL_OVF_CTRL,GLOBAL_STATUS);
	MyDbgPrint("GLOBAL_STATUS= %llx ", 	GLOBAL_STATUS);

#endif


	MyCode();

Exit:

	//clear interrupt flag 
	pa.QuadPart=PERF_COUNTER_APIC;
	APIC=(UINT32*)MmMapIoSpace(pa,sizeof(UINT32),MmNonCached);
	*APIC=ORIGINAL_APIC_VALUE;
	MmUnmapIoSpace(APIC,sizeof(UINT32));

	if(	DS_BASE->pebs_index==DS_BASE->pebs_thresh)
	{
		DS_BASE->pebs_index=DS_BASE->pebs_base;
	}

		init_PMCx(OutSet);

#ifdef _Standard_PMI_
		__writemsr(MSR_IA32_PEBS_ENABLE, _PEB_PMCx_);

		tmp=	 __readmsr(MSR_IA32_PERF_GLOBAL_CTRL);
		tmp = tmp | _Counter_;
		__writemsr(MSR_IA32_PERF_GLOBAL_CTRL, tmp);
#endif
} 


void *perfmon_hook = PMI;
void *restore_hook = NULL;

void unhook_handler()
{
	NTSTATUS status;
	status = HalSetSystemInformation(HalProfileSourceInterruptHandler,sizeof(PVOID*),&restore_hook);
}

void hook_handler()
{
	NTSTATUS status;
	status = HalSetSystemInformation(HalProfileSourceInterruptHandler,sizeof(PVOID*),&perfmon_hook);s
}