/* 
Ref: Intel Manual
17.12     LAST BRANCH, CALL STACK, INTERRUPT, AND EXCEPTION RECORDING 
FOR PROCESSORS BASED ON SKYLAKE MICROARCHITECTURE  (Page 3318)
*/

/*----------------------------------------------------------    
							Register For LBR
----------------------------------------------------------  */

//#define LBR_SIZE 16
int LBR_SIZE;
#define LBR_MSR 473
#define MSR_IA32_DEBUGCTL 0x1D9

#define LBR_ENABLE 1
#define LBR_DISABLE 0
#define FREEZE_LBRs_ON_PMI 1<<11
//When set, the LBR stack is frozen on a hardware PMI request (e.g. when a counter overflows and is configured to trigger PMI). See Section 17.4.7 for details.
// FREEZE_LBRS_ON_PMI doesn't work in combination with PEBS
#define FREEZE_PERFMON_ON_PMI 1<<12
 //When set, the performance counters (IA32_PMCx and IA32_FIXED_CTRx) are frozen on a PMI request. See Section 17.4.7 for details.
/*				17.4.7      Freezing LBR and Performance Counters on PMI 
				17.12.2    Streamlined Freeze_LBRs_On_PMI Operation  (Page 3319  )
*/
//MSR_IA32_DEBUGCTL bit 0 is for LBR

/*----------------------------------------------------------    
							Register For LBR
			Skylake (and newer) last-branch recording 
#define MSR_SKL_LASTBRANCH_TOS    0x000001c9
481
#define MSR_SKL_LASTBRANCH_0_FROM_IP  0x00000680
482
#define MSR_SKL_LASTBRANCH_0_TO_IP  0x000006c0
483
#define MSR_SKL_LASTBRANCH_0_INFO 0x00000dc0
484
#define NUM_MSR_SKL_LASTBRANCH    32
----------------------------------------------------------  */
#define MSR_LBR_SELECT 456
#define LBR_BASE_FROM 0x680
#define LBR_BASE_TO 0x6c0
#define MSR_LBR_TOS 0x1C9
#define LBR_BITMASK 31
/* 
Table 17-13.   MSR_LBR_SELECT for Intel® microarchitecture code name Haswell  (Page 3316)
*/
#define LBR_DISABLE_KERNEL 1
#define LBR_DISABLE_USER 1<<1
//When set, do not capture conditional branches
#define LBR_DISABLE_JCC 1<<2
//When set, do not capture near relative calls
#define LBR_DISABLE_REL_CALL 1<<3
//When set, do not capture near indirect calls
#define LBR_DISABLE_IND_CALL 1<<4
#define LBR_DISABLE_NEAR_RET 1<<5 
//When set, do not capture near indirect jumps
#define LBR_DISABLE_NEAR_IND_JMP 1<<6
//When set, do not capture near relative jumps
#define LBR_DISABLE_NEAR_REL_JMP 1<<7
//When set, do not capture far branches
#define LBR_DISABLE_FAR_BRANCH 1<<8
//Enable LBR stack to use LIFO filtering to capture Call stack profile
//Must set valid combination of bits 0-8 in conjunction with bit 9 (as described below),
//otherwise the contents of the LBR MSRs are undefined.
#define EN_CALLSTACK 1<<9
// 
#define LBR_NO_INFO2	(1ULL<< 63)
#define LBR_NO_INFO	0x8000000000000000


/*----------------------------------------------------------    

17.12     LAST BRANCH, CALL STACK, INTERRUPT, AND EXCEPTION RECORDING 
FOR PROCESSORS BASED ON SKYLAKE MICROARCHITECTURE

—  MSR_LBR_INFO_x, stores branch prediction flag, TSX info, and elapsed cycle data.

						
17.12.1    MSR_LBR_INFO_x MSR
							Table 17-16.   MSR_LBR_INFO_x  (P3318)
----------------------------------------------------------  */

#define MSR_LBR_INFO_0			0x00000dc0 /* ... 0xddf for _31 */
#define BIT_ULL(nr)		(1ULL << (nr))
//When set, indicates either the target of the branch was mispredicted and/or the  direction (taken/non-taken) was mispredicted; otherwise, the target branch was predicted.
#define LBR_INFO_MISPRED		BIT_ULL(63)
#define LBR_INFO_IN_TX			BIT_ULL(62)
#define LBR_INFO_ABORT			BIT_ULL(61)
//Elapsed core clocks since last update to the LBR stack.
#define LBR_INFO_CYCLES			0xFFFF  //BIT 15:0





 void enable_LBR();
void disable_LBR();
void get_LBR(PLIST_ENTRY ddDataLA ,UINT64 LBR_FROM,UINT64 LBR_TO, UINT64 ESP);
void  get_LBR_disableLBR();

int IndexofLBR;
int IndexofOuterLoop;
int IndexofInnerLoop;
int TotalInnerLoop;
int IndexofPeriod;

int previous_LBR_TOS;

#define INVALID_TOS_VALUE 0x0000FFFF
