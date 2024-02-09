

#pragma once
#include "config.h"
typedef unsigned long long  u64;

  void init_PMCx(int outset);

#define PERF_COUNTER_APIC 0xFEE00340
#define ORIGINAL_APIC_VALUE 254
#define MSR_DS_AREA 0x00000600

/* 
the performance event select MSRs (PerfEvtSel0 and PerfEvtSel1) 
and the performance counter MSRs (PerfCtr0 and PerfCtr1)
 These registers can be read from and written to using the RDMSR and WRMSR instructions, respectively. They can be accessed using these 
instructions only when operating at privilege level 0. The PerfCtr0 and PerfCtr1 MSRs can be read from any privilege 
level using the RDPMC (read performance-monitoring counters) instruction.

The WRMSR instruction cannot arbitrarily write to the performance-monitoring counter MSRs (PerfCtr0 and 
PerfCtr1). Instead, the lower-order 32 bits of each MSR may be written with any value, and the high-order 8 bits 
are sign-extended according to the value of bit 31. This operation allows writing both positive and negative values 
to the performance counters.
*/

//The performance-counter MSRs 
#define MSR_IA32_PERFCTR0               0x000000c1
#define MSR_IA32_PERFCTR1               0x000000c2
#define MSR_IA32_PERFCTR2               0x000000c3
#define MSR_IA32_PERFCTR3               0x000000c4
//[64-ia-32] IA32_PERFEVTSELx MSRs start at address 186H and occupy a contiguous block of MSR address space.
/*Table 18-1
EN—Enable counters
When set, performance counting is enabled in the corresponding 
performance-monitoring counter; when clear, the corresponding counter is disabled. The event logic unit for a 
UMASK must be disabled by setting IA32_PERFEVTSELx[bit 22] = 0, before writing to IA32_PMCx.
INT—APIC interrupt enable
 When set, the logical processor generates an exception through its local APIC on counter overflow.
OS—Operating system mode 
 Specifies that the selected microarchitectural condition is  counted when the logical processor is operating at privilege level 0.
USR—User Mode 
 Specifies that the selected microarchitectural condition is counted when the logical processor is operating at privilege levels 1, 2 or 3
*/

/*   ------------------------    Start and stop PMU    ------------------------
the counters begin counting following the execution of a WRMSR instruction. The counters can 
be stopped by clearing the enable counters flag or by clearing all the bits in the PerfEvtSel0 and PerfEvtSel1 MSRs.
*/
/*   ------------------------    overflows   ------------------------------------
generating a local APIC interrupt when a performance-monitoring counter overflows. 
This mechanism is enabled by setting the interrupt enable flag in either the PerfEvtSel0 or the PerfEvtSel1 MSR. 

Provide an interrupt vector for handling the counter-overflow interrupt.
• Initialize the APIC PERF local vector entry to enable handling of performance-monitor counter overflow events.
• Provide an entry in the IDT that points to a stub exception handler that returns without executing any instruc-
tions.
• Provide an event monitor driver that provides the actual interrupt handler and modifies the reserved IDT entry 
to point to its interrupt routine.

When interrupted by a counter overflow, the interrupt handler needs to perform the following actions:
• Save the instruction pointer (EIP register), code-segment selector, TSS segment selector, counter values and 
other relevant information at the time of the interrupt.
• Reset the counter to its initial setting and return from the interrupt.
*/
#define MSR_IA32_EVNTSEL0               0x00000186
#define MSR_IA32_EVNTSEL1               0x00000187
#define MSR_IA32_EVNTSEL2               0x00000188
#define MSR_IA32_EVNTSEL3               0x00000189



#define MSR_FIXED_CTR0                  0x00000309/*inst_retired.any*/
#define MSR_FIXED_CTR1                  0x0000030a/*cpu_clk_unhalted*/

#define MSR_IA32_GLOBAL_CTRL 		0x38F
#define MSR_PERF_FIXED_CTR_CTRL         0x0000038d

#define MSR_IA32_PERF_CABABILITIES  	  0x00000345
#define MSR_IA32_PERF_GLOBAL_STATUS 	  0x0000038e
// PEBS_Ov Indicates that the PEBS buffer threshold was reached and microcode scheduled a performance interrupt to indicate this condition.
#define PEBS_Ovf 62

#define MSR_IA32_PERF_GLOBAL_CTRL 	    0x0000038f
#define MSR_IA32_PERF_GLOBAL_OVF_CTRL 	0x00000390
#define MSR_IA32_PEBS_ENABLE		        0x000003f1
#define MSR_IA32_DS_AREA     		        0x00000600


#define MSR_IA32_PERF_CAPABILITIES	0x00000345

/*  18.2.3      Architectural Performance Monitoring Version 3   p3365
Each IA32_PERFEVTSELx MSR (starting at MSR address 186H) support the bit field layout defined in Figure 
18-6.
*/
#define EVTSEL_USR 1<<16
#define EVTSEL_OS  1<<17
//#define EVTSEL_OS  0/*BIT(17)*/

//INT—APIC interrupt enable
//When set, the processor generates an exception through its local APIC on counter overflow.
#define EVTSEL_INT 1<<20
//EN—Enable counter
#define EVTSEL_EN  1<<22

#define 	MSR_PERF_GLOBAL_STATUS   0x0000038E
#define IA32_PERF_GLOBAL_OVF_CTRL 0x00000390 
#define  IA32_PERF_GLOBAL_STATUS_RESET 0x00000390 



unsigned long long      u64_instr;
typedef 	unsigned long long u64 ;

unsigned long long get_inst(BOOLEAN bPrint);
unsigned long long get_cycle(BOOLEAN bPrint);
 unsigned long long  get_PMCx(int index, BOOLEAN bPrint);

void enable_PEB( );
void disable_PEB( );
void setup_PEBDS(  );
void clean_PEBDS(  );


struct pebs_v1 {
	u64 flags;
	u64 ip;
	u64 ax;
	u64 bx;
	u64 cx;
	u64 dx;
	u64 si;
	u64 di;
	u64 bp;
	u64 sp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 status;
	u64 dataLA; //Data Linear Address 98H
	u64 dse; //Data Source Encoding
	u64 lat; //Latency value (core cycles)
};

typedef struct pebs_v2 {
	struct pebs_v1 v1;
	u64 eventing_ip;
	u64 tsx; // Transactional Synchronization Extensions 18.3.6.5.1    Intel TSX and PEBS Supports
}TPEBS2_BUFFER,*PTPEBS2_BUFFER;

typedef struct pebs_v3 {
	struct pebs_v2 v2;
	//Time Stamp Counter,  17.17     TIME-STAMP COUNTER (P3327)
	//monitor and identify the relative time occurrence of processor events
	u64 tsc; 

}TPEBS3_BUFFER,*PTPEBS3_BUFFER;


typedef struct st_DSBASE
{
	u64 bts_base;
	u64 bts_index;
	u64 bts_max;
	u64 bts_thresh;

	u64 pebs_base;
	u64 pebs_index;
	u64 pebs_max;
	u64 pebs_thresh;
	u64 pebs_reset[4];
}TDS_BASE,*PTDS_BASE;

PTDS_BASE DS_BASE;

PVOID64 PEBS_BUFFER;
unsigned long Old_DS;

void FILL_DS_WITH_BUFFER(PTDS_BASE DS_BASE,PVOID64 PEBS_BUFFER);


void  MyCode();



/*
Table 18-12.  PEBS Performance Events for Intel® Microarchitecture Code Name Sandy Bridge

Table 19-10.  Performance Events in the Processor Core of 4th Generation Intel® Core™ Processors

Enable specific PMC
0: INST_RETIRED.ANY_P:							EventSel=C0H, UMask=00H,
1: INST_RETIRED.PREC_DIST:					EventSel=C0H, UMask=01H, Precise
2:BR_INST_RETIRED.CONDITIONAL:		EventSel=C4H, UMask=01H, Precise 
3:BR_INST_RETIRED.NOT_TAKEN:			EventSel=C4H, UMask=10H
BR_INST_RETIRED.NEAR_CALL:			EventSel=C4H, UMask=02H, Precise
BR_INST_RETIRED.NEAR_RETURN		EventSel=C4H, UMask=08H, Precise
BR_INST_RETIRED.ALL_BRANCHES,	EventSel=C4H, UMask=00H, Precise
BR_MISP_RETIRED.CONDITIONAL,		EventSel=C5H, UMask=01H, Precise
MACHINE_CLEARS.SMC,						EventSel=C3H, UMask=04H

MEM_INST_RETIRED.ALL_LOADS  EventSel=D0H, UMask=81H, Precise All retired load instructions.
MEM_INST_RETIRED.ALL_STORES   EventSel=D0H, UMask=82H, Precise All retired store instructions.

---------------- MEM_INST_RETIRED  D0H ; PMC4-7 not support ----------------
LOCK_LOADS 21H
SPLIT_LOADS 41H
SPLIT_STORES 42H
ALL_LOADS 81H
ALL_STORES 82H

---------------- MEM_LOAD_RETIRED D1H ; PMC4-7 not support  ----------------
L1_HIT 01H
L2_HIT 02H
L3_HIT 04H
L1_MISS 08H
L2_MISS 10H
L3_MISS 20H
HIT_LFB 40H
*/
#define Mask_INST_ANY 0x00C0
#define Mask_INST_PREC_DIST 0x01C0//Precise
#define BR_CONDITIONAL 0x01C4 //Precise
#define BR_NOTTAKEN  0x10C4  

#define Mask_NEAR_CALL  0x02C4 //Precise
#define   Mask_NEAR_RETURN   0x08C4 //Precise
#define  ALL_BRANCHS   0x00C4//Precise
//#define  Mask_ALL_BRANCHS   0x04C4//Precise

// New Event
#define     BR_MISP_CONDITIONAL 0x01C5
#define BR_MISP_NEAR_TAKEN  0x20C5




//------------  precious

#define  ALL_LOADS   0x81D0
#define  ALL_STORES    0x82D0
#define MEM_UOPS_RETIRED_STLB_MISS_LOADS 0x11D0
#define MEM_UOPS_RETIRED_STLB_MISS_STORES 0x12D0
#define L1_MISS  0x08D1
#define L2_MISS  0x10D1
#define L3_MISS  0x20D1
#define L1_HIT  0x01D1
#define L2_HIT  0x02D1
#define L3_HIT  0x04D1


//------------           Non precious
 #define BRANCHES_PS 0x04C4
 #define BRANCHES 0x00C4
#define  DTLB_LOAD_MISSES_MISS_CAUSES_A_WALK 0x0108
 #define DTLB_STORE_MISSE_MISS_CAUSES_A_WALK 0x0149
#define BR_MISP 0x00C5
#define BR_MISP_PS 0x04C4
//Feature Importance<0.01
#define LLC_Misses 0x412E
 #define ICACHE_Misses 0x0280
#define SMC  0x04C3	

#define LBR_INSERTS 0x20CC //Increments when an entry is added to the Last Branch Record
 

enum PMC
{
	PMC0, 
	PMC1 ,
	PMC2, 
	PMC3,
	PMC4, 
	PMC5 ,
	PMC6, 
	PMC7
};

int IndexofPMI;
int IndexofMyCode;
int pebs_record_size ;

UINT64 previous_INST;
UINT64 previous_CYCLE;
UINT64 previous_PMC0;
UINT64 previous_PMC1;
UINT64 previous_PMC2;
UINT64 previous_PMC3;
UINT64 previous_PMC4;
UINT64 previous_PMC5;
UINT64 previous_PMC6;
UINT64 previous_PMC7;

int iOld_PERFCTR;
int  NumberofPEBS;

/****************
custom Define
************/

enum staticMask_codes
{
	_EVENT_TYPE_, /* In case of no error */
	_bINT_, /* In case of CPU is not Intel */
	_PERIOD_, /* In case of a 32-bit CPU */
	_COUNTER_DELTA_
};



#define MAX_PMCx 2
#define STORE_PERIOD		1
#define BRANCH_PERIOD		1


INT16 NumberofPMCs;
 

#define SIZE_PEBS_BUFFER 1+1
#define Target_Core_Number  KeNumberProcessors-1
#define  _PEB_PMCx_  ((1<<PMC0) | (1<<PMC1)  )
#define  _Counter_  (0x000000000 | _PEB_PMCx_ )
#define Multiplexing 2
#define OutSet 1
int Multiplex_Index;
DWORD32 	 StaticMask[Multiplexing][MAX_PMCx][4];