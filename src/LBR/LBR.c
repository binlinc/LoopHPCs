

#include<fltKernel.h>
#include<intrin.h>
#include "LBR.h"
#include "simple-pebs.h"
#include "List\list.h"
#include "dbg\debug.h"



void enable_LBR()
{
	UINT64 mask;
		UINT64 debugctl;

	 DbgPrint("enable LBR");



	 mask = 		LBR_DISABLE_REL_CALL|  //1<<3
		 LBR_DISABLE_IND_CALL	|  //1<<4
		 LBR_DISABLE_NEAR_RET	 | // 1<<5 
		 LBR_DISABLE_NEAR_IND_JMP	| //1<<6
	//	 LBR_DISABLE_NEAR_REL_JMP	| //1<<7
		 LBR_DISABLE_FAR_BRANCH		|  // 1<<8
			LBR_DISABLE_KERNEL; // 1

		MyDbgPrint("mask:%llx,LBR_NO_INFO: %llx ",mask, mask&LBR_NO_INFO );


	__writemsr(MSR_LBR_SELECT,mask);

	mask=__readmsr(MSR_LBR_SELECT);
	MyDbgPrint("read mask= %llx",mask);


	
	debugctl=__readmsr(LBR_MSR);
	debugctl |= LBR_ENABLE ;
//	debugctl |= (LBR_ENABLE | FREEZE_LBRs_ON_PMI);
	__writemsr(LBR_MSR,debugctl);
}

void disable_LBR()
{
	UINT64 debugctl;
		
	DbgPrint("disable LBR");

	debugctl=__readmsr(LBR_MSR);
		debugctl &= ~LBR_ENABLE ;
//	debugctl &= ~(LBR_ENABLE | FREEZE_LBRs_ON_PMI);
	__writemsr(LBR_MSR,debugctl);
}

UINT64 LBR_Info;
void get_LBR(PLIST_ENTRY DLAHeader ,UINT64 LBR_FROM,UINT64 LBR_TO, UINT64 ESP)
{
	int Layer=0;
	int i=0;

	UINT64 current_INST=0;
	UINT64 current_CYCLE=0;
	UINT64 current_PMC[MAX_PMCx]={0};
	UINT64 previous_PMC[MAX_PMCx]={0};


	for (i=OutSet;i<MAX_PMCx;i++)
	{
		current_PMC[i]=get_PMCx(i,FALSE);
	}


	// Not from Bin or to Bin
	if(LBR_FROM>=_MAX_BIN_||LBR_FROM==0x0
		||LBR_TO>=_MAX_BIN_||LBR_TO==0x0
		)
	{
		return;
	}

	Find_Insert_LoopList(LBR_FROM,LBR_TO,1, ESP, current_INST, current_CYCLE, current_PMC,DLAHeader);

}

