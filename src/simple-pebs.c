
#include "simple-pebs.h"
#include "dbg\debug.h"
#include "checks\checks.h"

#define wrmsrl __writemsr
#define rdmsrl __readmsr


DWORD32 	 StaticMask[Multiplexing][MAX_PMCx][4]=
{
	{	
		{ALL_BRANCHS,		0,STORE_PERIOD,0},  
		{ALL_STORES,	0,STORE_PERIOD,0},
#if MAX_PMCx >=3
		{ALL_STORES ,		0,0,0},  
#endif 
#if MAX_PMCx >=4
		{ALL_LOADS,		0,0,0},   
#endif

	},
#if Multiplexing>=2
	{	
		{ALL_BRANCHS,		0,STORE_PERIOD,0},
		{ICACHE_Misses,	0,0,0},
#if MAX_PMCx >=3
		{ALL_STORES ,		0,0,0},		 
	#endif 
#if MAX_PMCx >=4
		{ALL_LOADS,		0,0,0}     
	#endif
	},
#endif
#if Multiplexing>=3
	{
		{ALL_STORES,		0,STORE_PERIOD,0},
		{ALL_BRANCHS,	0,STORE_PERIOD,0},
		{L2_MISS ,		0,0,0},	
		{L2_HIT,		0,0,0}  

	},
#endif
#if Multiplexing>=4
	{	
		{ALL_STORES,		0,STORE_PERIOD,0},
		{ALL_BRANCHS,	0,STORE_PERIOD,0},
		{L3_MISS ,		0,0,0},	
		{L3_HIT,		0,0,0}     
	}
#endif
};

 void init_instr(){
  unsigned long long tmp;
 tmp= rdmsrl(MSR_PERF_FIXED_CTR_CTRL);
  tmp = tmp | 0x2;
	wrmsrl(MSR_PERF_FIXED_CTR_CTRL, tmp);/*enable fixed counter for #instr*/
  wrmsrl(MSR_FIXED_CTR0, 0);
}


 unsigned long long get_inst( BOOLEAN bPrint)
 {
	 if(bPrint)
	 {
		 MyDbgPrint("get_current_instr=%d ", rdmsrl(MSR_FIXED_CTR0));
	 }
	 return rdmsrl(MSR_FIXED_CTR0);
 }

  void init_cycle(){
	  unsigned long long tmp;
	  tmp=rdmsrl(MSR_PERF_FIXED_CTR_CTRL);
	  tmp = tmp | 0x20;
	  wrmsrl(MSR_PERF_FIXED_CTR_CTRL, tmp);/*enable fixed counter for #cycle*/
	  wrmsrl(MSR_FIXED_CTR1, 0);
  }

  unsigned long long get_cycle(BOOLEAN bPrint){
	  if(bPrint)
	  {
		  MyDbgPrint("get_current_cycle=%d ",rdmsrl(MSR_FIXED_CTR1));
	  }
	  return rdmsrl(MSR_FIXED_CTR1);
  }


  void init_PMCx(int outset)
  {
	  int i;
	  unsigned long long mask;
	  for (i=outset;i<MAX_PMCx;i++)
	  {
		  // Select the event for  MSR_IA32_EVNTSELx 
		  mask=StaticMask[Multiplex_Index][i][0]|StaticMask[Multiplex_Index][i][1]
		  | EVTSEL_EN | EVTSEL_USR;
		  wrmsrl(MSR_IA32_EVNTSEL0+i,mask);

		  __writemsr(MSR_IA32_PERFCTR0+i,-(int)StaticMask[Multiplex_Index][i][2]);
	  }  
  }


  unsigned long long  get_PMCx(int index, BOOLEAN bPrint)
  {
	  unsigned long long PMCxValue=0;


	  if(index<=NumberofPEBS-1)
	  {
		  if(bPrint)
		  {
			  MyDbgPrint("PMC%d=%d",index,(rdmsrl(MSR_IA32_PERFCTR0+index)+StaticMask[Multiplex_Index][index][_COUNTER_DELTA_]) );
		  }
		  return rdmsrl(MSR_IA32_PERFCTR0+index)+StaticMask[Multiplex_Index][index][_COUNTER_DELTA_];
	  }
	  //Invalid  PMCx
	  else
	  {
		  MyDbgPrint("[get_PMCx] NumberofPEBS=%d, Unused  PMC%d",NumberofPEBS, index);
		  return 0;
	  }
  }




 void enable_PEB( )
{
	int i;
	unsigned long long tmp=1;

	 for (i=0;i<4;i++)
	 {
		 if(tmp&_PEB_PMCx_)
		 {
			DbgPrint("enable_PEB  for PMC%d", i);
		 }
		 tmp=tmp<<1;
	 }

	 //First disable PMU to avoid races
	 __writemsr(MSR_IA32_PEBS_ENABLE, 0);
	 __writemsr(MSR_IA32_GLOBAL_CTRL, 0);


  init_instr();
	 init_cycle();	 

	 init_PMCx(0);
	
		 //enable special PMU register again 
	 wrmsrl(MSR_IA32_PEBS_ENABLE, _PEB_PMCx_);
	 tmp=	  rdmsrl(MSR_IA32_PERF_GLOBAL_CTRL);
	 // enable the  fixed counter 0-1, and PMC 0-3.

	tmp = tmp | _Counter_;
	 wrmsrl(MSR_IA32_PERF_GLOBAL_CTRL, tmp);

	 return;
}

 void disable_PEB()
 {
	 LARGE_INTEGER pa;
	 UINT32* APIC;
	 int IndexofPMC;

	 DbgPrint("disable_PEB");

	 get_PMCx(PMC0,TRUE);
	 get_PMCx(PMC1,TRUE);
	 get_PMCx(PMC2,TRUE);
	 get_PMCx(PMC3,TRUE);
	 get_PMCx(PMC4,TRUE);
	 get_PMCx(PMC5,TRUE);
	 get_PMCx(PMC6,TRUE);
	 get_PMCx(PMC7,TRUE);

	 wrmsrl(MSR_IA32_PERF_GLOBAL_CTRL, 0);
	 wrmsrl(MSR_IA32_PEBS_ENABLE, 0);

	 //clear special PMU
	 for(IndexofPMC=0;IndexofPMC<MAX_PMCx;IndexofPMC++)
	 {
		 __writemsr(MSR_IA32_EVNTSEL0+IndexofPMC, 0);
		 __writemsr(MSR_IA32_PERFCTR0+IndexofPMC, 0);
	 }

	 //stop special PMU
	 __writemsr(MSR_PERF_FIXED_CTR_CTRL, 0);

	 //restore the APIC value
	 pa.QuadPart=PERF_COUNTER_APIC;
	 APIC=(UINT32*)MmMapIoSpace(pa,sizeof(UINT32),MmNonCached);
	 *APIC=ORIGINAL_APIC_VALUE;
	 MmUnmapIoSpace(APIC,sizeof(APIC));
	 //In a new thread, it must return at the end of function, otherwise, it will incur BoS.
	 return;
 }



// Changed from BTS to PEBS
void FILL_DS_WITH_BUFFER(PTDS_BASE DS_BASE,PVOID64 PEBS_BUFFER)
{
	int i;

	DS_BASE->pebs_base=PEBS_BUFFER;
	DS_BASE->pebs_index=PEBS_BUFFER;
	DS_BASE->pebs_max=DS_BASE->pebs_base+(SIZE_PEBS_BUFFER - 1) *pebs_record_size + 1;
	DS_BASE->pebs_thresh=(UINT64)PEBS_BUFFER+(SIZE_PEBS_BUFFER -1) * pebs_record_size;
	for (i=0;i<4;i++)
	{
		DS_BASE->pebs_reset[i]=-(int)StaticMask[Multiplex_Index][i][_PERIOD_];
	}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
}


void setup_PEBDS()
{
	LARGE_INTEGER pa;
	UINT32* APIC;


	DS_BASE= (PTDS_BASE)ExAllocatePool(NonPagedPool,sizeof(TDS_BASE));
	memset(DS_BASE, 0,sizeof(TDS_BASE));
	PEBS_BUFFER=ExAllocatePool(NonPagedPool,	pebs_record_size*SIZE_PEBS_BUFFER);
	memset(PEBS_BUFFER, 0, 	pebs_record_size*SIZE_PEBS_BUFFER);
	FILL_DS_WITH_BUFFER(DS_BASE,PEBS_BUFFER);

#ifdef _PMI_
		/* change APIC */
	pa.QuadPart=PERF_COUNTER_APIC;
	APIC=(UINT32*)MmMapIoSpace(pa,sizeof(UINT32),MmNonCached);
	*APIC=ORIGINAL_APIC_VALUE;
	MmUnmapIoSpace(APIC,sizeof(APIC));
#endif

	/* set ds area register */
	Old_DS=__readmsr(MSR_IA32_DS_AREA);
	__writemsr(MSR_DS_AREA,(UINT_PTR)DS_BASE);
	return ;
}


void clean_PEBDS(  )
{
#ifndef _PMI_
	LARGE_INTEGER pa;
	UINT32* APIC;
#endif


#ifndef _PMI_
		/* change APIC */
	pa.QuadPart=PERF_COUNTER_APIC;
	APIC=(UINT32*)MmMapIoSpace(pa,sizeof(UINT32),MmNonCached);
	*APIC=ORIGINAL_APIC_VALUE;
	MmUnmapIoSpace(APIC,sizeof(APIC));
#endif
	__writemsr(MSR_DS_AREA,(UINT_PTR)Old_DS);

	ExFreePool(PEBS_BUFFER);
	ExFreePool(DS_BASE);
}



